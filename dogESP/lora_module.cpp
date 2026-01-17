#include <Arduino.h>
#include "lora_module.h"
#include <SPI.h>
#include <LoRa.h>
#include "led_function.h"

// T-Beam SX1276 typical pins
#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_SS    18
#define LORA_RST   14
#define LORA_DIO0  26

#define LORA_FREQ  925E6

static uint32_t seq = 0;

void loraInit(void) {
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa init failed!");
    while (1) delay(100);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.enableCrc();

  Serial.println("LoRa init OK");
}

void loraSendSnapshot(
  const BatterySnapshot& b,
  const GpsSnapshot& g,
  float temp,
  float ax, float ay, float az,
  int motion, int state,
  long steps, float speed, float distance
) {
  seq++;

  // seq,batt,temp,gpsFix,sats,hdop,lat,lng,ax,ay,az,motion,state,steps,speed,distance
  char msg[240];
  snprintf(msg, sizeof(msg),
           "%lu,%d,%.1f,%d,%d,%.1f,%.6f,%.6f,%.3f,%.3f,%.3f,%d,%d,%ld,%.2f,%.2f",
           (unsigned long)seq,
           b.percent,
           temp,
           g.online ? 1 : 0,
           g.sats,
           g.hdop,
           g.lat, g.lng,
           ax, ay, az,
           motion, state, steps,
           speed, distance);

  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();

  Serial.print("[LoRa TX] ");
  Serial.println(msg);
}

bool loraReceiveLine(String &outLine, int &outRssi, float &outSnr) {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return false;
  String s;
  while (LoRa.available()) s += (char)LoRa.read();
  outLine = s;
  outRssi = LoRa.packetRssi();
  outSnr  = LoRa.packetSnr();
  return true;
}

void loraHandleIncoming() {
  String line;
  int rssi = 0;
  float snr = 0;
  if (!loraReceiveLine(line, rssi, snr)) return;

  if (line.startsWith("L,")) {

    int mode, color, brightness;
    // This is an LED command
    // Parse LED command
    if (sscanf(line.c_str(), "L,%d,%d,%d", &mode, &color, &brightness) == 3) {
      Serial.printf("[LoRa RX] LED cmd: mode=%d color=%d brightness=%d\n",
                    mode, color, brightness);

      isBlinking = true;

    
    }

    else {
      Serial.println("[LoRa RX] Invalid LED command");
    }
  }

  else {
        Serial.println("[LoRa RX] Received unknown packet: " + line);
    }


}