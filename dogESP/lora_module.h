#pragma once
#include <stdint.h>

void loraInit(void);
void loraRead(void);
// GPS 
void loraSendGps(float lat, float lng);
void loraSendGpsStatus(bool gpsStatus);
// DHT Temperature
void loraSendTemp(float temp);
// Battery / Power Off
void loraSendBattery(int battery);
void loraSendPower(bool power);
// MPU
void loraSendMovement(float ax, float ay, float az, float motion, const String& state, int stepCount);