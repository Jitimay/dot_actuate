// =================================================================
// TTGO T-Call SIM800L + Moonbeam RPC - Final Debugging Sketch
// =================================================================
// This sketch incorporates all the best practices we've discussed:
// 1. Time synchronization from the cellular network.
// 2. A correctly formatted Trust Anchor file (`trust_anchors.h`).
// 3. A large memory buffer for the SSL/TLS handshake.
//
// If this sketch still fails with a "Trust Anchor" error, the issue
// is likely not in the code itself but in the environment, such as:
// - Library version incompatibilities (TinyGSM, SSLClient).
// - A hardware power supply issue.
// - A specific firmware issue on the SIM800L module.
// =================================================================

// --- LIBRARIES ---
#include <TinyGsmClient.h>
#include <SSLClient.h>
#include "time.h"
#include "trust_anchors.h" // Contains the Root CA for the server

// --- DEBUGGING AND MODEM ---
#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_MODEM_SIM800

// --- GPRS SETTINGS ---
const char apn[]      = "internet"; // Your APN
const char gprsUser[] = "";
const char gprsPass[] = "";

// --- SERVER DETAILS ---
const char server[]        = "rpc.api.moonbase.moonbeam.network";
const int  port            = 443;
const char contractAddress[] = "0xd9145CCE52D386f254917e481eB44e9943F39138";

// --- TTGO T-CALL PINOUT ---
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

// --- CLIENT INITIALIZATION ---
TinyGsm modem(SerialAT);

// Use a standard TinyGSM client as the transport layer
TinyGsmClient gsm_client(modem);

// Wrap the GSM client with SSLClient to handle the TLS encryption.
// The TAs and TAs_NUM are from `trust_anchors.h`.
// The buffer size (4096) is increased to ensure enough memory for the handshake.
SSLClient secure_client(gsm_client, TAs, TAs_NUM, 4096);

// =================================================================
// SETUP
// =================================================================
void setup() {
  SerialMon.begin(115200);
  delay(10);
  SerialMon.println("--- Starting Up ---");

  // --- Power on the Modem ---
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  
  // --- Initialize Modem ---
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX); 
  SerialMon.println("Initializing modem...");
  if (!modem.init()) {
      SerialMon.println("Failed to init modem, restarting.");
      ESP.restart();
  }
  SerialMon.print("Modem Info: ");
  SerialMon.println(modem.getModemInfo());

  // --- Connect to Network & GPRS ---
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
  SerialMon.print("GPRS Connected. IP Address: ");
  SerialMon.println(modem.getLocalIP());

  // --- Synchronize Time for SSL ---
  // This is a CRITICAL step for validating SSL certificates.
  SerialMon.print("Getting network time...");
  int year, month, day, hour, minute, second;
  float timezone;
  if (modem.getNetworkTime(&year, &month, &day, &hour, &minute, &second, &timezone)) {
    SerialMon.println(" OK");
    
    struct tm t;
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    
    time_t timeSinceEpoch = mktime(&t);
    struct timeval tv = { .tv_sec = timeSinceEpoch };
    settimeofday(&tv, NULL);

    SerialMon.print("Time set to: ");
    SerialMon.print(asctime(&t));
  } else {
    SerialMon.println(" FAIL. SSL connections will likely fail.");
  }
}

// =================================================================
// LOOP
// =================================================================
void loop() {
  SerialMon.println("\n---------------------------------");
  SerialMon.print("Attempting to connect to ");
  SerialMon.println(server);

  // Connect using the secure client
  if (!secure_client.connect(server, port)) {
    SerialMon.println("--> Connection failed.");
  } else {
    SerialMon.println("--> Connection successful!");

    // --- Build and Send Request ---
    String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0x5a7d06c9\"},\"latest\"],\"id\":1}";
    
    SerialMon.println("Sending HTTPS POST request...");
    secure_client.print(String("POST / HTTP/1.1\r\n"));
    secure_client.print(String("Host: ") + server + "\r\n");
    secure_client.print("Content-Type: application/json\r\n");
    secure_client.print("Content-Length: " + String(payload.length()) + "\r\n");
    secure_client.print("Connection: close\r\n\r\n");
    secure_client.print(payload);

    // --- Read Response ---
    SerialMon.println("Request sent. Reading response:");
    unsigned long timeout = millis();
    while (secure_client.connected() && millis() - timeout < 15000L) {
      while (secure_client.available()) {
        char c = secure_client.read();
        SerialMon.print(c);
        timeout = millis(); // Reset timeout while data is being received
      }
    }
    
    if (!secure_client.connected()) {
        SerialMon.println("\nServer disconnected.");
    } else {
        SerialMon.println("\nResponse timeout.");
    }
    secure_client.stop();
  }

  SerialMon.println("---------------------------------");
  SerialMon.println("Waiting 30 seconds...");
  delay(30000);
}
