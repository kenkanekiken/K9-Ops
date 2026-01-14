#include <Arduino.h>
#include "firebase.h"
#include <Firebase_ESP_Client.h>

// ===== Firebase =====
// #define FIREBASE_HOST "https://k9ops-259ff-default-rtdb.asia-southeast1.firebasedatabase.app/" //kenneth
// #define FIREBASE_AUTH "FXlxJfJXEaN7ZQiz5lNzGEerfiENiL6KuF1rc3Xf"

#define FIREBASE_HOST "https://k9ops-2f11f-default-rtdb.asia-southeast1.firebasedatabase.app/" //FoundboxSG@gmail.com
#define FIREBASE_AUTH "BcwrEh7pDkIQteADlGTUzTMOmN9cfCakqbTghhi1"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void firebaseInit(void) {
  // Firebase config
  config.database_url = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH; // legacy token / secret method
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.RTDB.setBool(&fbdo, "/devices/trainer/power", true);
  Serial.println("Firebase ready");
  delay(3000);
}