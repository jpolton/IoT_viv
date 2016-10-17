# IoT_viv

ESP8266 or Arduino (nano) management and control of vivarium temperature, using Arduino IDE.
Would better be called microcontroller_viv. But I've changed the projects name once already.

The nano version is the same except without the internet connectivity.

Method:
* The temperature and humidity data is measured on two DHT22 sensors.
* A light dependent resistor reads the light levels (or internet clock gets time - planned) and sets the threshold temperatures to day or night settings.
* A relay is switched depending on measured temperatures relative to a thresholds for turning on the heater and a fan. 
* Temperatures are transmitted to neighbouring 433MHz RF receiver from an IROX weather station. See https://bitbucket.org/fuzzillogic/433mhzforarduino/wiki/browse/

_For the ESP8266 network capable version:_
* Every 20, or so 30s loops, The data are sent using a PUT request to a data.sparkfun.com data stream.
* Google charts javascript, embedded in an html file, load and plot the sparkfun data stream. 

The pass keys for the wireless network and the private key for the data.sparkfun web page are stored in a separate file of the form:

// ESP8266_dht22_sparkfun_keys.h
const char* ssid     = "....";
const char* password = "....";
const char* publicKey = "aG8bAlQybziD5Qa780yY";
const char* privateKey = "....";


To-do:
* Assess stability of code
* Memory leek or something with the 433MHz transmission code for the ESP8266. The blue light stays on.

Wiring:
As much as possible I've tried to keep the pins harmonised between the two chips
DHTPIN_bot 2     // lower DHT sensor data pin
DHTPIN_top 3     // upper DHT sensor data pin
RFPIN	4	// 433Mhz RF transmitter data pin
fan	 7              // fan relay switch pin
heater	 6           // heater relay switch pin
LDR	0	// light dependant resistor Analogue input pin


Note:
Temperature is read at the top and bottom of the vivarium. The top temperature controls the heater. The lower temperature controls whether a fan should mix the warm air down.

This code development is managed on github from <script src="https://gist.github.com/jpolton/611681f366708f26a97cca3895066dcf.js"></script>.

Contents:
README.md:
This file.

ESP8266_dht22_sparkfun.ino:
sketch for the ESP8266

esp8266_dht22_x2.html:
Corresponding html code to plot the data from two DHT22 sensors.

nano_dht22_ldr_rf.ino

ESP8266_BMP_serial_plot.py
Read streaming data from the serial port and plot graphs of temperature and pressure.
Data stream created by ESP8266_BMP_serial_plot.ino
