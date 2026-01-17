#pragma once
#include <Arduino.h>
#include <stdint.h>

void loraInit();
bool loraReceiveLine(String &outLine, int &outRssi, float &outSnr);
void loraSendLedCommand(int mode, int color, int brightness);