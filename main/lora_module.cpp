#include <Arduino.h>
#include "lora_module.h"

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
}

void loraRead(void) {
  static int counter = 0;
  static uint32_t lastTime = 0;

  if (millis() - lastTime >= 1000) {
    lastTime = millis();
    Serial.print("Sending packet ");
    Serial.println(counter);

    LoRa.beginPacket();
    LoRa.print("Hello from TX #");
    LoRa.print(counter++);
  }
}

