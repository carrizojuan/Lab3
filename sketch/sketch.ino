 // La función setup corre una vez, al prenderse el Arduino
// o bien al reiniciar mediante el botón reset
  //anodo comun 
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

#define SLAVE_ADDR 9
#define ANSWERSIZE 5


//definimos NTPClient para poder consultar el tiempo.
const long utcOffset = -10800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"pool.ntp.org",utcOffset);

//definimos wifiClient para conectarnos a una red
WiFiClient wifiClient;
const char* ssid = "FiberCorp WiFi887 2.4GHz";
const char* password = "0041285223";

const int OFF = HIGH;
int var = 0;
int cajas = 0;
int alfajores_total = 0;
int alfajores_caja = 0;
int sensorValue;
int relayStatus = 0;
char json[21];
String start_caja;
String Serializ;




void setup()
{   
  Wire.begin(SLAVE_ADDR);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");  
  }
  Serial.println("Wifi Connected");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
  pinMode(5,OUTPUT);   //Relay
  timeClient.begin();
}


void loop(){
  delay(50);
  sensorValue = analogRead(A0); 
  if(sensorValue < 30){
    if (alfajores_caja == 0){
      cajas ++;
      timeClient.update();
      httpPOST("start",timeClient.getFormattedTime());
    }
    delay(50);
    alfajores_caja ++; // lleva el conteo de alfajores en la caja actual
    alfajores_total ++; // lleva el conteo de alfajores totales
    Serial.println("Leyo");
 }
 if(alfajores_caja == 20){
    digitalWrite(5, HIGH);
    delay(5000);
    digitalWrite(5, LOW);
    timeClient.update();
    httpPOST("finish",timeClient.getFormattedTime());
    relayStatus = 1;
    alfajores_caja = 0;  
  }  
}

void httpPOST(String timer, String date){
  int contRepeat = 0;
  HTTPClient http;
  char *url = "http://192.168.0.7:4200/post";
  http.begin(wifiClient,url);
  http.addHeader("Content-Type","application/json");
  DynamicJsonDocument postMessage(2048);
  postMessage["message"] = timer;
  postMessage["time"] = date;
  String jsonBody;
  serializeJson(postMessage,jsonBody);
  Serial.println(jsonBody);
  int resCode = http.POST(jsonBody);
  Serial.println(resCode);
  while(resCode != 201 && contRepeat < 3){
    int resCode = http.POST(jsonBody);
    Serial.println(resCode);
    contRepeat++;
  } 
  String res = http.getString();
  parserMessage(res);
  http.end();
}
void httpGET(){
  HTTPClient http;
  char *url = "http://192.168.0.7:4200/";
  http.begin(wifiClient,url);
  int resCode = http.GET();
  Serial.println(resCode);
  String res = http.getString();
  Serial.println(res);
  parserMessage(res);
  http.end();
}
void parserMessage(String res){
  DynamicJsonDocument doc(2048);
  deserializeJson(doc,res);
  const char* message = doc["message"];
  Serial.println(message);
}



void requestEvent() {
 Serial.println("I2C Request received");
 StaticJsonDocument<300> doc;
 doc["controller_name"] = "NodeMCU-ESP32";
 timeClient.update();
 doc["date"] = timeClient.getFormattedTime();

 //array actuador
 JsonArray arrActuator = doc.createNestedArray("actuators");
 StaticJsonDocument<52> act;
 JsonObject actuator = act.to<JsonObject>();
 actuator["type"]= "relay";
 actuator["current_value"] = relayStatus;
 arrActuator.add(actuator);

 //array sensor
 JsonArray arrSensor = doc.createNestedArray("sensor");
 StaticJsonDocument<52> sen;
 JsonObject sensor = sen.to<JsonObject>();
 sensor["type"] = "hall";
 sensor["current_value"] = sensorValue;
 arrSensor.add(sensor);

 serializeJsonPretty(doc,json);
 Serial.print("Json:");
 Serial.print(json);
 
}
