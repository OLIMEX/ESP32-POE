#include <ETH.h>
#include <PubSubClient.h> // go to Sketch --> Include Library --> Manage Libraries... --> search for PubSubClient by Nick O'Leary
#include <string.h>

#include "DS3231.h"
DS3231 RTC;

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "emqx/ESP32-POE_I2C";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    ETH.begin();

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
    client.publish(topic, "Hi, I'm ESP32 ^^");
    client.subscribe(topic);
}

void callback(char *topic_, byte *payload, unsigned int length)
{
  char OutputMessage[32], ShowDate=0, ShowTime=0;
  payload[length]=0;
  RTC.UpdateData ();
  ShowTime = strstr ((char*)payload, "time")!=NULL;
  ShowDate = strstr ((char*)payload, "date")!=NULL;
  if (ShowTime)
  {
    sprintf (OutputMessage, "Time: %02d:%02d:%02d", RTC.Hours, RTC.Minutes, RTC.Seconds);
    client.publish((const char*)topic, (const char*)OutputMessage, strlen(OutputMessage));
  }

  if (ShowDate)
  {
    sprintf (OutputMessage, "Date: %04d/%02d/%02d (%s)", RTC.Year+2000, RTC.Month, RTC.Date, DaysOfTheWeek[RTC.Day]);
    client.publish((const char*)topic, (const char*)OutputMessage, strlen(OutputMessage));
  }
}

void loop() {
    client.loop();
}
