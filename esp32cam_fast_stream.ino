#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include "mbedtls/sha256.h"

const char* ssid = "Josh";
const char* password = "Jitimay$";

WebServer server(81);

String generateImageHash(camera_fb_t* fb) {
  unsigned char hash[32];
  mbedtls_sha256_context ctx;
  
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(&ctx, fb->buf, fb->len);
  mbedtls_sha256_finish(&ctx, hash);
  mbedtls_sha256_free(&ctx);
  
  String hashString = "";
  for(int i = 0; i < 32; i++) {
    if(hash[i] < 16) hashString += "0";
    hashString += String(hash[i], HEX);
  }
  return hashString;
}

void handleCapture() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera Error");
    return;
  }
  
  String imageHash = generateImageHash(fb);
  Serial.println("📸 Image Hash: " + imageHash);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("X-Image-Hash", imageHash);
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void handleHash() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera Error");
    return;
  }
  
  String imageHash = generateImageHash(fb);
  esp_camera_fb_return(fb);
  
  Serial.println("📸 Image Hash: " + imageHash);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", imageHash);
}

void handleStream() {
  WiFiClient client = server.client();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println("Access-Control-Allow-Origin: *");
  client.println();
  
  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (fb) {
      client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
      client.write(fb->buf, fb->len);
      client.print("\r\n");
      esp_camera_fb_return(fb);
    }
    delay(100); // Slower frame rate
  }
}

void setup() {
  Serial.begin(115200);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;  // Smaller size
  config.jpeg_quality = 15;           // Lower quality
  config.fb_count = 1;

  esp_camera_init(&config);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  Serial.print("✅ Fast Stream: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":81/stream");
  Serial.print("📸 Capture: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":81/capture");
  Serial.print("🔐 Hash: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":81/hash");
  
  server.on("/stream", handleStream);
  server.on("/capture", handleCapture);
  server.on("/hash", handleHash);
  server.begin();
}

void loop() {
  server.handleClient();
}
