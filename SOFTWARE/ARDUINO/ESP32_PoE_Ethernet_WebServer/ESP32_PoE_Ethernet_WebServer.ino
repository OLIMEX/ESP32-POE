/*
 * This example combines the Espressif examples:
 * WebServer --> HelloServer
 * Ethernet --> ETH_LAN8720
 * It implements a webserver using the ethernet interface (instead of WiFi)
 *
 * Once the sktech is compiled and uploaded, the board will log
 * its IP address to the Serial Monitor (should be set to 115000 bauds)
 * 
 * Open the IP in your browser and you should see
 * a message on the homepage: "hello from esp32!"
 * If you append "/inline" to the end of the address
 * you should see: "this works as well"
 * 
 * Tested with Espressif package (2.0.11)
 * and Olimex board ESP32-PoE and ESP32-EVB
*/

#include <ETH.h>
#include <WebServer.h>
#include <ESPmDNS.h>

static bool eth_connected = false;
WebServer server(80);

const int led = 13;

// Web Server: handle a request to / (root of the server)
void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp32!");
  delay(100);    // Wait x ms so we have time to see the LEd blinking
  digitalWrite(led, 0);
}

// Web Server: handle a request to an unknown URI (unknown "File")
void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  // digitalWrite(led, 0);  // If this is commented out, the LED will stay on in case of 404 error
}

// Handle Ethernet Events:
void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {

    case ARDUINO_EVENT_ETH_START:
      // This will happen during setup, when the Ethernet service starts
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;

    case ARDUINO_EVENT_ETH_CONNECTED:
      // This will happen when the Ethernet cable is plugged 
      Serial.println("ETH Connected");
      break;

    case ARDUINO_EVENT_ETH_GOT_IP:
    // This will happen when we obtain an IP address through DHCP:
      Serial.print("Got an IP Address for ETH MAC: ");
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

      // Uncomment to automatically make a test connection to a server:
      // testClient( "192.168.0.1", 80 );

      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      // This will happen when the Ethernet cable is unplugged 
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;

    case ARDUINO_EVENT_ETH_STOP:
      // This will happen when the ETH interface is stopped but this never happens
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;

    default:
      break;
  }
}

// Try to read something from a webserver:
void testClient(const char * host, uint16_t port)
{
  Serial.print("\nConnecting to ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available());
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
}

// Initializing everything at start up / after reset:
void setup()
{
  // Wait for the hardware to initialize:
  delay(500);

  // This sketch will log some information to the serial console:


  Serial.begin(115200); // Assuming computer will be connected to serial port at 115200 bauds
  Serial.print("Setup...");
  
  // Add a handler for network events. This is misnamed "WiFi" because the ESP32 is historically WiFi only,
  // but in our case, this will react to Ethernet events.
  Serial.print("Registering event handler for ETH events...");
  WiFi.onEvent(WiFiEvent);
  
  // Starth Ethernet (this does NOT start WiFi at the same time)
  Serial.print("Starting ETH interface...");
  ETH.begin();

  
  // multicast DNS (mDNS) allows to resolve hostnames to IP addresses without a DNS server
  if (MDNS.begin("esp32")) {  // using mDNS name "esp32"
    Serial.println("MDNS responder started");
  }

  // Web Server handlers: 
  // Handle a request to / (root of the server)
  server.on("/", handleRoot);
  // Minimalistic handling of another URI (LED will not flash on this one):
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  // Handle all other URIs:
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  pinMode( led, OUTPUT);  // Initialize the LED pin as a digital output (on/off)
}

void loop ()
{
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}
