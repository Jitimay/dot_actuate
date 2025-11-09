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

void sendATCommand(String cmd) {
  SerialAT.println(cmd);
  delay(1000);
  while (SerialAT.available()) {
    SerialMon.print((char)SerialAT.read());
  }
}

void loop() {
  SerialMon.println("Setting up HTTPS connection...");
  
  // Configure SSL
  sendATCommand("AT+CIPSSL=1");
  
  // Connect to server
  sendATCommand("AT+CIPSTART=\"TCP\",\"" + String(server) + "\",443");
  delay(3000);
  
  // Send data
  String payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"" + String(contractAddress) + "\",\"data\":\"0x5a7d06c9\"},\"latest\"],\"id\":1}";
  String request = "POST / HTTP/1.1\r\nHost: " + String(server) + "\r\nContent-Type: application/json\r\nContent-Length: " + String(payload.length()) + "\r\nConnection: close\r\n\r\n" + payload;
  
  sendATCommand("AT+CIPSEND=" + String(request.length()));
  delay(500);
  SerialAT.print(request);
  
  // Read response
  SerialMon.println("Response:");
  delay(5000);
  while (SerialAT.available()) {
    SerialMon.print((char)SerialAT.read());
  }
  
  // Close connection
  sendATCommand("AT+CIPCLOSE");
  
  delay(30000);
}
