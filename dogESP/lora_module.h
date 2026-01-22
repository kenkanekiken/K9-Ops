#ifndef LORA_MODULE_H
#define LORA_MODULE_H

#include <Arduino.h>
#include "battery.h"      // Required for BatterySnapshot
#include "gps_module.h"   // Required for GpsSnapshot

// Initialize the LoRa radio
void loraInit(void);

// The new function that listens for commands (Buzzer/LED)
void loraHandleIncoming();

// The function to send data back to the Trainer
void loraSendSnapshot(
  const BatterySnapshot& b, 
  const GpsSnapshot& g, 
  float temp,
  float ax, float ay, float az, 
  int motion, int state,
  long steps, float speed, float distance
);

#endif