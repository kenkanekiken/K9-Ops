#include <Arduino.h>
#include "battery.h"
#include <Wire.h>
#include <XPowersLib.h>
#include "firebase.h"
#include <Firebase_ESP_Client.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define POWER_BTN 38   // example GPIO

extern FirebaseData fbdo;
XPowersAXP2101 PMU;

int batteryPercentFromVoltage(float v) {
  // Simple Li-ion estimate (good enough for dashboard)
  if (v >= 4.20f) return 100;
  if (v <= 3.30f) return 0;
  return (int)((v - 3.30f) / (4.20f - 3.30f) * 100.0f + 0.5f);
}

void batteryInit(void) {
  pinMode(POWER_BTN, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
  if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println("AXP2101 init failed");
    while (1) delay(1000);
  }
  PMU.enableBattVoltageMeasure();
  Serial.println("T-Beam v1.2 battery monitor ready");
}

void batteryRead(void) {
  static uint32_t lastTime = 0;

  if (millis() - lastTime >= 10000) { // 30 Second
    lastTime = millis();

    float vbat = PMU.getBattVoltage() / 1000.0f;
    int pct = batteryPercentFromVoltage(vbat);
    bool vbus = PMU.isVbusIn();
    bool chg  = PMU.isCharging();

    Serial.printf("VBAT=%.3fV  %d%%  USB=%d  CHG=%d\n", vbat, pct, vbus, chg);

    if (Firebase.RTDB.setInt(&fbdo, "/devices/latest/battery", pct)) {
      Serial.println("Battery uploaded OK");
    } else {
      Serial.print("Firebase error: ");
      Serial.println(fbdo.errorReason());
    }
  }
}

void powerOff(void) {
  static uint32_t pressedAt = 0;
  bool pressed = (digitalRead(POWER_BTN) == LOW);

  if (pressed && pressedAt == 0) {
    pressedAt = millis();
  }
  if (!pressed) {
    pressedAt = 0;
  }
  if (pressedAt && (millis() - pressedAt >= 2000)) {
    Serial.println("Powering off...");
    delay(50);
    if (Firebase.RTDB.setString(&fbdo, "/devices/latest/power", false)) {
      Serial.println("GPS offline uploaded OK");
    } else {
      Serial.print("Firebase error: ");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.setBool(&fbdo, "/devices/latest/gpsStatus", false)) {
      Serial.println("GPS offline uploaded OK");
    } else {
      Serial.print("Firebase error: ");
      Serial.println(fbdo.errorReason());
    }
    PMU.shutdown();
  }
}