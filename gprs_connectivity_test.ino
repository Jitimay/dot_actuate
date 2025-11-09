// =================================================================
// Basic GPRS Connectivity Test
// =================================================================
// This sketch is the simplest possible test to confirm that your
// TTGO T-Call and SIM card can connect to the internet.
// It connects to GPRS and makes a plain HTTP request to example.com.
//
// If this works, you will see the HTML of the example.com webpage
// in the Serial Monitor.
//
// If it fails, the problem is with your hardware, power supply,
// SIM card, or APN settings, not with SSL or security.
// =================================================================

// --- LIBRARIES ---
#define TINY_GSM_MODEM_SIM800 // Define the modem model before including the library
#include <TinyGsmClient.h>

// --- DEBUGGING AND MODEM ---
#define SerialMon Serial
#define SerialAT Serial1

// --- GPRS SETTINGS ---
const char apn[]      = "internet"; // Your APN
const char gprsUser[] = "";
const char gprsPass[] = "";

// --- TEST SERVER (PLAIN HTTP) ---
const char server[] = "example.com";
const int  port     = 80; // Port 80 for plain HTTP

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
  SerialMon.println("--- GPRS Connectivity Test ---");

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
  SerialMon.print("GPRS Connected. IP Address: ");
  SerialMon.println(modem.getLocalIP());
}

// =================================================================
// LOOP
// =================================================================
void loop() {
  SerialMon.println("\n---------------------------------");
  SerialMon.print("Connecting to ");
  SerialMon.println(server);

  if (!client.connect(server, port)) {
    SerialMon.println("--> Connection failed.");
    delay(10000);
    return;
  }
  
  SerialMon.println("--> Connection successful!");

  // Make a simple HTTP GET request
  SerialMon.println("Sending HTTP GET request...");
  client.print(String("GET / HTTP/1.1\r\n"));
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");

  // Read and print the response
  SerialMon.println("Reading response:");
  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    while (client.available()) {
      char c = client.read();
      SerialMon.print(c);
      timeout = millis();
    }
  }
  client.stop();
  
  SerialMon.println("\n---------------------------------");
  SerialMon.println("Test complete. Waiting 60 seconds...");
  delay(60000);
}
