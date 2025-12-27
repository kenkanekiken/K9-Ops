#include <Arduino.h>
#include "DHTesp.h"
#include "buzzer.h"

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
