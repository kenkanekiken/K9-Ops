#include <Arduino.h>
#include "ble_module.h"
//BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHAR_COMMAND_UUID   "abcd1234-1234-1234-1234-abcdef123456"

class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    String cmd = pChar->getValue();
    if (cmd.length() > 0) {
      Serial.print("Received BLE cmd: ");
      Serial.println(cmd);
      if (cmd == "Recall") {
        Serial.println("Sound the buzzer");
      }
    }
  }
};

void bleInit(void) {
  // *Pointer flow diagram for references
  //BLEDevice
  // ↓ createServer()
  //BLEServer
  // ↓ createService(UUID)
  //BLEService
  // ↓ createCharacteristic(UUID)
  //BLECharacteristic

  BLEDevice::init("TBeam_BLE");
  BLEServer *server = BLEDevice::createServer(); //Call the createServer() function that belongs to the BLEDevice class, and store the pointer it returns in server.

  BLEService *service = server->createService(SERVICE_UUID); //Server point to createService() and store the pointer it return in service?

  BLECharacteristic *commandChar = service->createCharacteristic(
    CHAR_COMMAND_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR
  );

  commandChar->setCallbacks(new CommandCallback());

  service->start();
  BLEDevice::getAdvertising()->start();

  Serial.println("BLE ready. Use LightBlue to send commands.");
}

