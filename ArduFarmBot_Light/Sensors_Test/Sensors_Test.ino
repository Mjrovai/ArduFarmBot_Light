/***************************************************** 
*ArduFarmBot Light
*Sensors Test 
*
* DHT (2-wire Air Temperature/Humidity digital sensor)  ==> Pin D11
* DS18B20 (1-Wire Temperature digital Sensor)           ==> Pin D05
* LDR (Light Dependent Resistor - Analog Sensor)        ==> Pin A1
* LM394/YL70 (Soil Humidity Analog Sensor)              ==> Pin A0
*     
* MJRoBot.org 16_Dec_17
*****************************************************/

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

void setup()
{
  Serial.begin(9600); 
  DS18B20.begin();
  dht.begin();
}

void loop()
{
  readSensors();
  displaySensors();
  delay (10000);
}

/********* Read Sensors value *************/
void readSensors(void)
{
  airTemp = dht.readTemperature();
  airHum = dht.readHumidity();

  DS18B20.requestTemperatures(); 
  soilTemp = DS18B20.getTempCByIndex(0); // Sensor 0 will capture Soil Temp in Celcius
  
  soilHum = map(analogRead(soilHumPIN), 1023, 0, 0, 100);             
 
  light = map(analogRead(ldrPIN), 1023, 0, 0, 100); //LDRDark:0  ==> light 100%  

}

/********* Display Sensors value *************/
void displaySensors(void)
{
  Serial.print ("airTemp  (oC): ");
  Serial.println (airTemp);
  Serial.print ("airHum    (%): ");
  Serial.println (airHum);
  Serial.print ("soilTemp (oC): ");
  Serial.println (soilTemp);
  Serial.print ("soilHum   (%): ");
  Serial.println (soilHum);
  Serial.print ("light     (%): ");
  Serial.println (light);
  Serial.println ("");
}


