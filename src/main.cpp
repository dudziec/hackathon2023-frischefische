#include "secrets.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define THING_NAME "esp32"
#define AWS_IOT_SUBSCRIBE_TOPIC "lightIntensitySub"
#define AWS_IOT_PUBLISH_TOPIC "lightIntensityPub"
#define AWS_IOT_ENDPOINT "a20vk5db81a0va-ats.iot.us-east-1.amazonaws.com" 

int sensorPin = 11;
int value = 0;  

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

boolean establishWifiConnection() {
  const char* ssid = "ENO"; 
  const char* password = "02826ENO@innolabs!";

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    
  int attempts = 0;
  while(WiFi.status() != WL_CONNECTED && attempts < 5) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
      Serial.println("Status:" + WiFi.status());
      attempts++;
  }

  return WiFi.status() == WL_CONNECTED;
}

void setCertificate() {
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
}

void connectToAWS() {
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  //client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THING_NAME))
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
void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);

  if(establishWifiConnection()) {
    Serial.println("Connected to the WiFi network");

    //setCertificate();
    //connectToAWS();
  }

}

void publishMessage(int value)
{
  StaticJsonDocument<200> doc;
  doc["value"] = value;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void loop() {
  delay(4000);
  value = analogRead(sensorPin);
  float voltage = value * (5.0/1025) * 1000;
  float resistance = 10000 * ( voltage / ( 5000.0 - voltage) );
  int lux = (500.0 / resistance)*1000;
  Serial.println(lux);
  //publishMessage(lux);
}
