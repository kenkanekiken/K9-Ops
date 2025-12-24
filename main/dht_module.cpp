#include <Arduino.h>
#include "DHTesp.h"

DHTesp dht;

const int DHT_PIN = 25;

void dhtInit(void) {
  dht.setup(DHT_PIN, DHTesp::DHT11);
}

void dhtRead(void) {
  auto data = dht.getTempAndHumidity();
  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("Failed to read DHT11");
  } else {
    Serial.printf("T=%.1fC  H=%.1f%%\n", data.temperature, data.humidity);
  }
  delay(1000);
}
