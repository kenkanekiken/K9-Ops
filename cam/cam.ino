#include "cam_stream.h"

void setup() {
  Serial.begin(115200);
  cameraInit();
  serverInit();
}

void loop() {
  serverRead();
}