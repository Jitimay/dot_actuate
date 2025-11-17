/*
  DotActuate - Fixed firmware
  - Uses WiFi + WiFiClientSecure
  - Detects transactions via eth_getLogs -> eth_getTransactionByHash
  - Handles active-LOW relay modules (common)
  - Extracts ASCII-like calldata to find keywords: PUMP, MOTOR, SERVO, LAMP
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// ---------- CONFIG ----------
const char* ssid = "Josh";
const char* password = "Jitimay$";

const char* rpcHost = "rpc.api.moonbase.moonbeam.network";
const int   rpcPort = 443;

const char* contractAddress = "0x6e2ec30DD6093f247023019e408E226a345e5769";

// pins
#define PUMP_PIN 2
#define MOTOR_PIN 4
#define SERVO_PIN 5
#define STATUS_LED_PIN 13

// Relay wiring: many relay boards are ACTIVE LOW.
// Define RELAY_ON and RELAY_OFF so logic is clear:
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// polling
const unsigned long POLL_INTERVAL_MS = 8000;
unsigned long lastPollTime = 0;
unsigned long lastBlockChecked = 0;

// network client
WiFiClientSecure client;

// -------------- helpers --------------
String postRpc(const String &payload, unsigned long connectTimeout = 8000, unsigned long respTimeout = 8000) {
  if (!client.connect(rpcHost, rpcPort)) {
    Serial.println("RPC connect failed");
    return "";
  }

  client.print(String("POST / HTTP/1.1\r\n"));
  client.print(String("Host: ") + rpcHost + "\r\n");
  client.print("User-Agent: ESP32\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: " + String(payload.length()) + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(payload);

  unsigned long start = millis();
  while (!client.available() && millis() - start < respTimeout) {
    delay(10);
  }
  if (!client.available()) {
    client.stop();
    Serial.println("No RPC response");
    return "";
  }

  // Skip headers
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
    if (millis() - start > respTimeout) { client.stop(); return ""; }
  }

  String response = "";
  start = millis();
  while (client.available() && millis() - start < respTimeout) {
    response += (char)client.read();
  }
  client.stop();
  return response;
}

// Convert hex string starting with 0x into ascii-printable string (non-printable bytes ignored)
String hexInputToAscii(const String &hex) {
  if (hex.length() < 3) return "";
  String s = hex;
  if (s.startsWith("0x")) s = s.substring(2);
  String out = "";
  for (int i = 0; i + 1 < (int)s.length(); i += 2) {
    String byteStr = s.substring(i, i + 2);
    char c = (char) strtoul(byteStr.c_str(), NULL, 16);
    if (c >= 32 && c <= 126) out += c; // printable ASCII only
    // else ignore non-printables
  }
  return out;
}

// Send a simple status blink
void blinkStatus(int times = 3) {
  for (int i = 0; i < times; ++i) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(150);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(150);
  }
}

// ---------- Blockchain helper functions ----------

// get current block number
unsigned long getCurrentBlockNumber() {
  String req = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_blockNumber\",\"params\":[],\"id\":1}";
  String resp = postRpc(req);
  if (resp.length() == 0) return lastBlockChecked; // don't advance

  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, resp);
  if (err) {
    Serial.println("JSON parse error (blockNumber)");
    return lastBlockChecked;
  }
  if (!doc.containsKey("result")) return lastBlockChecked;

  String hexBlock = doc["result"].as<String>();
  if (hexBlock.startsWith("0x")) hexBlock = hexBlock.substring(2);
  unsigned long block = strtoul(hexBlock.c_str(), NULL, 16);
  return block;
}

// get logs for contract between fromBlock..toBlock
// returns JSON string response, caller should parse
String getLogs(unsigned long fromBlock, unsigned long toBlock) {
  if (toBlock < fromBlock) return "";
  String fromHex = "0x" + String(fromBlock, HEX);
  String toHex   = "0x" + String(toBlock, HEX);

  // Build params: [ { address: contractAddress, fromBlock: ..., toBlock: ... } ]
  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getLogs\",\"params\":[{\"address\":\"";
  payload += String(contractAddress);
  payload += "\",\"fromBlock\":\"" + fromHex + "\",\"toBlock\":\"" + toHex + "\"}],\"id\":1}";

  String resp = postRpc(payload);
  return resp;
}

// get transaction by hash
String getTransactionByHash(const String &txHash) {
  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getTransactionByHash\",\"params\":[\"" + txHash + "\"],\"id\":1}";
  return postRpc(payload);
}

// ---------- Actuation logic ----------
void executeCommand(String command) {
  command.toUpperCase();
  Serial.println("\n=== EXECUTE COMMAND: " + command + " ===");

  if (command.indexOf("PUMP") >= 0) {
    Serial.println("Activating PUMP");
    digitalWrite(PUMP_PIN, RELAY_ON);
    delay(5000);
    digitalWrite(PUMP_PIN, RELAY_OFF);
    Serial.println("Pump deactivated");
  } else if (command.indexOf("MOTOR") >= 0 || command.indexOf("LAMP") >= 0) {
    Serial.println("Activating MOTOR/LAMP");
    digitalWrite(MOTOR_PIN, RELAY_ON);
    delay(3000);
    digitalWrite(MOTOR_PIN, RELAY_OFF);
    Serial.println("Motor/Lamp deactivated");
  } else if (command.indexOf("SERVO") >= 0) {
    Serial.println("Activating SERVO (simulated PWM pulses)");
    // crude servo pulses using digitalWrite (not ideal for real servo lib)
    for (int pos = 0; pos <= 180; pos += 30) {
      for (int i = 0; i < 4; ++i) {
        digitalWrite(SERVO_PIN, RELAY_ON);
        delayMicroseconds(1000 + pos * 500 / 180);
        digitalWrite(SERVO_PIN, RELAY_OFF);
        delay(20);
      }
    }
    Serial.println("Servo action complete");
  } else {
    Serial.println("Unknown command: defaulting to PUMP");
    digitalWrite(PUMP_PIN, RELAY_ON);
    delay(3000);
    digitalWrite(PUMP_PIN, RELAY_OFF);
  }

  // ensure all off
  digitalWrite(PUMP_PIN, RELAY_OFF);
  digitalWrite(MOTOR_PIN, RELAY_OFF);
  digitalWrite(SERVO_PIN, RELAY_OFF);

  blinkStatus(3);
}

// ---------- Setup & Loop ----------
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== DotActuate firmware (fixed) ===");

  // pins
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);

  // ensure relays are OFF according to active-low wiring
  digitalWrite(PUMP_PIN, RELAY_OFF);
  digitalWrite(MOTOR_PIN, RELAY_OFF);
  digitalWrite(SERVO_PIN, RELAY_OFF);
  digitalWrite(STATUS_LED_PIN, LOW);

  // connect WiFi
  Serial.print("Connecting WiFi...");
  WiFi.begin(ssid, password);
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 20000) {
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection failed - continuing and will retry in loop");
  } else {
    Serial.println("\nWiFi connected");
  }

  // use insecure for demo (skip cert check) - ok for hackathon; use real CA in production
  client.setInsecure();

  // set initial block
  lastBlockChecked = getCurrentBlockNumber();
  Serial.print("Starting from block: ");
  Serial.println(lastBlockChecked);

  Serial.println("Ready.");
}

void loop() {
  // reconnect WiFi if needed
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected - attempting reconnect...");
    WiFi.begin(ssid, password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnect failed, will retry later");
      delay(5000);
      return;
    }
    Serial.println("WiFi reconnected");
  }

  unsigned long now = millis();
  if (now - lastPollTime < POLL_INTERVAL_MS) {
    delay(200);
    return;
  }
  lastPollTime = now;

  Serial.println("\nPolling blockchain for logs...");

  unsigned long currentBlock = getCurrentBlockNumber();
  Serial.print("Current block: "); Serial.println(currentBlock);
  if (currentBlock <= lastBlockChecked) {
    Serial.println("No new blocks");
    return;
  }

  unsigned long fromB = lastBlockChecked + 1;
  unsigned long toB = currentBlock;
  // Limit how many blocks we query at once to avoid huge responses
  if (toB - fromB > 20) toB = fromB + 20;

  Serial.printf("Query logs from %lu to %lu\n", fromB, toB);

  String logsResp = getLogs(fromB, toB);
  if (logsResp.length() == 0) {
    Serial.println("getLogs returned nothing");
    // advance lastBlockChecked anyway to avoid requerying same block repeatedly
    lastBlockChecked = currentBlock;
    return;
  }

  // parse logs response
  StaticJsonDocument<2000> logsDoc;
  DeserializationError err = deserializeJson(logsDoc, logsResp);
  if (err) {
    Serial.println("Failed to parse logs JSON");
    Serial.println(err.c_str());
    lastBlockChecked = currentBlock;
    return;
  }

  JsonArray logs = logsDoc["result"].as<JsonArray>();
  if (logs.size() == 0) {
    Serial.println("No logs for contract in this range");
    lastBlockChecked = currentBlock;
    return;
  }

  // We have logs -> process first log (could iterate for multiple)
  JsonObject firstLog = logs[0].as<JsonObject>();
  String txHash = firstLog["transactionHash"].as<String>();
  Serial.print("Found log txHash: "); Serial.println(txHash);

  // Get transaction payload
  String txResp = getTransactionByHash(txHash);
  if (txResp.length() == 0) {
    Serial.println("Failed to get transaction details");
    lastBlockChecked = currentBlock;
    return;
  }

  StaticJsonDocument<2000> txDoc;
  DeserializationError err2 = deserializeJson(txDoc, txResp);
  if (err2) {
    Serial.println("Failed to parse tx JSON");
    lastBlockChecked = currentBlock;
    return;
  }

  if (!txDoc.containsKey("result")) {
    Serial.println("Transaction result missing");
    lastBlockChecked = currentBlock;
    return;
  }

  String inputHex = txDoc["result"]["input"].as<String>();
  Serial.print("Tx input (hex): ");
  Serial.println(inputHex);

  String ascii = hexInputToAscii(inputHex);
  Serial.print("ASCII extracted: ");
  Serial.println(ascii);

  // find keyword
  String command = "";
  if (ascii.indexOf("PUMP") >= 0) command = "PUMP";
  else if (ascii.indexOf("MOTOR") >= 0 || ascii.indexOf("LAMP") >= 0) command = "MOTOR";
  else if (ascii.indexOf("SERVO") >= 0) command = "SERVO";
  else {
    // fallback: check event data for certain bytes, else default to pump
    Serial.println("No keyword found in calldata -> defaulting to PUMP");
    command = "PUMP";
  }

  // execute command
  executeCommand(command);

  // update last checked block to currentBlock to avoid reprocessing
  lastBlockChecked = currentBlock;
}
