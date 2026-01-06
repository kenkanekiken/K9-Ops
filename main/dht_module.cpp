#include <Arduino.h>
#include "DHTesp.h"
#include "buzzer.h"
#include "firebase.h"
#include <Firebase_ESP_Client.h>

// Firebase objects
extern FirebaseData fbdo;
// DHT objects
DHTesp dht;
const int DHT_PIN = 25;

void dhtInit(void) {
  dht.setup(DHT_PIN, DHTesp::DHT11);
}

void dhtRead(void) {
  static uint32_t lastTime = 0;

  if (millis() - lastTime >= 120000) { // 2 Minutes
    lastTime = millis();
    auto data = dht.getTempAndHumidity();

    if (isnan(data.temperature) || isnan(data.humidity)) {
      Serial.println("Failed to read DHT11");
    } else {
      Serial.printf("T=%.1fC  H=%.1f%%\n", data.temperature, data.humidity);
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/devices/latest/temperature", data.temperature)) {
      Serial.println("Temp uploaded OK");
      } else {
        Serial.print("Firebase error: ");
        Serial.println(fbdo.errorReason());
    }
  }
}

