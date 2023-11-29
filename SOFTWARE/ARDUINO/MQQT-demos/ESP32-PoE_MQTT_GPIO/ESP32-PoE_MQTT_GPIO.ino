#include <WiFi.h>
#include <PubSubClient.h> // go to Sketch --> Include Library --> Manage Libraries... --> search for PubSubClient by Nick O'Leary
#include <string.h>

#define BUTTON  34

// WiFi
const char *ssid = "Your SSID"; // Enter your Wi-Fi name
const char *password = "Your password";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "emqx/ESP32-POE_Button";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // Publish and subscribe
    client.publish(topic, "Hi, I'm Olimex board ESP32-POE\n\rThis is button example\n\rType \"SB\" to scan button level");
    client.subscribe(topic);

    pinMode (BUTTON, INPUT_PULLUP);
}

void callback(char *topic_, byte *payload, unsigned int length)
{
  char OutputMessage[32], SB=0;
  payload[length]=0;
  SB  = strstr ((char*)payload, "SB")!=NULL;

  if (SB)
  {
    sprintf (OutputMessage, "Button is %d\n\r", digitalRead(BUTTON));
    client.publish((const char*)topic, (const char*)OutputMessage, strlen(OutputMessage));
  }
}

void loop() {
    client.loop();
}
