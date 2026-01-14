#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "lora_module.h"

// T-Beam SX1276 pins
#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_SS    18
#define LORA_RST   14
#define LORA_DIO0  26

#define LORA_FREQ  925E6

void loraInit() {
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("[LoRa] init failed");
    while (1) delay(100);
  }

  // MUST match DogESP
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.enableCrc();

  Serial.println("[LoRa] init OK");
}

// Returns true if a packet was received
bool loraReceiveLine(String &outLine, int &outRssi, float &outSnr) {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return false;

  String s;
  s.reserve(256);

  while (LoRa.available()) {
    s += (char)LoRa.read();
  }

  outLine = s;
  outRssi = LoRa.packetRssi();
  outSnr  = LoRa.packetSnr();
  return true;
}