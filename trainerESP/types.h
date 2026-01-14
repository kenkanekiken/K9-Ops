#pragma once
#include <Arduino.h>
#include <stdint.h>

struct DogData {
  uint32_t seq = 0;

  int   battPct = -1;
  bool  alive = true;

  float tempC = NAN;

  bool  gpsOnline = false;
  int   sats = 0;
  float hdop = 100.0f;
  double lat = 0.0;
  double lng = 0.0;

  float ax = 0, ay = 0, az = 0;
  float motion = 0;
  int   state = 0;
  long  steps = 0;

  float speed = 0;
  float distance = 0;

  int rssi = 0;
  float snr = 0;

  uint32_t lastRxMs = 0;
};