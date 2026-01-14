#pragma once
#include <Arduino.h>
#include <stdint.h>

struct GpsSnapshot {
  bool  online;
  float lat;
  float lng;
  int   sats;
  float hdop;

  float speedKmh;
  float totalDistanceMeters;
};

void gpsInit(void);
void gpsUpdate(void);          // call often
GpsSnapshot gpsGetSnapshot();  // read-only snapshot