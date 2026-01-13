#include <Arduino.h>
#include "gps_module.h"

// Enable GPS power and GPS decode
#include <Wire.h>
#include <axp20x.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// LoRa sender (we will call loraSend)
#include "lora_module.h"

AXP20X_Class axp;
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
static const uint32_t SEND_MS = 1000; // send once per second

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
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();
    Serial.printf("GPS Online: %s | Age(ms): %ld | Lat: %.6f | Lng: %.6f\n",
                  gpsOnline ? "YES" : "NO",
                  hasFix ? (long)ageMs : -1L,
                  lat, lng);
  }

  // 5) Send via LoRa once per second
  if (millis() - lastSend < SEND_MS) return;
  lastSend = millis();

  // send even if offline so trainer can show "offline"
  loraSendGps(lat, lng);
  loraSendGpsStatus(gpsOnline);
}