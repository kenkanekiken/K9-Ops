#include <Arduino.h>
#include "lora_module.h"
#include <SPI.h>
#include <LoRa.h>
#include "led_function.h" 
#include "buzzer.h"       

// ===========================
// T-BEAM PIN DEFINITIONS (The Stable Ones)
// ===========================
#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_SS    18
#define LORA_RST   23 // T-Beam V1.1 uses 23
#define LORA_DIO0  26

#define LORA_FREQ  923E6 

void loraInit(void) {
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa Init Failed");
    while (1) delay(100);
  }
  Serial.println("LoRa Init OK");
}

bool loraReceiveLine(String &outLine, int &outRssi, float &outSnr) {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return false;
  
  String s = "";
  while (LoRa.available()) s += (char)LoRa.read();
  
  outLine = s;
  outRssi = LoRa.packetRssi();
  outSnr  = LoRa.packetSnr();
  return true;
}

void loraHandleIncoming() {
  String line;
  int rssi; float snr;
  if (!loraReceiveLine(line, rssi, snr)) return;

  Serial.print("RX: "); Serial.println(line);

  // LED Parsing
  if (line.startsWith("L,")) {
    int m, c, b;
    if (sscanf(line.c_str(), "L,%d,%d,%d", &m, &c, &b) == 3) {
      setLedProperties(m, c, b);
    }
  }
  // Buzzer Parsing
  else if (line.startsWith("B,")) {
     int state;
     if (sscanf(line.c_str(), "B,%d", &state) == 1) {
         if (state == 1) buzzerOn();
         else buzzerOff();
     }
  }
}
