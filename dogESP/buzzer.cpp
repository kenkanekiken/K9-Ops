#include <Arduino.h>
#include "buzzer.h"

// ===== PWM BUZZER CONFIG =====
#define BUZZER_PIN 14     // change to your buzzer GPIO
#define BUZZER_FREQ 2000  // Hz (tone pitch)
#define BUZZER_RES 8      // resolution (8-bit: 0–255)

void buzzerInit(void) {
  // New API (ESP32 core 3.x): attaches PWM to this PIN, channel auto-selected
  ledcAttach(BUZZER_PIN, BUZZER_FREQ, BUZZER_RES);
  ledcWrite(BUZZER_PIN, 0); // OFF
}

void buzzerOn(void) {  // 50% duty by default
  ledcWrite(BUZZER_PIN, 128);
}
void buzzerOff(void) {
  ledcWrite(BUZZER_PIN, 0);
}