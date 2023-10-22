#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//pins
const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
const float SOUND_SPEED = 0.034;
const float CM_TO_INCH = 0.393701;

long duration;
float distanceCm;
int currentSensorState;
int previousSensorState;
float distanceTreshold; 

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC "hackathon2023/distance"
#define AWS_IOT_SUBSCRIBE_TOPIC "hackathon2023/threshold"
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  distanceTreshold = doc["distance"];
  Serial.println(distanceTreshold);

}
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Connecting to Wi-Fi");

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
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void publishMessage(float message, float value)
{
  StaticJsonDocument<200> doc;
  doc["distance"] = message;
  doc["value"] = value;
  doc["treshold"] = distanceTreshold;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

float readDistance(){
   digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  return distanceCm;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  distanceTreshold = 100;
  connectAWS();
}

void loop() {
  client.loop();
  
  // Calculate the distance
  distanceCm = readDistance();

  if (distanceCm < distanceTreshold){
    currentSensorState = 1;
  } else {
    currentSensorState = 0;
  }
  if (currentSensorState != previousSensorState){
    publishMessage(currentSensorState, distanceCm);
    previousSensorState = currentSensorState;
  }
  delay(500);
}
