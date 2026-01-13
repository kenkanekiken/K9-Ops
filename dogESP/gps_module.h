#pragma once
#include <stdint.h>

extern float lat;
extern float lng;
extern bool gpsOnline;

void pmicInit(void);
void gpsInit(void);
void gpsRead(void);