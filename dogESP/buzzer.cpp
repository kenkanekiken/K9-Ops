#include <Arduino.h>
#include "buzzer.h"

// IMPORTANT:
// - <Arduino.h> provides pinMode/digitalWrite/delay
// - "buzzer.h" provides buzzerOn/buzzerOff prototypes

// If your buzzer pin is defined elsewhere, keep that.
// If not, set it here to match your hardware.
#ifndef BUZZER_PIN
#define BUZZER_PIN 14
#endif

void buzzerInit(void) {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void buzzerOn(void) {
  digitalWrite(BUZZER_PIN, HIGH);
}

void buzzerOff(void) {
  digitalWrite(BUZZER_PIN, LOW);
}

// Keep compatibility if you already call initBuzzer() somewhere
void initBuzzer() {
  buzzerInit();
}

// ✅ This is the only behavior you needed
void playBuzzerPattern(int pattern) {
  switch (pattern) {
    // 1️⃣ Single tap
    case 1:
      buzzerOn();
      delay(120);
      buzzerOff();
      break;

    // 2️⃣ Double tap
    case 2:
      for (int i = 0; i < 2; i++) {
        buzzerOn();
        delay(120);
        buzzerOff();
        delay(120);
      }
      break;

    // 3️⃣ Continuous (3 seconds)
    case 3:
      buzzerOn();
      delay(3000);
      buzzerOff();
      break;

    default:
      buzzerOff();
      break;
  }
}