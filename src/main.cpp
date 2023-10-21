#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define AWS_IOT_SUBSCRIBE_TOPIC "hackathon2023/lightSub"
#define AWS_IOT_PUBLISH_TOPIC "hackathon2023/lightPub"
#define AWS_IOT_ENDPOINT "a20vk5db81a0va-ats.iot.eu-west-1.amazonaws.com"

#define THINGNAME "hackathon2023-esp32-light-intensity"

#define DAY 1
#define NIGHT 0

int DAY_NIGHT_LIGHT_VALUE_THRESHOLD = 3000;
int sensorPin = 35;
int value = 0; 

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  DAY_NIGHT_LIGHT_VALUE_THRESHOLD = doc["threshold"];
  
  Serial.println("MessageHandler");
  Serial.println(DAY_NIGHT_LIGHT_VALUE_THRESHOLD);
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
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage(int val)
{
  StaticJsonDocument<200> doc;
  doc["light"] = val;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  connectAWS();
}

void loop() {

  delay(1000);
  value = analogRead(sensorPin);
  Serial.println(value);
  publishMessage(value < DAY_NIGHT_LIGHT_VALUE_THRESHOLD ? DAY : NIGHT);
  client.loop();
}
