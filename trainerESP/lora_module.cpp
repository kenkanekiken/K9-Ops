#include <Arduino.h>
#include "lora_module.h"
#include <SPI.h>
#include <LoRa.h>

// ===========================
// T-BEAM PIN DEFINITIONS
// ===========================
#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_SS    18
#define LORA_RST   23 // T-Beam V1.1 uses 23 (V1.0 uses 14)
#define LORA_DIO0  26

// CRITICAL: MUST MATCH THE DOG EXACTLY
#define LORA_FREQ  923E6 

void loraInit() {
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa Init Failed");
    while (1);
  }
  // Optional: Boost power to max for better range
  LoRa.setTxPower(20); 
  Serial.println("LoRa Init OK (923 MHz)");
}

void loraSendBuzzerCommand(int state) {
  LoRa.beginPacket();
  LoRa.print("B,");
  LoRa.print(state);
  LoRa.endPacket();
  Serial.print("[LoRa TX] Sent Buzzer: "); Serial.println(state);
}

void loraSendLedCommand(int mode, int color, int brightness) {
  LoRa.beginPacket();
  LoRa.print("L,");
  LoRa.print(mode);
  LoRa.print(",");
  LoRa.print(color);
  LoRa.print(",");
  LoRa.print(brightness);
  LoRa.endPacket();
  Serial.println("[LoRa TX] Sent LED Command");
}

