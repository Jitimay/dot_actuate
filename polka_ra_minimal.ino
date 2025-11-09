#include <WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* contractAddress = "0xd9145CCE52D386f254917e481eB44e9943F39138";

WiFiClientSecure client;
String lastCommand = "";

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

void loop() {
  Serial.println("Checking blockchain for commands...");
  
  String response = getCommand();
  
  if (response.indexOf("result") > 0 && response.indexOf("0x") > 0) {
    String currentCommand = "COMMAND_RECEIVED";
    
    if (currentCommand != lastCommand) {
      Serial.println("New command received!");
      
      // Execute pump action
      digitalWrite(PUMP_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      Serial.println("✅ Pump activated");
      
      delay(5000);
      
      digitalWrite(PUMP_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      Serial.println("⏹️ Pump stopped");
      
      lastCommand = currentCommand;
    }
  }
  
  delay(10000);
}
