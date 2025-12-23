#include "led_function.h"
#include "gps_module.h"

void setup() {
  Serial.begin(115200);
  pinMode(25, OUTPUT);
  pmicInit();
  gpsInit();
}

void loop() {
  blink_led(25);
  gpsRead();
}
