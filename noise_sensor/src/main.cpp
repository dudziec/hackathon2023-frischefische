#include <arduino.h>  

#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

# define AWS_IOT_SUBSCRIBE_TOPIC "hackathon2023/threshold"
# define AWS_IOT_PUBLISH_TOPIC "hackathon2023/noisePub"

int Analog; 
int Max;                          
int LoopTime = 500;                 
int AvgTime = LoopTime * 10; 
int AvgCount = AvgTime / 1000; 
int n;
int k = 0;                                
int MaxSum = 0;
int MaxAvg = 0;
int publishThreshold = 2300;

int SOUND_PIN = 34;

unsigned long ReadStartTime;             

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  publishThreshold = doc["noise"];
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
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
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

void publishMessage(int h, int j)
{
  StaticJsonDocument<200> doc;
  doc["noise"] = h;
  doc["noiseVal"] = j;
  doc["noiseThreshold"] = publishThreshold;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.print(h);
  Serial.println(" sent!");
}

void readMax(){
  Max = 0;
  n = 0;                                  
 
  ReadStartTime = millis();                

  while (millis() - ReadStartTime < LoopTime)            
  {
    Analog = abs(analogRead(SOUND_PIN));
    if (Analog > Max)                                    
      Max = Analog;
  }
  Serial.print (" Max = ");   
  Serial.println (Max);

  MaxSum += Max;
  k++;
  if(k >= AvgCount){
    Serial.print("Avg.: ");
    MaxAvg = MaxSum / k;
    Serial.println(MaxAvg);
    if(MaxAvg >= publishThreshold)
      publishMessage(1, MaxAvg);
    else 
      publishMessage(0, MaxAvg);
    k = 0;
    MaxSum = 0;
  }
}

void setup()
{   
  Serial.begin(9600);           
  connectAWS();
}

void loop()
{
  if(!client.connect(THINGNAME))
    connectAWS();
  client.loop();
  readMax();
}