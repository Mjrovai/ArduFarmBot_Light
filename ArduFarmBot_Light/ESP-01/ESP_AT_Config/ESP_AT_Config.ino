/****************************************************************
* ESP8266
* 
* Arduino UNO or NANO Using SoftwareSerial library
* AT commands examples for test:
*    AT     =====> ESP8266 returns OK
*    AT+RST =====> ESP8266 restart and returns OK
*    AT+GMR =====> ESP8266 returns AT Version; SDK version; id; OK
*    AT+CWMODE? => ESP8266 returns mode type
*    AT+CWLAP ===> ESP8266 returs close access points
*    AT+CIFSR ===> ESP8266 returs designided IP
*    AT+CIPMUX=1          ==>Set ESP8266 for multoples conections 
*    AT+CIOBAUD=9600      ==>Change Baudrate (temporariamente) ==> ESP8266 returs OK
*    AT+UART_DEF=9600,8,1,0,0 ==> Change Baudrate definitivamente
*    AT+CIPSERVER=1,80    ==> set modo SERVER port: 80
*    AT+CWMODE=3          ==> Conect ESP8266 ascombined mode (Access Point (2) and Server (1))
*    AT+CWSAP="Acc_Point_name","password",wifi_Channel,cript# ==> ej. AT+CWSAP="ESP_8266_AP,"1234",3,0
*    AT+CWJAP="SSID","password" ==> Connect to WiFi network
*
* Marcelo Jose Rovai 18_Oct_17
******************************************************************/

#include <SoftwareSerial.h>   
SoftwareSerial esp8266(6,7);  //Rx ==> Pin 6; TX ==> Pin7 

#define speed8266 9600 // <==  ********* This is the speed that worked with my ESP8266
#define DEBUG true

void setup() 
{
  esp8266.begin (speed8266); 
  Serial.begin(speed8266);
  Serial.println("ESP8266 Setup test - use AT coomands");
}

void loop() 
{
  while(esp8266.available())
  {
    Serial.write(esp8266.read());
  }
  while(Serial.available())
  {
    esp8266.write(Serial.read());
  }
}
