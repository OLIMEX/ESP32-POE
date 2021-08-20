/**
 * A simple Azure IoT example for sending telemetry.
 * This example combines two default examples (to add Azure feature with Ethernet as well) from the Espressif Package:
 * 1) ESP32 Azure IoT Arduino --> SimpleMQTT
 * 2) WiFi --> ETH_LAN8720
 */

#define ETHERNET  // commenting this line wil switch to WiFi

#ifdef  ETHERNET
#include <ETH.h>
#else
#include <WiFi.h>
#endif

#include "Esp32MQTTClient.h"


#ifndef  ETHERNET
// Please input the SSID and password of WiFi
const char* ssid     = "";
const char* password = "";
#endif

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
//static const char* connectionString = "HostName=YourHostName.azure-devices.net;DeviceId=YourDeviceID;SharedAccessKey=zGLJMK05fn4jhyHAXl5rX2eLb4023j8C6zLvBoG2D8Y="; // you have to change this string with your actual taken from the Azure generator

static bool hasIoTHub = false;

#ifdef  ETHERNET
static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
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
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}
#endif

void setup() {
  Serial.begin(115200);
  delay(10);
  #ifdef  ETHERNET
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  #else
  Serial.println("Starting connecting WiFi.");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  #endif

  if (!Esp32MQTTClient_Init((const uint8_t*)connectionString))
  {
    hasIoTHub = false;
    Serial.println("Initializing IoT hub failed.");
    return;
  }
  hasIoTHub = true;
}

void loop() {
  Serial.println("start sending events.");
  if (hasIoTHub)
  {
    char buff[128];

    // replace the following line with your data sent to Azure IoTHub
    snprintf(buff, 128, "{\"topic\":\"iot\"}");
    
    if (Esp32MQTTClient_SendEvent(buff))
    {
      Serial.println("Sending data succeed");
    }
    else
    {
      Serial.println("Failure...");
    }
    delay(5000);
  }
}
