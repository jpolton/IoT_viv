//    FILE: nano_dht22_ldr_rf_oled.ino
//  AUTHOR: Jeff Polton
// VERSION: 25.05.2021
// PURPOSE: DHT22 temperature measurement, 
//          decision controlled heater and fan switching, 
//          oled display, dimmer control ...
//   BOARD: Arduino Nano
// PROCESSOR: ATmega328P (Old Bootloader)
//PROGRAMMER: AVRISP mkII
//FIRMWARE: Arduino IDE
//     URL: 
// HISTORY:
// 0.1.00 initial version: ESP8266_dht22_sparkfun.ino v3.2.0
// 1.0.00 Cleaned out the ESP8266 WiFi commands
// 1.0.01 Switch to IR heater controlled by Ttop. No fan
// 1.1.00 Update DHT sensor library
// 1.2.00 Add OLED display
// 1.3.00 Dimmer: https://github.com/circuitar/Dimmer
//        Removed RF transmitter (not compatible)
// 1.3.01 Add loop counter
//
//  STATUS: v1.0.00 WORKS

// How to add graphs: https://github.com/jigsawnz/Arduino-Multitasking/blob/master/clock_graph_temp_hum/clock_graph_temp_hum.ino
/*
  Displays results on 128 x 64 OLED display
  Uses Adafruit SSD1306 OLED Library
  Uses Adafruit GFX Graphics Library

  WIRING: (OLED -- Nano)
  VCC - 5V
  GND - GND
  SCL - A5
  SDA - A4

  Processor: ATmega328P (old bootloader)
*/
#include <DHT.h>
#include <DHT_U.h>
#include "Dimmer.h"

// Include Wire Library for I2C
#include <Wire.h>
 
// Include Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN_bot 8
#define DHTPIN_top 9
#define OLED_RESET 5 // Reset pin not used but needed for library
#define outputPin  12  // RBD dimmer
#define zerocross  2 // RBD can not change on nano

int fan = 7;
int heater = 6;
int lightPin = 0;  //define a pin for Photo resistor
int clock_int = 0; // clock "loop" counter


float Ttop_night = 16; //28.0; // 25 // Top temperature to activate heater
float Tbot_night = 16; //28.0; // 18 // Bottom temperature to active fan
float Ttop_day = 35.0; // 35.0; //30.0// Top temperature to activate fan //heater
float Tbot_day = 23.0; // Bottom temperature to active fan
float Tbot_threshold = 0; // initialise
float Ttop_threshold = 0;
//int Heater_int = 0; // Heater on/off

DHT dht_bot(DHTPIN_bot, DHT22); //, 30); // 30 is for cpu clock of esp8266 80Mhz
DHT dht_top(DHTPIN_top, DHT22); //, 30);

Dimmer dimmer(outputPin, DIMMER_RAMP, 1.5);

// Reset pin not used but needed for library
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
  Serial.begin(115200);
  dht_bot.begin();
  dht_top.begin();
  pinMode(fan, OUTPUT);
  pinMode(heater, OUTPUT);
  dimmer.begin();

  // Start Wire library for I2C
  Wire.begin();
  // initialize OLED with I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //display.startscrollright(0x00,0x0F);  
}

void serial_disp(float t_top, float t_bot, float ldr, float h_top, float h_bot, float Ttop_threshold, float Tbot_threshold, int Heater_int, int Fan_bool){
  // Display variables on serial display
  /////////////////////////////////////////////////////////////////////////////
  Serial.print("Humidity bot: ");
  Serial.print(h_bot);
  Serial.print(" %\t\t");
  Serial.print("Temperature bot: ");
  Serial.print(t_bot);
  Serial.print(" *C\t\t");
  Serial.print("T threshold bot: ");
  Serial.print(Tbot_threshold);
  Serial.println(" *C");
  
  Serial.print("Humidity top: ");
  Serial.print(h_top);
  Serial.print(" %\t\t");
  Serial.print("Temperature top: ");
  Serial.print(t_top);
  Serial.print(" *C\t\t");
  Serial.print("T theshold top: ");
  Serial.print(Ttop_threshold);
  Serial.println(" *C");

  Serial.print("LDR: ");
  Serial.println(ldr);

  //Serial.print("T theshold top: ");
  //Serial.print(Ttop_threshold);
  //Serial.print(" *C\t\t");
  //Serial.print("T threshold bot: ");
  //Serial.print(Tbot_threshold);
  //Serial.println(" *C");

  Serial.print("Heater:");
  Serial.println(Heater_int);

  Serial.print("Fan:");
  Serial.println(Fan_bool);
  }

void oled(float t_top, float t_bot, float ldr, float h_top, float h_bot, int Heater_int, int Fan_bool, int clock_int){
  // Display variables on OLED display
  /////////////////////////////////////////////////////////////////////////////
  // Clear the display
  display.stopscroll(); //right(0x00,0x0F);  
  display.clearDisplay();
  display.display();
  delay(1000);

  //Set the color - always use white despite actual display color
  //display.setTextColor(WHITE);
  display.setTextColor(WHITE,BLACK); // Draw white text. Avoid bad pixels? Not sure.
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0,0);
  display.print(" ldr:");
  display.print(round(ldr));
  display.print(" heat:");
  display.print(Heater_int);  
  display.print(" fan:");
  display.print(Fan_bool);  
  display.setCursor(0,10); 
  //display.print("   "); 
  display.print(t_top,1); // 1 decimal place
  display.print(" C");

  display.print("  "); 
  display.print(round(h_top));
  display.print(" %");
  
  display.setCursor(0,20);
  //display.print("   "); 
  display.print(t_bot,1); // 1 decimal place
  display.print(" C");

  display.print("  "); 
  display.print(round(h_bot));
  display.print(" %");

  display.print(" hr:"); 
  display.print(float(clock_int)*0.01,2); // 2 decimal place
    
  display.display();


  if (ldr < 10) {
    // Clear the display if it is dark
    delay(2000);
    display.stopscroll(); //right(0x00,0x0F);  
    display.clearDisplay();
    display.display();
  }
  else {
    display.startscrollright(0x00,0x0F);  
  }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

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

            
  // Check Light levels and switch between day and night settings
  // Connected via a 10k Ohm resistor, ambient light seems about 1000. Darkish room is about 300.
  //////////////////////////////////////////////////////////////////////////////////////////////		
  if (ldr < 700) { // 500
    Tbot_threshold = Tbot_night;
    Ttop_threshold = Ttop_night;
    clock_int = 0; // reset daylight clock
    Serial.print("Clock:");
    Serial.println(float(clock_int)*0.01,2); // 2 decimal place
  }
  else {
    Tbot_threshold = Tbot_day;
    Ttop_threshold = Ttop_day;
    clock_int = clock_int+1; // 1s (oled) + 35s (loop). loop=100 is 1hr
    Serial.print("Clock:");
    Serial.println(float(clock_int)*0.01,2); // 2 decimal place

  }
 

    
  // Check temperatures and switch the relay on and off.
  // NOTE: relay LOW = ON / HIGH = OFF
  //////////////////////////////////////////////////////
  if (t_bot < Tbot_threshold) {
    digitalWrite(fan,HIGH); // Fan OFF
  }
  else {
    digitalWrite(fan,HIGH); // Fan OFF
  }      
  if (t_top < Ttop_threshold - 2) {
    digitalWrite(heater,LOW); // Heater ON
    //dimmer.setPower(50); // RBD setPower(0-100%);
    dimmer.set(50); // intensity. Accepts values from 0 to 100.
    // 50%-32C
    //Serial.print("Dimmer intensity: ");
    //Serial.println(dimmer.getValue());
  }
  if (t_top > Ttop_threshold + 2) {
    digitalWrite(heater,HIGH); // Heater OFF
    //dimmer.setPower(0); // RBD setPower(0-100%);
    dimmer.set(0); // intensity. Accepts values from 0 to 100
    //Serial.print("Dimmer intensity: ");
    //Serial.println(dimmer.getValue());
  }

  //Serial.print("NEWHeater:");
  //Serial.println(digitalRead(heater));
  int Fan_bool = !digitalRead(fan);
  //int Heater_int = !digitalRead(heater);
  int Heater_int = dimmer.getValue(); // Heater_int is actually type: int
  //Serial.print("Test:");
  //Serial.println(test);
  
  // Adjust dimmer intensity. (range: 0-100)
  ///////////////////////////////////////////////////////////
  //dimmer.set(50); // intensity. Accepts values from 0 to 100
  //Serial.print("Dimmer intensity: ");
  //Serial.println(dimmer.getValue());

  // Display variables on OLED display
  /////////////////////////////////////////////////////////////////////////////
  oled(t_top, t_bot, ldr, h_top, h_bot, Heater_int, Fan_bool, clock_int);

  // Display variables on serial display
  /////////////////////////////////////////////////////////////////////////////
  serial_disp(t_top, t_bot, ldr, h_top, h_bot, Ttop_threshold, Tbot_threshold, Heater_int, Fan_bool);


  delay(30000); // Pause 30s
  //delay(5000); // 5s
}
