#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// =====================
// YOUR WIFI
// =====================
const char* WIFI_SSID = "Jun Leis S23+";
const char* WIFI_PASS = "lmaoooooo";

// =====================
// YOUR CAMERA PIN MAP
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

// =====================
WebServer server(80);

// =====================
// Camera Init
// =====================
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

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // ===== Quality vs Stability =====
  config.frame_size   = FRAMESIZE_VGA;   // 640x480 (stable + clear)
  config.jpeg_quality = 10;               // lower = better quality
  config.fb_count     = 2;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.grab_mode    = CAMERA_GRAB_LATEST;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init failed: 0x%x\n", err);
    return false;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_vflip(s, 1);     
    s->set_hmirror(s, 1);
  }

  Serial.println("✅ Camera init OK");
  return true;
}

// =====================
// Routes
// =====================
void handleRoot() {
  String html =
    "<h2>ESP32-S3 Camera</h2>"
    "<ul>"
    "<li><a href='/jpg'>/jpg</a></li>"
    "<li><a href='/stream'>/stream</a></li>"
    "</ul>";
  server.send(200, "text/html", html);
}

void handleJpg() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera failed");
    return;
  }

  WiFiClient client = server.client();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.println("Content-Length: " + String(fb->len));
  client.println();
  client.write(fb->buf, fb->len);

  esp_camera_fb_return(fb);
}

void handleStream() {
  char boundary[] = "frame";
  WiFiClient client = server.client();

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  
  // ADDED THE CORS HEADER BELOW
  server.sendContent("HTTP/1.1 200 OK\r\n"
                     "Access-Control-Allow-Origin: *\r\n" // <--- CRITICAL LINE
                     "Content-Type: multipart/x-mixed-replace; boundary=" + String(boundary) + "\r\n\r\n");

  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      delay(1);
      continue;
    }

    String header = "--" + String(boundary) + "\r\n";
    header += "Content-Type: image/jpeg\r\n";
    header += "Content-Length: " + String(fb->len) + "\r\n\r\n";
    
    server.sendContent(header);
    client.write(fb->buf, fb->len);
    server.sendContent("\r\n");

    esp_camera_fb_return(fb);
    delay(20); 
  }
}

// =====================
// Setup
// =====================
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println("\nBooting...");
  Serial.print("PSRAM: ");
  Serial.println(psramFound() ? "OK" : "NOT FOUND");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (!initCamera()) {
    Serial.println("❌ Camera error");
    while (true);
  }

  server.on("/", handleRoot);
  server.on("/jpg", handleJpg);
  server.on("/stream", handleStream);
  server.begin();

  Serial.println("✅ Web server started");
  Serial.println("================================");
  Serial.print("OPEN  : http://"); Serial.println(WiFi.localIP());
  Serial.print("JPEG  : http://"); Serial.print(WiFi.localIP()); Serial.println("/jpg");
  Serial.print("STREAM: http://"); Serial.print(WiFi.localIP()); Serial.println("/stream");
  Serial.println("================================");
}

void loop() {
  server.handleClient();
}
