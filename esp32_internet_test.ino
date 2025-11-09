#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

#define SerialMon Serial
#define SerialAT Serial1

const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

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
  
  if (!modem.init()) {
    SerialMon.println("Modem init failed");
    ESP.restart();
  }

  if (!modem.waitForNetwork()) {
    SerialMon.println("Network failed");
    return;
  }
  SerialMon.println("Network OK");

  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println("GPRS failed");
    return;
  }
  SerialMon.println("GPRS OK");
  SerialMon.print("IP: ");
  SerialMon.println(modem.getLocalIP());
}

void loop() {
  // Test HTTP connection to Google
  SerialMon.println("Testing HTTP to google.com...");
  
  if (!client.connect("google.com", 80)) {
    SerialMon.println("HTTP test failed");
  } else {
    SerialMon.println("HTTP test OK");
    client.print("GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n");
    
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 5000L) {
      if (client.available()) {
        SerialMon.print(client.readString());
        break;
      }
    }
    client.stop();
  }

  delay(10000);
}
