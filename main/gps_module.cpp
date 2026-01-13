#include <Arduino.h>
#include "gps_module.h"
#include "firebase.h"
#include <Firebase_ESP_Client.h>

// Enable GPS power and GPS decode
#include <Wire.h>
#include <axp20x.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

extern FirebaseData fbdo;

AXP20X_Class axp;
TinyGPSPlus gps;
HardwareSerial GPSSerial(1);   // UART1 for GPS

static const int GPS_RX_PIN = 34;   // ESP32 RX <- GPS TX
static const int GPS_TX_PIN = 12;   // ESP32 TX -> GPS RX
static const uint32_t GPS_BAUD = 9600;

// ---------- Distance / Speed ----------
double lastLat = 0;
double lastLng = 0;
unsigned long lastCalculationTime = 0;
const long interval = 20000; // 20s
double totalDistanceMeters = 0;

// Store last 25 speeds (km/h)
const int MAX_SPEED_HISTORY = 25;
double speedHistory[MAX_SPEED_HISTORY];
int speedHistoryIndex = 0;
bool speedHistoryFull = false;

// Upload timing (prevent spamming)
static uint32_t lastGpsUpload = 0;
static const uint32_t GPS_UPLOAD_MS = 1000; // try upload lat/lng at most once per second

void pmicInit(void) {
  Wire.begin(21, 22);
  axp.begin(Wire, 0x34);
  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);
}

void gpsInit(void) {
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

// Helper: update gpsStatus based on upload success
static void uploadGpsStatus(bool online) {
  if (!Firebase.ready()) return;

  if (!Firebase.RTDB.setBool(&fbdo, "/devices/latest/gpsStatus", online)) {
    Serial.print("Firebase error gpsStatus: ");
    Serial.println(fbdo.errorReason());
  }
}

void gpsRead(void) {
  // Feed GPS decoder
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  // Print basic debug once per second
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();

    Serial.print("Fix: ");
    Serial.print(gps.location.isValid() ? "YES" : "NO");

    Serial.print(" | Age(ms): ");
    Serial.print(gps.location.isValid() ? (int)gps.location.age() : -1);

    if (gps.location.isValid()) {
      Serial.print(" | Lat: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" | Lng: ");
      Serial.print(gps.location.lng(), 6);
    }
    Serial.println();
  }

  // =========================================================
  // gpsStatus = true only when lat+lng are successfully sent
  // =========================================================

  // If no fix, immediately mark offline (no usable coordinate to send)
  if (!gps.location.isValid()) {
    uploadGpsStatus(false);
    return;
  }

  // Optional: require "fresh" fix (prevents using stale cached values)
  // You can tune this number if needed.
  if (gps.location.age() > 3000) {
    uploadGpsStatus(false);
    return;
  }

  // Rate limit coordinate uploads
  if (millis() - lastGpsUpload < GPS_UPLOAD_MS) {
    return;
  }
  lastGpsUpload = millis();

  bool latOk = false;
  bool lngOk = false;

  // Only attempt Firebase writes when Firebase is ready
  if (Firebase.ready()) {
    const float lat = (float)gps.location.lat();
    const float lng = (float)gps.location.lng();

    latOk = Firebase.RTDB.setFloat(&fbdo, "/devices/latest/latitude", lat);
    if (!latOk) {
      Serial.print("Firebase error GPS Lat: ");
      Serial.println(fbdo.errorReason());
    }

    lngOk = Firebase.RTDB.setFloat(&fbdo, "/devices/latest/longitude", lng);
    if (!lngOk) {
      Serial.print("Firebase error GPS Lng: ");
      Serial.println(fbdo.errorReason());
    }

    // gpsStatus true ONLY if both succeed
    const bool gpsOnline = latOk && lngOk;
    uploadGpsStatus(gpsOnline);

    if (gpsOnline) {
      // Serial.println("GPS lat/lng uploaded OK -> gpsStatus ONLINE");
    } else {
      Serial.println("GPS upload incomplete -> gpsStatus OFFLINE");
    }
  } else {
    // Firebase not ready => can't deliver data
    uploadGpsStatus(false);
  }

  // =========================================================
  // distance + speed history logic 
  // =========================================================
  unsigned long currentMillis = millis();

  if (currentMillis - lastCalculationTime >= interval) {
    if (gps.location.isValid()) {
      double currentLat = gps.location.lat();
      double currentLng = gps.location.lng();

      if (lastLat != 0 && lastLng != 0) {
        double distanceMeters =
            TinyGPSPlus::distanceBetween(currentLat, currentLng, lastLat, lastLng);

        if (distanceMeters > 1.0) { // ignore tiny movements
          totalDistanceMeters += distanceMeters;
        }

        // speed in km/h (distance over interval)
        double speedKmh = (distanceMeters / (interval / 1000.0)) * 3.6;

        // Store speed history
        speedHistory[speedHistoryIndex] = speedKmh;
        speedHistoryIndex = (speedHistoryIndex + 1) % MAX_SPEED_HISTORY;
        if (speedHistoryIndex == 0) speedHistoryFull = true;

        if (Firebase.ready()) {
          Firebase.RTDB.setFloat(&fbdo, "/devices/latest/total_distance_meters", totalDistanceMeters);
          Firebase.RTDB.setFloat(&fbdo, "/devices/latest/total_distance_km", totalDistanceMeters / 1000.0);

          // Upload speed history array
          FirebaseJson json;
          for (int i = 0; i < (speedHistoryFull ? MAX_SPEED_HISTORY : speedHistoryIndex); i++) {
            int idx = (speedHistoryIndex - 1 - i + MAX_SPEED_HISTORY) % MAX_SPEED_HISTORY;
            json.set(String(i), speedHistory[idx]);
          }

          if (Firebase.RTDB.setJSON(&fbdo, "/devices/latest/speed_history_array", &json)) {
            Serial.println("Speed history array uploaded OK");
          } else {
            Serial.print("Firebase error Speed Array: ");
            Serial.println(fbdo.errorReason());
          }

         // Push speed log entry
          String path = "/devices/latest/speed";
          FirebaseJson jsonSingle;
          jsonSingle.set("speed", speedKmh);
          jsonSingle.set("ts", (int)currentMillis);

          if (Firebase.RTDB.pushJSON(&fbdo, path, &jsonSingle)) {
            Serial.println("Speed data pushed to Firebase");
          } else {
            Serial.print("Firebase error Speed push: ");
            Serial.println(fbdo.errorReason());
          }
        }
      }

      // Update last coordinates
      lastLat = currentLat;
      lastLng = currentLng;
      lastCalculationTime = currentMillis;
    }
  } 
}