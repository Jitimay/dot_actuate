#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

#define SerialMon Serial
#define SerialAT Serial1

const char apn[] = "internet";
const char server[] = "rpc.api.moonbase.moonbeam.network";
const char contractAddress[] = "0xd9145CCE52D386f254917e481eB44e9943F39138";

#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26

TinyGsm modem(SerialAT);
TinyGsmClientSecure client(modem);

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
  
  if (!modem.init()) {
    SerialMon.println("Modem init failed");
    ESP.restart();
  }

  if (!modem.waitForNetwork()) {
    SerialMon.println("Network failed");
    return;
  }
  SerialMon.println("Network OK");

  if (!modem.gprsConnect(apn, "", "")) {
    SerialMon.println("GPRS failed");
    return;
  }
  SerialMon.println("GPRS OK");
}

void loop() {
  SerialMon.println("Connecting to Moonbeam...");
  
  if (!client.connect(server, 443)) {
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

    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 10000L) {
      while (client.available()) {
        SerialMon.print((char)client.read());
        timeout = millis();
      }
    }
    client.stop();
  }

  delay(30000);
}
