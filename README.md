# ESP8266_viv
ESP8266 management and control of vivarium temperature.

Method:
* The temperature and humidity data is measured on two DHT22 sensors using an ESP8266 flashed using the Arduino IDE.
* The data are sent using a PUT request to a data.sparkfun.com data stream.
* Google charts javascript, embedded in an html file, load and plot the sparkfun data stream. 

To-do:
* Add private key to .ino file
* Add network key to .ino file
* Assess stability of code
* Add relay control of fan and heater

Wiring:
#define DHTPIN_bot D5     // D5 pin of ESP8266
#define DHTPIN_top D3     // D3 pin of ESP8266

Plan:
Inputs- temperature from DHT22 sensor x2.
Output- Relay control of heater and fan.
Temperature is read at the top and bottom of the vivarium. The top temperature controls the heater. The lower temperature controls whether a fan should mix the warm air down.

This code development is managed on github from <script src="https://gist.github.com/jpolton/611681f366708f26a97cca3895066dcf.js"></script>.

