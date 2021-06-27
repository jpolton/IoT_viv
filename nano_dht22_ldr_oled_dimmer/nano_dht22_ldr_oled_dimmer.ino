//      FILE: nano_dht22_ldr_rf_oled.ino
//    AUTHOR: Jeff Polton
//   VERSION: 25.05.2021
//   PURPOSE: DHT22 temperature measurement, 
//            decision controlled heater and fan switching, 
//            oled display, dimmer control ...
//     BOARD: Arduino Nano
// PROCESSOR: ATmega328P (Old Bootloader)
//PROGRAMMER: AVRISP mkII
//  FIRMWARE: Arduino IDE
//       URL: 
//   HISTORY:
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
#include <SPI.h> //i2c and the display libraries
#include <Wire.h>
 
// Include Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);


#define DHTPIN_bot (uint8_t)8
#define DHTPIN_top (uint8_t)9
//#define OLED_WIDTH 128 // OLED display width, in pixels
//#define OLED_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET (uint8_t)5   // Reset pin not used but needed for library
#define outputPin  (uint8_t)12  // RBD dimmer
#define zerocross  (uint8_t)2   // RBD can not change on nano

#define FANPIN    (uint8_t)7    // Fan pin - Relay control
#define HEATERPIN (uint8_t)6    // Ceraminc heater pin - Relay control - NON DIMMABLE, BUT ON/OFF SWITHABLE
#define LIGHTPIN  (uint8_t)0    //define a pin for Photo resistor

#define MAX (uint8_t)40 // length of variable storage array
//const int MAX = 40; // 40 works, 50 does not. Though there is some noise on the oled screen w/ MAX=40

//int clock_int = 0;
uint8_t clock_int = 0; // clock "loop" counter


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
//Adafruit_SSD1306 display(OLED_RESET);
//Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)


//int tempArray[ MAX ];
//int humArray[ MAX ];
uint8_t tempArray[ MAX ];
uint8_t humArray[ MAX ];

//********************************************************************
void setup() {
  Serial.begin(115200);
  dht_bot.begin();
  dht_top.begin();
  pinMode(FANPIN, OUTPUT);
  pinMode(HEATERPIN, OUTPUT);
  dimmer.begin();

  // Start Wire library for I2C
  Wire.begin();
  // initialize OLED with I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //display.startscrollright(0x00,0x0F);  

  //Set the color - always use white despite actual display color
  //display.setTextColor(WHITE);
  display.setTextColor(WHITE,BLACK); // Draw white text. Avoid bad pixels? Not sure.
  //Set the font size
  display.setTextSize(1);

/*
  if ( SSD1306_LCDHEIGHT != 64 )
  {
    Serial.print(F("Height incorrect, please fix Adafruit_SSD1306.h!"));
  }

  Serial.println(SSD1306_LCDHEIGHT);
  Serial.println(SSD1306_LCDWIDTH);
*/ 

  // Initialise arrays
  for ( uint8_t i = 0; i < MAX; i++ )
  {
    tempArray[ i ] = 0;
    humArray[ i ] = 0;
  }    
}

//********************************************************************
void serial_disp(float t_top, float t_bot, uint16_t ldr, uint8_t h_top, uint8_t h_bot, float Ttop_threshold, float Tbot_threshold, int Heater_int, bool Fan_bool){
  // Display variables on serial display
  /////////////////////////////////////////////////////////////////////////////
  Serial.print(F("Humidity bot: "));
  Serial.print(h_bot);
  Serial.print(F(" %\t\t"));
  Serial.print(F("Temperature bot: "));
  Serial.print(t_bot);
  Serial.print(F(" *C\t\t"));
  Serial.print(F("T threshold bot: "));
  Serial.print(Tbot_threshold);
  Serial.println(F(" *C"));
  
  Serial.print(F("Humidity top: "));
  Serial.print(h_top);
  Serial.print(F(" %\t\t"));
  Serial.print(F("Temperature top: "));
  Serial.print(t_top);
  Serial.print(F(" *C\t\t"));
  Serial.print(F("T theshold top: "));
  Serial.print(Ttop_threshold);
  Serial.println(F(" *C"));

  Serial.print(F("LDR: "));
  Serial.println(ldr);

  //Serial.print("T theshold top: ");
  //Serial.print(Ttop_threshold);
  //Serial.print(" *C\t\t");
  //Serial.print("T threshold bot: ");
  //Serial.print(Tbot_threshold);
  //Serial.println(" *C");

  Serial.print(F("Heater:"));
  Serial.println(Heater_int);

  Serial.print(F("Fan:"));
  Serial.println(Fan_bool);
  }

//********************************************************************
void oled(float t_top, float t_bot, uint16_t ldr, uint8_t h_top, uint8_t h_bot, int Heater_int, bool Fan_bool, uint8_t clock_int){
  // Display variables on OLED display
  /////////////////////////////////////////////////////////////////////////////
  // Clear the display
  display.stopscroll(); //right(0x00,0x0F);  
  display.clearDisplay();
  display.display();
  delay(1000);

  //Set the cursor coordinates
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(F(" ldr:"));
  display.print(ldr);
  display.print(F(" heat:"));
  display.print(Heater_int);  
  display.print(F(" fan:"));
  display.print(Fan_bool);  
  
  display.setTextSize(1);
  display.setCursor(0,10); 
  //display.print("   "); 
  display.print(t_top,1); // 1 decimal place
  display.print(F(" C"));

  display.print(F("  ")); 
  //display.print(round(h_top));
  display.print(h_top);
  display.print(F(" %"));
  
  display.setTextSize(1);
  display.setCursor(0,20);
  //display.print("   "); 
  display.print(t_bot,1); // 1 decimal place
  display.print(F(" C"));

  display.print(F("  ")); 
  display.print(h_bot);
  //display.print(round(h_bot));
  display.print(F(" %"));

  display.print(F(" hr:")); 
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
    display.startscrollright(0x00,0x07);   // right(0x00,0x0F);  
  }
}


//********************************************************************
void storeTemp()
{
  uint8_t temp = dht_top.readTemperature();
  uint8_t hum  = dht_top.readHumidity();
  static int i = 0;
  if ( isnan( ( uint8_t ) temp ) ) // if no data
    if ( i < MAX ) 
    {
      tempArray[ i ] = 0;
      humArray[ i ] = 0;
      i++;
    }
    else
    {
      for ( uint8_t j = 0; j < MAX - 1; j++ )
      {
        tempArray[ j ] = tempArray[ j + 1 ];
        tempArray[ MAX - 1 ] = 0;
        humArray[ j ] = humArray[ j + 1 ];
        humArray[ MAX - 1 ] = 0; 
      }
    }
  else // else if data not nan
  {
    if ( i < MAX ) // add new datapoint
    {
      tempArray[ i ] = temp;
      humArray[ i ] = hum;
      i++;
    }
    else // shift data along array. Drop oldest value.
    {
      for ( uint8_t j = 0; j < MAX - 1; j++ )
      {
        tempArray[ j ] = tempArray[ j + 1 ];
        tempArray[ MAX - 1 ] = temp;
        humArray[ j ] = humArray[ j + 1 ];
        humArray[ MAX - 1 ] = hum; 
      }
    }
  }
}

//********************************************************************
void drawTempGraph()
{
  drawGraph();
  display.setCursor( 9, 54 );
  display.print(F("Temp:"));
  display.print( ( float ) dht_top.readTemperature(), 1 );
  display.println(F("C"));
  display.setCursor( 0, 0 );
  display.write( 24 );  // up arrow
  display.setCursor( 0, 8 );
  display.print(F("T")); 
  for (uint8_t i = 0; i < MAX; i++ )
    //display.drawFastHLine( 128 - MAX * 2 + i * 2, 64 - tempArray[ i ] * 2, 2, WHITE ); 
    display.drawFastHLine( 128 - MAX * 3 + i * 3, 64 - (tempArray[ i ]-21) * 4, 2, WHITE ); //MAX=40, T=21,37
}
//********************************************************************
void drawHumGraph()
{
  drawGraph();
  display.setCursor( 9, 54 );
  display.print(F("Hum: "));
  display.print( ( float ) dht_top.readHumidity(), 1 );
  display.println(F("%"));
  display.setCursor( 0, 0 );
  display.write( 24 );  // up arrow
  display.setCursor( 0, 8 );
  display.print(F("H")); 
  for (uint8_t i = 0; i < MAX; i++ )
    display.drawFastHLine( 128 - MAX * 2 + i * 2, 64 - humArray[ i ] / 2, 2, WHITE ); 
}  
//********************************************************************
void drawGraph()
{
  display.drawPixel( 6, 13, WHITE ); 
  display.drawPixel( 6, 23, WHITE ); 
  display.drawPixel( 6, 33, WHITE ); 
  display.drawPixel( 6, 43, WHITE ); 
  display.drawPixel( 6, 53, WHITE ); 
  /* x-axis ticks
  display.drawPixel( 27, 62, WHITE ); 
  display.drawPixel( 47, 62, WHITE ); 
  display.drawPixel( 67, 62, WHITE ); 
  display.drawPixel( 87, 62, WHITE ); 
  display.drawPixel( 107, 62, WHITE ); 
  */
  display.drawFastVLine( 7, 0, 100, WHITE );
  display.drawFastHLine( 7, 63, 120, WHITE );
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void loop() {
  delay(5000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  /////////////////////////////////////////////////////////////////////////////
  uint8_t h_bot = dht_bot.readHumidity();
  uint8_t h_top = dht_top.readHumidity();

  // Read temperature as Celsius
  float t_bot = dht_bot.readTemperature();
  float t_top = dht_top.readTemperature();
 
  // Read LDR
  uint16_t ldr = analogRead(LIGHTPIN);
  
  // Check if any reads failed and exit early (to try again). DONT HAVE ERROR TRAPPING ON LDR
  if (isnan(h_top) || isnan(t_top) || isnan(h_bot) || isnan(t_bot)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Store data
  storeTemp();

            
  // Check Light levels and switch between day and night settings
  // Connected via a 10k Ohm resistor, ambient light seems about 1000. Darkish room is about 300.
  //////////////////////////////////////////////////////////////////////////////////////////////		
  if (ldr < 700) { // 500
    Tbot_threshold = Tbot_night;
    Ttop_threshold = Ttop_night;
    uint8_t clock_int = 0; // reset daylight clock
    Serial.print(F("Clock:"));
    Serial.println(float(clock_int)*0.01,2); // 2 decimal place
  }
  else {
    Tbot_threshold = Tbot_day;
    Ttop_threshold = Ttop_day;
    clock_int++; // = clock_int+1; // 1s (oled) + 35s (loop). loop=100 is 1hr
    Serial.print(F("Clock:"));
    Serial.println(float(clock_int)*0.01,2); // 2 decimal place

  }
 

    
  // Check temperatures and switch the relay on and off.
  // NOTE: relay LOW = ON / HIGH = OFF
  //////////////////////////////////////////////////////
  if (t_bot < Tbot_threshold) {
    digitalWrite(FANPIN,HIGH); // Fan OFF
  }
  else {
    digitalWrite(FANPIN,HIGH); // Fan OFF
  }      
  if (t_top < Ttop_threshold - 2) {
    digitalWrite(HEATERPIN,LOW); // Heater ON
    //dimmer.setPower(50); // RBD setPower(0-100%);
    dimmer.set(50); // intensity. Accepts values from 0 to 100.
    // 50%-32C
    //Serial.print("Dimmer intensity: ");
    //Serial.println(dimmer.getValue());
  }
  if (t_top > Ttop_threshold + 2) {
    digitalWrite(HEATERPIN,HIGH); // Heater OFF
    //dimmer.setPower(0); // RBD setPower(0-100%);
    dimmer.set(0); // intensity. Accepts values from 0 to 100
    //Serial.print("Dimmer intensity: ");
    //Serial.println(dimmer.getValue());
  }

  //Serial.print("NEWHeater:");
  //Serial.println(digitalRead(HEATERPIN));
  bool Fan_bool = !digitalRead(FANPIN);
  //int Heater_int = !digitalRead(HEATERPIN);
  uint8_t Heater_int = dimmer.getValue(); // Heater_int is actually type: int
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


  delay(15000); // Pause 15s

  // Display Temperature timeseries display
  /////////////////////////////////////////////////////////////////////////////
  display.stopscroll(); //right(0x00,0x0F);  
  display.clearDisplay();
  display.display();
  drawTempGraph();
  display.display();
      
  delay(15000); // Pause 15s
  //delay(5000); // 5s
}
