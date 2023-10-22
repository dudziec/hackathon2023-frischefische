#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//pins

const int PIR_SENSOR_OUTPUT_PIN = 35;
int warm_up;
int currentState;
int previousState;

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC "hackathon2023/motion"
#define AWS_IOT_SUBSCRIBE_TOPIC "hackathon2023/threshold"
WiFiClientSecure net = WiFiClientSecure();
//MQTTClient client = MQTTClient(256);
PubSubClient client(net);

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
  //client.setCallback(messageHandler);
  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }
  //client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void publishMessage(float message, float value)
{
  StaticJsonDocument<200> doc;
  doc["motion"] = message;
  doc["value"] = value;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void sendState(){
  if (currentState != previousState){
      publishMessage(currentState, currentState);
      previousState = currentState;
    }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PIR_SENSOR_OUTPUT_PIN, INPUT); // Sets the echoPin as an Input
  connectAWS();
}

void loop() {
  int sensor_output;
  client.loop();
    // Clears the trigPin
  currentState = digitalRead(PIR_SENSOR_OUTPUT_PIN);
  Serial.print("Sensor output: ");
  Serial.println(currentState);
  if( currentState == LOW )
  {
    if( warm_up == 1 )
     {
      Serial.print("Warming Up\n\n");
      warm_up = 0;
      delay(2000);
    }
    Serial.print("No object in sight\n\n");
    sendState();
    delay(1000);
  }
  else
  {
    Serial.print("Object detected\n\n");   
    sendState();
    warm_up = 1;
    delay(1000);
  } 
}
