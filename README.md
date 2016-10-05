# ESP8266_viv
ESP8266 management and control of vivarium temperature. Inputs: temperature from DHT22 sensor x2. Output: Relay control of heater and fan.
Temperature is read at the top and bottom of the vivarium. The top temperature controls the heater. The lower temperature controls whether a fan should mix the warm air down.

This code development is managed on github from <script src="https://gist.github.com/jpolton/611681f366708f26a97cca3895066dcf.js"></script>.

The temperature and humidity data is read using an ESP8266 flashed using the Arduino IDE.
The data are sent using a PUT request to a data.sparkfun.com data stream.
Google charts javascript, embedded in an html file, load and plot the sparkfun data stream. 

To-do:
* Add private key to .ino file
* Add network key to .ino file

