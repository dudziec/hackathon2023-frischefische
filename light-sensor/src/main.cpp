#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SPI.h>  
#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define AWS_IOT_SUBSCRIBE_TOPIC "hackathon2023/lightSub"
#define AWS_IOT_PUBLISH_TOPIC "hackathon2023/lightPub"
#define AWS_IOT_ENDPOINT "a20vk5db81a0va-ats.iot.eu-west-1.amazonaws.com"

#define THINGNAME "hackathon2023-esp32-light-intensity"

#define DAY 1
#define NIGHT 0

int dayNightThreshold = 100;
int sensorPin = 35;
int value = 0; 

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_101MS, TCS34725_GAIN_4X);

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  dayNightThreshold = doc["light"];
}

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");

 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage(int isDay, int lux)
{
  StaticJsonDocument<200> doc;
  doc["light"] = isDay;
  doc["value"] = lux;
  doc["threshold"] = dayNightThreshold;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void setup() {
  Serial.begin(9600);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  pinMode(sensorPin, INPUT);
  connectAWS();
}

void loop() {
  delay(1000);
  uint16_t r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);

  lux = tcs.calculateLux(r, g, b);
  Serial.println("Lux");
  Serial.println(lux);
  
  publishMessage(lux > dayNightThreshold ? DAY : NIGHT,  lux);
  client.loop();
}
