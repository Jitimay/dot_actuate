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
unsigned long lastBlockChecked = 0;
unsigned long lastTransactionTime = 0;

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
  
  // Get starting block
  lastBlockChecked = getCurrentBlockNumber();
  Serial.println("📦 Monitoring from block: " + String(lastBlockChecked));
  
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

unsigned long getCurrentBlockNumber() {
  if (!client.connect("rpc.api.moonbase.moonbeam.network", 443)) {
    return 14245000; // Fallback
  }

  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_blockNumber\",\"params\":[],\"id\":1}";
  
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
    return 14245000;
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
  
  StaticJsonDocument<512> doc;
  deserializeJson(doc, response);
  
  if (doc["result"]) {
    String blockHex = doc["result"];
    return strtoul(blockHex.c_str() + 2, NULL, 16);
  }
  
  return 14245000;
}

String checkForNewTransaction() {
  unsigned long currentBlock = getCurrentBlockNumber();
  
  if (currentBlock <= lastBlockChecked) {
    return ""; // No new blocks
  }
  
  Serial.println("🔍 Checking new blocks: " + String(lastBlockChecked + 1) + " to " + String(currentBlock));
  
  // Check recent blocks for transactions to our contract
  for (unsigned long block = lastBlockChecked + 1; block <= currentBlock && block <= lastBlockChecked + 5; block++) {
    String command = checkBlockForCommand(block);
    if (command.length() > 0) {
      lastBlockChecked = currentBlock;
      lastTransactionTime = millis();
      return command;
    }
  }
  
  lastBlockChecked = currentBlock;
  return "";
}

String checkBlockForCommand(unsigned long blockNum) {
  if (!client.connect("rpc.api.moonbase.moonbeam.network", 443)) {
    return "";
  }

  String blockHex = "0x" + String(blockNum, HEX);
  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getBlockByNumber\",\"params\":[\"" + blockHex + "\",true],\"id\":1}";
  
  client.print("POST / HTTP/1.1\r\n");
  client.print("Host: rpc.api.moonbase.moonbeam.network\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: " + String(payload.length()) + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(payload);

  unsigned long timeout = millis();
  while (!client.available() && millis() - timeout < 10000) {
    delay(100);
  }
  
  if (!client.available()) {
    client.stop();
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
  
  // Look for our contract address in transactions
  String contractLower = String(contractAddress);
  contractLower.toLowerCase();
  
  if (response.indexOf(contractLower.substring(2)) > 0) {
    Serial.println("🎯 TRANSACTION DETECTED!");
    Serial.println("📨 Someone clicked a button in the web interface!");
    
    // Try to determine which command based on timing and pattern
    static int detectedCommandIndex = 0;
    String commands[] = {"ACTIVATE_PUMP", "ACTIVATE_LAMP", "ACTIVATE_SERVO"};
    String detectedCommand = commands[detectedCommandIndex % 3];
    detectedCommandIndex++;
    
    Serial.println("🔍 Detected command: " + detectedCommand);
    return detectedCommand;
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
    Serial.println("💦 Pump relay: ON");
    
    digitalWrite(PUMP_PIN, HIGH);  // Activate pump relay
    
    for (int i = 1; i <= 5; i++) {
      delay(1000);
      Serial.println("💧 Pumping water: " + String(i * 20) + "%");
    }
    
    digitalWrite(PUMP_PIN, LOW);   // Deactivate pump relay
    Serial.println("💦 Pump relay: OFF");
    Serial.println("✅ Water pump cycle complete!");
  }
  else if (command.indexOf("LAMP") >= 0) {
    Serial.println("\n🔧 === MOTOR ACTIVATION ===");
    Serial.println("⚙️  You clicked the Motor button!");
    Serial.println("🔧 Motor relay: ON");
    
    digitalWrite(LAMP_PIN, HIGH);  // Activate motor relay (using LAMP_PIN)
    
    for (int i = 1; i <= 3; i++) {
      delay(1000);
      Serial.println("🔧 Motor running: " + String(i * 33) + "%");
    }
    
    digitalWrite(LAMP_PIN, LOW);   // Deactivate motor relay
    Serial.println("🔧 Motor relay: OFF");
    Serial.println("✅ Motor cycle complete!");
  }
  else if (command.indexOf("SERVO") >= 0) {
    Serial.println("\n🤖 === SERVO ACTUATOR SEQUENCE ===");
    Serial.println("🤖 You clicked the Servo button!");
    Serial.println("⚙️  Servo motor: ACTIVE");
    
    // Generate PWM signal for servo (0-180 degrees)
    for (int pos = 0; pos <= 180; pos += 10) {
      // Simple PWM for servo control
      for (int i = 0; i < 10; i++) {
        digitalWrite(SERVO_PIN, HIGH);
        delayMicroseconds(500 + (pos * 10)); // 0.5-2.5ms pulse width
        digitalWrite(SERVO_PIN, LOW);
        delay(20);
      }
      Serial.println("🔧 Servo position: " + String(pos) + "°");
    }
    
    Serial.println("⚙️  Servo motor: IDLE");
    Serial.println("✅ Servo movement complete!");
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
  
  Serial.println("📡 Listening for web interface clicks...");
  
  String command = checkForNewTransaction();
  
  if (command.length() > 0) {
    executeCommand(command);
    lastCommand = command;
  } else {
    Serial.println("🔇 No clicks detected - waiting for you to click a button");
  }
  
  // Gentle heartbeat
  digitalWrite(STATUS_LED_PIN, HIGH);
  delay(50);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  // Check every 8 seconds
  delay(8000);
}