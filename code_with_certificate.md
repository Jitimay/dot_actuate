```cpp
// ===============================
// TTGO T-Call SIM800L + Moonbeam RPC Test
// Final version with Root CA Certificate for SSLClient
// ===============================

// Debug serial console
#define SerialMon Serial

// Serial connection to the SIM800L module
#define SerialAT Serial1

// Tell TinyGSM we're using SIM800
#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <SSLClient.h>
#include "trust_anchors.h" // For the certificate

// --- YOUR LUMITEL SETTINGS ---
const char apn[]  = "internet"; // APN for Lumitel
const char gprsUser[] = "";     // Leave blank
const char gprsPass[] = "";     // Leave blank
// -----------------------------

// --- MOONBEAM RPC DETAILS ---
const char server[] = "rpc.api.moonbase.moonbeam.network";
const int  port = 443; // HTTPS
const char contractAddress[] = "0xd9145CCE52D386f254917e481eB44e9943F39138";
// -----------------------------

// GsmClient for internet connectivity
TinyGsm modem(SerialAT);

// Standard, unencrypted client from TinyGSM
TinyGsmClient gsm_client(modem);

// SSLClient wrapper for handling encryption.
// We now provide the trust anchors (root certificates).
SSLClient secure_client(gsm_client, TAs, (size_t)TAs_NUM, 512);


// TTGO T-Call pin definitions
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

void setup() {
  // Start serial console
  SerialMon.begin(115200);
  delay(10);

  // Power setup for modem
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);

  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Start serial communication with modem
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

  SerialMon.println("\nInitializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  // Wait for the GSM network
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork(30000L)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  // Connect to GPRS
  SerialMon.print("Connecting to GPRS (");
  SerialMon.print(apn);
  SerialMon.print(")...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");
  }
}

void loop() {
  SerialMon.print("\nConnecting to ");
  SerialMon.print(server);
  SerialMon.print("...");
  // Use the secure_client to connect
  if (!secure_client.connect(server, port)) {
    SerialMon.println(" fail");
  } else {
    SerialMon.println(" success");

    // JSON-RPC payload (cleaned up)
    String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0x5a7d06c9\"},\"latest\"],\"id\":1}";

    // Build HTTP POST request using the secure_client
    secure_client.print(String("POST / HTTP/1.1\r\n"));
    secure_client.print(String("Host: ") + server + "\r\n");
    secure_client.print("User-Agent: ESP32\r\n");
    secure_client.print("Content-Type: application/json\r\n");
    secure_client.print("Connection: close\r\n");
    secure_client.print("Content-Length: " + String(payload.length()) + "\r\n\r\n");
    secure_client.print(payload);

    SerialMon.println("Request sent. Waiting for response...");

    unsigned long timeout = millis();
    while (secure_client.connected() && millis() - timeout < 15000L) {
      while (secure_client.available()) {
        char c = secure_client.read();
        SerialMon.print(c);
        timeout = millis();
      }
    }
    secure_client.stop();
    SerialMon.println("\nDisconnected from server.");
  }

  // Wait before sending the next request
  delay(30000);
}
```