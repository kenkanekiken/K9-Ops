#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "mqtt_handler.h"
#include "lora_module.h"

const char* mqtt_server = "test.mosquitto.org"; // Public testing server
const char* cmd_topic   = "k9ops/trainer/cmd";

WiFiClient espClient;
PubSubClient client(espClient);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (int i=0; i<length; i++) msg += (char)payload[i];
    
    Serial.print("[MQTT RX]: "); Serial.println(msg);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, msg);

    if (!error) {
        const char* target = doc["target"];
        if (target && strcmp(target, "Dog") == 0) {
            
            // CHECK FOR BUZZER COMMAND
            if (doc.containsKey("command") && strcmp(doc["command"], "buzzer") == 0) {
                int val = doc["value"]; // 1 or 0
                loraSendBuzzerCommand(val);
            }
            // CHECK FOR LED COMMAND
            else if (doc.containsKey("value") && doc["value"].is<JsonObject>()) {
                 int m = doc["value"]["mode"];
                 int c = doc["value"]["color"];
                 int b = doc["value"]["brightness"];
                 loraSendLedCommand(m, c, b);
            }
        }
    }
}

void mqttReconnect() {
    while (!client.connected()) {
        String id = "Trainer-" + String(WiFi.macAddress());
        if (client.connect(id.c_str())) {
            client.subscribe(cmd_topic);
            Serial.println("[MQTT] Connected & Subscribed");
        } else {
            delay(5000);
        }
    }
}

void mqttInit() {
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqttCallback);
}

void mqttLoop() {
    if (!client.connected()) mqttReconnect();
    client.loop();
}
