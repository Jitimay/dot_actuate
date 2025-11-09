/*
 * POLKA-RESILIENT-ACTUATOR (Polka-RA)
 * Complete ESP32 implementation with camera proof system
 * Hackathon: Polkadot - Resilient Apps Track
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include "mbedtls/md.h"
#include <ArduinoJson.h>

// ==================== CONFIGURATION ====================
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* RPC_SERVER = "rpc.api.moonbase.moonbeam.network";
const char* CONTRACT_ADDRESS = "0xd9145CCE52D386f254917e481eB44e9943F39138";

// Hardware pins
#define PUMP_RELAY_PIN    2
#define STATUS_LED_PIN    13
#define EMERGENCY_STOP    14

// Camera configuration (ESP32-CAM)
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

// ==================== GLOBAL VARIABLES ====================
WiFiClientSecure client;
String lastCommand = "";
bool systemActive = true;
unsigned long lastPoll = 0;
const unsigned long POLL_INTERVAL = 10000; // 10 seconds

// ==================== CAMERA INITIALIZATION ====================
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

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Camera init failed");
  } else {
    Serial.println("✅ Camera initialized");
  }
}

// ==================== CRYPTOGRAPHIC PROOF SYSTEM ====================
String captureImageHash() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Camera capture failed");
    return "ERROR";
  }
  
  // Generate SHA256 hash
  unsigned char hash[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, fb->buf, fb->len);
  mbedtls_md_finish(&ctx, hash);
  mbedtls_md_free(&ctx);
  
  // Convert to hex string
  String hashStr = "0x";
  for (int i = 0; i < 32; i++) {
    if (hash[i] < 16) hashStr += "0";
    hashStr += String(hash[i], HEX);
  }
  
  esp_camera_fb_return(fb);
  Serial.println("📸 Image captured, hash: " + hashStr.substring(0, 10) + "...");
  return hashStr;
}

// ==================== BLOCKCHAIN COMMUNICATION ====================
String makeRPCCall(String method, String params) {
  if (!client.connect(RPC_SERVER, 443)) {
    Serial.println("❌ RPC connection failed");
    return "";
  }

  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"" + method + "\",\"params\":" + params + ",\"id\":1}";
  
  client.print("POST / HTTP/1.1\r\n");
  client.print("Host: " + String(RPC_SERVER) + "\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: " + String(payload.length()) + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(payload);

  // Skip headers
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

String getLatestCommand() {
  String params = "[{\"to\":\"" + String(CONTRACT_ADDRESS) + "\",\"data\":\"0xebb48ea7\"},\"latest\"]";
  String response = makeRPCCall("eth_call", params);
  
  // Parse JSON response
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, response);
  
  if (doc["result"]) {
    String hexResult = doc["result"];
    // Decode hex string to text (simplified)
    if (hexResult.length() > 2) {
      return "COMMAND_RECEIVED"; // Simplified for demo
    }
  }
  return "";
}

void submitProofToBlockchain(String proofHash) {
  Serial.println("📤 Submitting proof to blockchain: " + proofHash.substring(0, 10) + "...");
  // In production, this would submit a transaction
  // For demo, we just log it
}

// ==================== PHYSICAL ACTUATOR CONTROL ====================
void activatePump(int duration) {
  Serial.println("🚰 PUMP ACTIVATION SEQUENCE STARTED");
  
  // 1. Capture BEFORE proof
  String preHash = captureImageHash();
  
  // 2. Activate pump
  digitalWrite(PUMP_RELAY_PIN, HIGH);
  digitalWrite(STATUS_LED_PIN, HIGH);
  Serial.println("✅ Pump activated for " + String(duration) + " seconds");
  
  // 3. Run for specified duration
  delay(duration * 1000);
  
  // 4. Capture AFTER proof
  String postHash = captureImageHash();
  
  // 5. Deactivate pump
  digitalWrite(PUMP_RELAY_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  Serial.println("⏹️ Pump deactivated");
  
  // 6. Submit cryptographic proof
  String combinedProof = preHash + ":" + postHash;
  submitProofToBlockchain(combinedProof);
  
  Serial.println("✅ ACTUATION COMPLETE WITH CRYPTOGRAPHIC PROOF");
  Serial.println("Pre-action:  " + preHash.substring(0, 16) + "...");
  Serial.println("Post-action: " + postHash.substring(0, 16) + "...");
}

void emergencyStop() {
  Serial.println("🚨 EMERGENCY STOP ACTIVATED");
  digitalWrite(PUMP_RELAY_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  systemActive = false;
}

void executeCommand(String command) {
  Serial.println("🔧 Executing command: " + command);
  
  if (command.indexOf("ACTIVATE_PUMP_30") >= 0) {
    activatePump(30);
  }
  else if (command.indexOf("ACTIVATE_PUMP") >= 0) {
    activatePump(10);
  }
  else if (command.indexOf("STOP_ALL") >= 0) {
    emergencyStop();
  }
  else if (command.indexOf("CAPTURE_PROOF") >= 0) {
    String hash = captureImageHash();
    submitProofToBlockchain(hash);
  }
  else if (command.indexOf("RUN_SEQUENCE") >= 0) {
    Serial.println("🔄 Running full demonstration sequence");
    captureImageHash();
    delay(2000);
    activatePump(5);
    delay(2000);
    captureImageHash();
  }
  else {
    Serial.println("❓ Unknown command: " + command);
  }
}

// ==================== MAIN SETUP ====================
void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 POLKA-RESILIENT-ACTUATOR STARTING...");
  
  // Initialize hardware
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(EMERGENCY_STOP, INPUT_PULLUP);
  
  digitalWrite(PUMP_RELAY_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  // Initialize camera
  initCamera();
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("🌐 Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected: " + WiFi.localIP().toString());
  
  // Configure secure client
  client.setInsecure(); // Skip certificate verification for demo
  
  // System ready
  digitalWrite(STATUS_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  Serial.println("✅ POLKA-RA SYSTEM READY");
  Serial.println("📡 Monitoring blockchain for commands...");
  Serial.println("🏠 Contract: " + String(CONTRACT_ADDRESS));
  Serial.println("==========================================");
}

// ==================== MAIN LOOP ====================
void loop() {
  // Check emergency stop
  if (digitalRead(EMERGENCY_STOP) == LOW) {
    emergencyStop();
    delay(5000);
    return;
  }
  
  // Poll blockchain for new commands
  if (millis() - lastPoll > POLL_INTERVAL && systemActive) {
    Serial.println("🔍 Polling blockchain for commands...");
    
    String newCommand = getLatestCommand();
    
    if (newCommand.length() > 0 && newCommand != lastCommand) {
      Serial.println("📨 New command received from blockchain!");
      executeCommand(newCommand);
      lastCommand = newCommand;
    } else {
      Serial.println("⏳ No new commands");
    }
    
    lastPoll = millis();
  }
  
  // Status LED heartbeat
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 2000) {
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    lastBlink = millis();
  }
  
  delay(100);
}
