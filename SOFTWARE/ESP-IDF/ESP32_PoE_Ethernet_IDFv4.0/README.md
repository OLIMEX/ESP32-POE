This is a simple demo for Olimex ESP32-PoE revision B (or later) board.
The project is based on the ethernet example located in "examples/ethernet/basic" of ESP-IDF release v4.0: https://github.com/espressif/esp-idf/tree/release/v4.0
Pin numbers (RST removed and RMII clock to 17) in menuconfig are modified to match those of ESP32-PoE.

The demo sets up Ethernet connection and acquires IP address.

1) Setup the ESP-IDF development framework. You can download and install it by following the instructions here  https://github.com/espressif/esp-idf
If you use the Windows installer: https://dl.espressif.com/dl/esp-idf-tools-setup-2.1.exe when you choose version of the ESP-IDF select v4.0

2) Open terminal (or command prompt) and navigate to the project

3) If you want to change any settings open menuconfig and save the changes.

4) Connect ESP32-PoE to the USB. 

5) Compile this example and upload it to the board

6) Wait until firmware is flashed and open terminal on the ESP32-PoE port.

7) Wait until it is initialized. Your network needs DHCP.

8) When you get IP address you can ping the printed IP.


IMPORTANT: The example won't work with older (and very likely newer) versions of the ESP-IDF due to the changed structure of the libraries in the ESP-IDF.
If you want to ESP-IDF v3.xx you should use the project: ESP32_PoE_Ethernet_IDFv3.x

YYYY/MM/DD
2019/12/10
