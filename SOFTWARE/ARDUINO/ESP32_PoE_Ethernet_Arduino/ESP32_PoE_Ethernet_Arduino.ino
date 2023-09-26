/*
    This sketch shows the Ethernet event usage

*/
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

#include <ETH.h>
#define TESTHOSTNAME "google.com"

static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
    case 20:  //ESP32 ethernet start
      Serial.print("ETH Started ");
      Serial.println((int)event);
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;

    case SYSTEM_EVENT_ETH_CONNECTED:
    case 22:  //ESP32 ethernet phy link up
      Serial.print("ETH Connected ");
      Serial.println((int)event);
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
    case 24:  //ESP32 ethernet got IP
      Serial.print("ETH Got an IP address ");
      Serial.println((int)event);
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
    case 23:  // ESP32 ethernet phy link down
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
    case 21:  //ESP32 ethernet stop
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;

    default:
      Serial.print("Unknown network event ");
      Serial.println((int)event);
      break;
  }
}

void testClient(const char* host, uint16_t port) {
  Serial.print("\nconnecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available())
    ;
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Boot");
  delay(200);
  Serial.println("Starting ETH");
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  delay(200);
  Serial.print("ETH MAC: ");
  Serial.println(ETH.macAddress());
}

void loop() {
  Serial.print(", IPv4: ");
  Serial.println(ETH.localIP());
  if (eth_connected) {
    testClient(TESTHOSTNAME, 80);
    Serial.print("Sleeping...");
    delay(50000);
    Serial.println(".");
  }
  delay(1000);
}
