#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LoRa.h>  // Required to send LoRa packets
#include "mqtt_handler.h"
#include "lora_module.h"

// Configuration
const char* mqtt_server = "test.mosquitto.org";
const char* cmd_topic = "k9ops/trainer/cmd";

WiFiClient espClient;
PubSubClient client(espClient);

// This function runs when an MQTT message arrives from Flutter
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) { message += (char)payload[i]; }
    
    Serial.print("[MQTT] Received: ");
    Serial.println(message);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("JSON Parse failed");
        Serial.println(error.f_str());
        return;
    }

    const char* target = doc["target"];
    const char* cmd = doc["cmd"]; // Get the command type ("LED" or "VIBE")


    if (target != nullptr && strcmp(target, "Dog") == 0) {

            if (cmd != nullptr && strcmp(cmd, "LED") == 0) {
                Serial.println("[LoRa] Forwarding LED command to Dog...");
                int mode = doc["value"]["mode"];
                int color = doc["value"]["color"];
                int brightness = doc["value"]["brightness"];

                loraSendCommand('L' ,mode, color, brightness);
            }

            else if (cmd != nullptr && strcmp(cmd, "Vibration") == 0) {
            Serial.println("[LoRa] Forwarding Vibration command to Dog...");

                int vibMode = doc["value"]["mode"]; 
            
                // Call your LoRa vibration function
                loraSendCommand('V' ,vibMode, 0, 0);    

            }
    }
}

void mqttReconnect() {
    while (!client.connected()) {
        Serial.print("[MQTT] Attempting connection...");
        // Unique ID for the trainer
        String clientId = "K9Trainer-" + String(WiFi.macAddress());
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            client.subscribe(cmd_topic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            delay(5000);
        }
    }
}

void mqttInit() {
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqttCallback);
}

void mqttLoop() {
    if (!client.connected()) {
        mqttReconnect();
    }
    client.loop();
}