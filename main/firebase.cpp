#include <Arduino.h>
#include "firebase.h"
#include <Firebase_ESP_Client.h>

// ===== Firebase =====
#define FIREBASE_HOST "https://k9ops-259ff-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "FXlxJfJXEaN7ZQiz5lNzGEerfiENiL6KuF1rc3Xf"

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
  Serial.println("Firebase ready");
}