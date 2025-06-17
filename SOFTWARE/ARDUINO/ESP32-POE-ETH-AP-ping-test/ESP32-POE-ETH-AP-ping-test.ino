#include <WiFi.h>
#include <ETH.h>
//requires ESPping library, find and install from Sketch -> Manage Libraries
#include <ESPping.h>

const char* ssid = "YOUR-WIFI-NAME";
const char* password = "YOUR-WIFI-PASSWORD";
const char* pingTarget = "8.8.8.8"; // Google DNS

static bool eth_connected = false;
static bool wifi_connected = false;

void onEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("[ETH] Started");
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("[ETH] Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      eth_connected = true;
      Serial.print("[ETH] Got IP: ");
      Serial.println(ETH.localIP());
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
    case ARDUINO_EVENT_ETH_LOST_IP:
    case ARDUINO_EVENT_ETH_STOP:
      eth_connected = false;
      Serial.println("[ETH] Connection Lost");
      break;
    default:
      break;
  }
}

void onWiFi(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("[WiFi] Connected to AP.");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      wifi_connected = true;
      Serial.print("[WiFi] Got IP: ");
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      wifi_connected = false;
      Serial.println("[WiFi] Disconnected. Reconnecting...");
      WiFi.begin(ssid, password);
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.onEvent(onWiFi);
  Network.onEvent(onEvent);

  WiFi.begin(ssid, password);
  Serial.println("[WiFi] Connecting...");

  ETH.begin();
  Serial.println("[ETH] Starting...");
}

void loop() {
  static unsigned long lastPing = 0;
  if (millis() - lastPing > 10000) {
    lastPing = millis();

    Serial.println("=== Network Status ===");

    Serial.print("WiFi: ");
    Serial.println(wifi_connected ? "Connected" : "Disconnected");
    if (wifi_connected) {
      Serial.print("WiFi IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("Pinging from WiFi: ");
      Serial.println(pingTarget);
      bool wifi_ok = Ping.ping(pingTarget);
      Serial.println(wifi_ok ? "WiFi ping OK" : "WiFi ping failed");
    }

    Serial.print("Ethernet: ");
    Serial.println(eth_connected ? "Connected" : "Disconnected");
    if (eth_connected) {
      Serial.print("Ethernet IP: ");
      Serial.println(ETH.localIP());
      Serial.print("Pinging from Ethernet: ");
      Serial.println(pingTarget);
      bool eth_ok = Ping.ping(pingTarget);
      Serial.println(eth_ok ? "Ethernet ping OK" : "Ethernet ping failed");
    }

    Serial.println("======================\n");
  }
}
