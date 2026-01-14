#include "lora_module.h"
#include "power.h"
#include "firebase.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#define POWER_BTN 38   // example GPIO
// ============ WiFi =============
// const char* ssid = "K&K";
// const char* password = "Songbird7108";
// const char* ssid = "Jun Leis S23+";
// const char* password = "lmaoooooo";
const char* ssid = "kenkanekiken";
const char* password = "12345678";

void wifiInit(void) {
  WiFi.begin(ssid, password);
  Serial.print("WiFi connecting");
  if (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print("Wifi not connected");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  wifiInit();
  firebaseInit();
  loraInit();
  powerInit();
}

void loop() {
  powerOff();
  loraRead();
  if (millis() - lastPacketMs > 10000) {
    rxPower = false;
    rxGpsOnline = false;
  }
}
