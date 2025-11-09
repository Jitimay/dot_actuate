#include <WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* server = "rpc.api.moonbase.moonbeam.network";
const char* contractAddress = "0xd9145CCE52D386f254917e481eB44e9943F39138";

WiFiClientSecure client;

#define PUMP_PIN 2
#define LED_PIN 13

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected - Ready for commands");
  
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
  Serial.println("Executing: " + cmd);
  
  if (cmd.indexOf("ACTIVATE_PUMP") >= 0) {
    digitalWrite(PUMP_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    Serial.println("✓ Pump activated");
  }
  else if (cmd.indexOf("STOP_PUMP") >= 0) {
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    Serial.println("✓ Pump stopped");
  }
  else if (cmd.indexOf("TAKE_PHOTO") >= 0) {
    Serial.println("✓ Photo captured");
  }
}

void loop() {
  Serial.println("Checking for new commands...");
  String response = getCommand();
  
  if (response.length() > 0 && response.indexOf("result") >= 0) {
    executeCommand(response);
  }
  
  delay(10000);
}
