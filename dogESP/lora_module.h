#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "gps_module.h"
#include "battery.h"

// ================= Init =================
void loraInit(void);

void loraSendSnapshot(
  const BatterySnapshot& b,
  const GpsSnapshot& g,
  float temp,
  float ax, float ay, float az,
  int motion, int state,
  long steps, float speed, float distance
);

bool loraReceiveLine(String &outLine, int &outRssi, float &outSnr);
void loraHandleIncoming(); 