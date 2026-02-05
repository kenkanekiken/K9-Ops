#pragma once
#include <Arduino.h>
#include <stdint.h>

void loraInit();
bool loraReceiveLine(String &outLine, int &outRssi, float &outSnr);
void loraSendCommand(char func, int v1, int v2, int v3); 