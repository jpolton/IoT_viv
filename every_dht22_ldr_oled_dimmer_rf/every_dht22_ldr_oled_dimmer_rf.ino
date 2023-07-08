//      FILE: every_dht22_ldr_oled_dimmer_rf.ino
//    AUTHOR: Jeff Polton
//   VERSION: 25.05.2021
//   PURPOSE: DHT22 temperature measurement, 
//            decision controlled heater and fan switching, 
//            oled display, dimmer control, RF broadcast ...
//     BOARD: Arduino megaAVR boards --> arduino nano every
// PROCESSOR: ATMEGA4809
//PROGRAMMER: Onboard Atmel mEDBG (UNO WiFi Rev2)
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
// 1.3.01 Add loop counter. Add graphs.
// 1.4.01 Add graphs. E.g. https://github.com/jigsawnz/Arduino-Multitasking/blob/master/clock_graph_temp_hum/clock_graph_temp_hum.ino (Numbering catchup)
// 1.5.00 delay() --> millis()
// 1.5.01 remove all delay() statements
// 1.6.00 Replay RB dimmer with Krida PWM dimmer (PWM AC Dimmer TRIAC 8A SSR RELAY Module 50Hz)
// 2.0.00 Nano Every
// 2.1.00 Add rf broadcast
// 2.2.00 Restructure around ramping IR radiation. Temperature control only if too hot.
// 2.3.00 Add 433Mhz RF control of main socket switches.

//  STATUS: v2.3.00 WORKS

/*
  Displays results on 128 x 64 OLED display
  Uses Adafruit SSD1306 OLED Library
  Uses Adafruit GFX Graphics Library

  WIRING: (OLED -- Nano Every)
  VCC - 5V
  GND - GND
  SCL - A5
  SDA - A4

*/

#include <DHT.h>
#include <DHT_U.h>
#include <SensorTransmitter.h>
#include <RCSwitch.h> // Needed to add "megaavr" to allowed architectures in library.properties: architectures=avr,esp8266,esp32,stm32,megaavr

// Include Wire Library for I2C
#include <SPI.h> //i2c and the display libraries
#include <Wire.h>
 
// Include Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);


#define DHTPIN_bot (uint8_t)8
#define DHTPIN_top (uint8_t)9
#define RFPIN (uint8_t)4
#define OLED_RESET (uint8_t)5   // Reset pin not used but needed for library

#define HELIOS_PWM_PIN  (uint8_t)11  // HELIOS PWM dimmer - for IR heater
#define HELIOS_RELAY_PIN (uint8_t)6    // HELIOS LED pin - Relay control - NON DIMMABLE, BUT ON/OFF SWITHABLE
#define FAN_PIN    (uint8_t)7    // Fan pin - Relay control
#define LIGHT_PIN  (uint8_t)0    //define a pin for Photo resistor

#define MAX (uint8_t)120 //40 nano // length of variable storage array
// nano: 40 works, 50 does not. Though there is some noise on the oled screen w/ MAX=40
// every 120 - 6 hr display with pixel every 6mins
#define Ttop_night (float)16 //28.0; // 25 // Top temperature to activate heater
#define Tbot_night (float)16 //28.0; // 18 // Bottom temperature to active fan
#define Ttop_day (float)35.0 //30.0// Top temperature to activate fan //heater
#define Tbot_day (float)23.0 // Bottom temperature to active fan

#define MID_AM  (uint16_t) 300    // 3*100: hours*100 since SUNRISE to MID MORNING
#define MID_PM  (uint16_t) 900    // 9*100: hours*100 since SUNRISE to MID AFTERNOON
#define NOON  (uint16_t) 600      // 6*100: hours*100 since SUNRISE to NOON

// CAN INITIALISE START HOUR*100 SINCE SUNRISE HERE.
uint16_t clock_int = 0 ; //660; //0; // clock "loop" counter. 100 per hr. range[0:1200]

boolean day_bool = 0; // day->true, night->false

float Tbot_threshold = 0; // initialise
float Ttop_threshold = 0;

DHT dht_bot(DHTPIN_bot, DHT22); //, 30); // 30 is for cpu clock of esp8266 80Mhz
DHT dht_top(DHTPIN_top, DHT22); //, 30);

// Initializes a ThermoHygroTransmitter on pin RFPIN, with "random" ID 0, on channel 2.
ThermoHygroTransmitter transmitter(RFPIN, 0, 2);

// initialise RCSwitch to communicate with 433Mhz mains socket switches
RCSwitch mySwitch = RCSwitch();

// Reset pin not used but needed for library
//Adafruit_SSD1306 display(OLED_RESET);
//Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)

// Carried variables
uint8_t t_bot_arr[ MAX ];
uint8_t t_top_arr[ MAX ];
uint8_t dim_arr[ MAX ];
uint8_t Heater_int = 0; // Heater [0,255]

unsigned long time_now = millis(); // timer for loop() control
unsigned long time_log = millis(); // timer for logging control

//********************************************************************
void setup() {
  Serial.begin(9600);
  dht_bot.begin();
  dht_top.begin();
  
  digitalWrite(FAN_PIN,HIGH); // Fan OFF
  pinMode(FAN_PIN, OUTPUT);
  
  digitalWrite(HELIOS_RELAY_PIN,HIGH); // HELEOS LED OFF
  pinMode(HELIOS_RELAY_PIN, OUTPUT);
  
  pinMode(HELIOS_PWM_PIN, OUTPUT);

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

  // Transmitter is connected to RFPIN  
  mySwitch.enableTransmit(RFPIN);
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
    t_bot_arr[ i ] = 0;
    t_top_arr[ i ] = 0;
    dim_arr[ i ] = 0;
  }    
}

//********************************************************************
void serial_disp(float t_top, float t_bot, uint16_t ldr, uint8_t h_top, uint8_t h_bot, float Ttop_threshold, float Tbot_threshold, bool Fan_bool, uint16_t clock_int, bool day_bool){
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

  Serial.print(F("Heater:"));
  Serial.println(Heater_int);

  Serial.print(F("Fan:"));
  Serial.println(Fan_bool);
  
  Serial.print(F("Day:"));
  Serial.println(day_bool);
  
  Serial.print(F("hr:")); 
  Serial.println(float(clock_int)*0.01,2); // 2 decimal place
  }

//********************************************************************
void oled(float t_top, float t_bot, uint16_t ldr, uint8_t h_top, uint8_t h_bot, float Ttop_threshold, float Tbot_threshold, bool Fan_bool, uint16_t clock_int, bool day_bool){
  // Display variables on OLED display
  /////////////////////////////////////////////////////////////////////////////
  // Clear the display
  display.stopscroll(); //right(0x00,0x0F);  
  display.clearDisplay();
  display.display();
  //delay(1000);

  //Set the cursor coordinates
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(F("ldr:"));
  display.print(ldr);
  display.print(F(" heat:"));
  display.print(Heater_int);  
 
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print(F("fan:"));
  display.print(Fan_bool); 

  display.print(F("  ")); 
  display.print(F(" hr:")); 
  display.print(float(clock_int)*0.01,2); // 2 decimal place

  display.setTextSize(1);
  display.setCursor(0,20);
  if ( day_bool > 0 )
  {
    display.print(F("day"));
  }
  else
  {
    display.print(F("night"));
  }
    
  display.setTextSize(1);
  display.setCursor(0,40); 
  //display.print("   "); 
  display.print(t_top,1); // 1 decimal place
  display.print(F(" C"));

  display.print(F("  ")); 
  //display.print(round(h_top));
  display.print(h_top);
  display.print(F(" %"));
  
  display.print(F("  (")); 
  display.print(Ttop_threshold,0);  // 0 decimal place
  display.print(F(" C)"));
  
  display.setTextSize(1);
  display.setCursor(0,50);
  //display.print("   "); 
  display.print(t_bot,1); // 1 decimal place
  display.print(F(" C"));

  display.print(F("  ")); 
  display.print(h_bot);
  //display.print(round(h_bot));
  display.print(F(" %"));

  display.print(F("  (")); 
  display.print(Tbot_threshold,0);  // 0 decimal place
  display.print(F(" C)"));
  

    
  display.display();
}


//********************************************************************
void storeTemp()
{
  uint8_t temp1 = dht_bot.readTemperature();
  uint8_t temp2 = dht_top.readTemperature();
  //bool Helios_bool = !digitalRead(HELIOS_RELAY_PIN);
  uint8_t dim  = Heater_int;
  static int i = 0;
  if ( isnan( ( uint8_t ) temp1 ) ) // if no data
    if ( i < MAX ) 
    {
      t_bot_arr[ i ] = 0;
      t_top_arr[ i ] = 0;
      dim_arr[ i ] = 0;
      i++;
    }
    else
    {
      for ( uint8_t j = 0; j < MAX - 1; j++ )
      {
        t_bot_arr[ j ] = t_bot_arr[ j + 1 ];
        t_top_arr[ j ] = t_top_arr[ j + 1 ];
        t_bot_arr[ MAX - 1 ] = 0;
        t_top_arr[ MAX - 1 ] = 0;
        dim_arr[ j ] = dim_arr[ j + 1 ];
        dim_arr[ MAX - 1 ] = 0; 
      }
    }
  else // else if data not nan
  {
    if ( i < MAX ) // add new datapoint
    {
      t_bot_arr[ i ] = temp1;
      t_top_arr[ i ] = temp2;
      dim_arr[ i ] = dim;
      i++;
    }
    else // shift data along array. Drop oldest value.
    {
      for ( uint8_t j = 0; j < MAX - 1; j++ )
      {
        t_bot_arr[ j ] = t_bot_arr[ j + 1 ];
        t_bot_arr[ MAX - 1 ] = temp1;
        t_top_arr[ j ] = t_top_arr[ j + 1 ];
        t_top_arr[ MAX - 1 ] = temp2;
        dim_arr[ j ] = dim_arr[ j + 1 ];
        dim_arr[ MAX - 1 ] = dim; 
      }
    }
  }
}

//********************************************************************
void drawGraphData()
{
  drawGraphAxes();
  display.setCursor( 9, 44 );
  display.print(F("Temp:"));
  display.print( ( float ) dht_bot.readTemperature(), 1 );
  display.print(F("/"));
  display.print( ( float ) dht_top.readTemperature(), 1 );
  display.println(F("C"));
  display.setCursor( 0, 0 );
  display.write( 24 );  // up arrow
  display.setCursor( 0, 8 );
  display.print(F("T")); 
  for (uint8_t j = 0; j < MAX; j++ )
    //display.drawFastHLine( 128 - MAX * 2 + j * 2, 64 - t_bot_arr[ j ] * 2, 2, WHITE ); 
    //display.drawFastHLine( 128 - MAX * 3 + j * 3, 64 - (t_bot_arr[ j ]-21) * 4, 2, WHITE ); //MAX=40, T=21,37
    display.drawFastHLine( 128 - MAX + j, 64 - (t_top_arr[ j ]-15) * 3, 2, WHITE ); //MAX=120, T=21,37
  for (uint8_t j = 0; j < MAX; j++ )
    display.drawFastHLine( 128 - MAX + j, 64 - (t_bot_arr[ j ]-15) * 3, 3, WHITE );
  for (uint8_t j = 0; j < MAX; j++ )
    //display.drawFastHLine( 128 - MAX * 3 + j * 3, 64 - dim_arr[ j ] * 64/100, 2, WHITE ); //MAX=40, Dim=0,100
    display.drawFastHLine( 128 - MAX + j, 64 - dim_arr[ j ] * 64/255, 2, WHITE );
}
//********************************************************************
void drawGraphAxes()
{
  display.drawPixel( 6, 13, WHITE ); 
  display.drawPixel( 6, 23, WHITE ); 
  display.drawPixel( 6, 33, WHITE ); 
  display.drawPixel( 6, 43, WHITE ); 
  display.drawPixel( 6, 53, WHITE ); 
  // x-axis ticks
  display.drawPixel( 27, 62, WHITE ); 
  display.drawPixel( 47, 62, WHITE ); 
  display.drawPixel( 67, 62, WHITE ); 
  display.drawPixel( 87, 62, WHITE ); 
  display.drawPixel( 107, 62, WHITE ); 
  //
  display.drawFastVLine( 7, 0, 100, WHITE );
  //display.drawFastHLine( 7, 63, 120, WHITE );
  display.drawFastHLine( 7, 61, 120, WHITE );
}

//********************************************************************
void errorTrap(uint8_t h_top, uint8_t h_bot,uint16_t t_top, uint16_t t_bot)
{
  // Check if any reads failed and exit early (to try again). DONT HAVE ERROR TRAPPING ON LDR
  if (isnan(h_top) || isnan(t_top) || isnan(h_bot) || isnan(t_bot)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    display.setTextSize(1);
    display.setCursor(0,10); 
    display.print(F("Failed to read sensor!")); 
    display.setCursor(0,20); 
    display.print(t_top); 
    display.print(t_bot); 
    display.display();
    return;
  }
}

//********************************************************************
void HELIOS_val(int clock_int)
{
    // Calculate value for HELIOS DIMMER
    // value range: [0-255]
    // leds only turn on in range 120 - 255
    // 120 didn't seem to fire on every, though ok on nano // OLD NOTES
    if (clock_int < NOON) { // Ramp up to NOON: [0-255]
      Heater_int = int(clock_int * 0.425);  // (255 / 600) = 0.425  ms->mins over 6hrs scaling from 0:255
      //Serial.println(F("A.M."));
    } 
    else {  // Ramp down from NOON: [255-0] 
      //Heater_int = int((NOON + NOON - millis() + time_sunrise)/float(NOON)*255);
      Heater_int = int((NOON + NOON - clock_int) * 0.425); //  scaling over 6hrs scaling from 255:0
      //Heater_int = (NOON + NOON - millis() + time_sunrise) / 84705;  // (60000 / 255 * 6*60) = 84,705  ms->mins over 6hrs scaling from 255:0
      //Serial.println(F("P.M."));
    }

    //Serial.print(F("clock_int:"));
    //Serial.println(clock_int);
    //Serial.print(F("Heater calc:"));
    //Serial.println((int(10 * clock_int * 0.425)));
    return;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void loop() {

  // Store data
  if(millis() - time_log > 360000) // 6 mins
  {
    time_log = millis();
    storeTemp();
  }
  
  if(millis() - time_now > 36000) // 36s loop --> 100 loops per hr
  //if(millis() - time_now > 3600) // 3.6s loop --> 1000 loops per hr // FOR DEBUGGING
  {
    time_now = millis();
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    /////////////////////////////////////////////////////////////////////////////
    uint8_t h_bot = dht_bot.readHumidity();
    uint8_t h_top = dht_top.readHumidity();
  
    // Read temperature as Celsius
    float t_bot = dht_bot.readTemperature();
    float t_top = dht_top.readTemperature();
    
    // Check if any reads failed and exit early (to try again). DONT HAVE ERROR TRAPPING ON LDR
    errorTrap(h_top, h_bot, t_top, t_bot);
   
    // Read LDR
    uint16_t ldr = analogRead(LIGHT_PIN);

    //int Heater_int = 0;
    //int Heater_int = 140;   // value range: [0-255]
                            // leds only turn on in range 120 - 255
                            // 120 didn't seem to fire on every, though ok on nano


    
  
                
    // Check Light levels and switch between day and night settings
    // Connected via a 10k Ohm resistor, ambient light seems about 1000. Darkish room is about 300.
    // Summer morning ~650
    //////////////////////////////////////////////////////////////////////////////////////////////    
    if (ldr < 500) { // 700
      Tbot_threshold = Tbot_night;
      Ttop_threshold = Ttop_night;
      clock_int = 0; // reset daylight clock
      day_bool = 0; // night
    }
    else {
      Tbot_threshold = Tbot_day;
      Ttop_threshold = Ttop_day;
      day_bool = 1; // day
      clock_int++; // loop=100 is 1hr
    }
   
  
    // Check TIME. MID MORNING/AFTERNOON:  switch the relay BRIGHT LIGHTS on/off.
    // NOTE: relay LOW = ON / HIGH = OFF
    //////////////////////////////////////////////////////
    if((clock_int > MID_AM) && (clock_int < MID_PM)) {
      digitalWrite(HELIOS_RELAY_PIN,LOW); // HELEOS LED ON
      Serial.println(F("LIGHTS ON."));
    }
    else {
      digitalWrite(HELIOS_RELAY_PIN,HIGH); // HELEOS LED OFF
      Serial.println(F("LIGHTS OFF."));
    }
    Serial.println(clock_int > MID_AM);
    Serial.println(clock_int < MID_PM);
      
    // DAY TIME
    // Check TIME. MORNING: RAMP the HEATER UP. AFTERNOON RAMP the HEATER DOWN
    //////////////////////////////////////////////////////
    if (day_bool) {
      HELIOS_val(clock_int);
      Serial.print(F("Heater reset:"));
      Serial.println(Heater_int);
    }
    else { // NIGHT
      Heater_int = 0;      
    }
    
    // EXCEPT Heater if cold
    //////////////////////////////////////////////////////
    if (t_top < Tbot_threshold) { // COLD. I.e. if heatable end cooler than lowest threshold temp
      Heater_int = 140;  // an old setting when always on [0,255]      
    }
      
    // Check temperatures and switch the BLUE FAN relay on and off.
    // NOTE: relay LOW = ON / HIGH = OFF
    //////////////////////////////////////////////////////
    //if ((t_top > Ttop_threshold - 2) && (day_bool)) {
    if ((t_bot < Ttop_threshold - 2) && (day_bool)) { // VERTICAL MIXING IF NOT TOO HOT
      digitalWrite(FAN_PIN,LOW); // Fan ON
    }
    else {
      digitalWrite(FAN_PIN,HIGH); // Fan OFF
    }


    // Check temperatures and switch the BLACK (EXIT) FAN relay on and off.
    // NOTE: relay LOW = ON / HIGH = OFF
    //////////////////////////////////////////////////////
    if ((t_bot > Ttop_threshold - 5) && (day_bool)) { // EJECT HOT AIR + LIMIT IR HEATING
      // RF transmit // Fan ON
      mySwitch.send(1397079, 24); // Button 3: ON (found notes on Evernote: Decoding RF signal on RPi)
      if (Heater_int > 140){  // LIMIT IR ONLY IF OVER THRESHOLD
        Heater_int = 140;  // an old setting when always on [0,255]      
      }
    }
    else {
      // RF transmit // Fan OFF
      mySwitch.send(1397076, 24); // Button 3: OFF
    }

    if ((t_bot > Ttop_threshold - 4) && (day_bool)) {  // TOO HOT
      digitalWrite(HELIOS_RELAY_PIN,HIGH); // HELIOS LED OFF
      Heater_int = 0;                      // HELIOS IR OFF
    }    

    // Set IR heating
    //////////////////////////////////////////////////////
    analogWrite(HELIOS_PWM_PIN,Heater_int); 

    bool Fan_bool = !digitalRead(FAN_PIN);
    //bool Helios_bool = !digitalRead(HELIOS_RELAY_PIN);


    // Display variables on serial display
    /////////////////////////////////////////////////////////////////////////////
    serial_disp(t_top, t_bot, ldr, h_top, h_bot, Ttop_threshold, Tbot_threshold, Fan_bool, clock_int, day_bool);

    // Display variables on OLED display
    /////////////////////////////////////////////////////////////////////////////
    oled(t_top, t_bot, ldr, h_top, h_bot, Ttop_threshold, Tbot_threshold, Fan_bool, clock_int, day_bool);

    // Transmit the top and bottom temperatures to the weather station display
    /////////////////////////////////////////////////////////////////////////////
    // Temperatures are passed at 10 times the real value,
    // to avoid using floating point math.
    transmitter.sendTempHumi(t_top*10, t_bot);
  
  }

 
  
  if (analogRead(LIGHT_PIN) > 10) // display management if it is LIGHT
  {
    if((millis() - time_now > 15000) && (millis() - time_now < 16000))
    {
      // OLED display refresh management
      /////////////////////////////////////////////////////////////////////////////
      display.stopscroll(); //right(0x00,0x0F);  
      display.clearDisplay();
      display.display();
    }

    if(millis() - time_now > 16000)
    {  
      // Display Temperature timeseries display
      // clearDisplay --> draw --> display seems to have the 'cleanest' screen
      /////////////////////////////////////////////////////////////////////////////
      //display.stopscroll(); //right(0x00,0x0F);  
      //display.clearDisplay();
      drawGraphData();
      display.display();
      //display.startscrollright(0x00,0x07);   // right(0x00,0x0F);  
    }   
  }
  else // Clear the display if it is DARK
  {
    if(millis() - time_now > 3000)
    {
      display.clearDisplay();
      display.display();        
    }
  }
  


}
