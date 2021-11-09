/* ESP8266 AWS IoT
 *  
 * Simplest possible example (that I could come up with) of using an ESP8266 with AWS IoT.
 * No messing with openssl or spiffs just regular pubsub and certificates in string constants
 * 
 * This is working as at 7th Aug 2021 with the current ESP8266 Arduino core release 3.0.2
 * 
 * Author: Anthony Elder 
 * License: Apache License v2
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid = "CovilBaleia";
const char *password = "senha";

// Find this awsEndpoint in the AWS Console: Manage - Things, choose your thing
// choose Interact, its the HTTPS Rest endpoint
const char *awsEndpoint = "a3f3ubc1y6ermt-ats.iot.us-east-2.amazonaws.com";

// For the two certificate strings below paste in the text of your AWS
// device certificate and private key:

// xxxxxxxxxx-certificate.pem.crt
static const char certificatePemCrt[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
CERTS
-----END CERTIFICATE-----
)EOF";

// xxxxxxxxxx-private.pem.key
static const char privatePemKey[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
CERTS
-----END RSA PRIVATE KEY-----
)EOF";

// This is the AWS IoT CA Certificate from:
// https://docs.aws.amazon.com/iot/latest/developerguide/managing-device-certs.html#server-authentication
// This one in here is the 'RSA 2048 bit key: Amazon Root CA 1' which is valid
// until January 16, 2038 so unless it gets revoked you can leave this as is:
static const char caPemCrt[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
CERTS
-----END CERTIFICATE-----
)EOF";

BearSSL::X509List client_crt(certificatePemCrt);
BearSSL::PrivateKey client_key(privatePemKey);
BearSSL::X509List rootCert(caPemCrt);

WiFiClientSecure wiFiClient;
void msgReceived(char *topic, byte *payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient);

unsigned long lastPublish;
int msgCount;

void msgReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message received on ");
  Serial.print(topic);
  Serial.print(": ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void pubSubCheckConnect()
{
  if (!pubSubClient.connected())
  {
    Serial.print("PubSubClient connecting to: ");
    Serial.print(awsEndpoint);
    while (!pubSubClient.connected())
    {
      Serial.print(pubSubClient.state());
      WiFi.mode(WIFI_STA);
      pubSubClient.connect("hjafksdhfkjahsdflqilwehfjklrhfjdahldfshjk");
    }
    Serial.println(" connected");
    pubSubClient.subscribe("inTopic");
  }
  pubSubClient.loop();
}

void setCurrentTime()
{
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("ESP8266 AWS IoT Example");
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());

  // get current time, otherwise certificates are flagged as expired
  setCurrentTime();

  wiFiClient.setClientRSACert(&client_crt, &client_key);
  wiFiClient.setTrustAnchors(&rootCert);
}

void loop()
{

  pubSubCheckConnect();

  if (millis() - lastPublish > 10000)
  {
    String msg = String("Hello from ESP8266: ") + ++msgCount;
    pubSubClient.publish("esp8266/1/sample", msg.c_str());
    Serial.print("Published: ");
    Serial.println(msg);
    lastPublish = millis();
  }
}

// #include <Stream.h>
// #include <espnow.h>
// #include <FS.h>
// #include <ESP8266WiFi.h>
// #include <PubSubClient.h> //https://www.arduinolibraries.info/libraries/pub-sub-client
// #include <NTPClient.h>    //https://www.arduinolibraries.info/libraries/ntp-client
// #include <WiFiUdp.h>

// // Update these with values suitable for your network.
// const char *ssid = "CovilBaleia";
// const char *password = "13371337";

// WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, "pool.ntp.org");

// const char *AWS_endpoint = "a3f3ubc1y6ermt-ats.iot.us-east-2.amazonaws.com"; //MQTT broker ip

// void callback(char *topic, byte *payload, unsigned int length)
// {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.print("] ");
//   for (int i = 0; i < length; i++)
//   {
//     Serial.print((char)payload[i]);
//   }
//   Serial.println();
// }

// WiFiClientSecure espClient;
// PubSubClient client(AWS_endpoint, 8883, callback, espClient); //set MQTT port number to 8883 as per //standard

// //============================================================================
// #define BUFFER_LEN 256
// long lastMsg = 0;
// char msg[BUFFER_LEN];
// int value = 0;
// byte mac[6];
// char mac_Id[18];
// //============================================================================

// void setup_wifi()
// {
//   delay(10);
//   // We start by connecting to a WiFi network
//   espClient.setBufferSizes(512, 512);
//   Serial.println();
//   Serial.print("Connecting to ");
//   Serial.println(ssid);
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());
//   timeClient.begin();
//   while (!timeClient.update())
//   {
//     timeClient.forceUpdate();
//   }
//   espClient.setX509Time(timeClient.getEpochTime());
// }
// void reconnect()
// {
//   // Loop until we're reconnected
//   while (!client.connected())
//   {
//     Serial.print("Attempting MQTT connection...");
//     // Attempt to connect
//     if (client.connect("920995cb-cf02-4de6-a6be-2a7f3a25801e"))
//     {
//       Serial.println("connected");
//       // Once connected, publish an announcement...
//       client.publish("outTopic", "hello world");
//       // ... and resubscribe
//       client.subscribe("esp8266/1/sample");
//     }
//     else
//     {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 5 seconds");
//       char buf[256];
//       espClient.getLastSSLError(buf, 256);
//       Serial.print("WiFiClientSecure SSL error: ");
//       Serial.println(buf);
//       // Wait 5 seconds before retrying
//       delay(5000);
//     }
//   }
// }

// typedef struct struct_message
// {
//   float t; //temperature
//   float h; //humidity
// } struct_message;

// // Create a struct_message called myData
// struct_message myData;

// // callback function that will be executed when data is received
// void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
// {
//   memcpy(&myData, incomingData, sizeof(myData));
//   Serial.print("Bytes received: ");
//   Serial.println(len);
//   Serial.println();
//   Serial.print("Temperatura: ");
//   Serial.println(myData.t);
//   Serial.println();
//   Serial.print("Umidade: ");
//   Serial.println(myData.h);
//   Serial.println();
// }

// void setup()
// {

//   Serial.begin(115200);
//   Serial.setDebugOutput(true);

//   // Set device as a Wi-Fi Station
//   WiFi.mode(WIFI_STA);

//   // initialize digital pin LED_BUILTIN as an output.
//   pinMode(LED_BUILTIN, OUTPUT);
//   setup_wifi();
//   delay(1000);
//   if (!SPIFFS.begin())
//   {
//     Serial.println("Failed to mount file system");
//     return;
//   }
//   Serial.print("Heap: ");
//   Serial.println(ESP.getFreeHeap());
//   // Load certificate file
//   File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
//   if (!cert)
//   {
//     Serial.println("Failed to open cert file");
//   }
//   else
//     Serial.println("Success to open cert file");
//   delay(1000);
//   if (espClient.loadCertificate(cert))
//     Serial.println("cert loaded");
//   else
//     Serial.println("cert not loaded");
//   // Load private key file
//   File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
//   if (!private_key)
//   {
//     Serial.println("Failed to open private cert file");
//   }
//   else
//     Serial.println("Success to open private cert file");
//   delay(1000);
//   if (espClient.loadPrivateKey(private_key))
//     Serial.println("private key loaded");
//   else
//     Serial.println("private key not loaded");
//   // Load CA file
//   File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
//   if (!ca)
//   {
//     Serial.println("Failed to open ca ");
//   }
//   else
//     Serial.println("Success to open ca");
//   delay(1000);
//   if (espClient.loadCACert(ca))
//     Serial.println("ca loaded");
//   else
//     Serial.println("ca failed");
//   Serial.print("Heap: ");
//   Serial.println(ESP.getFreeHeap());
//   //===========================================================================
//   WiFi.macAddress(mac);
//   snprintf(mac_Id, sizeof(mac_Id), "%02x:%02x:%02x:%02x:%02x:%02x",
//            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//   Serial.print(mac_Id);
//   //============================================================================

//   // Init ESP-NOW
//   if (esp_now_init() != 0)
//   {
//     Serial.println("Error initializing ESP-NOW");
//     return;
//   }

//   // Once ESPNow is successfully Init, we will register for recv CB to
//   // get recv packer info
//   esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
//   esp_now_register_recv_cb(OnDataRecv);
// }

// void loop()
// {
//   if (!client.connected())
//   {
//     reconnect();
//   }
//   client.loop();
//   long now = millis();
//   if (now - lastMsg > 2000)
//   {
//     lastMsg = now;
//     //============================================================================
//     String macIdStr = mac_Id;
//     uint8_t randomNumber = random(20, 50);
//     String randomString = String(random(0xffff), HEX);
//     snprintf(msg, BUFFER_LEN, "{\"mac_Id\" : \"%s\", \"random_number\" : %d, \"random_string\" : \"%s\"}", macIdStr.c_str(), randomNumber, randomString.c_str());
//     Serial.print("Publish message: ");
//     Serial.println(msg);
//     //mqttClient.publish("outTopic", msg);
//     client.publish("esp8266/1/sample", msg);
//     //=============================================================================
//     Serial.print("Heap: ");
//     Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
//   }
//   digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
//   delay(100);                      // wait for a second
//   digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
//   delay(100);                      // wait for a second
// }
