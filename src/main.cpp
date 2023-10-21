#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

int sensorPin = 11;
int value = 0; 

const char* awsEndpoint = "a20vk5db81a0va-ats.iot.us-east-1.amazonaws.com";
const char* awsTopic = "lightValue";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void establishWifiConnection() {
  const char* ssid = "ENO"; 
  const char* password = "02826ENO@innolabs!";

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
      Serial.println("Status:" + WiFi.status());
  }
  Serial.println("Connected to the WiFi network");
}

void setup() {
  Serial.begin(9600);
  
  establishWifiConnection();

  pinMode(sensorPin, INPUT);
}

void loop() {
  delay(2000);
  value = analogRead(sensorPin);
  float voltage = value * (5.0/1025) * 1000;
  float resistance = 10000 * ( voltage / ( 5000.0 - voltage) );
  int lux = (500.0 / resistance)*1000;
  Serial.println(lux);
}
