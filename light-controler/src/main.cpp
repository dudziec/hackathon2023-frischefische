#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <iostream>
#include <cstring>
//pins
const int ledPin = 1;


// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_SUBSCRIBE_TOPIC "hackathon2023/street_light"


WiFiClientSecure net = WiFiClientSecure();
//MQTTClient client = MQTTClient(256);
PubSubClient client(net);

void blink(int count){
  for (int i = 1; i <= count; ++i)  {
    //digitalWrite(ledPin, LOW);
    delay(500);
    //digitalWrite(ledPin, HIGH);
  }
}

void publishMessage(int message)
{
  StaticJsonDocument<200> doc;
  doc["light"] = message;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_SUBSCRIBE_TOPIC, jsonBuffer);
}

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  //Serial.print("incoming: ");
  //Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  //deserializeJson(doc, payload);
  int light;
  
  //std::memcpy(&light, payload, sizeof(int));
  light = char(int(payload[0]));
  //payload; 
 
  Serial.println(light);
  if (light == '1'){
    digitalWrite(ledPin, LOW);

  } else if (light == '0'){
    digitalWrite(ledPin, HIGH);
  }
}
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  //client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  // Create a message handler
  client.setCallback(messageHandler);
  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT); // Sets the trigPin as an Output
  digitalWrite(ledPin, HIGH);
  connectAWS();
  blink(10);
}

void loop() {
  client.loop();
  //Serial.println("loop");
  //delay(1000);
}
