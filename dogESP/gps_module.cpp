#include <Arduino.h>
#include "gps_module.h"
// Enable GPS power and GPS decode
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
// LoRa sender (we will call loraSend)
#include "lora_module.h"

TinyGPSPlus gps;
HardwareSerial GPSSerial(1);   // UART1 for GPS

static const int GPS_RX_PIN = 34;
static const int GPS_TX_PIN = 12;
static const uint32_t GPS_BAUD = 9600;

float lat = 0.0f;
float lng = 0.0f;
bool gpsOnline = false;

// Send rate limit
static uint32_t lastSend = 0;

// ---------- Distance / Speed ----------
float lastLat = 0;
float lastLng = 0;
unsigned long lastCalculationTime = 0;
float totalDistanceMeters = 0;
float totalDistanceKm = 0;

// Store last 25 speeds (km/h)
const int MAX_SPEED_HISTORY = 25;
double speedHistory[MAX_SPEED_HISTORY];
int speedHistoryIndex = 0;
bool speedHistoryFull = false;
float speedKmh = 0;

// Upload timing (prevent spamming)
static uint32_t lastGpsUpload = 0;

void gpsInit(void) {
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

void gpsRead(void) {
  // 1) Feed GPS decoder
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  // 2) Determine online state (fresh fix)
  const bool hasFix = gps.location.isValid();
  const uint32_t ageMs = gps.location.age();
  gpsOnline = hasFix && (ageMs < 3000);

  // 3) Update globals if valid
  if (gpsOnline) {
    lat = (float)gps.location.lat();
    lng = (float)gps.location.lng();
  }

  // 4) Debug print once per second
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 4000) { // Send every 4 second
    lastPrint = millis();
    Serial.printf("GPS Online: %s | Age(ms): %ld | Lat: %.6f | Lng: %.6f\n",
                  gpsOnline ? "YES" : "NO",
                  hasFix ? (long)ageMs : -1L,
                  lat, lng);
    // send even if offline so trainer can show "offline"
    loraSendGps(lat, lng);
    loraSendGpsStatus(gpsOnline);
  }

  // 5) Send via LoRa once per second
  if (millis() - lastSend < 2000) return;
  lastSend = millis();

  // =========================================================
  // distance + speed history logic 
  // =========================================================
  unsigned long currentMillis = millis();

  if (currentMillis - lastCalculationTime >= 20000) { // Send every 20 second
    if (gps.location.isValid()) {
      float currentLat = gps.location.lat();
      float currentLng = gps.location.lng();

      if (lastLat != 0 && lastLng != 0) {
        float distanceMeters =
            TinyGPSPlus::distanceBetween(currentLat, currentLng, lastLat, lastLng);

        if (distanceMeters > 1.0) { // ignore tiny movements
          totalDistanceMeters += distanceMeters;
        }
        totalDistanceKm = totalDistanceMeters / 1000.0;

        // speed in km/h (distance over 20s)
        speedKmh = (distanceMeters / (20000 / 1000.0)) * 3.6;

        // Store speed history
        speedHistory[speedHistoryIndex] = speedKmh;
        speedHistoryIndex = (speedHistoryIndex + 1) % MAX_SPEED_HISTORY;
        if (speedHistoryIndex == 0) speedHistoryFull = true;

        loraSendVelocity(totalDistanceMeters, totalDistanceKm, speedKmh);
      }
      // Update last coords and time so next 20s calc works
      lastLat = currentLat;
      lastLng = currentLng;
      lastCalculationTime = currentMillis;
    }
  }
}