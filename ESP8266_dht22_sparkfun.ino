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
// 2.0.00 Relay control added and tested. (Note that the LED on the fan seems to sometime
//        rapidly flash when the fan is switched on.) 
// 3.0.00 Add light dependent resistor control to temperature thresholds (night and day target temperatures).
//        only writes to web every (e.g. 20) loops.
// 3.1.00 Add infrastructure to use either LDR or internet time based switching. Not working. Not activated.
// 3.2.00 Add 433MHz transmission of temperatures to weather receiver.

//  STATUS: v3.2.00 WORKS. 
/*
    NOTES:
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.

    Posts data e.g.:
	https://data.sparkfun.com/input/aG8bAlQybziD5Qa780yY?private_key=.....&hum_bot=50.7&hum_top=27.1&temp_bot=16.3&temp_top=29.7
 */
#include <ESP8266WiFi.h>
#include "DHT.h"
//#include <SensorTransmitter.h>
 
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

// Switch day/night mode either with LDR or internet time
const char* switc = "LDR"; 
//const char* switc = "internet"; 
int hour = 0; // initialise internet clock hour variable

float Ttop_night = 25.0; // Top temperature to activate heater
float Tbot_night = 18.0; // Bottom temperature to active fan
float Ttop_day = 35.0; // Top temperature to activate heater
float Tbot_day = 28.0; // Bottom temperature to active fan
float Tbot_threshold = 0; // initialise
float Ttop_threshold = 0;
int lightPin = 0;  //define a pin for Photo resistor
int count = 0; // loop counter: only GET request web post every 10 cycles
DHT dht_bot(DHTPIN_bot, DHT22, 30); // 30 is for cpu clock of esp8266 80Mhz
DHT dht_top(DHTPIN_top, DHT22, 30);

// Initializes a ThermoHygroTransmitter on pin 2=D4 (first field), with "random" ID 0, on channel 2.
//ThermoHygroTransmitter transmitter(2, 0, 2); // pin2=D4 in ESP8266
 
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
 
  // Read LDR
  int ldr = analogRead(lightPin);
  
  // Check if any reads failed and exit early (to try again). DONT HAVE ERROR TRAPPING ON LDR
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
  Serial.println(" *C\t");

  Serial.print("LDR: ");
  Serial.println(ldr);

  // Transmit the top and bottom temperatures to the weather station display
  /////////////////////////////////////////////////////////////////////////////
  // Temperatures are passed at 10 times the real value,
  // to avoid using floating point math.
  //transmitter.sendTempHumi(t_top*10, t_bot);

            
  // Check Light levels and switch between day and night settings
  // Connected via a 10k Ohm resistor, ambient light seems about 1000. Darkish room is about 300.
  //////////////////////////////////////////////////////////////////////////////////////////////
  if (switc == "LDR") {		
    if (ldr < 500) {
      Tbot_threshold = Tbot_night;
      Ttop_threshold = Ttop_night;
    }
    else {
      Tbot_threshold = Tbot_day;
      Ttop_threshold = Ttop_day;
    }
  }
  else if (switc == "internet") {
    if (hour > 8 || hour < 20) {
      Tbot_threshold = Tbot_day;
      Ttop_threshold = Ttop_day;
    }
    else {
      Tbot_threshold = Tbot_night;
      Ttop_threshold = Ttop_night;
    }
  }
  else {
    Serial.print("This switch option is not expected!");
  }

  Serial.print("T theshold top: ");
  Serial.print(Ttop_threshold);
  Serial.print(" %\t");
  Serial.print("T threshold bottom: ");
  Serial.print(Tbot_threshold);
  Serial.println(" *C\t");
    
  // Check temperatures and switch the relay on and off.
  // NOTE: relay LOW = ON / HIGH = OFF
  //////////////////////////////////////////////////////
  if (t_bot < Tbot_threshold) {
    digitalWrite(fan,LOW); // Fan ON
  }
  else {
    digitalWrite(fan,HIGH); // Fan OFF
  }      
  if (t_top < Ttop_threshold) {
    digitalWrite(heater,LOW); // Heater ON
  }
  else {
    digitalWrite(heater,HIGH); // Heater OFF
  }




  // Now do web stuff
  // Only do every 20 loops
  ///////////////////
  if (count > 20) {
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

    // Get the clock hour
    //Serial.println(getTime());
    //hour = getTime().toInt();

    // reset counter
    count = 0;
  }
  count++;
  Serial.println(count);



  delay(30000); // Pause 30s
//  delay(5000); // 5s
}


// Never called!
String getTime() {
  WiFiClient client;
  int i=0;
  while (!!!client.connect("google.com", 80) & i < 10) {
    Serial.println("connection failed, retrying...");
    i++;
  }
  if(i = 10) {
    Serial.println("Failed: Need to reset wifi");
    // initWifi();
  }

  client.print("HEAD / HTTP/1.1\r\n\r\n");
 
  while(!!!client.available()) {
     yield();
  }

  while(client.available()){
    if (client.read() == '\n') {    
      if (client.read() == 'D') {    
        if (client.read() == 'a') {    
          if (client.read() == 't') {    
            if (client.read() == 'e') {    
              if (client.read() == ':') {    
                client.read();
                String theDate = client.readStringUntil('\r');
                client.stop();
//                String timeZone = theDate.substring(20,25); // not implemented
                String theHour = theDate.substring(14,16);
                return theHour;
              }
            }
          }
        }
      }
    }
  }
}

