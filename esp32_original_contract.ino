#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* CONTRACT_ADDRESS = "0xd9145CCE52D386f254917e481eB44e9943F39138";

WiFiClientSecure client;
String lastCommand = "";

#define PUMP_PIN 2
#define LED_PIN 13

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  client.setInsecure();
}

String getLatestCommand() {
  if (!client.connect("rpc.api.moonbase.moonbeam.network", 443)) return "";

  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(CONTRACT_ADDRESS) + "\",\"data\":\"0xebb48ea7\"},\"latest\"],\"id\":1}";
  
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
  
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, response);
  
  if (doc["result"]) {
    String hexResult = doc["result"];
    if (hexResult != "0x") {
      return "COMMAND_FOUND";
    }
  }
  return "";
}

void executeCommand(String cmd) {
  Serial.println("Executing: " + cmd);
  
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  Serial.println("✅ Pump activated");
  
  delay(5000);
  
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  Serial.println("⏹️ Pump stopped");
}

void loop() {
  Serial.println("Checking blockchain...");
  
  String newCommand = getLatestCommand();
  
  if (newCommand.length() > 0 && newCommand != lastCommand) {
    Serial.println("📨 New command received!");
    executeCommand(newCommand);
    lastCommand = newCommand;
  }
  
  delay(10000);
}
