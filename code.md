```cpp
// Define the serial console for debug prints
#define SerialMon Serial

// Define the serial connection to the SIM800L module
#define SerialAT Serial1

// Configure TinyGSM library for the SIM800L
#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>

// --- YOUR LUMITEL SETTINGS ---
const char apn[]  = "internet"; // APN for Lumitel
const char gprsUser[] = "";     // Leave blank
const char gprsPass[] = "";     // Leave blank
// -----------------------------

// --- MOONBEAM RPC DETAILS ---
const char server[] = "rpc.api.moonbase.moonbeam.network";
const int  port = 443; // Use 443 for HTTPS

// Your deployed smart contract address
const char contractAddress[] = "0xYOUR_CONTRACT_ADDRESS_HERE"; // <-- PASTE YOUR ADDRESS HERE

// GsmClient for internet connectivity
TinyGsm modem(SerialAT);
// Use TinyGsmClientSecure for HTTPS
#include <TinyGsmClientSecure.h>
TinyGsmClientSecure client(modem);


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

  // Set modem power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);

  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  
  // Start serial communication with the modem
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  
  SerialMon.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  // Wait for the network
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
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
  SerialMon.print("Connecting to ");
  SerialMon.print(server);
  if (!client.connect(server, port)) {
    SerialMon.println(" fail");
  } else {
    SerialMon.println(" success");
    
    // Construct the JSON-RPC payload
    String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0x5a7d06c9\"},\"latest\"],\"id\":1}";
    
    // Make an HTTP POST request
    client.print(String("POST / HTTP/1.1\r\n"));
    client.print(String("Host: ") + server + "\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: " + String(payload.length()) + "\r\n");
    client.print("Connection: close\r\n\r\n");
    client.print(payload);
    client.print("\r\n");

    SerialMon.println("Request sent. Reading response...");
    // Read the response
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        SerialMon.print(c);
      }
    }
    client.stop();
    SerialMon.println("\nServer disconnected");
  }

  // Wait 30 seconds before the next request
  delay(30000);
}
```