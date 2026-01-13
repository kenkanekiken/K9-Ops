#include "led_function.h"
#include "gps_module.h"
#include "dht_module.h"
#include "lora_module.h"
#include "ble_module.h"
#include "mpu.h"
#include "buzzer.h"
#include "battery.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Pin 25 DHT
// Pin 14 buzzer
// Pin 2 LED
#define POWER_BTN 38   // example GPIO
// ===== WiFi =====
// const char* ssid = "K&K";
// const char* password = "Songbird7108";
const char* ssid = "Jun Leis S23+";
const char* password = "lmaoooooo";
// const char* ssid = "kenkanekiken";
// const char* password = "12345678";

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
  pinMode(2, OUTPUT);
  pmicInit();
  gpsInit();
  dhtInit();
  batteryInit();
  loraInit();
  bleInit();
  mpuInit();
  buzzerInit();
}

void loop() {
  powerOff();
  // blink_led(2);
  batteryRead();
  dhtRead();
  mpuRead();
  gpsRead();
}
