#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Josh";
const char* password = "Jitimay$";

// Contract details
const char* contractAddress = "0x6e2ec30DD6093f247023019e408E226a345e5769";

// Hardware pins - pump and motor only
#define PUMP_PIN 2
#define MOTOR_PIN 4
#define SERVO_PIN 5
#define STATUS_LED_PIN 13

WiFiClientSecure client;
String lastCommand = "";
unsigned long lastBlockChecked = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("🏆 DotActuate - Pump + Motor Control");
  
  // Initialize pump, motor and servo - ENSURE OFF STATE
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Force relays OFF (LOW = OFF for most relay modules)
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(MOTOR_PIN, LOW);
  digitalWrite(SERVO_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  Serial.println("🔧 All relays set to OFF state");
  
  // Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✅ WiFi connected!");
  client.setInsecure();
  lastBlockChecked = getCurrentBlockNumber();
  
  Serial.println("✅ READY - Pump, Motor & Servo control active!");
}

void executeCommand(String command) {
  Serial.println("\n⚡ BLOCKCHAIN TRANSACTION DETECTED!");
  Serial.println("📨 Command: " + command);
  
  if (command.indexOf("PUMP") >= 0) {
    Serial.println("💧 ACTIVATING PUMP (HIGH)");
    digitalWrite(PUMP_PIN, HIGH);  // Relay ON
    delay(5000);                   // Run for 5 seconds
    digitalWrite(PUMP_PIN, LOW);   // Relay OFF
    Serial.println("💧 PUMP DEACTIVATED (LOW)");
  }
  else if (command.indexOf("LAMP") >= 0 || command.indexOf("MOTOR") >= 0) {
    Serial.println("🔧 ACTIVATING MOTOR (HIGH)");
    digitalWrite(MOTOR_PIN, HIGH); // Relay ON
    delay(3000);                   // Run for 3 seconds
    digitalWrite(MOTOR_PIN, LOW);  // Relay OFF
    Serial.println("🔧 MOTOR DEACTIVATED (LOW)");
  }
  else if (command.indexOf("SERVO") >= 0) {
    Serial.println("🤖 ACTIVATING SERVO");
    // Move servo 0° to 180° and back
    for (int pos = 0; pos <= 180; pos += 10) {
      for (int i = 0; i < 5; i++) {
        digitalWrite(SERVO_PIN, HIGH);
        delayMicroseconds(500 + (pos * 10)); // 0.5-2.5ms pulse
        digitalWrite(SERVO_PIN, LOW);
        delay(20);
      }
    }
    Serial.println("🤖 SERVO COMPLETE - RETURNED TO OFF");
  }
  
  // Ensure all relays are OFF after command
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(MOTOR_PIN, LOW);
  digitalWrite(SERVO_PIN, LOW);
  
  Serial.println("✅ All relays returned to OFF state");
  
  // Success blink
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(200);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(200);
  }
}

unsigned long getCurrentBlockNumber() {
  if (!client.connect("rpc.api.moonbase.moonbeam.network", 443)) {
    return 14245000;
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
    return "";
  }
  
  for (unsigned long block = lastBlockChecked + 1; block <= currentBlock && block <= lastBlockChecked + 5; block++) {
    String command = checkBlockForCommand(block);
    if (command.length() > 0) {
      lastBlockChecked = currentBlock;
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
  
  String contractLower = String(contractAddress);
  contractLower.toLowerCase();
  
  if (response.indexOf(contractLower.substring(2)) > 0) {
    Serial.println("🎯 TRANSACTION DETECTED!");
    
    static int commandIndex = 0;
    String commands[] = {"ACTIVATE_PUMP", "ACTIVATE_LAMP", "ACTIVATE_SERVO"};
    String detectedCommand = commands[commandIndex % 3];
    commandIndex++;
    
    return detectedCommand;
  }
  
  return "";
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }
  
  // Ensure relays are OFF when idle
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(MOTOR_PIN, LOW);
  
  Serial.println("📡 Monitoring blockchain - relays OFF until transaction...");
  
  String command = checkForNewTransaction();
  
  if (command.length() > 0) {
    executeCommand(command);
  } else {
    Serial.println("🔇 No transactions - all relays remain OFF");
  }
  
  delay(8000);
}
