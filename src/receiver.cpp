#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Arduino.h>
#include <Stream.h>
#include <espnow.h>
//AWS
#include "sha256.h"
#include "Utils.h"
//WEBSockets
#include <Hash.h>
#include <WebSocketsClient.h>
//MQTT PUBSUBCLIENT LIB
#include <PubSubClient.h>
//AWS MQTT Websocket
#include "Client.h"
#include "AWSWebSocketClient.h"
#include "CircularByteBuffer.h"

extern "C"
{
#include "user_interface.h"
}

//AWS IOT config, change these:
char wifi_ssid[] = "CovilBaleia";
char wifi_password[] = "";
char aws_endpoint[] = "a3f3ubc1y6ermt-ats.iot.us-east-2.amazonaws.com";
char aws_key[] = "";
char aws_secret[] = "";
char aws_region[] = "us-east-2";
const char *aws_topic = "esp8266/1/sample";
int port = 443;

//MQTT config
const int maxMQTTpackageSize = 512;
const int maxMQTTMessageHandlers = 1;

ESP8266WiFiMulti WiFiMulti;
AWSWebSocketClient awsWSclient(1000);
PubSubClient client(awsWSclient);

typedef struct struct_message
{
  float t; //temperature
  float h; //humidity
} struct_message;

//# of connections
long connection = 0;

//generate random mqtt clientID
char *generateClientID()
{
  char *cID = new char[23]();
  for (int i = 0; i < 22; i += 1)
    cID[i] = (char)random(1, 256);
  return cID;
}

//count messages arrived
int arrivedcount = 0;

//callback to handle mqtt messages
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//connects to websocket layer and mqtt layer
bool connect()
{
  if (client.connected())
  {
    client.disconnect();
  }

  //delay is not necessary... it just help us to get a "trustful" heap space value
  delay(1000);
  Serial.print(millis());
  Serial.print(" - conn: ");
  Serial.print(++connection);
  Serial.print(" - (");
  Serial.print(ESP.getFreeHeap());
  Serial.println(")");

  //creating random client id
  char *clientID = generateClientID();

  client.setServer(aws_endpoint, port);
  if (client.connect(clientID))
  {
    Serial.println("connected");
    return true;
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    return false;
  }
}

//subscribe to a mqtt topic
void subscribe()
{
  client.setCallback(callback);
  client.subscribe(aws_topic);
  //subscript to a topic
  Serial.println("MQTT subscribed");
}

//send a message to a mqtt topic
void sendmessage()
{
  //send a message
  char buf[100];
  strcpy(buf, "{\"state\":{\"reported\":{\"on\": false}, \"desired\":{\"on\": false}}}");
  int rc = client.publish(aws_topic, buf);
}

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.println();
  Serial.print("Temperatura: ");
  Serial.println(myData.t);
  Serial.println();
  Serial.print("Umidade: ");
  Serial.println(myData.h);
  Serial.println();
}

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{
}

// #include <ESP8266WiFi.h>

// void setup(){
//   Serial.begin(115200);
//   Serial.println();
//   Serial.print("ESP Board MAC Address:  ");
//   Serial.println(WiFi.macAddress());
// }

// void loop(){

// }