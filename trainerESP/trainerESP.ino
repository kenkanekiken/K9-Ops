
#include <Arduino.h>
#include <WiFi.h>
#include "lora_module.h"
#include "mqtt_handler.h"

// WIFI
const char* ssid = "Iphone 19 pro max"; 
const char* pass = "00009999";

void setup() {
  Serial.begin(115200);
  
  // 1. Start LoRa
  loraInit();

  // 2. Start WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, pass);
  
  // This loop prints dots until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");

  // 3. Start MQTT
  mqttInit();
}

void loop() {
  mqttLoop();
}