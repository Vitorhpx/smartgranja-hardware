/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_ENABLED 0
#define DEEP_SLEEP_ENABLED 1

#define OLED_SDA 5
#define OLED_SCL 4
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//WAKE UP parameters
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 30       // Time in seconds

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// REPLACE WITH YOUR RECEIVER MAC Address
//5C:CF:7F:F0:8A:6E
uint8_t broadcastAddress[] = {0x5C, 0xCF, 0x7F, 0xF0, 0x8A, 0X6E};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message
{
  float t; //temperature=
  float h; //humidity
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void printMessage(const esp_err_t result, const struct_message data)
{
  display.clearDisplay();
  if (result == ESP_OK)
  {
    display.print("Sent with success \n");
    display.print("\n");
    display.print(myData.t);
    display.print("\n");
    display.print(myData.h);
  }
  else
  {
    display.print("Error sending the data");
  }
  display.setCursor(0, 0);
  display.display();
}

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  //start display
  if (OLED_ENABLED)
  {
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);
    // Reset display

    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
    { // Address 0x3C for 128x32
      Serial.println(F("SSD1306 allocation failed"));
      for (;;)
        ; // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
  }

  

  // Set ESP32 to wake up every 5 seconds
  if (DEEP_SLEEP_ENABLED)
  {
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " seconds");
  }
}

void loop()
{
  delay(1000);
  
  if (DEEP_SLEEP_ENABLED) {
    Serial.println("I woke up now");
  }

  // Set values to send
  myData.t = random(1, 500) / 100.0;
  myData.h = random(1, 500) / 100.0;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (OLED_ENABLED)
  {
    printMessage(result, myData);
  }
  delay(2000);


  if (DEEP_SLEEP_ENABLED) {
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();
  }
}
