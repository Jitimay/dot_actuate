#include <WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* server = "rpc.api.moonbase.moonbeam.network";
const char* contractAddress = "0xd9145CCE52D386f254917e481eB44e9943F39138";

WiFiClientSecure client;
bool commandSet = false;

void makeRequest(String method, String params) {
  if (!client.connect(server, 443)) {
    Serial.println("Connection failed");
    return;
  }

  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"" + method + "\",\"params\":" + params + ",\"id\":1}";
  
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
  
  while (client.available()) {
    Serial.print((char)client.read());
  }
  Serial.println();
  client.stop();
}

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  client.setInsecure();
}

void loop() {
  if (!commandSet) {
    Serial.println("Checking if contract exists...");
    makeRequest("eth_getCode", "[\"" + String(contractAddress) + "\",\"latest\"]");
    commandSet = true;
  } else {
    Serial.println("Reading latest command...");
    makeRequest("eth_call", "[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0xebb48ea7\"},\"latest\"]");
  }

  delay(10000);
}
