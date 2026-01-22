#include "buzzer.h"

void initBuzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void buzzerOn() {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("[Buzzer] ON");
}

void buzzerOff() {
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("[Buzzer] OFF");
}
