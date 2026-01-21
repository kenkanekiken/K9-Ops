#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// =====================
// AP (Router) Settings
// =====================
const char* AP_SSID = "K9-Ops-Vest-Cam";
const char* AP_PASS = "12345678";

// =====================
// YOUR CAMERA PIN MAP (Freenove S3)
// =====================
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     5
#define Y9_GPIO_NUM       4
#define Y8_GPIO_NUM       6
#define Y7_GPIO_NUM       7
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       17
#define Y4_GPIO_NUM       21
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       16
#define VSYNC_GPIO_NUM    1
#define HREF_GPIO_NUM     2
#define PCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM     8
#define SIOC_GPIO_NUM     9
#define LED_GPIO_NUM      47

WebServer server(80);

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  // Boost XCLK to 20MHz for faster data transfer
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // ===== MAX QUALITY SETTINGS (UXGA) =====
  if (psramFound()) {
      // 1. Resolution: UXGA (1600x1200) - The sensor's maximum
      config.frame_size = FRAMESIZE_UXGA; 
      
      // 2. Quality: 6 (0-63). 
      // 10 is "High", 6 is "Very High". 
      // Going lower than 6 often crashes the buffer or provides diminishing returns.
      config.jpeg_quality = 6; 
      
      // 3. Buffer Count: 1
      // UXGA frames are massive. 2 buffers might crash your RAM.
      // We accept lower FPS for stability.
      config.fb_count = 1;
      
      // 4. Grab Mode
      config.grab_mode = CAMERA_GRAB_LATEST; 
  } else {
      // Fallback if PSRAM fails
      config.frame_size = FRAMESIZE_SVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init failed: 0x%x\n", err);
    return false;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    // Optional: Boost saturation/contrast slightly for "richer" look
    s->set_saturation(s, 1); 
  }

  Serial.println("✅ Camera init OK (UXGA Mode)");
  return true;
}

// =====================
// Stream Handler
// =====================
void handleStream() {
  WiFiClient client = server.client();

  // Send MJPEG Header
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n"
  );

  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Frame capture failed");
      continue;
    }

    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.print("Content-Length: " + String(fb->len) + "\r\n\r\n");
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    esp_camera_fb_return(fb);
    
    // Tiny delay to let the wifi stack breathe processing huge packets
    delay(50); 
  }
}

// =====================
// Main Setup
// =====================
void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial time to wake up

  Serial.println("\nBooting K9-Ops 2K Cam...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("✅ AP IP: ");
  Serial.println(WiFi.softAPIP()); 

  if (!initCamera()) {
    Serial.println("❌ Camera Error");
    // Don't halt, let's see if it prints error code
  }

  server.on("/", [](){
    server.send(200, "text/html", "<h2><a href='/stream'>Start 2K Stream</a></h2>");
  });
  server.on("/stream", handleStream);
  server.begin();

  Serial.println("✅ Server Started");
}

void loop() {
  server.handleClient();
}
