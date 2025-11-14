#include "esp_camera.h"
#include <Arduino.h> // For Serial.begin and delay

// ESP32-CAM (AI-Thinker) PIN Map - Adjust if your board is different
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

#define LED_GPIO_NUM      33 // Flash LED (optional, not used in this test)

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("ESP32-CAM Camera Test Sketch");

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
  config.psram_ctrl_en = false; // Explicitly disable PSRAM for testing stability
  
  // Force the absolute lowest resolution for maximum memory stability
  config.frame_size = FRAMESIZE_QQVGA; // 160x120
  config.jpeg_quality = 12;
  config.fb_count = 1; // Use only one frame buffer


  Serial.println("Attempting to initialize camera...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    Serial.println("Possible causes:");
    Serial.println("- Incorrect board model selected in Arduino IDE (Tools > Board)");
    Serial.println("- Incorrect camera module pin definitions in sketch");
    Serial.println("- Camera module not properly connected");
    Serial.println("- Faulty camera module");
    return;
  }
  Serial.println("Camera initialized successfully!");

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped horizontally and vertically
  s->set_vflip(s, 1); // flip it back
  s->set_hmirror(s, 1); // flip it back

  Serial.println("Attempting to capture a single frame...");
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed!");
    return;
  }
  Serial.printf("Frame captured! Size: %zu bytes, Format: %d\n", fb->len, fb->format);
  esp_camera_fb_return(fb); // Return the frame buffer
  Serial.println("Frame buffer returned.");
  Serial.println("Camera test complete.");
}

void loop() {
  // Nothing to do in loop for this test
  delay(1000);
}
