// =================================================================
// TTGO T-Call + Gateway Server Example
// =================================================================
// This sketch connects to your PC gateway server over plain HTTP.
// It does NOT use SSLClient, trust anchors, or time synchronization,
// making it much simpler and more reliable for the ESP32.
// =================================================================

// --- LIBRARIES ---
#include <TinyGsmClient.h>

// --- DEBUGGING AND MODEM ---
#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_MODEM_SIM800

// --- GPRS SETTINGS ---
const char apn[]      = "internet"; // Your APN
const char gprsUser[] = "";
const char gprsPass[] = "";

// =================================================================
// !!! IMPORTANT !!!
// REPLACE THIS with the hostname you got from ngrok.
// Do NOT include "https://".
// For example: "b7a9-123-45-67-89.ngrok-free.app"
// =================================================================
const char server[]   = "<YOUR_NGROK_HOSTNAME>"; 
const char resource[] = "/rpc"; // The path we defined in our Python server
const int  port       = 80;     // Use port 80 for plain HTTP

// --- CONTRACT DETAILS ---
const char contractAddress[] = "0xd9145CCE52D386f254917e481eB44e9943F39138";

// --- TTGO T-CALL PINOUT ---
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

// --- CLIENT INITIALIZATION ---
TinyGsm modem(SerialAT);
TinyGsmClient client(modem); // A standard, non-secure client

// =================================================================
// SETUP
// =================================================================
void setup() {
  SerialMon.begin(115200);
  delay(10);
  SerialMon.println("--- Starting Up ---");

  // Power on the Modem
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  
  // Initialize Modem
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX); 
  SerialMon.println("Initializing modem...");
  if (!modem.init()) {
      SerialMon.println("Failed to init modem, restarting.");
      ESP.restart();
  }

  // Connect to Network & GPRS
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" FAIL");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print("Connecting to GPRS...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" FAIL");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");
}

// =================================================================
// LOOP
// =================================================================
void loop() {
  SerialMon.println("\n---------------------------------");
  SerialMon.print("Connecting to gateway: ");
  SerialMon.println(server);

  if (!client.connect(server, port)) {
    SerialMon.println("--> Connection to gateway failed.");
    delay(10000);
    return;
  }
  
  SerialMon.println("--> Connection to gateway successful!");

  // --- Build and Send Request ---
  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0x5a7d06c9\"},\"latest\"],\"id\":1}";
  
  SerialMon.println("Sending HTTP POST request...");
  client.print(String("POST ") + resource + " HTTP/1.1\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: " + String(payload.length()) + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(payload);

  // --- Read Response ---
  SerialMon.println("Request sent. Reading response:");
  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 15000L) {
    while (client.available()) {
      char c = client.read();
      SerialMon.print(c);
      timeout = millis();
    }
  }
  client.stop();
  SerialMon.println("\n---------------------------------");
  SerialMon.println("Waiting 30 seconds...");
  delay(30000);
}
