#include <Arduino.h>
#include "DHTesp.h"
#include "buzzer.h"
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char* ssid = "K&K";
const char* password = "Songbird7108";

// MQTT broker (example: Mosquitto on your laptop/RPi)
const char* mqtt_host = "192.168.1.50";   // broker IP
const int   mqtt_port = 1883;

// Topics
const char* topic_pub = "k9ops/esp32s3/status";
const char* topic_sub = "k9ops/esp32s3/cmd";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long lastPubMs = 0;


DHTesp dht;
const int DHT_PIN = 25;

void dhtInit(void) {
  dht.setup(DHT_PIN, DHTesp::DHT11);
}

void dhtRead(void) {
  auto data = dht.getTempAndHumidity();
  static uint32_t lastTime = 0;

  if (millis() - lastTime >= 5000) {
    lastTime = millis();

    if (isnan(data.temperature) || isnan(data.humidity)) {
      Serial.println("Failed to read DHT11");
    } else {
      Serial.printf("T=%.1fC  H=%.1f%%\n", data.temperature, data.humidity);
    }
  }
  if (data.temperature > 29) {
    buzzerOn();
    delay(500);
    buzzerOff();
  }
}
