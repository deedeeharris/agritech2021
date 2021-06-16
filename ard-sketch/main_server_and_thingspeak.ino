// Edited by Yedidya Harris and Yehuda Yungstien students at HUJI
// https://github.com/deedeeharris/agritech2021 

// import libraries
#include <WiFi.h> // import wifi library
#include <WebServer.h> // import library for web server
#include <Wire.h>
#include <WiFiUdp.h> // import library for Network Time Protocol
#include <NTPClient.h> // import library for Network Time Protocol
#include "HX711.h" // import HX711 library
#include <Adafruit_ADS1X15.h> // import Adafruit library for ADS1115
#include "ThingSpeak.h" // import library for ThingSpeak


float weight, pressure_up, pressure_down; // global vars to upload to ThingSpeak

/*************** ThingsSpeak creds start***************/
unsigned long myChannelNumber = 00000;
const char * myWriteAPIKey = "AAAA";
const char* server = "api.thingspeak.com";
WiFiClient  client;
/*************** ThingsSpeak creds end***************/


/*************** Wifi creds start***************/
const char* ssid = "AAAA";  // Enter SSID here
const char* password = "11111";  // Enter Password here
WebServer wserver(80); // important: we called the websever as 'wserver' and not 'server', since 'server' is already used for ThingSpeak above
/*************** Wifi creds end***************/


/*************** NTP (getting time from the netowrk) start***************/
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Global variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
/*************** NTP (getting time from the netowrk) end***************/

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


            
 
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("Entering setup"); // for debugging
  
  ads.setGain(GAIN_ONE); // set the input range for the ADS1115:  +/- 4.096V  1 bit = 0.125mV
  ads.begin(); // start our ADS board (gets input from our pressure sensors)


  /*************** WIFI + WEB SERVER + NTP + ThingSpeak SETUP start***************/
  Serial.println("Connecting to "); // wifi
  Serial.println(ssid); // wifi

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP()); // print the esp32 ip address

  wserver.on("/", handle_OnConnect); // web server
  wserver.onNotFound(handle_NotFound); // web server
  
  wserver.begin(); // start the web server
  Serial.println("HTTP server started");
  ThingSpeak.begin(client); // start the ThingSpeak client
  
  
  // NTP - get time from the internet. For printing the time on the local browser on phone/pc
  // Set offset time in seconds to adjust for your timezone.
  // We used the following reference to get the local time in Rehovot, Israel https://www.cisco.com/c/en/us/support/docs/broadband-cable/cable-modem-termination-systems-cmts/12188-calculate-hexadecimal-dhcp.html
  timeClient.begin(); // Initialize a NTPClient 
  timeClient.setTimeOffset(10800);

  /*************** WIFI + WEB SERVER + NTP + ThingSpeak SETUP end***************/


  /*************** SCALE SETUP start***************/
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  delay(1000);
  if (scale.is_ready()) {
    long reading = scale.read();
    zero = reading*0.0189-157017; // Ravital and Adiel from course 71252 HUJI 2021 calculated the linear deviation of the scale - Calibration Factor. You may need to use a diff factor for your scale.
  } else {
    Serial.println("HX711 not found.");
  }
  /*************** SCALE SETUP end***************/

  Serial.println("Exiting setup"); // for debugging

}

void loop() {

 for (int i = 0; i <= 15; i++) {
    wserver.handleClient(); // delivers the requested HTML page (to see the readings in the browser)
    readScale(); // calling a function to get readings from scale
    readPressure(); // calling a function to get readings from pressure sensors
    ThingSpeak.setField(1,weight); // Save finalReading to ThingSpeak 'hyprop' field 1 - weight
    ThingSpeak.setField(2,pressure_down); // Save finalReading to ThingSpeak 'hyprop' field 2 - Pressure Bottom
    ThingSpeak.setField(3,pressure_up); // Save finalReading to ThingSpeak 'hyprop' field 3 - Pressure Top
       
    delay(1000); // delay a second. after 15 secs upload readings to ThingSpeak
  }  
  
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); // upload the fields to ThingSpeak server
  client.stop(); // stop the ThingSpeak client
 
  Serial.println("Waiting to upload next reading...");
  Serial.println();    
  
}



// function to get readings form the scale
void readScale(){
  
  Serial.println("Entering readScale function"); // for debugging
  
  if (scale.is_ready()) {
    long reading = scale.read(); // get a reading from the scale
    long finalReading = reading*0.0189-157017-zero; // Calibration of the result
    weight = finalReading; // save reading in the global variable
    Serial.print(finalReading);
    Serial.print(" g");
    Serial.println(); //change line
    
  } else {
    Serial.println("HX711 not found.");

  }

  Serial.println("Exiting readScale function"); // for debugging
}


// function to get readings from the Pressure sensors (ADS1115)
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
  
  pressure_down = (voltage_pres_down/5 - 0.04) / 0.009;
  pressure_up = (voltage_pres_up/5 - 0.04) / 0.009;
  
 
  Serial.print("Pressure Bottom: "); 
  Serial.print(pressure_down);
  Serial.println();
  
  Serial.print("Pressure Top: "); 
  Serial.print(pressure_up);
  Serial.println();;  
  
  Serial.println("Exiting readPressure function"); // for debugging
}


// function for posting to HTML page uncluding extracting the time from NTP
void handle_OnConnect() {

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  delay(1000);
  
  wserver.send(200, "text/html", SendHTML(weight,pressure_up,pressure_down,dayStamp,timeStamp)); // post HTML using the SendHTML function, passing on our readings and time from the global vars (in the parentheses)
}

// function for webserver, responds with an HTTP status 404 (Not Found)
void handle_NotFound(){
  wserver.send(404, "text/plain", "Not found");
}

// function for the context of the webpage - here you can personalize your HTML page
String SendHTML(float weight,float pressure_up,float pressure_down, String dayStamp, String timeStamp){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP32 HYPROP HUJI 2021</title>\n"; // the title of the webpage
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"5\" >\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>ESP32 HYPROP HUJI</h1>\n"; // the heading at top of the webapge

  // display date and time on webpage
  ptr +="<p>Date: "; 
  ptr +=dayStamp;
  ptr +="<p>Time: ";
  ptr +=timeStamp;

  // display weight reading on webpage
  ptr +="<p>Weight: ";
  ptr +=weight;
  ptr +=" g</p>";

  // display pressure readings on webpage
  ptr +="<p>Pressure Top: ";
  ptr +=pressure_up;
  ptr +="m</p>";
  ptr +="<p>Pressure Bottom: ";
  ptr +=pressure_down;
  ptr +="m</p>";

  // add here more lines to display on webpage, like the following template (includes 3 lines):
  // ptr +="<p>TITLE HERE: ";
  // ptr +=THE_VARIABLE;
  // ptr +="units or more text</p>";
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
