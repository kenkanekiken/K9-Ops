#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <XPowersLib.h>   // 🔥 REQUIRED for XPowersAXP2101

#define POWER_BTN 38

struct BatterySnapshot {
  float voltage;
  int   percent;
  bool  vbus;
  bool  charging;
  bool  alive;
};

extern XPowersAXP2101 PMU;

void batteryInit(void);
void batteryUpdate(void);
void batteryPreparePowerOff(void);
BatterySnapshot batteryGetSnapshot(void);