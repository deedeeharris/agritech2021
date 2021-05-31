// THIS CODE IS BASED ON Ravital+Adiel code.
// Edited by Yedidya Harris and Yehuda Yungstien students at HUJI
// https://github.com/deedeeharris/agritech2021 

#include "HX711.h" // import HX711 library
#include <WiFi.h> // import wifi library

/*************** ThingsSpeak creds start***************/
#include "ThingSpeak.h"
unsigned long myChannelNumber = 1401176;
const char * myWriteAPIKey = "OUZ4TE3F2E9ZIE1O";
const char* server = "api.thingspeak.com";
/*************** ThingsSpeak creds end***************/

/*************** Wifi creds start***************/
const char* ssid = "HUJI-guest"; // your wifi SSID name
const char* password = "" ; // wifi password
WiFiClient client;

/*************** Wifi creds end***************/

/*************** SCALE vars start***************/
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 5; // GPIO5 = 50 on BB
const int LOADCELL_SCK_PIN = 2; // GPIO2 = 55 on BB
float zero;
HX711 scale; // defines the HX711 as 'scale'
/*************** SCALE vars end***************/

 
void setup() {
  
  Serial.begin(57600);

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

}
 
void loop() {

/*************** SCALE READING start***************/
  if (scale.is_ready()) {
    long reading = scale.read();
    long finalReading = reading*0.0189-157017-zero;
    ThingSpeak.setField(1,finalReading); // Save finalReading to ThingSpeak 'hyprop' field 1 - weight
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    Serial.println("uploaded to Thingspeak server....");
    Serial.print(finalReading);
    Serial.print(" g");
    Serial.println(); //change line
  } else {
    Serial.println("HX711 not found.");
  }

  client.stop();
 
  Serial.println("Waiting to upload next reading...");
  Serial.println();  
  delay(20000);   // thingspeak needs minimum 15 sec delay between updates

/*************** SCALE READING end***************/
}
