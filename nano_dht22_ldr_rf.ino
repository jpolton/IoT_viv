//
//    FILE: nano_dht22_ldr_rf.ino
//  AUTHOR: Jeff Polton
// VERSION: 09.10.16
// PURPOSE: DHT22 temperature measurement, RF broadcast, decision controlled heater and fan switching ...
//   BOARD: Arduino Nano, ATmega328
//FIRMWARE: Arduino IDE
//     URL: 
// HISTORY:
// 0.1.00 initial version: ESP8266_dht22_sparkfun.ino v3.2.0
// 1.0.00 Cleaned out the ESP8266 WiFi commands

//  STATUS: v1.0.00 WORKS


#include "DHT.h"
#include <SensorTransmitter.h>

#define DHTPIN_bot 2
#define DHTPIN_top 3
#define RFPIN 4
int fan = 7;
int heater = 6;
int lightPin = 0;  //define a pin for Photo resistor

float Ttop_night = 25.0; // Top temperature to activate heater
float Tbot_night = 18.0; // Bottom temperature to active fan
float Ttop_day = 35.0; // Top temperature to activate heater
float Tbot_day = 28.0; // Bottom temperature to active fan
float Tbot_threshold = 0; // initialise
float Ttop_threshold = 0;

DHT dht_bot(DHTPIN_bot, DHT22, 30); // 30 is for cpu clock of esp8266 80Mhz
DHT dht_top(DHTPIN_top, DHT22, 30);

// Initializes a ThermoHygroTransmitter on pin RFPIN, with "random" ID 0, on channel 2.
ThermoHygroTransmitter transmitter(RFPIN, 0, 2);
 
void setup() {
  Serial.begin(115200);
  dht_bot.begin();
  dht_top.begin();
  pinMode(fan, OUTPUT);
  pinMode(heater, OUTPUT);
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
  Serial.print(" %\t\t");
  Serial.print("Temperature top: ");
  Serial.print(t_top);
  Serial.println(" *C\t");

  Serial.print("LDR: ");
  Serial.println(ldr);


  // Transmit the top and bottom temperatures to the weather station display
  /////////////////////////////////////////////////////////////////////////////
  // Temperatures are passed at 10 times the real value,
  // to avoid using floating point math.
  transmitter.sendTempHumi(t_top*10, t_bot);


            
  // Check Light levels and switch between day and night settings
  // Connected via a 10k Ohm resistor, ambient light seems about 1000. Darkish room is about 300.
  //////////////////////////////////////////////////////////////////////////////////////////////		
  if (ldr < 500) {
    Tbot_threshold = Tbot_night;
    Ttop_threshold = Ttop_night;
  }
  else {
    Tbot_threshold = Tbot_day;
    Ttop_threshold = Ttop_day;
  }
 
 

  Serial.print("T theshold top: ");
  Serial.print(Ttop_threshold);
  Serial.print(" *C\t\t");
  Serial.print("T threshold bottom: ");
  Serial.print(Tbot_threshold);
  Serial.println(" *C");
    
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



  delay(30000); // Pause 30s
//  delay(5000); // 5s
}




