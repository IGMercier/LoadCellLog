
//MODIFIED BY ISMAEL MERCIER
//this script serves the purpose of logging various datapoints used in testing different lunar wheel designs

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include "HX711.h"
#include <Adafruit_INA219.h>


/////////////DATA LOGGER///////////

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  100 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL LOG_INTERVAL // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()
                           //0 for standalone 1 for PC connection

// the digital pins that connect to the LEDs
#define redLEDpin 2
#define greenLEDpin 3


RTC_DS1307 RTC; // define the Real Time Clock object
// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;
// the logging file
File logfile;
//////////////////

/////////////LOAD CELL AMP//////////////
#define DOUT 3
#define CLK 2
//radius of wheel from pivot point in meters
float RADIUS = 0.5;
HX711 scale;
float calibration_factor = -7050; //change to real calibration factor
/////////////////////////////////////


/////////CURRENT SENSOR///////////
Adafruit_INA219 ina219; // define current sensor
///////////////////////////////

void error(String str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while(1);
}

void setup(void)
{
  Serial.begin(9600);
  Serial.println();
  
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  


  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  
  //define axes of file 
  lognprint("Millis,Current,Voltage,Force,Torque");    
  newline();
 
  // If you want to set the aref to something other than 5v
 //analogReference(EXTERNAL);

  //initialize load cell amp
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();

  //initialize current sensor
  uint32_t currentFrequency;
  ina219.begin();
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();

}

void loop(void)
{
 
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  digitalWrite(greenLEDpin, HIGH);
  
  // log milliseconds since starting
  uint32_t m = millis();
  lognprint(String(m));           // milliseconds since start

  //load cell values
  float force = scale.get_units(5); //averages  readings    
  float torque = force * RADIUS;  //calculates torque

  //current sensor values
  float shuntvoltage = ina219.getShuntVoltage_mV();
  float busvoltage = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  float power_mW = ina219.getPower_mW();
  float loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  lognprint(String(force));
  lognprint(String(torque));
  lognprint(String(current_mA));
  lognprint(String(loadvoltage));
  
  newline();
  
  digitalWrite(greenLEDpin, LOW);

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  
  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW); 
}


void lognprint(String word)
{
  logfile.print(word);
  logfile.print(", ");
  #if ECHO_TO_SERIAL
    Serial.print(word);
    Serial.print(", ");
  #endif
  return;
}

void newline()
{
  logfile.println();
  #if  ECHO_TO_SERIAL
    Serial.println();
  #endif
  return;
}

