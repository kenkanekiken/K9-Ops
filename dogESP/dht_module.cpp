#include <Arduino.h>
#include "DHTesp.h"
#include "dht_module.h"

DHTesp dht;
const int DHT_PIN = 25;

static float tempC = NAN;

void dhtInit(void) {
  dht.setup(DHT_PIN, DHTesp::DHT11);
}

void dhtUpdate(void) {
  static uint32_t lastTime = 0;

  if (millis() - lastTime >= 10000) { // 10s sampling
    lastTime = millis();

    auto data = dht.getTempAndHumidity();

    if (isnan(data.temperature)) {
      Serial.println("[DHT] Read failed");
      return;
    }

    tempC = data.temperature;
    Serial.printf("[DHT] T=%.1fC  H=%.1f%%\n",
                  data.temperature, data.humidity);
  }
}

float dhtGetTemperature(void) {
  return tempC;   // returns last valid reading
}