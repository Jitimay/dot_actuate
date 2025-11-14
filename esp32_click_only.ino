#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Josh";
const char* password = "Jitimay$";

// Contract details
const char* contractAddress = "0x6e2ec30DD6093f247023019e408E226a345e5769";

// Hardware pins
#define PUMP_PIN 2
#define LAMP_PIN 4
#define SERVO_PIN 5
#define STATUS_LED_PIN 13

WiFiClientSecure client;
String lastCommand = "";
unsigned long lastBlockChecked = 0; // This variable is no longer used after the change, but keeping it for now.
// unsigned long lastTransactionTime = 0; // Removed as it's no longer needed.

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("🏆 POLKA-RA - Click-Controlled Demo");
  Serial.println("🌟 Only executes when you click web buttons");
  Serial.println("🔗 Monitoring Moonbase Alpha blockchain");
  
  // Initialize hardware
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LAMP_PIN, LOW);
  digitalWrite(SERVO_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  Serial.println("🔧 Hardware initialized - 3 actuators ready");
  
  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("📶 Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✅ WiFi connected!");
  Serial.println("📍 IP: " + WiFi.localIP().toString());
  
  client.setInsecure();
  
  // No longer getting starting block here as we directly query the contract state
  // lastBlockChecked = getCurrentBlockNumber();
  // Serial.println("📦 Monitoring from block: " + String(lastBlockChecked));
  
  Serial.println("✅ SYSTEM READY!");
  Serial.println("🖱️  Click buttons in web interface to control hardware");
  Serial.println("🌐 Web interface: http://localhost:8080/web_moonbase_fixed.html");
  
  // Simple startup indicator
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(200);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(200);
  }
  
  Serial.println("🔄 Waiting for your clicks...\n");
}

String getLatestCommandFromContract() {
  if (!client.connect("rpc.api.moonbase.moonbeam.network", 443)) {
    Serial.println("❌ RPC connection failed for getLatestCommandFromContract");
    return "";
  }

  // Function selector for latestCommand() getter is 0x060cc0bc
  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0x060cc0bc\"},\"latest\"],\"id\":1}";
  Serial.println("RPC Payload: " + payload);
  
  client.print("POST / HTTP/1.1\r\n");
  client.print("Host: rpc.api.moonbase.moonbeam.network\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: " + String(payload.length()) + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(payload);

  unsigned long timeout = millis();
  while (!client.available() && millis() - timeout < 8000) {
    delay(100);
  }
  
  if (!client.available()) {
    client.stop();
    Serial.println("❌ No RPC response for getLatestCommandFromContract");
    return "";
  }
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }
  
  String response = "";
  while (client.available()) {
    response += (char)client.read();
  }
  client.stop();
  
  Serial.println("RPC Response: " + response);
  
  StaticJsonDocument<512> doc;
  deserializeJson(doc, response);
  
  if (doc["result"]) {
    String hexData = doc["result"].as<String>();
    Serial.println("Hex Data: " + hexData);

    if (hexData.length() < 130) { // Minimum length for 0x prefix, offset, and length of string
        Serial.println("❌ RPC result too short to parse command. Length: " + String(hexData.length()));
        return "";
    }

    // For a single string return:
    // 0x (2 chars) + offset_to_string (64 chars) + length_of_string (64 chars) + data_of_string
    // The offset to string is usually 0x20 (32) for the first string, meaning the data starts after 32 bytes (64 hex chars)
    // The length of the string is at hexData[66] for 64 chars (32 bytes)
    // The actual string data starts at hexData[130]

    // Extract length of the string
    String lengthHex = hexData.substring(66, 66 + 64); // 32 bytes for length
    unsigned int length = strtoul(lengthHex.c_str(), NULL, 16);
    Serial.println("Length Hex: " + lengthHex + ", Length: " + String(length));

    if (length == 0) {
        return ""; // No command set
    }

    // Extract the actual command string
    String commandHex = hexData.substring(130, 130 + (length * 2));
    Serial.println("Command Hex: " + commandHex);
    String command = "";
    for (unsigned int i = 0; i < commandHex.length(); i += 2) {
      char c = (char)strtol(commandHex.substring(i, i + 2).c_str(), NULL, 16);
      command += c;
    }
    return command;
  }
  
  return "";
}

void executeCommand(String command) {
  Serial.println("\n🚀 ===== EXECUTING YOUR CLICK =====");
  Serial.println("⚡ Command: " + command);
  Serial.println("🖱️  Triggered by web interface click!");
  
  // Click response LED sequence
  for (int i = 0; i < 5; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(100);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(100);
  }
  
  if (command.indexOf("PUMP") >= 0) {
    Serial.println("\n💧 === WATER PUMP ACTIVATION ===");
    Serial.println("🌊 You clicked the Water Pump button!");
    Serial.println("💦 Pump motor: ON");
    
    digitalWrite(PUMP_PIN, HIGH);
    
    for (int i = 1; i <= 4; i++) {
      delay(1000);
      Serial.println("💧 Irrigation progress: " + String(i * 25) + "%");
    }
    
    digitalWrite(PUMP_PIN, LOW);
    Serial.println("💦 Pump motor: OFF");
    Serial.println("✅ Water pump cycle complete!");
  }
  else if (command.indexOf("LAMP") >= 0) {
    Serial.println("\n💡 === GROW LAMP ACTIVATION ===");
    Serial.println("🌱 You clicked the Lamp button!");
    Serial.println("💡 LED array: ON");
    
    digitalWrite(LAMP_PIN, HIGH);
    
    for (int i = 1; i <= 6; i++) {
      delay(1000);
      Serial.println("🌞 Light therapy progress: " + String(i * 16) + "%");
    }
    
    digitalWrite(LAMP_PIN, LOW);
    Serial.println("💡 LED array: OFF");
    Serial.println("✅ Grow lamp cycle complete!");
  }
  else if (command.indexOf("SERVO") >= 0) {
    Serial.println("\n🔧 === SERVO ACTUATOR SEQUENCE ===");
    Serial.println("🤖 You clicked the Servo button!");
    Serial.println("⚙️  Servo motor: ACTIVE");
    
    for (int i = 0; i < 20; i++) {
      digitalWrite(SERVO_PIN, HIGH);
      delay(100);
      digitalWrite(SERVO_PIN, LOW);
      delay(100);
      
      if (i % 4 == 0) {
        Serial.println("🔧 Adjustment progress: " + String((i + 1) * 5) + "%");
      }
    }
    
    Serial.println("⚙️  Servo motor: IDLE");
    Serial.println("✅ Servo adjustment complete!");
  }
  
  Serial.println("\n🎉 === CLICK RESPONSE COMPLETE ===");
  Serial.println("✅ Hardware responded to your web click!");
  Serial.println("🖱️  Ready for your next click...");
  
  // Success LED sequence
  for (int i = 0; i < 4; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(300);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(300);
  }
  
  Serial.println("================================\n");
}

void loop() {
  // WiFi check
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("📶 Reconnecting WiFi...");
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }
  
  Serial.println("📡 Polling smart contract for latest command...");
  
  String currentCommand = getLatestCommandFromContract();
  
  if (currentCommand.length() > 0 && currentCommand != lastCommand) {
    Serial.println("✅ New command detected: " + currentCommand);
    executeCommand(currentCommand);
    lastCommand = currentCommand;
  } else if (currentCommand.length() == 0) {
    Serial.println("🔇 No command set in contract.");
  } else {
    Serial.println("🔄 Command unchanged: " + currentCommand);
  }
  
  // Gentle heartbeat
  digitalWrite(STATUS_LED_PIN, HIGH);
  delay(50);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  // Poll every 8 seconds
  delay(8000);
}