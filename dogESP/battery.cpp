#include <Arduino.h>
#include <Wire.h>
#include <XPowersLib.h>   // 🔥 REQUIRED
#include "battery.h"

#define I2C_SDA 21
#define I2C_SCL 22

XPowersAXP2101 PMU;
static bool pmuReady = false;

// cached snapshot
static BatterySnapshot snap = {
  0.0f, 0, false, false, true
};

int batteryPercentFromVoltage(float v) {
  if (v >= 4.20f) return 100;
  if (v <= 3.30f) return 0;
  return (int)((v - 3.30f) / (4.20f - 3.30f) * 100.0f + 0.5f);
}

void batteryInit() {
  if (pmuReady) return;

  pinMode(POWER_BTN, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println("AXP2101 init failed");
    while (1) delay(1000);
  }

  PMU.enableBattVoltageMeasure();
  pmuReady = true;
}

void batteryUpdate(void) {
  static uint32_t lastTime = 0;

  if (millis() - lastTime >= 15000) {
    lastTime = millis();

    snap.voltage  = PMU.getBattVoltage() / 1000.0f;
    snap.percent  = batteryPercentFromVoltage(snap.voltage);
    snap.vbus     = PMU.isVbusIn();
    snap.charging = PMU.isCharging();
    snap.alive    = true;

    Serial.printf("[BAT] %.3fV  %d%%  USB=%d  CHG=%d\n",
                  snap.voltage, snap.percent,
                  snap.vbus, snap.charging);
  }
}

void batteryPreparePowerOff(void) {
  // mark device as going offline
  snap.alive = false;

  Serial.println("[BAT] prepare power off (alive=false)");

  // short delay so main loop can send final packet
  delay(50);
}

BatterySnapshot batteryGetSnapshot(void) {
  return snap;
}