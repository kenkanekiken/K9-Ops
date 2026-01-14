#pragma once
#include <stdint.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define POWER_BTN 38

void batteryInit(void);
void batteryRead(void);
void powerOff(void);