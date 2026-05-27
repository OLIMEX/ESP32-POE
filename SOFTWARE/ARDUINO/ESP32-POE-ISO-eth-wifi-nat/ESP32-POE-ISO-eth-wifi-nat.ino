/*
  Olimex ESP32-POE-ISO Ethernet-to-WiFi NAT Router Demo
  =====================================================

  Board:
    Olimex ESP32-POE-ISO
    ESP32-WROOM module
    LAN8720A / LAN87xx Ethernet PHY

  Demo purpose:
    Ethernet is used as WAN/uplink.
    WiFi SoftAP is used as LAN/downlink.
    A phone/laptop connects to the ESP32 WiFi AP and gets internet through
    Ethernet using NAT/NAPT.
    Debug info printed over the ESP32's serial; test page available on the phone/laptop

  WiFi AP:
    SSID:     OLIMEX-ESP32-POE-ISO
    Password: 12345678

  Debug page:
    http://192.168.4.1/

  Tested reference environment:
    Arduino IDE: 1.8.19
    ESP32 Arduino core: 3.3.8
    Selected board: ESP32 Dev Module
    CPU frequency: 240 MHz
    Flash mode: QIO
    Flash frequency: 80 MHz
    Flash size: 4 MB
    PSRAM: Disabled
    Upload speed: 921600

  ESP32-POE-ISO Ethernet settings:
    PHY type:   ETH_PHY_LAN8720
    PHY addr:   0
    PHY power:  GPIO12
    MDC:        GPIO23
    MDIO:       GPIO18
    RMII clock: ESP32 outputs 50 MHz clock on GPIO17

    ESP32-POE-ISO:
      ESP32 outputs 50 MHz RMII clock -> PHY
      ETH_CLOCK_GPIO17_OUT

  Important PHY power/clock note:
    On ESP32-POE-ISO, GPIO12 controls PHY power/reset behavior.
    The PHY should not be powered before the ESP32 RMII clock is available.
    Therefore this sketch first holds GPIO12 LOW, then lets ETH.begin()
    initialize Ethernet using ETH_CLOCK_GPIO17_OUT and GPIO12 as the power pin.

  NAT/DNS notes:
    NAT must be enabled from lwIP TCP/IP context via tcpip_callback().
    SoftAP DHCP must advertise a real DNS server, here 1.1.1.1.
    Otherwise Android may connect but report "no internet", and normal web
    browsing may fail.
*/

#include <WiFi.h>
#include <ETH.h>
#include <WebServer.h>
#include <NetworkClient.h>

extern "C" {
  #include "lwip/lwip_napt.h"
  #include "lwip/tcpip.h"
  #include "lwip/netif.h"
  #include "lwip/ip4_addr.h"
  #include "esp_netif.h"
}

// -----------------------------------------------------------------------------
// WiFi SoftAP configuration
// -----------------------------------------------------------------------------

#define AP_SSID "OLIMEX-ESP32-POE-ISO"
#define AP_PASS "12345678"

#define AP_IP      IPAddress(192, 168, 4, 1)
#define AP_GATEWAY IPAddress(192, 168, 4, 1)
#define AP_NETMASK IPAddress(255, 255, 255, 0)

// DNS advertised to WiFi clients by the SoftAP DHCP server.
#define AP_DNS_A 1
#define AP_DNS_B 1
#define AP_DNS_C 1
#define AP_DNS_D 1

// -----------------------------------------------------------------------------
// Ethernet configuration for Olimex ESP32-POE-ISO
// -----------------------------------------------------------------------------

#define ETH_PHY_TYPE  ETH_PHY_LAN8720
#define ETH_PHY_ADDR  0
#define ETH_PHY_POWER 12
#define ETH_MDC_PIN   23
#define ETH_MDIO_PIN  18

// ESP32 outputs 50 MHz RMII clock on GPIO17.
#define ETH_CLK_MODE  ETH_CLOCK_GPIO17_OUT

#ifndef OFFER_DNS
  #define OFFER_DNS 2
#endif

// -----------------------------------------------------------------------------
// Global state
// -----------------------------------------------------------------------------

WebServer server(80);

bool ethUp = false;
bool natEnabled = false;
bool natRequested = false;

uint32_t natEnableAttempts = 0;
uint32_t lastWanTestMs = 0;
bool lastWanTcpTest = false;

esp_netif_t *ethNetif = NULL;
esp_netif_t *apNetif = NULL;

// -----------------------------------------------------------------------------
// ESP-NETIF helpers
// -----------------------------------------------------------------------------

void findNetifs() {
  ethNetif = esp_netif_get_handle_from_ifkey("ETH_DEF");
  apNetif  = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

  Serial.println();
  Serial.println("[ESP-NETIF] Handles:");

  Serial.print("[ESP-NETIF] ETH_DEF: ");
  Serial.println((uint32_t)ethNetif, HEX);

  Serial.print("[ESP-NETIF] WIFI_AP_DEF: ");
  Serial.println((uint32_t)apNetif, HEX);
}

// -----------------------------------------------------------------------------
// Configure SoftAP DHCP DNS
// -----------------------------------------------------------------------------

void setApDnsDhcpOption() {
  esp_netif_t *ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

  if (!ap) {
    Serial.println("[DNS] WIFI_AP_DEF not found");
    return;
  }

  Serial.println();
  Serial.println("[DNS] Reconfiguring SoftAP DHCP server DNS option");

  esp_err_t err;

  err = esp_netif_dhcps_stop(ap);
  Serial.print("[DNS] esp_netif_dhcps_stop: ");
  Serial.println(err == ESP_OK ? "OK" : "FAILED/ALREADY STOPPED");

  esp_netif_dns_info_t dns;
  memset(&dns, 0, sizeof(dns));

  dns.ip.type = IPADDR_TYPE_V4;
  IP4_ADDR(&dns.ip.u_addr.ip4, AP_DNS_A, AP_DNS_B, AP_DNS_C, AP_DNS_D);

  err = esp_netif_set_dns_info(ap, ESP_NETIF_DNS_MAIN, &dns);
  Serial.print("[DNS] esp_netif_set_dns_info DNS_MAIN = 1.1.1.1: ");
  Serial.println(err == ESP_OK ? "OK" : "FAILED");

  uint8_t dns_offer = OFFER_DNS;

  err = esp_netif_dhcps_option(
    ap,
    ESP_NETIF_OP_SET,
    ESP_NETIF_DOMAIN_NAME_SERVER,
    &dns_offer,
    sizeof(dns_offer)
  );

  Serial.print("[DNS] esp_netif_dhcps_option OFFER_DNS: ");
  Serial.println(err == ESP_OK ? "OK" : "FAILED");

  err = esp_netif_dhcps_start(ap);
  Serial.print("[DNS] esp_netif_dhcps_start: ");
  Serial.println(err == ESP_OK ? "OK" : "FAILED/ALREADY STARTED");

  Serial.println("[DNS] Use DHCP on the phone/client.");
  Serial.println("[DNS] Forget the WiFi network after flashing, then reconnect.");
}

// -----------------------------------------------------------------------------
// lwIP netif scan and NAT enable by AP netif number
// -----------------------------------------------------------------------------

void printLwipNetifsAndEnableNatOnAp() {
  Serial.println();
  Serial.println("[LWIP] netif list:");

  IPAddress apIP = WiFi.softAPIP();

  for (struct netif *n = netif_list; n != NULL; n = n->next) {
    String ip   = String(ip4addr_ntoa(netif_ip4_addr(n)));
    String gw   = String(ip4addr_ntoa(netif_ip4_gw(n)));
    String mask = String(ip4addr_ntoa(netif_ip4_netmask(n)));

    Serial.print("[LWIP] name=");
    Serial.print(n->name[0]);
    Serial.print(n->name[1]);
    Serial.print(" num=");
    Serial.print(n->num);
    Serial.print(" ip=");
    Serial.print(ip);
    Serial.print(" gw=");
    Serial.print(gw);
    Serial.print(" mask=");
    Serial.print(mask);
    Serial.print(" flags=0x");
    Serial.println(n->flags, HEX);

    if (ip == apIP.toString()) {
      Serial.println("[LWIP] This netif matches SoftAP IP");
      Serial.print("[LWIP] Enabling NAPT by netif number: ");
      Serial.println(n->num);

      ip_napt_enable_no(n->num, 1);

      Serial.println("[LWIP] ip_napt_enable_no() called");
    }
  }
}

// -----------------------------------------------------------------------------
// NAT enable callback; runs in lwIP TCP/IP context
// -----------------------------------------------------------------------------

void enableNatCore(void *arg) {
  natEnableAttempts++;

  Serial.println();
  Serial.println("[NAT] tcpip_callback context");
  Serial.print("[NAT] Attempt: ");
  Serial.println(natEnableAttempts);

  findNetifs();

  if (ethNetif) {
    esp_err_t err = esp_netif_set_default_netif(ethNetif);
    Serial.print("[NAT] esp_netif_set_default_netif(ETH_DEF): ");
    Serial.println(err == ESP_OK ? "OK" : "FAILED");
  } else {
    Serial.println("[NAT] ETH_DEF handle not found");
  }

#if ESP_IDF_VERSION_MAJOR >= 5
  if (apNetif) {
    esp_err_t err = esp_netif_napt_enable(apNetif);
    Serial.print("[NAT] esp_netif_napt_enable(WIFI_AP_DEF): ");
    Serial.println(err == ESP_OK ? "OK" : "FAILED/ALREADY ENABLED");
  } else {
    Serial.println("[NAT] WIFI_AP_DEF handle not found");
  }
#endif

  IPAddress apIP = WiFi.softAPIP();

  Serial.print("[NAT] Enabling old lwIP NAPT by AP IP: ");
  Serial.println(apIP);

  ip_napt_enable(apIP, 1);

  Serial.println("[NAT] Scanning lwIP netifs and enabling NAPT by AP netif number");
  printLwipNetifsAndEnableNatOnAp();

  natEnabled = true;
  natRequested = false;

  Serial.println("[NAT] NAT enable sequence finished");
}

void requestNatEnable() {
  if (natEnabled || natRequested) return;

  if (!ethUp) {
    Serial.println("[NAT] Ethernet not up yet");
    return;
  }

  if (WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {
    Serial.println("[NAT] AP IP is 0.0.0.0");
    return;
  }

  natRequested = true;

  Serial.println("[NAT] Scheduling NAT enable via tcpip_callback()");
  err_t err = tcpip_callback(enableNatCore, NULL);

  if (err != ERR_OK) {
    Serial.print("[NAT] tcpip_callback failed, err=");
    Serial.println((int)err);
    natRequested = false;
  }
}

// -----------------------------------------------------------------------------
// WAN test from ESP32 itself
// -----------------------------------------------------------------------------

bool testWanTcp() {
  if (!ethUp) return false;

  NetworkClient client;

  Serial.println("[WAN TEST] ESP32 connecting to 1.1.1.1:80 over Ethernet route...");

  bool ok = client.connect(IPAddress(1, 1, 1, 1), 80, 3000);

  if (ok) {
    Serial.println("[WAN TEST] ESP32 TCP connect OK");
    client.stop();
    return true;
  }

  Serial.println("[WAN TEST] ESP32 TCP connect FAILED");
  return false;
}

// -----------------------------------------------------------------------------
// Local debug web page at http://192.168.4.1/
// -----------------------------------------------------------------------------

void handleRoot() {
  String s;

  s += "<!doctype html><html><head>";
  s += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  s += "<meta http-equiv='refresh' content='3'>";
  s += "<title>Olimex ESP32-POE-ISO ETH/WIFI NAT Debug</title>";
  s += "</head><body>";

  s += "<h1>Olimex ESP32-POE-ISO</h1>";
  s += "<h2>Ethernet to WiFi NAT Router Demo</h2>";

  s += "<p>";
  s += "Ethernet is WAN/uplink. WiFi SoftAP is LAN/downlink. ";
  s += "Clients connected to this AP are routed through Ethernet using NAPT.";
  s += "</p>";

  s += "<h2>Ethernet WAN</h2>";
  s += "ETH up: ";
  s += ethUp ? "YES" : "NO";
  s += "<br>ETH IP: ";
  s += ETH.localIP().toString();
  s += "<br>Gateway: ";
  s += ETH.gatewayIP().toString();
  s += "<br>DNS from Ethernet DHCP: ";
  s += ETH.dnsIP().toString();
  s += "<br>ETH MAC: ";
  s += ETH.macAddress();

  s += "<h2>WiFi AP LAN</h2>";
  s += "SSID: ";
  s += AP_SSID;
  s += "<br>AP IP / Gateway: ";
  s += WiFi.softAPIP().toString();
  s += "<br>AP MAC: ";
  s += WiFi.softAPmacAddress();
  s += "<br>Connected clients: ";
  s += String(WiFi.softAPgetStationNum());
  s += "<br>DHCP DNS offered to clients: 1.1.1.1";

  s += "<h2>NAT/NAPT</h2>";
  s += "NAT enabled flag: ";
  s += natEnabled ? "YES" : "NO";
  s += "<br>NAT requested flag: ";
  s += natRequested ? "YES" : "NO";
  s += "<br>NAT enable attempts: ";
  s += String(natEnableAttempts);

  s += "<h2>WAN test from ESP32</h2>";
  s += "TCP connect to 1.1.1.1:80: ";
  s += lastWanTcpTest ? "OK" : "FAILED";

  s += "<h2>Client setup</h2>";
  s += "Use DHCP, not static IP.<br>";
  s += "After flashing, forget this WiFi network and reconnect.";

  s += "<h2>Client tests</h2>";
  s += "1. This page: <b>http://192.168.4.1/</b><br>";
  s += "2. Plain HTTP: <a href='http://neverssl.com/'>http://neverssl.com/</a><br>";
  s += "3. Plain HTTP: <a href='http://example.com/'>http://example.com/</a><br>";
  s += "4. HTTPS/IP: <a href='https://1.1.1.1/'>https://1.1.1.1/</a><br>";

  s += "<h2>Build environment used</h2>";
  s += "Arduino IDE: 1.8.19<br>";
  s += "ESP32 Arduino core: 3.3.8<br>";
  s += "Board: ESP32 Dev Module<br>";
  s += "CPU: 240 MHz<br>";
  s += "Flash mode: QIO<br>";
  s += "Flash frequency: 80 MHz<br>";
  s += "Flash size: 4 MB<br>";
  s += "PSRAM: Disabled<br>";

  s += "</body></html>";

  server.send(200, "text/html", s);
}

// -----------------------------------------------------------------------------
// Serial status
// -----------------------------------------------------------------------------

void printStatus() {
  Serial.println();
  Serial.println("========== ROUTER STATUS ==========");

  Serial.print("ETH up: ");
  Serial.println(ethUp ? "YES" : "NO");

  Serial.print("ETH IP: ");
  Serial.println(ETH.localIP());

  Serial.print("ETH gateway: ");
  Serial.println(ETH.gatewayIP());

  Serial.print("ETH DNS: ");
  Serial.println(ETH.dnsIP());

  Serial.print("ETH MAC: ");
  Serial.println(ETH.macAddress());

  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  Serial.print("AP MAC: ");
  Serial.println(WiFi.softAPmacAddress());

  Serial.print("AP clients: ");
  Serial.println(WiFi.softAPgetStationNum());

  Serial.print("NAT enabled flag: ");
  Serial.println(natEnabled ? "YES" : "NO");

  Serial.print("NAT requested flag: ");
  Serial.println(natRequested ? "YES" : "NO");

  Serial.print("NAT attempts: ");
  Serial.println(natEnableAttempts);

  Serial.print("WAN TCP test from ESP32: ");
  Serial.println(lastWanTcpTest ? "OK" : "FAILED");

  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());

  Serial.println("===================================");
}

// -----------------------------------------------------------------------------
// Arduino WiFi/Ethernet event handler
// -----------------------------------------------------------------------------

void onEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      ETH.setHostname("olimex-esp32-poe-iso");
      Serial.println("[ETH] Started");
      break;

    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("[ETH] Link up");
      break;

    case ARDUINO_EVENT_ETH_GOT_IP:
      ethUp = true;

      Serial.println();
      Serial.println("[ETH] GOT IP");
      Serial.print("[ETH] IP: ");
      Serial.println(ETH.localIP());
      Serial.print("[ETH] Gateway: ");
      Serial.println(ETH.gatewayIP());
      Serial.print("[ETH] DNS: ");
      Serial.println(ETH.dnsIP());
      Serial.print("[ETH] MAC: ");
      Serial.println(ETH.macAddress());

      requestNatEnable();
      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      ethUp = false;
      natEnabled = false;
      natRequested = false;
      Serial.println("[ETH] Link down");
      break;

    case ARDUINO_EVENT_ETH_STOP:
      ethUp = false;
      natEnabled = false;
      natRequested = false;
      Serial.println("[ETH] Stopped");
      break;

    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.println("[AP] Started");
      Serial.print("[AP] IP: ");
      Serial.println(WiFi.softAPIP());
      break;

    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("[AP] Client connected");
      Serial.print("[AP] Clients now: ");
      Serial.println(WiFi.softAPgetStationNum());
      break;

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("[AP] Client disconnected");
      Serial.print("[AP] Clients now: ");
      Serial.println(WiFi.softAPgetStationNum());
      break;

    default:
      break;
  }
}

// -----------------------------------------------------------------------------
// Arduino setup()
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Olimex ESP32-POE-ISO Ethernet-to-WiFi NAT router demo");
  Serial.println("Arduino IDE 1.8.19, ESP32 Arduino core 3.3.8");
  Serial.println();

  WiFi.onEvent(onEvent);

  // Important for ESP32-POE-ISO:
  // Keep PHY power disabled before ETH.begin().
  // The LAN8720A/LAN87xx PHY must not be powered before the ESP32 RMII clock
  // is available on GPIO17.
  pinMode(ETH_PHY_POWER, OUTPUT);
  digitalWrite(ETH_PHY_POWER, LOW);
  delay(200);

  // Start WiFi SoftAP first.
  WiFi.mode(WIFI_AP);

  WiFi.softAPConfig(
    AP_IP,
    AP_GATEWAY,
    AP_NETMASK
  );

  bool apOk = WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("[AP] softAP result: ");
  Serial.println(apOk ? "OK" : "FAILED");

  // Reconfigure SoftAP DHCP DNS option.
  delay(500);
  setApDnsDhcpOption();

  // Start local debug page.
  server.on("/", handleRoot);
  server.begin();

  Serial.println("[HTTP] Debug server started");
  Serial.println("[HTTP] Open from AP client: http://192.168.4.1/");
  Serial.println("[CLIENT] Use DHCP. Forget WiFi after flashing, then reconnect.");

  // Start Ethernet.
  //
  // ESP32 Arduino core 3.3.8 ETH.begin() order:
  //   ETH.begin(type, phy_addr, mdc, mdio, power, clk_mode);
  //
  // For ESP32-POE-ISO:
  //   type      = LAN8720
  //   phy_addr  = 0
  //   power     = GPIO12
  //   clk_mode  = GPIO17 output
  ETH.begin(
    ETH_PHY_TYPE,
    ETH_PHY_ADDR,
    ETH_MDC_PIN,
    ETH_MDIO_PIN,
    ETH_PHY_POWER,
    ETH_CLK_MODE
  );
}

// -----------------------------------------------------------------------------
// Arduino loop()
// -----------------------------------------------------------------------------

void loop() {
  server.handleClient();

  if (ethUp && !natEnabled && !natRequested) {
    requestNatEnable();
  }

  if (millis() - lastWanTestMs > 10000) {
    lastWanTestMs = millis();
    lastWanTcpTest = testWanTcp();
    printStatus();
  }
}
