/***************************************************** 
* ArduFarmBot Light
* Sending Status to ThingSpeak.com
* ID Channel: Status (Actuators and Sensors): 999999
* 
* DHT (2-wire Air Temperature/Humidity digital sensor)  ==> Pin D11
* DS18B20 (1-Wire Temperature digital Sensor)           ==> Pin D05
* LDR (Light Dependent Resistor - Analog Sensor)        ==> Pin A1
* LM394/YL70 (Soil Humidity Analog Sensor)              ==> Pin A0
* 
* FREEZE_LED:       ==> Pin D13
* ESP-01 HW RESET   ==> Pin D08
*     
* MJRoBot.org 18_Dec_17
*****************************************************/

// Thingspeak  
String statusChWriteKey = "ENTER YOUR WRITE KEY HERE";  // Status Channel id: 999999

#include <SoftwareSerial.h>
SoftwareSerial EspSerial(6, 7); // Rx,  Tx
#define HARDWARE_RESET 8

// DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 5 // DS18B20 on pin D5 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
int soilTemp = 0;

//DHT
#include "DHT.h"
#include <stdlib.h>
int pinoDHT = 11;
int tipoDHT =  DHT22;
DHT dht(pinoDHT, tipoDHT); 
int airTemp = 0;
int airHum = 0;

// LDR (Light)
#define ldrPIN 1
int light = 0;

// Soil humidity
#define soilHumPIN 0
int soilHum = 0;

// Variables to be used with timers
long writeTimingSeconds = 17; // ==> Define Sample time in seconds to send data
long startWriteTiming = 0;
long elapsedWriteTime = 0;

// Variables to be used with Actuators
boolean pump = 0; 
boolean lamp = 0; 

int spare = 0;
boolean error;

void setup()
{
  Serial.begin(9600);
  
  pinMode(HARDWARE_RESET,OUTPUT);
  
  digitalWrite(HARDWARE_RESET, HIGH);
  
  DS18B20.begin();
  dht.begin();

  EspSerial.begin(9600); // Comunicacao com Modulo WiFi
  EspHardwareReset(); //Reset do Modulo WiFi
  startWriteTiming = millis(); // starting the "program clock"
}

void loop()
{
  start: //label 
  error=0;
  
  elapsedWriteTime = millis()-startWriteTiming; 
  
  if (elapsedWriteTime > (writeTimingSeconds*1000)) 
  {
    readSensors();
    writeThingSpeak();
    startWriteTiming = millis();   
  }
  
  if (error==1) //Resend if transmission is not completed 
  {       
    Serial.println(" <<<< ERROR >>>>");
    delay (2000);  
    goto start; //go to label "start"
  }
}

/********* Read Sensors value *************/
void readSensors(void)
{
  airTemp = dht.readTemperature();
  airHum = dht.readHumidity();

  DS18B20.requestTemperatures(); 
  soilTemp = DS18B20.getTempCByIndex(0); // Sensor 0 will capture Soil Temp in Celcius
             
  light = map(analogRead(ldrPIN), 1023, 0, 0, 100); //LDRDark:0  ==> light 100%  
  soilHum = map(analogRead(soilHumPIN), 1023, 0, 0, 100); 

}

/********* Conexao com TCP com Thingspeak *******/
void writeThingSpeak(void)
{

  startThingSpeakCmd();

  // preparacao da string GET
  String getStr = "GET /update?api_key=";
  getStr += statusChWriteKey;
  getStr +="&field1=";
  getStr += String(pump);
  getStr +="&field2=";
  getStr += String(lamp);
  getStr +="&field3=";
  getStr += String(airTemp);
  getStr +="&field4=";
  getStr += String(airHum);
  getStr +="&field5=";
  getStr += String(soilTemp);
  getStr +="&field6=";
  getStr += String(soilHum);
  getStr +="&field7=";
  getStr += String(light);
  getStr +="&field8=";
  getStr += String(spare);
  getStr += "\r\n\r\n";

  sendThingSpeakGetCmd(getStr); 
}

/********* Reset ESP *************/
void EspHardwareReset(void)
{
  Serial.println("Reseting......."); 
  digitalWrite(HARDWARE_RESET, LOW); 
  delay(500);
  digitalWrite(HARDWARE_RESET, HIGH);
  delay(8000);//Tempo necessário para começar a ler 
  Serial.println("RESET"); 
}

/********* Start communication with ThingSpeak*************/
void startThingSpeakCmd(void)
{
  EspSerial.flush();//limpa o buffer antes de começar a gravar
  
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // Endereco IP de api.thingspeak.com
  cmd += "\",80";
  EspSerial.println(cmd);
  Serial.print("enviado ==> Start cmd: ");
  Serial.println(cmd);

  if(EspSerial.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}

/********* send a GET cmd to ThingSpeak *************/
String sendThingSpeakGetCmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  EspSerial.println(cmd);
  Serial.print("enviado ==> lenght cmd: ");
  Serial.println(cmd);

  if(EspSerial.find((char *)">"))
  {
    EspSerial.print(getStr);
    Serial.print("enviado ==> getStr: ");
    Serial.println(getStr);
    delay(500);//tempo para processar o GET, sem este delay apresenta busy no próximo comando

    String messageBody = "";
    while (EspSerial.available()) 
    {
      String line = EspSerial.readStringUntil('\n');
      if (line.length() == 1) 
      { //actual content starts after empty line (that has length 1)
        messageBody = EspSerial.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    EspSerial.println("AT+CIPCLOSE");     // alert user
    Serial.println("ESP8266 CIPSEND ERROR: RESENDING"); //Resend...
    spare = spare + 1;
    error=1;
    return "error";
  } 
}

