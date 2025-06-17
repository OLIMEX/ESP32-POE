/*
 * ESP32-POE-ISO Web Ping Monitor
 * 
 * Requirements:
 * - ESP Async WebServer: https://github.com/me-no-dev/ESPAsyncWebServer
 * - AsyncTCP: https://github.com/me-no-dev/AsyncTCP
 * - ESPping (Library Manager: "ESPping" by Blaz Zupan)
 * 
 * Required ESP32 libraries (included with ESP32 board core):
 * - WiFi.h
 * - ETH.h
 * - ESPmDNS.h
 */

#include <WiFi.h>
#include <ETH.h>
#include <ESPAsyncWebServer.h>
#include <ESPping.h>
#include <ESPmDNS.h>
#include <time.h>

// Network Credentials and Settings
const char* ssid = "YOUR-WIFI-NAME";
const char* password = "YOUR-WIFI-PASSWORD";
const char* pingTarget = "8.8.8.8"; // Google DNS
const char* mdnsName = "esp32";     // mDNS name: http://esp32.local/

AsyncWebServer server(80);

// Network state tracking
bool eth_connected = false;
bool wifi_connected = false;
String eth_ip = "";
String wifi_ip = "";

String lastPingWiFi = "Not pinged yet.";
String lastPingEth = "Not pinged yet.";

unsigned long lastPingTimeWiFi = 0;
unsigned long lastPingTimeEth = 0;
const unsigned long pingInterval = 5000; // 5 seconds

// Get formatted current time
String getTimestamp() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

// WiFi Event Handler
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      wifi_connected = true;
      wifi_ip = WiFi.localIP().toString();
      Serial.print("[WiFi] Got IP: ");
      Serial.println(wifi_ip);
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      wifi_connected = false;
      wifi_ip = "";
      Serial.println("[WiFi] Disconnected. Reconnecting...");
      WiFi.begin(ssid, password);
      break;
    default:
      break;
  }
}

// Ethernet Event Handler
void EthEvent(arduino_event_id_t event) {
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
      eth_ip = ETH.localIP().toString();
      Serial.print("[ETH] Got IP: ");
      Serial.println(eth_ip);
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
    case ARDUINO_EVENT_ETH_LOST_IP:
    case ARDUINO_EVENT_ETH_STOP:
      eth_connected = false;
      eth_ip = "";
      Serial.println("[ETH] Connection Lost");
      break;
    default:
      break;
  }
}

// Initialize time using NTP server
void setupTime() {
  configTime(0, 0, "time.google.com");
  Serial.print("Waiting for time sync");

  unsigned long start = millis();
  const unsigned long timeout = 20000;  // 20 sec
  time_t now = 0;

  while (millis() - start < timeout) {
    time(&now);
    if (now > 100000) {
      Serial.println("\nTime synchronized.");
      return;
    }
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nFailed to sync time within 20 seconds.");
}

// Setup function
void setup() {
  Serial.begin(115200);
  delay(300);

  WiFi.onEvent(WiFiEvent);
  Network.onEvent(EthEvent);

  // Start WiFi first
  Serial.println("[WiFi] Connecting...");
  WiFi.begin(ssid, password);

  // Then start Ethernet
  Serial.println("[ETH] Starting...");
  ETH.begin();

  // Setup NTP time
  setupTime();

  // Setup mDNS
  if (MDNS.begin(mdnsName)) {
    Serial.printf("[mDNS] Started: http://%s.local/\n", mdnsName);
  } else {
    Serial.println("[mDNS] Failed to start");
  }

  // HTTP server route
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><head><meta http-equiv='refresh' content='5'></head><body>";
    html += "<h1>ESP32 Network Ping Monitor</h1>";
    html += "<p><strong>Time:</strong> " + getTimestamp() + "</p>";

    html += "<h2>WiFi</h2>";
    html += "<p>Status: " + String(wifi_connected ? "Connected" : "Disconnected") + "</p>";
    html += "<p>IP: " + wifi_ip + "</p>";
    html += "<p>Last Ping: " + lastPingWiFi + "</p>";

    html += "<h2>Ethernet</h2>";
    html += "<p>Status: " + String(eth_connected ? "Connected" : "Disconnected") + "</p>";
    html += "<p>IP: " + eth_ip + "</p>";
    html += "<p>Last Ping: " + lastPingEth + "</p>";

    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.begin();
}

// Loop - Ping targets periodically
void loop() {
  unsigned long now = millis();

  if (wifi_connected && now - lastPingTimeWiFi >= pingInterval) {
    lastPingTimeWiFi = now;
    if (Ping.ping(pingTarget, 1)) {
      lastPingWiFi = "Ping OK @ " + getTimestamp();
    } else {
      lastPingWiFi = "Ping failed @ " + getTimestamp();
    }
  }

  if (eth_connected && now - lastPingTimeEth >= pingInterval) {
    lastPingTimeEth = now;
    if (Ping.ping(pingTarget, 1)) {
      lastPingEth = "Ping OK @ " + getTimestamp();
    } else {
      lastPingEth = "Ping failed @ " + getTimestamp();
    }
  }

  delay(100);
}
