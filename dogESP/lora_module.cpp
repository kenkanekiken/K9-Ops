#include <Arduino.h>
#include "lora_module.h"
#include "dht_module.h"
#include "battery.h"
// TX Transmitter
#include <SPI.h>
#include <LoRa.h>

#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_CS    18
#define LORA_RST   23
#define LORA_DIO0  26   // if no packets later, try 33

void loraInit(void) {
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(923E6)) {
    Serial.println("LoRa.begin FAILED");
    while (1);
  }

  Serial.println("LoRa init OK");
  loraSendPower(true);
  loraSendGpsStatus(true);
}

void loraSendGps(float lat, float lng) {
  LoRa.beginPacket();
  LoRa.print("GPS,");
  LoRa.print(lat, 6);
  LoRa.print(",");
  LoRa.print(lng, 6);
  LoRa.endPacket();

  // Debug
  Serial.printf("LoRa TX: GPS,%.6f,%.6f\n", lat, lng);
}

void loraSendGpsStatus(bool gpsOnline) {
  LoRa.beginPacket();
  LoRa.print("GPSStatus,");
  LoRa.print(gpsOnline ? 1 : 0);
  LoRa.endPacket();

  // Debug
  Serial.printf("LoRa TX: GPSStatus,%d\n", gpsOnline ? 1 : 0);
}

void loraSendTemp(float temp) {
  LoRa.beginPacket();
  LoRa.print("Temperature,");
  LoRa.print(temp, 1);
  LoRa.endPacket();

  // Debug
  Serial.printf("LoRa TX: Temperature,%.1f\n", temp);
}

void loraSendPower(bool power) {
  LoRa.beginPacket();
  LoRa.print("Power,");
  LoRa.print(power ? 1 : 0);
  LoRa.endPacket();

  // Debug
  Serial.printf("LoRa TX: Power,%d\n", power ? 1 : 0);
}

void loraSendBattery(int pct) {
  LoRa.beginPacket();
  LoRa.print("Battery,");
  LoRa.print(pct);
  LoRa.endPacket();

  // Debug
  Serial.printf("LoRa TX: Battery,%d\n", pct);
}

void loraSendMovement(float ax, float ay, float az, float motion, const String& state, int stepCount) {
  LoRa.beginPacket();
  LoRa.print("Movement,");
  LoRa.print(ax, 5); LoRa.print(",");
  LoRa.print(ay, 5); LoRa.print(",");
  LoRa.print(az, 5); LoRa.print(",");
  LoRa.print(motion, 5); LoRa.print(",");
  LoRa.print(state); LoRa.print(",");
  LoRa.print(stepCount);
  LoRa.endPacket();

  // Debug
  Serial.printf(
    "LoRa TX: Movement,%.5f,%.5f,%.5f,%.5f,%s,%d\n",
    ax, ay, az, motion, state.c_str(), stepCount
  );
}

// void loraRead(void) {
//   static int counter = 0;
//   static uint32_t lastTime = 0;

//   if (millis() - lastTime >= 1000) {
//     lastTime = millis();
//     Serial.print("Sending Temperature: ");
//     Serial.println(temp);
//     Serial.print("Sending Latitude: ");
//     Serial.println(lat, 6);
//     Serial.print("Sending Longtitude: ");
//     Serial.println(lng, 6);
//     Serial.print("Sending Battery: ");
//     Serial.println(pct);

//     // CSV format: lat,lng,temp,pct
//     LoRa.beginPacket();
//     LoRa.print(lat, 6);
//     LoRa.print(",");
//     LoRa.print(lng, 6);
//     LoRa.print(",");
//     LoRa.print(temp;
//     LoRa.print(",");
//     LoRa.print(pct);
//     LoRa.endPacket();  // ✅ MUST HAVE
//   }
// }

