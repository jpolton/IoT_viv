//
//    FILE: ESP8266_dht22_sparkfun.ino
//  AUTHOR: Jeff Polton
// VERSION: 04.10.16
// PURPOSE: DHT22 send GET request to data.sparkfun stream
//   BOARD: ESP8266 NodeMCU 1.0 (ESP-12E module)
//FIRMWARE: Arduino IDE
//     URL: http://data.sparkfun.com/streams/XGYoZzy8ExFQYmQr3xRd
            http://pcwww.liv.ac.uk/~jpolton/plots/esp8266_dht22.html 
//          /Users/jeff/Public/liv.ac.uk/plots/esp8266_dht22.html
// HISTORY:
// 0.1.00 initial version: http://hpclab.blogspot.co.uk/2015/06/esp8266-based-wifi-weather-monitoring.html
// 0.1.01 Edit to accommodate two sensors.
// 
//  STATUS: WORKS. Though I've not actually tried in on the Vivarium..


/*
    NOTES:
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.

    Posts data e.g.:
      data.sparkfun.com/input/XGYoZzy8ExFQYmQr3xRd?private_key=1JvDzeYjNPSEb9ExzyAa&temperature=22.74&humidity=77.7      
 */

#include <ESP8266WiFi.h>
#include "DHT.h"
#define DHTPIN_bot D5     // D5 pin of ESP8266
#define DHTPIN_top D3     // D3 pin of ESP8266
const char* ssid     = "....";
const char* password = "...";
const char* host = "data.sparkfun.com";
const char* publicKey = "XGYoZzy8ExFQYmQr3xRd";
const char* privateKey = "...";
DHT dht_bot(DHTPIN_bot, DHT22, 30); // 30 is for cpu clock of esp8266 80Mhz
DHT dht_top(DHTPIN_top, DHT22, 30);
void setup() {
  Serial.begin(115200);
  dht.begin();
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
}