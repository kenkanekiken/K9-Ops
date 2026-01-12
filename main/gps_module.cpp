#include <Arduino.h>
#include "gps_module.h"
#include "firebase.h"
#include <Firebase_ESP_Client.h>

//Enable GPS POwer and calling GPS
#include <Wire.h>
#include <axp20x.h>
#include <TinyGPSPlus.h>

extern FirebaseData fbdo;
AXP20X_Class axp;
TinyGPSPlus gps;

static const int GPS_RX_PIN = 34;   // ESP32 RX <- GPS TX
static const int GPS_TX_PIN = 12;   // ESP32 TX -> GPS RX
static const uint32_t GPS_BAUD = 9600;
double lastLat = 0;
double lastLng = 0;
unsigned long lastCalculationTime = 0;
const long interval = 20000; 
double totalDistanceMeters = 0;

// New: Array to store last 25 speeds (in m/s)
const int MAX_SPEED_HISTORY = 25;
double speedHistory[MAX_SPEED_HISTORY];
int speedHistoryIndex = 0;
bool speedHistoryFull = false;

HardwareSerial GPSSerial(1);

void pmicInit(void) {
  // PMIC init (AXP at 0x34)
  Wire.begin(21, 22);
  axp.begin(Wire, 0x34);
  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);
}

void gpsInit(void) {
  //GPS UART
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

void gpsRead(void) {
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();

    Serial.print("Fix: ");
    Serial.print(gps.location.isValid() ? "YES" : "NO");

    Serial.print(" | Sats: ");
    Serial.print(gps.satellites.isValid() ? gps.satellites.value() : 0);

    Serial.print(" | HDOP: ");
    Serial.print(gps.hdop.isValid() ? gps.hdop.hdop() : 99.9, 1);

    if (gps.location.isValid()) {
      Serial.print(" | Lat: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" | Lng: ");
      Serial.print(gps.location.lng(), 6);
      Serial.print(" | Sat: ");
      Serial.print(gps.satellites.value(), 6);
      if (Firebase.RTDB.setFloat(&fbdo, "/devices/latest/latitude", gps.location.lat())) {
        Serial.println("Latitude uploaded OK");
      } else {
        Serial.print("Firebase error GPS Lat: ");
        Serial.println(fbdo.errorReason());
      }
      if (Firebase.RTDB.setFloat(&fbdo, "/devices/latest/longitude", gps.location.lng())) {
        Serial.println("Longitude uploaded OK");
      } else {
        Serial.print("Firebase error GPS Lng: ");
        Serial.println(fbdo.errorReason());
      }
    }
    Serial.println();
  }

  unsigned long currentMillis = millis();

  if (currentMillis - lastCalculationTime >= interval) {
    if (gps.location.isValid()) {
      double currentLat = gps.location.lat();
      double currentLng = gps.location.lng();

      if (lastLat != 0 && lastLng != 0) {
        // 1. Calculate distance in meters
        double distanceMeters = TinyGPSPlus::distanceBetween(currentLat, currentLng, lastLat, lastLng);
        
        if (distanceMeters > 1.0) { // Ignore very small movements
          totalDistanceMeters += distanceMeters;
        }
        // 2. Calculate speed (m/s) -> (Distance / Time)
        // interval is in ms, so divide by 1000
        double speedKmh = (distanceMeters / (interval / 1000.0)) * 3.6; // Convert m/s to km/h

        // New: Store speed in history array
        speedHistory[speedHistoryIndex] = speedKmh;
        speedHistoryIndex = (speedHistoryIndex + 1) % MAX_SPEED_HISTORY;
        if (speedHistoryIndex == 0) {
          speedHistoryFull = true;
        }

        Firebase.RTDB.setFloat(&fbdo, "/devices/latest/total_distance_meters", totalDistanceMeters);
        Firebase.RTDB.setFloat(&fbdo, "/devices/latest/total_distance_km", totalDistanceMeters / 1000.0);

        // New: Upload the entire speed history array to Firebase
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

        // Optional: Still push individual for other uses, but now we have the array
        String path = "/devices/latest/speed";
        FirebaseJson jsonSingle;
        jsonSingle.set("speed", speedKmh); // Speed in km/h
        jsonSingle.set("ts", currentMillis); // Timestamp

        if (Firebase.RTDB.pushJSON(&fbdo, path, &jsonSingle)) {
          Serial.println("Speed data pushed to Firebase");
        }
      }

      // Update "last" coordinates for next cycle
      lastLat = currentLat;
      lastLng = currentLng;
      lastCalculationTime = currentMillis;
    }
  }
}




