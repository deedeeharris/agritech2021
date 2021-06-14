// Edited by Yedidya Harris and Yehuda Yungstien students at HUJI
// https://github.com/deedeeharris/agritech2021 

#include "HX711.h" // import HX711 library
#include <WiFi.h> // import wifi library
#include <Wire.h> // import Wire library for ADS1115
#include <LiquidCrystal_I2C.h> // include LCD library
#include <Adafruit_ADS1X15.h> // import Adafruit library for ADS1115
// #include <Adafruit_ADS1015.h> // import Adafruit library for ADS1115 - the ADS1015 import doesn't work. 


/*************** ThingsSpeak creds start***************/
#include "ThingSpeak.h"
unsigned long myChannelNumber = 1401176;
const char * myWriteAPIKey = "OUZ4TE3F2E9ZIE1O";
const char* server = "api.thingspeak.com";
/*************** ThingsSpeak creds end***************/

/*************** Wifi creds start***************/
const char* ssid = "SSID"; // your wifi SSID name
const char* password = "PASSWORD" ; // wifi password
WiFiClient client;

/*************** Wifi creds end***************/

/*************** SCALE vars start***************/
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 5; // GPIO5 = 50 on BB
const int LOADCELL_SCK_PIN = 2; // GPIO2 = 55 on BB
float zero;
HX711 scale; // defines the HX711 as 'scale'
/*************** SCALE vars end***************/

/*************** PRESSURE vars start***************/
Adafruit_ADS1115 ads; // construct an ads1115 at address 0x48 (default)
float voltage_pres_up = 0.0; // is connected to A1 on ADS
float voltage_pres_down = 0.0; // is connected to A0 on ADS
/*************** PRESSURE vars end***************/

/*************** LCD vars start***************/
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); // // set LCD address, number of columns and rows
/*************** LCD vars end***************/

// setup function 
void setup() {
  
  Serial.begin(57600);
  
  Serial.println("Entering setup"); // for debugging
 
  lcd.init();  // initialize LCD                 
  lcd.backlight();  // turn on LCD backlight     
  
  ads.setGain(GAIN_ONE); // set the input range for the ADS1115:  +/- 4.096V  1 bit = 0.125mV
  ads.begin(); // start our ADS board (gets input from our pressure sensors)


/*************** WIFI+ThingSpeak SETUP start***************/
  WiFi.disconnect();
  delay(10);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  ThingSpeak.begin(client);
 
  WiFi.begin(ssid, password);
  
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("NodeMcu connected to wifi...");
  Serial.println(ssid);
  Serial.println();
/*************** WIFI+ThingSpeak SETUP end***************/

/*************** SCALE SETUP start***************/
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  delay(1000);
  if (scale.is_ready()) {
    long reading = scale.read();
    zero = reading*0.0189-157017; // Ravital and Erez calc the linear change of the scale
  } else {
    Serial.println("HX711 not found.");
  }
/*************** SCALE SETUP end***************/

Serial.println("Exiting setup"); // for debugging

}


// main loop function
void loop() {

  Serial.println("Entering loop"); // for debugging
 
  readScale(); // calling a function to get readings from scale
  readPressure(); // calling a function to get readings from pressure sensors
  Serial.println("Exiting loop"); // for debugging
  
}


// function to get readings from the scale - print to serial and upload to thinkspeak
void readScale(){
  
  Serial.println("Entering readScale function"); // for debugging
  if (scale.is_ready()) {
    long reading = scale.read();
    long finalReading = reading*0.0189-157017-zero;
    ThingSpeak.setField(1,finalReading); // Save finalReading to ThingSpeak 'hyprop' field 1 - weight
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    Serial.println("uploaded to Thingspeak server....");
    Serial.print(finalReading);
    Serial.print(" g");
    Serial.println(); //change line

    lcd.setCursor(0, 0); // set cursor to first column, first row
    lcd.print("Hello world");
    lcd.clear();
  } else {
    Serial.println("HX711 not found.");

  }

  client.stop();
 
  Serial.println("Waiting to upload next reading...");
  Serial.println();  
  // delay(20000);   // thingspeak needs minimum 15 sec delay between updates

  Serial.println("Exiting readScale function"); // for debugging
}


// function to get readings from the Pressure sensors (ADS1115) - print to serial and upload to thinkspeak
void readPressure(){
  

  Serial.println("Entering readPressure function"); // for debugging
  
  int16_t adc0; // initialize the 16 bit long integer variable adc0 which is used to store output from bottom pressure sensor
  int16_t adc1; // initialize the 16 bit long integer variable adc0 which is used to store output from top pressure sensor

  Serial.println("readPressure variables configured"); // for debugging
  
  adc0 = ads.readADC_SingleEnded(0); // read analog channel zero adc value and store this value in adc0 integer variable (bottom)
  adc1 = ads.readADC_SingleEnded(1); // read analog channel zero adc value and store this value in adc1 integer variable (top)

  Serial.println("readPressure readed signals"); // for debugging
  
  voltage_pres_down = (adc0 * 0.125 * 1.5)/1000; // divided by 1000 to convert  mV to V; 0.125 from SetGain; 1.5 voltage divider; (bottom)
  voltage_pres_up = (adc1 * 0.125 * 1.5)/1000; // divided by 1000 to convert  mV to V; 0.125 from SetGain; 1.5 voltage divider; (top)
  float pressure_down = (voltage_pres_down/5 - 0.04) / 0.009;
  float pressure_up = (voltage_pres_up/5 - 0.04) / 0.009;
  
  Serial.print("AIN0: "); 
  Serial.print(adc0);
  Serial.print("\tVoltage: ");
  Serial.println(voltage_pres_down, 7); 
  Serial.println();
  
  Serial.print("AIN1: "); 
  Serial.print(adc1);
  Serial.print("\tVoltage: ");
  Serial.println(voltage_pres_up, 7); 
  Serial.println();
  
  delay(1000);
  
  Serial.println("Exiting readPressure function"); // for debugging
}
