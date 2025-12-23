#include "led_function.h"

void setup() {
  pinMode(25, OUTPUT);
}

void loop() {
  blink_led(25);
}
