#include <TinyGsmClient.h>

#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_MODEM_SIM800

const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char server[] = "rpc.api.moonbase.moonbeam.network";
const int port = 80; // Use HTTP instead of HTTPS for testing
const char contractAddress[] = "0xd9145CCE52D386f254917e481eB44e9943F39138";

#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void setup() {
  SerialMon.begin(115200);
  delay(10);
  SerialMon.println("Starting...");

  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  SerialMon.println("Initializing modem...");
  if (!modem.init()) {
    SerialMon.println("Failed to init modem");
    ESP.restart();
  }
  SerialMon.println("Modem initialized");

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" FAIL");
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print("Connecting to GPRS...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" FAIL");
    return;
  }
  SerialMon.println(" OK");
  SerialMon.print("IP: ");
  SerialMon.println(modem.getLocalIP());
}

void loop() {
  SerialMon.println("\nConnecting to server...");
  
  if (!client.connect(server, port)) {
    SerialMon.println("Connection failed");
  } else {
    SerialMon.println("Connected!");

    String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0x5a7d06c9\"},\"latest\"],\"id\":1}";
    
    client.print("POST / HTTP/1.1\r\n");
    client.print("Host: " + String(server) + "\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: " + String(payload.length()) + "\r\n");
    client.print("Connection: close\r\n\r\n");
    client.print(payload);

    SerialMon.println("Response:");
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 10000L) {
      while (client.available()) {
        char c = client.read();
        SerialMon.print(c);
        timeout = millis();
      }
    }
    client.stop();
  }

  delay(30000);
}
