#include <ESP8266WiFi.h>
#include <espnow.h>

typedef struct struct_message
{
  float t; //temperature
  float h; //humidity
} struct_message;

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