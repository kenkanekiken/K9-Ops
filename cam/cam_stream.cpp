#include <Arduino.h>
#include "cam_stream.h"

// Snapshot + Streaming
#include "esp_camera.h"
#include <WiFi.h>
// Recommended for ESP32-CAM stability
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ========== WIFI ==========
const char* ssid = "K&K";
const char* password = "Songbird7108";

// ========== AI THINKER ESP32-CAM PINS ==========
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WiFiServer server(80);

// Simple page
static const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
  <head>
    <meta charset="utf-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1"/>
    <title>ESP32-CAM Stream</title>
    <style>
      body { font-family: Arial, sans-serif; padding: 16px; }
      button { padding: 10px 14px; font-size: 16px; margin-right: 8px; }
      img { margin-top: 12px; max-width: 100%; height: auto; border: 1px solid #ccc; }
      .row { margin-top: 10px; }
      .small { color: #666; font-size: 14px; margin-top: 8px; }
    </style>
  </head>
  <body>
    <h2>ESP32-CAM Snapshot + Stream</h2>
    <div class="row">
      <button onclick="snap()">Snapshot</button>
      <button onclick="stream()">Start Stream</button>
      <button onclick="stopStream()">Stop Stream</button>
    </div>
    <div class="small">
      Snapshot = /capture &nbsp; | &nbsp; Stream = /stream
    </div>
    <img id="img" alt="image will appear here"/>
    <script>
      function snap(){
        const img = document.getElementById('img');
        img.src = '/capture?t=' + Date.now();
      }
      function stream(){
        const img = document.getElementById('img');
        img.src = '/stream';
      }
      function stopStream(){
        const img = document.getElementById('img');
        img.src = '';
      }
    </script>
  </body>
</html>
)rawliteral";

void cameraInit(void) {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Stream
  config.frame_size   = FRAMESIZE_QVGA;  // Framesize - how big the image will be
  config.jpeg_quality = 18; // Compression - Higher the value, higher compression, Lower the value, better quality
  config.fb_count     = 2; // Buffer - How many frames can be held in memory at once / 1 or 2 buffer
  // Snapshot
  // config.frame_size = FRAMESIZE_VGA;
  // config.jpeg_quality = 10;
  // config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    while (true) delay(1000);
  }
  
  // Orientation fix (upside down)
  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
}

void serverInit(void) {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("Open: http://");
  Serial.println(WiFi.localIP());
  server.begin();
}

void sendIndex(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Connection: close");
  client.println();
  client.write((const uint8_t*)index_html, strlen(index_html));
}

void sendSnapshot(WiFiClient &client) {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Camera capture failed");
    return;
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.println("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
  client.println("Pragma: no-cache");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(fb->len);
  client.println();

  client.write(fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void sendStream(WiFiClient &client) {
  // MJPEG stream headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println("Cache-Control: no-cache");
  client.println("Connection: close");
  client.println();

  // Stream loop: keep sending frames until client disconnects
  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Stream capture failed");
      break;
    }

    client.println("--frame");
    client.println("Content-Type: image/jpeg");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();

    client.write(fb->buf, fb->len);
    client.println();

    esp_camera_fb_return(fb);

    // Small delay = smoother + less crash risk
    delay(80);
  }
}

void handleClient(WiFiClient &client) {
  String req = client.readStringUntil('\r');
  client.readStringUntil('\n');

  // Drain headers
  while (client.connected() && client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r" || line.length() == 1) break;
  }

  if (req.indexOf("GET /stream") >= 0) {
    Serial.println("Client streaming...");
    sendStream(client);
    Serial.println("Stream ended.");
    return;
  }

  if (req.indexOf("GET /capture") >= 0) {
    sendSnapshot(client);
    return;
  }

  sendIndex(client);
}

void serverRead(void) {
  WiFiClient client = server.available();
  if (!client) return;

  unsigned long start = millis();
  while (!client.available() && (millis() - start < 1500)) {
    delay(1);
  }

  if (client.available()) {
    handleClient(client);
  }

  delay(1);
  client.stop();
}