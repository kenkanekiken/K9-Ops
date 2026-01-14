#include <Arduino.h>
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

#include "battery.h"
#include "gps_module.h"

TinyGPSPlus gps;
HardwareSerial GPSSerial(1);

static const int GPS_RX_PIN = 34;
static const int GPS_TX_PIN = 12;
static const uint32_t GPS_BAUD = 9600;

// cached snapshot
static GpsSnapshot snap = {
  false, 0, 0, 0, 100.0f, 0.0f, 0.0f
};

// distance / speed
static float lastLat = 0;
static float lastLng = 0;
static unsigned long lastCalc = 0;

void gpsPowerOn_AXP2101() {
  PMU.setALDO4Voltage(3300);
  PMU.enableALDO4();
  delay(200);
}

void gpsInit() {
  gpsPowerOn_AXP2101();
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("[GPS] UART started");
}

void gpsUpdate(void) {
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  const bool hasFix = gps.location.isValid();
  const uint32_t ageMs = gps.location.age();

  snap.online = hasFix && (ageMs < 3000);

  if (snap.online) {
    snap.lat  = gps.location.lat();
    snap.lng  = gps.location.lng();
    snap.sats = gps.satellites.isValid() ? gps.satellites.value() : 0;
    snap.hdop = gps.hdop.isValid() ? gps.hdop.hdop() : 100.0f;
  }

  // speed / distance every 20s
  unsigned long now = millis();
  if (now - lastCalc >= 20000 && snap.online) {
    if (lastLat != 0 && lastLng != 0) {
      float d =
        TinyGPSPlus::distanceBetween(snap.lat, snap.lng, lastLat, lastLng);

      if (d > 1.0f) {
        snap.totalDistanceMeters += d;
        snap.speedKmh = (d / 20.0f) * 3.6f;
      }
    }
    lastLat = snap.lat;
    lastLng = snap.lng;
    lastCalc = now;
  }
}

GpsSnapshot gpsGetSnapshot() {
  return snap;   // return cached data (safe & cheap)
}