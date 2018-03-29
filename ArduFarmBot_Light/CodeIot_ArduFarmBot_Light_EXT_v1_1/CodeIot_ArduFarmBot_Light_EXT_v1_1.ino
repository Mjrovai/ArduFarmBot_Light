/***************************************************** 
* ArduFarmBot Light - Remote controling a plantation
* 
* ThingSpeak ID Channels: 
*   Status (Actuators and Sensors): 999999
*   Actuator1: 999999 (Pump)
*   Actuator2: 999999 (Lamp)
* 
* Sensors:
*   DHT (2-wire Air Temperature/Humidity digital sensor)  ==> Pin D11
*   DS18B20 (1-Wire Temperature digital Sensor)           ==> Pin D05
*   LDR (Light Dependent Resistor - Analog Sensor)        ==> Pin A1
*   LM394/YL70 (Soil Humidity Analog Sensor)  
* 
* Actuators:
*   Actuator1         ==> Pin 10 (RED LED   ==> pump)
*   Actuator2         ==> Pin 12 (GREEN LED ==> lamp)
* 
* FREEZE_LED:       ==> Pin 13 (ESP-01 Freezing and Comm errors)
* HW RESET          ==> Pin 08
*     
* Version 1.1: Connection to WiFi using local credentials
*   Enter with your network credentials: "YOUR USERNAME\" and "YOUR PASSWORD\" directly 
*   on function connectWiFi()
* 
* MJRoBot.org 29_March_18 Version 1.1 
*****************************************************/

// Thingspeak  
String statusChWriteKey = "YOUR WRITE KEY HERE";  // Status Channel id: 385184

String canalID1 = "999999"; // Enter your Actuator1 Channel ID here
String canalID2 = "999999"; // Enter your Actuator1 Channel ID here

#include <SoftwareSerial.h>
SoftwareSerial EspSerial(6, 7); // Rx,  Tx

// HW Pins
#define FREEZE_LED 13
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
long readTimingSeconds = 10; // ==> Define Sample time in seconds to receive data
long startReadTiming = 0;
long elapsedReadTime = 0;
long startWriteTiming = 0;
long elapsedWriteTime = 0;

//Relays
#define ACTUATOR1 10 // RED LED   ==> Pump
#define ACTUATOR2 12 // GREEN LED ==> Lamp
boolean pump = 0; 
boolean lamp = 0; 

int spare = 0;
boolean error;

void setup()
{
  Serial.begin(9600);
  
  pinMode(ACTUATOR1,OUTPUT);
  pinMode(ACTUATOR2,OUTPUT);
  pinMode(FREEZE_LED,OUTPUT);
  pinMode(HARDWARE_RESET,OUTPUT);

  digitalWrite(ACTUATOR1, HIGH); //o módulo relé é ativo em LOW
  digitalWrite(ACTUATOR2, HIGH); //o módulo relé é ativo em LOW
  digitalWrite(FREEZE_LED, LOW);
  digitalWrite(HARDWARE_RESET, HIGH);
  
  DS18B20.begin();
  dht.begin();

  EspSerial.begin(9600); // Comunicacao com Modulo WiFi
  EspHardwareReset(); //Reset do Modulo WiFi
  startReadTiming = millis(); // starting the "program clock"
  startWriteTiming = millis(); // starting the "program clock"

  connectWiFi();
}

void loop()
{
  start: //label 
  error=0;
  
  
  elapsedWriteTime = millis()-startWriteTiming; 
  elapsedReadTime = millis()-startReadTiming; 

  if (elapsedReadTime > (readTimingSeconds*1000)) 
  {
    ESPcheck();//executar antes de qualquer leitura ou gravação
    int command = readThingSpeak(canalID1); 
    if (command != 9) pump = command; 
    delay (5000);
    command = readThingSpeak(canalID2); 
    if (command != 9) lamp = command; 

    takeActions();
    startReadTiming = millis();   
  }
  
  if (elapsedWriteTime > (writeTimingSeconds*1000)) 
  {
    ESPcheck();//executar antes de qualquer leitura ou gravação
    readSensors();
    writeThingSpeak();
    startWriteTiming = millis();   
  }
  
  if (error==1) //Resend if transmission is not completed 
  {       
    Serial.println(" <<<< ERROR >>>>");
    digitalWrite(FREEZE_LED, HIGH);
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

/********* Take actions based on ThingSpeak Commands *************/
void takeActions(void)
{
  Serial.print("Pump: ");
  Serial.println(pump);
  Serial.print("Lamp: ");
  Serial.println(lamp);
  if (pump == 1) digitalWrite(ACTUATOR1, LOW);
  else digitalWrite(ACTUATOR1, HIGH);
  if (lamp == 1) digitalWrite(ACTUATOR2, LOW);
  else digitalWrite(ACTUATOR2, HIGH);
}

/********* Read Actuators command from ThingSpeak *************/
int readThingSpeak(String channelID)
{
  startThingSpeakCmd();
  int command;
  // preparacao da string GET
  String getStr = "GET /channels/";
  getStr += channelID;
  getStr +="/fields/1/last";
  getStr += "\r\n";

  String messageDown = sendThingSpeakGetCmd(getStr);
  if (messageDown[5] == 49)
  {
    command = messageDown[7]-48; 
    Serial.print("Command received: ");
    Serial.println(command);
  }
  else command = 9;
  return command;
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

/********* Echo Command *************/
boolean echoFind(String keyword)
{
 byte current_char = 0;
 byte keyword_length = keyword.length();
 long deadline = millis() + 5000; // Tempo de espera 5000ms
 while(millis() < deadline){
  if (EspSerial.available()){
    char ch = EspSerial.read();
    Serial.write(ch);
    if (ch == keyword[current_char])
      if (++current_char == keyword_length){
       Serial.println();
       return true;
    }
   }
  }
 return false; // Tempo de espera esgotado
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

/********* Check ESP *************/
boolean ESPcheck(void)
{
  EspSerial.println("AT"); // Send "AT+" command to module
   
  if (echoFind("OK")) 
  {
    //Serial.println("ESP ok");
    digitalWrite(FREEZE_LED, LOW);
    return true; 
  }
  else //Freeze ou Busy
  {
    Serial.println("ESP Freeze ******************************************************");
    digitalWrite(FREEZE_LED, HIGH);
    EspHardwareReset();
    return false;  
  }
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

/***************************************************
* Connect WiFi
****************************************************/
void connectWiFi(void)
{
  sendData("AT+RST\r\n", 2000, 0); // reset
  sendData("AT+CWJAP=\"YOUR USERNAME\",\"YOUR PASSWORD\"\r\n", 2000, 0); //Connect network
  delay(3000);
  sendData("AT+CWMODE=1\r\n", 1000, 0);
  sendData("AT+CIFSR\r\n", 1000, 0); // Show IP Adress
  Serial.println("8266 Connected");
}

/***************************************************
* Send AT commands to module
****************************************************/

String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  EspSerial.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (EspSerial.available())
    {
      // The esp has data so display its output to the serial window
      char c = EspSerial.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}

