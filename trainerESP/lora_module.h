#ifndef LORA_MODULE_H
#define LORA_MODULE_H

#include <Arduino.h>

void loraInit(void);
bool loraReceiveLine(String &outLine, int &outRssi, float &outSnr);
void loraSendLedCommand(int mode, int color, int brightness);

// NEW: Buzzer function definition
void loraSendBuzzerCommand(int state); 

#endif
