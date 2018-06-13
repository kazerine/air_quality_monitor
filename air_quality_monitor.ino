
/* Arduino + Esp8266 + ThingSpeak + MQ135 + 16x2 LCD 
 * Project name = "Air Quality Monitor Using Arduino Uno, MQ135 and ESP8266"
 * About = "takes voltage reading from mq135 converts it to co2(in ppm), displays it to LCD 
 * and also uploads data to thingspeak channel using ESP8266 wifi module
 * github.com/kazerine
 */

// Code to use SoftwareSerial
#include <SoftwareSerial.h>
SoftwareSerial espSerial =  SoftwareSerial(2,3);      // arduino RX pin=2  arduino TX pin=3 w/ esp8266
#include "MQ135.h"                                    // header file for mq135 gas sensor
#include <LiquidCrystal.h>                            // header file for lcd
int Contrast=500;
LiquidCrystal lcd(12,11, 7, 6, 5, 4);                 // define pins of lcd connected to arduino

const int sensorPin = A0;                              // sensor connected to analog0 pin of arduino
int air_quality;

String apiKey = "16-char-write-key";                    // channel's thingspeak WRITE API key

String ssid="yournetworksssid";         // Wifi network SSID
String password ="yournetworkpassword";   // Wifi network password
boolean DEBUG=true;               // Enable debugging through serial monitor

//======================================================================== loop to show response of debug on serial monitor
void showResponse(int waitTime){
    long t=millis();
    char c;
    while (t+waitTime>millis()){
      if (espSerial.available()){
        c=espSerial.read();
        if (DEBUG) Serial.print(c);
      }
    }
                   
}

//======================================================================== loop to write data on Thingspeak channel
boolean thingSpeakWrite(float value1){
  String cmd = "AT+CIPSTART=\"TCP\",\"";                  // TCP connection
  cmd += "184.106.153.149";                               // api.thingspeak.com
  cmd += "\",80";                                         // port 80
  espSerial.println(cmd);
  if (DEBUG) Serial.println(cmd);
  if(espSerial.find("Error")){
    if (DEBUG) Serial.println("AT+CIPSTART error");
    return false;
  }
  
  
  String getStr = "GET /update?api_key=";   // prepare GET string
  getStr += apiKey;
  
  getStr +="&field1=";
  getStr += String(value1);
  //getStr +="&field2=";
  //getStr += String(value2);
  // getStr +="&field3=";
  // getStr += String(value3);
  // ...
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  espSerial.println(cmd);
  if (DEBUG)  Serial.println(cmd);
  
  delay(100);
  if(espSerial.find(">")){
    espSerial.print(getStr);
    if (DEBUG)  Serial.print(getStr);
  }
  else{
    espSerial.println("AT+CIPCLOSE");
    // alert user
    if (DEBUG)   Serial.println("AT+CIPCLOSE");
    return false;
  }
  return true;
}
//================================================================================ setup
void setup() {                
  
  DEBUG=true;             // enable debug serial
  Serial.begin(9600); 
  analogWrite(9,24836);
  analogWrite(10,Contrast);
  espSerial.begin(9600);  // enable software serial
                          // if esp8266 module's speed is not at 9600 change AT firmware to 9600. 
                          
  //enable next 4 lines to forcefully change baudrate to 9600bps (if not already).
  espSerial.println("AT+UART_CUR=9600,8,1,0,0");    // Enable this line to set esp8266 serial speed to 9600 bps (for current)
  showResponse(1000);
  
  espSerial.println("AT+UART_DEF=9600,8,1,0,0");    // Enable this line to set esp8266 serial speed to 9600 bps (for default)
  showResponse(1000);

  espSerial.println("AT+CWMODE=1");   // set esp8266 as client
  showResponse(1000);

  espSerial.println("AT+CWJAP=\""+ssid+"\",\""+password+"\"");  // set your home router SSID and password
  showResponse(5000);
  lcd.begin(16,2);
   if (DEBUG)  Serial.println("Setup completed");
}


// ====================================================================== loop
void loop() {

   // Read sensor values from analog pin A0
   MQ135 gasSensor = MQ135(A0);
   float rzeroCO = gasSensor.getRZeroCO();
   Serial.println("rzero co =" +String(rzeroCO)+ " //co resistance");
   
   float rzeroCO2 = gasSensor.getRZeroCO2();
   Serial.println("rzero co2 =" +String(rzeroCO2)+ " //co2 resistance");
   
   float co2_1 = gasSensor.getCO2PPM();
   float co_1 = gasSensor.getCOPPM();
   delay(1000);
   float co2_2 = gasSensor.getCO2PPM();
   float co_2 = gasSensor.getCOPPM();
   delay(1000);
   float co2_3 = gasSensor.getCO2PPM();
   float co_3 = gasSensor.getCOPPM();
   delay(1000);
   float co2_4 = gasSensor.getCO2PPM();
   float co_4 = gasSensor.getCOPPM();
   delay(1000);
   float co2_5 = gasSensor.getCO2PPM();
   float co_5 = gasSensor.getCOPPM();
   delay(1000);
   float air_quality_co2 = ( co2_1 + co2_2 + co2_3 + co2_4 + co2_5 ) / 5;
   lcd.setCursor (0, 0);
   lcd.print ("CO2: ");
   lcd.print (air_quality_co2);
   lcd.print (" PPM ");
   float air_quality_co = ( co_1 + co_2 + co_3 + co_4 + co_5 ) / 5;
   lcd.setCursor (0, 1);
   lcd.print (" CO: ");
   lcd.print (air_quality_co);
   lcd.print (" PPM ");
   thingSpeakWrite(air_quality);                                      // call write function to write values to thingspeak
   delay(3000);  
}
