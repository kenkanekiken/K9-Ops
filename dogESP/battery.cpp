#include <Arduino.h>
#include "battery.h"
#include <Wire.h>
#include <XPowersLib.h>
#include "lora_module.h"   

#define I2C_SDA   21
#define I2C_SCL   22
#define POWER_BTN 38

XPowersAXP2101 PMU;
int pct = 0;

int batteryPercentFromVoltage(float v) {
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

  if (millis() - lastTime >= 10000) { // 10s
    lastTime = millis();

    float vbat = PMU.getBattVoltage() / 1000.0f;
    pct = batteryPercentFromVoltage(vbat);

    bool vbus = PMU.isVbusIn();
    bool chg  = PMU.isCharging();

    // Consider "power" as device is alive (true). You can redefine later.
    bool power = true;

    Serial.printf("VBAT=%.3fV  %d%%  USB=%d  CHG=%d\n", vbat, pct, vbus, chg);

    // ✅ Send via LoRa
    loraSendBattery(pct);
  }
}

void powerOff(void) {
  static uint32_t pressedAt = 0;
  bool pressed = (digitalRead(POWER_BTN) == LOW);

  if (pressed && pressedAt == 0) pressedAt = millis();
  if (!pressed) pressedAt = 0;

  if (pressedAt && (millis() - pressedAt >= 2000)) {
    Serial.println("Powering off...");

    // Send a final packet: power = 0
    loraSendPower(false); // Power status off
    loraSendGpsStatus(false); // GPS status off 

    delay(80); // small delay so packet can finish TX
    PMU.shutdown();
  }
}