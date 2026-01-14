#include <Arduino.h>
#include "power.h"
#include <Wire.h>
#include <XPowersLib.h>
#include "firebase.h"
#include <Firebase_ESP_Client.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define POWER_BTN 38   // example GPIO

extern FirebaseData fbdo;
XPowersAXP2101 PMU;

void powerInit(void) {
  pinMode(POWER_BTN, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
}

void powerOff(void) {
  static uint32_t pressedAt = 0;
  bool pressed = (digitalRead(POWER_BTN) == LOW);

  if (pressed && pressedAt == 0) {
    pressedAt = millis();
  }
  if (!pressed) {
    pressedAt = 0;
  }
  if (pressedAt && (millis() - pressedAt >= 2000)) {
    Serial.println("Powering off...");
    delay(50);
    if (Firebase.RTDB.setBool(&fbdo, "/devices/trainer/power", false)) {
      Serial.println("Trainer offline uploaded OK");
    } else {
      Serial.print("Firebase error: ");
      Serial.println(fbdo.errorReason());
    }
    PMU.shutdown();
  }
}