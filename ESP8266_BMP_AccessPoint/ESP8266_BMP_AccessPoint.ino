//    FILE: ESP8266_BMP_AccessPoint.ino
//  AUTHOR: jpolton
// VERSION: 18.10.16
// PURPOSE: Create a ESP8266 WiFi Access Point
//          Read BMP280 pressure module on ESP8266.
//          Serve data on AP: e.g.
//          http://192.168.4.1/temp
//          http://192.168.4.1/pressure
//
//   BOARD: ESP8266 NodeMCU 1.0 (ESP-12E module)
//FIRMWARE: Arduino IDE
// SENSORS: BMP280
//     URL:
// HISTORY:
// 0.1.00 initial version. Gleaned from library demo. Works.
//
// STATUS: WORKS
//
// WIRING: (note in some places the D3 and D4 pins were reversed, but this doesn't work)
// VCC      Red   3V3
// GND      blk   GND
// SCK,SCL  blue  D3
// SDA,SDI  purp  D4
// CSB
// SDO      brn   3V3
//

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#//include <Wire.h>
#//include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

/* Set these to your desired credentials. */
const char *ssid = "ESP_BMP280";
const char *password = "thereisnospoon";

Adafruit_BMP280 bme; // I2C

ESP8266WebServer server(80);


float temp_c, pressure;  // Values read from sensor
String webString="";     // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 20;              // interval at which to read sensor (millisec)
float alpha = 0.7; // factor to tune for running average
float value = 0; // smoothed sensor value

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
	server.send(200, "text/html", "<h1>You are connected</h1>");
//  delay(100);
}

void setup() {
//	delay(1000);
	Serial.begin(115200);
	Serial.println();
	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
  
	Serial.println(myIP);
	server.on("/", handleRoot);

  server.on("/temp", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    readsensor();       // read sensor
    webString="Temperature: "+String((int)temp_c)+" C";   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });
  server.on("/pressure", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    readsensor();       // read sensor
    webString="Pressure: "+String((int)pressure)+" Pa";   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });

  
	server.begin();
	Serial.println("HTTP server started");

  Wire.begin(2,0); // GPIO2, GPIOO -> D4, D3
  Serial.println(F("BMP280 test"));  
  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
}

void loop() {
	server.handleClient();
}

void readsensor() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
 
    // Sensor readings
    //temp_c = bme.readTemperature();     // Read temperature in Celcius
      pressure = bme.readPressure();          // Read pressure in Pa
    // Check if any reads failed and exit early (to try again).
    if (isnan(pressure) || isnan(temp_c)) {
      Serial.println("Failed to read from BMP280 sensor!");
      return;
    }
  }
}
