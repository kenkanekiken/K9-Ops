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
    // Single tap
    case 1:
      buzzerOn();
      delay(120);
      buzzerOff();
      break;

    // Double tap
    case 2:
      for (int i = 0; i < 2; i++) {
        buzzerOn();
        delay(120);
        buzzerOff();
        delay(120);
      }
      break;

    // Continuous (3 seconds)
    case 3:
      buzzerOn();
      delay(3000);
      buzzerOff();
      break;

   // 🎾 Playing 
    case 4:
      buzzerOn();
      delay(80);
      buzzerOff();
      delay(150);
      buzzerOn();
      delay(120);
      buzzerOff();
      break;

  // 🎯 Training 
    case 5:
      for (int i = 0; i < 3; i++) {
        buzzerOn();
        delay(120);
        buzzerOff();
        delay(250);
      }
      break;

  // 🟣 Tracing 
    case 6:
      buzzerOn();
      delay(220);
      buzzerOff();
      delay(100);
      buzzerOn();
      delay(150);
      buzzerOff();
      delay(80);
      buzzerOn();
      delay(90);
      buzzerOff();
      break;

  // 🦺 Deployed 
    case 7:
      buzzerOn();
      delay(400);
      buzzerOff();
      delay(150);
      buzzerOn();
      delay(120);
      buzzerOff();
      break;

    default:
      buzzerOff();
      break;
  }
}