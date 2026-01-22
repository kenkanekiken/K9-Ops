#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>

// Initialize the MQTT connection
void mqttInit();

// Keep the connection alive (run this in loop)
void mqttLoop();

#endif