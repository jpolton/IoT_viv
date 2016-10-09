# IoT_viv
ESP8266 or Arduino (nano) management and control of vivarium temperature.

The nano version is the same except without the internet connectivity.

Method:
* The temperature and humidity data is measured on two DHT22 sensors using an ESP8266 flashed using the Arduino IDE.
* A light dependent resistor reads the light levels, or internet clock gets time, and sets the threshold temperatures to day or night settings.
* A relay is switched depending on measured temperatures relative to a thresholds for turning on the heater and a fan. 
* Temperatures are transmitter to neighbouring 433MHz RF receiver from IROX weather station. See https://bitbucket.org/fuzzillogic/433mhzforarduino/wiki/browse/
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
* Memory laek or something with the 433MHz transmission code. The blue light stays on.

Wiring:
#define DHTPIN_bot D2     // D2 pin of ESP8266
#define DHTPIN_top D3     // D3 pin of ESP8266
int fan = D7;              // D7 pin for relay to fan
int heater = D6;           // D6 pin for relay to heater

Note:
Temperature is read at the top and bottom of the vivarium. The top temperature controls the heater. The lower temperature controls whether a fan should mix the warm air down.

This code development is managed on github from <script src="https://gist.github.com/jpolton/611681f366708f26a97cca3895066dcf.js"></script>.

