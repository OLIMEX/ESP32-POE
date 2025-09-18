This code demonstrates Over-the-Air upgrade functionality over Ethernet (e.g. remote update).

This demo is compatible with ESP32-POE and ESP32-POE-ISO without any changes. 

WROVER variants of ESP32-POE and ESP32-POE-ISO require slight modifcation (changing GPIO17 to GPIO0).

The other Olimex-made boards require easy edit to fit the hardware setup, just change the defintions in:

ETH.begin(ETH_PHY_LAN8720, 0, 23, 18, 12, ETH_CLOCK_GPIO17_OUT);

What is this demo?

It allows OTA downloads via Ethernet. It creates web-server and prints in the serial terminal the IP address of the web-server. The downloads are done via the interface of the web-page at the IP address assigned by DHCP. The web-page also prints firmware version, IP address and size of binary.

How to use this demo?

Author used Arduino IDE 1.8.19 and version 3.1.3 of the esp32 package for Arduino released by espressif systems.

It is recommended to attach a LED between GND and GPIO4 at the UEXT (pins #2 and #3) for quicker debug.

Make sure the exact board is selected in Arduino IDE and proper COM then load the OTA-ETHERNET-PAGE.ino and then Upload it to the board.

Start serial terminal to see IP address asigned but if you missed it just reset the board from the RST1 button. Then navigate to the IP address via a web browser.

The interface requires binary to upload. You can create such binary via Arduino IDE from Sketch -> "Export compiled binary". Open OTA-ETHERNET-PAGE-BLINK1000.ino and OTA-ETHERNET-PAGE-BLINK5000.ino and click export compiled binary on both. Use these binaries to upload to the board via the Ethernet interface. If you have attached a LED to GPIO4 you'd see it start blinkig and also the web-page should get different firmware version for different binaries you use.

Refer to the pictures.