#include <Arduino.h>
#include "gps_module.h"

//Enable GPS POwer and calling GPS
#include <Wire.h>
#include <axp20x.h>
#include <TinyGPSPlus.h>

AXP20X_Class axp;
TinyGPSPlus gps;

static const int GPS_RX_PIN = 34;   // ESP32 RX <- GPS TX
static const int GPS_TX_PIN = 12;   // ESP32 TX -> GPS RX
static const uint32_t GPS_BAUD = 9600;

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
    }
    Serial.println();
  }
}




