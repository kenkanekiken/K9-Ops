#pragma once
#include <Arduino.h>
#include <stdint.h>

struct MpuSnapshot {
  bool  ok;
  float ax;
  float ay;
  float az;
  float motion;
  int   state;     // use enum int (better than String)
  long  steps;
};

enum MpuState : int {
  MPU_UNKNOWN = 0,
  MPU_STATIONARY = 1,
  MPU_WALKING = 2,
  MPU_RUNNING = 3
};

void mpuInit(void);
void mpuUpdate(void);          // replaces mpuRead()
MpuSnapshot mpuGetSnapshot(void);
