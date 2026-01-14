#pragma once
#include <stdint.h>

// ================= Init =================
void loraInit(void);
void loraRead(void);
// ================= GPS =================
void loraSendGps(float lat, float lng);
void loraSendGpsStatus(bool gpsStatus);
// ================= DHT Temperature =================
void loraSendTemp(float temp);
// ================= Battery / Power =================
void loraSendBattery(int battery);
void loraSendPower(bool power);
// ================= MPU Movement =================
void loraSendMovement(float ax, float ay, float az, float motion, const String& state, int stepCount);
void loraSendVelocity(float distance_in_meter, float distance_in_km, float speed);