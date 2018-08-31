// BY Ismael Mercier, imercier@andrew.cmu.edu
This project is for the purpose of testing different wheel designs
for the CMU CubeRover but can also be used to log load cell readings

Hardware:
	Arduino Uno
	Sparkfun HX711 Load Cell amp
	Load cell, any type
        Adafruit RTC Datalogger
	SD card

Directions:
	1.Upload and run calibrate.ino, follow instructions to get 
	  calibration factor.
	2.Change the calibraton_factor in torquelog.ino
	3.Change the RADIUS in torquelog.ino
	4.Upload and run torquelog.ino 
	


