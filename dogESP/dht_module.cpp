#include <Arduino.h>
#include "DHTesp.h"
#include "buzzer.h"
#include "dht_module.h"
#include "lora_module.h"   

DHTesp dht;
const int DHT_PIN = 25;

float temp = 0.0f;

void dhtInit(void) {
  dht.setup(DHT_PIN, DHTesp::DHT11);
}

void dhtRead(void) {
  static uint32_t lastTime = 0;

  if (millis() - lastTime >= 10000) { // 10s
    lastTime = millis();

    auto data = dht.getTempAndHumidity();

    if (isnan(data.temperature) || isnan(data.humidity)) {
      Serial.println("Failed to read DHT11");
      return;
    }

    temp = data.temperature;
    Serial.printf("T=%.1fC  H=%.1f%%\n", data.temperature, data.humidity);

    // ✅ Send temperature via LoRa
    loraSendTemp(temp);
  }
}