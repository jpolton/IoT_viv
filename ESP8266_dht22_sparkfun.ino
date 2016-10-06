//
//    FILE: ESP8266_dht22_sparkfun.ino
//  AUTHOR: Jeff Polton
// VERSION: 04.10.16
// PURPOSE: DHT22 send GET request to data.sparkfun stream
//   BOARD: ESP8266 NodeMCU 1.0 (ESP-12E module)
//FIRMWARE: Arduino IDE
//     URL: https://data.sparkfun.com/streams/aG8bAlQybziD5Qa780yY
//            http://pcwww.liv.ac.uk/~jpolton/plots/esp8266_dht22_x2.html 
//          /Users/jeff/Public/liv.ac.uk/plots/esp8266_dht22_x2.html
// HISTORY:
// 0.1.00 initial version: http://hpclab.blogspot.co.uk/2015/06/esp8266-based-wifi-weather-monitoring.html
// 0.1.01 Edit to accommodate two sensors.
// 1.0.00 Tested
// 2.0.00 Relay control added and tested. (Note that the LED on the fan seems to sometime rapidly flash when the fan is switched on.) 

//  STATUS: WORKS. 
/*
    NOTES:
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.

    Posts data e.g.:
	https://data.sparkfun.com/input/aG8bAlQybziD5Qa780yY?private_key=KEPqaDKYqzsawdVNE8AD&hum_bot=50.7&hum_top=27.1&temp_bot=16.3&temp_top=29.7
 */
#include <ESP8266WiFi.h>
#include "DHT.h"
/* Header file for storing private keys */
#include "ESP8266_dht22_sparkfun_keys.h"
/* Either include a header file (above) with these private keys or fill them in here.
const char* ssid     = "....";
const char* password = "....";
const char* publicKey = "....";
const char* privateKey = "....";
 */

#define DHTPIN_bot D2     // D2 pin of ESP8266
#define DHTPIN_top D3     // D3 pin of ESP8266
int fan = D7;              // D7 pin for relay to fan
int heater = D6;           // D6 pin for relay to heater
const char* host = "data.sparkfun.com";
float Ttop_threshold = 35.0; // Top temperature to activate heater
float Tbot_threshold = 28.0; // Bottom temperature to active fan

DHT dht_bot(DHTPIN_bot, DHT22, 30); // 30 is for cpu clock of esp8266 80Mhz
DHT dht_top(DHTPIN_top, DHT22, 30);

void setup() {
  Serial.begin(115200);
  dht_bot.begin();
  dht_top.begin();
  pinMode(fan, OUTPUT);
  pinMode(heater, OUTPUT);
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("ESP8266 Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(5000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  /////////////////////////////////////////////////////////////////////////////
  float h_bot = dht_bot.readHumidity();
  float h_top = dht_top.readHumidity();

  // Read temperature as Celsius
  float t_bot = dht_bot.readTemperature();
  float t_top = dht_top.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h_top) || isnan(t_top) || isnan(h_bot) || isnan(t_bot)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity bottom: ");
  Serial.print(h_bot);
  Serial.print(" %\t");
  Serial.print("Temperature bottom: ");
  Serial.print(t_bot);
  Serial.println(" *C\t");
  Serial.print("Humidity top: ");
  Serial.print(h_top);
  Serial.print(" %\t");
  Serial.print("Temperature top: ");
  Serial.print(t_top);
  Serial.print(" *C\t");
            

  // Check temperatures and switch the relay on and off.
  // NOTE: relay LOW = ON / HIGH = OFF
  //////////////////////////////////////////////////////
  if (t_bot < Tbot_threshold) {
    digitalWrite(fan,LOW);
    //Serial.print(digitalRead(fan), 1);
  }
  else {
    digitalWrite(fan,HIGH);
  }      
  if (t_top < Ttop_threshold) {
    digitalWrite(heater,LOW);
    //Serial.print(digitalRead(heater), 1);
  }
  else {
    digitalWrite(heater,HIGH);
  }




  // Now do web stuff
  ///////////////////
  Serial.print("connecting to ");
  Serial.println(host);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  // We now create a URI for the request. Use the variable names chosen for data.sparkfun
  String url = "/input/";
  url += publicKey;
  url += "?private_key=";
  url += privateKey;
  url += "&temp_top=";
  url += t_top;
  url += "&hum_top=";
  url += h_top;
  url += "&temp_bot=";
  url += t_bot;
  url += "&hum_bot=";
  url += h_bot;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");




  delay(600000); // Send data every 10 minutes
//  delay(30000); // Send data every 30s
}

