#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

#define SerialMon Serial
#define SerialAT Serial1

const char apn[] = "internet";
const char server[] = "moonbeam-alpha.api.onfinality.io";
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

  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  
  if (!modem.init()) ESP.restart();
  if (!modem.waitForNetwork()) return;
  SerialMon.println("Network OK");
  if (!modem.gprsConnect(apn, "", "")) return;
  SerialMon.println("GPRS OK");
}

void loop() {
  SerialMon.println("Getting latest command from blockchain...");
  
  if (!client.connect(server, 80)) {
    SerialMon.println("Connection failed");
  } else {
    // Correct function selector: latestCommand() = 0xebb48ea7
    String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0xebb48ea7\"},\"latest\"],\"id\":1}";
    
    client.print("POST /public HTTP/1.1\r\n");
    client.print("Host: " + String(server) + "\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: " + String(payload.length()) + "\r\n");
    client.print("Connection: close\r\n\r\n");
    client.print(payload);

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;
    }
    
    SerialMon.print("Response: ");
    while (client.available()) {
      SerialMon.print((char)client.read());
    }
    SerialMon.println();
    client.stop();
  }

  delay(30000);
}
