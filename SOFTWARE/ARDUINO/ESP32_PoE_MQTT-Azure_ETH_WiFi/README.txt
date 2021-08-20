This example combines two default examples (to add Azure feature with Ethernet as well) from the Espressif Package:
1) ESP32 Azure IoT Arduino --> SimpleMQTT
2) WiFi --> ETH_LAN8720
<<<<<<< HEAD
In order to work you have to choose the prefered network option (ethernet/WiFi) depending on the ETHERNET macro on the top of the project. In case it is commented out, WiFi will be used and you have to input the SSID and password of your WiFi network. Ethernet is chosen by default.
=======
In order to work you have to choose the prefered network option (ethernet/WiFi) depending on the ETHERNET macro on the top of the project. In case it is commented WiFi will be used and you have to input the SSID and password of your WiFi network. Ethernet is chosen by default.
>>>>>>> origin/master
Also you have to input in "connectionString" the HostName, DeviceID and SharedAccessKey from Azure. In order to get these values follow the instructions on the Microsoft guide:
https://docs.microsoft.com/en-us/samples/azure-samples/esp-samples/esp-samples/
https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal