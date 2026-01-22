#include <Arduino.h>
#include "lora_module.h"
#include "buzzer.h"
#include "led_function.h"
// #include "battery.h" // Uncomment if you have the battery wires set up
// #include "gps_module.h" // Uncomment if you have the GPS wired

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Dog ESP...");
  
  // 1. Init Hardware (Buzzer & LED)
  initBuzzer();    // Sets pin 14 to Output
  LedInit();       // Starts the NeoPixel on pin 4
  
  // 2. Init LoRa (The Radio)
  // This must be last to ensure SPI is ready
  loraInit();
}

void loop() {
  // 1. Check for incoming LoRa commands (Buzzer/LED)
  loraHandleIncoming();
  
  // 2. Keep LED animations running (Blink mode needs this)
  runLedAnimations();
}