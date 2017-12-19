/***************************************************** 
* ArduFarmBot Light
* Reading command from ThingSpeak.com
* ID Chnnel: 
*   Actuator1: 999999 (Pump)
*   Actuator2: 999999 (Lamp)
* 
* ESP-01 HW RESET   ==> Pin D08
*     
* MJRoBot.org 18_Dec_17
*****************************************************/

// Thingspeak  
String canalID1 = "999999"; //Enter your Actuator1 Channel ID here
String canalID2 = "999999"; //Enter your Actuator2 Channel ID here

#include <SoftwareSerial.h>
SoftwareSerial EspSerial(6, 7); // Rx,  Tx
#define HARDWARE_RESET 8

// Variables to be used with timers
long readTimingSeconds = 10; // ==> Define Sample time in seconds to receive data
long startReadTiming = 0;
long elapsedReadTime = 0;

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
  pinMode(HARDWARE_RESET,OUTPUT);

  digitalWrite(ACTUATOR1, HIGH); //o módulo relé é ativo em LOW
  digitalWrite(ACTUATOR2, HIGH); //o módulo relé é ativo em LOW
  digitalWrite(HARDWARE_RESET, HIGH);

  EspSerial.begin(9600); // Comunicacao com Modulo WiFi
  EspHardwareReset(); //Reset do Modulo WiFi
  startReadTiming = millis(); // starting the "program clock"
}

void loop()
{
  start: //label 
  error=0;
  
  elapsedReadTime = millis()-startReadTiming; 

  if (elapsedReadTime > (readTimingSeconds*1000)) 
  {
    int command = readThingSpeak(canalID1); 
    if (command != 9) pump = command; 
    delay (5000);
    command = readThingSpeak(canalID2); 
    if (command != 9) lamp = command; 
    takeActions();
    startReadTiming = millis();   
  }
  
  if (error==1) //Resend if transmission is not completed 
  {       
    Serial.println(" <<<< ERROR >>>>");
    delay (2000);  
    goto start; //go to label "start"
  }
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

