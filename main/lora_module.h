#pragma once
#include <stdint.h>

// Latest received values (optional for other files)
extern float rxLat;
extern float rxLng;
extern bool  rxGpsOnline;

extern float rxTemp;
extern int   rxPct;
extern bool  rxPower;

extern uint32_t lastPacketMs;

void loraInit(void);
void loraRead(void);