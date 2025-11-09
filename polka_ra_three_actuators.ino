#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* contractAddress = "0xd9145CCE52D386f254917e481eB44e9943F39138";

WiFiClientSecure client;
String lastCommand = "";

#define PUMP_PIN 2
#define LAMP_PIN 4
#define STATUS_LED_PIN 13

// Camera pins (ESP32-CAM)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_camera_init(&config);
  Serial.println("📸 Camera ready");
}

void capturePhoto() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (fb) {
    Serial.println("📸 Photo captured (" + String(fb->len) + " bytes)");
    esp_camera_fb_return(fb);
  } else {
    Serial.println("❌ Photo capture failed");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  initCamera();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  client.setInsecure();
}

String getCommand() {
  if (!client.connect("rpc.api.moonbase.moonbeam.network", 443)) return "";

  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0xebb48ea7\"},\"latest\"],\"id\":1}";
  
  client.print("POST / HTTP/1.1\r\n");
  client.print("Host: rpc.api.moonbase.moonbeam.network\r\n");
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
  Serial.println("🔧 Executing: " + cmd);
  
  if (cmd.indexOf("PUMP") >= 0) {
    digitalWrite(PUMP_PIN, HIGH);
    Serial.println("💧 Water pump ON");
    delay(3000);
    digitalWrite(PUMP_PIN, LOW);
    Serial.println("💧 Water pump OFF");
  }
  else if (cmd.indexOf("LAMP") >= 0) {
    digitalWrite(LAMP_PIN, HIGH);
    Serial.println("💡 Lamp ON");
    delay(3000);
    digitalWrite(LAMP_PIN, LOW);
    Serial.println("💡 Lamp OFF");
  }
  else if (cmd.indexOf("PHOTO") >= 0) {
    capturePhoto();
  }
  else if (cmd.indexOf("ALL") >= 0) {
    Serial.println("🔄 Running all actuators...");
    capturePhoto();
    delay(1000);
    digitalWrite(LAMP_PIN, HIGH);
    Serial.println("💡 Lamp ON");
    delay(2000);
    digitalWrite(PUMP_PIN, HIGH);
    Serial.println("💧 Pump ON");
    delay(3000);
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LAMP_PIN, LOW);
    Serial.println("✅ Sequence complete");
    capturePhoto();
  }
}

void loop() {
  Serial.println("📡 Checking blockchain...");
  
  String response = getCommand();
  
  if (response.indexOf("result") > 0 && response.indexOf("0x") > 0) {
    String currentCommand = "COMMAND_RECEIVED";
    
    if (currentCommand != lastCommand) {
      Serial.println("📨 New command from blockchain!");
      executeCommand(response);
      lastCommand = currentCommand;
    }
  }
  
  // Status blink
  digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
  
  delay(10000);
}
