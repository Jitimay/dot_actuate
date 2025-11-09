#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include "mbedtls/md.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* server = "rpc.api.moonbase.moonbeam.network";
const char* contractAddress = "0xd9145CCE52D386f254917e481eB44e9943F39138";

WiFiClientSecure client;

#define PUMP_PIN 2
#define LED_PIN 13

// Camera pins for ESP32-CAM
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

void initCamera() {
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
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_camera_init(&config);
}

String captureAndHash() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) return "ERROR";
  
  // Create SHA256 hash of image
  unsigned char hash[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, fb->buf, fb->len);
  mbedtls_md_finish(&ctx, hash);
  mbedtls_md_free(&ctx);
  
  // Convert to hex string
  String hashStr = "";
  for (int i = 0; i < 32; i++) {
    hashStr += String(hash[i], HEX);
  }
  
  esp_camera_fb_return(fb);
  return hashStr;
}

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  initCamera();
  Serial.println("Camera initialized");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nReady for blockchain commands");
  
  client.setInsecure();
}

String getCommand() {
  if (!client.connect(server, 443)) return "";

  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0xebb48ea7\"},\"latest\"],\"id\":1}";
  
  client.print("POST / HTTP/1.1\r\n");
  client.print("Host: " + String(server) + "\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: " + String(payload.length()) + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(payload);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }
  
  String response = "";
  while (client.available()) {
    response += (char)client.read();
  }
  client.stop();
  return response;
}

void executeCommand(String cmd) {
  if (cmd.indexOf("ACTIVATE_PUMP") >= 0) {
    Serial.println("=== EXECUTING PUMP ACTIVATION ===");
    
    // 1. Capture BEFORE image
    Serial.println("📸 Capturing pre-action proof...");
    String preHash = captureAndHash();
    Serial.println("Pre-hash: " + preHash);
    
    // 2. Execute action
    Serial.println("🔧 Activating pump...");
    digitalWrite(PUMP_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(5000); // Run pump for 5 seconds
    
    // 3. Capture AFTER image
    Serial.println("📸 Capturing post-action proof...");
    String postHash = captureAndHash();
    Serial.println("Post-hash: " + postHash);
    
    // 4. Stop pump
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("✅ Action completed with cryptographic proof!");
    Serial.println("Pre: " + preHash.substring(0,8) + "...");
    Serial.println("Post: " + postHash.substring(0,8) + "...");
  }
}

void loop() {
  Serial.println("Polling blockchain for commands...");
  String response = getCommand();
  
  if (response.length() > 0 && response.indexOf("result") >= 0) {
    executeCommand(response);
  }
  
  delay(15000);
}
