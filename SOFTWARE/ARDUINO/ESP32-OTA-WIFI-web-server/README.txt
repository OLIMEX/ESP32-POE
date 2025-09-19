This code demonstrates Over-the-Air upgrade functionality over WIFI (e.g. remote update).

This demo is compatible with ESP32-POE and ESP32-POE-ISO without any changes.

It is implementation of the free version of ElegantOTA. It is worth checking and supporting the ElegantOTA project.

How to use this demo?

Author used Arduino IDE 1.8.19 and version 3.1.3 of the esp32 package for Arduino released by espressif systems. You also need to install ElegantOTA via the Arduino Library Manager, follow the guide here:

https://docs.elegantota.pro/getting-started/installation

You need to change the WIFI credentials with yours (SSID, PASSWORD).

It is recommended to attach a LED between GND and GPIO4 at the UEXT (pins #2 and #3) for quicker empirical debug.

Make sure the exact board is selected in Arduino IDE and proper COM then open the ota-server.ino and then Upload it to the board. Check the serial for web-page address and go to the address via a browser and append /upload to land the upload page.

The interface requires binary to upload. You can create such binary via Arduino IDE from Sketch -> "Export compiled binary". Open ota-server-gpio1000.ino and ota-server-gpio5000.ino and click export compiled binary on both. Use these binaries to upload to the board via the Ethernet interface. If you have attached a LED to GPIO4 you'd see it start blink every 1s or every 5s depending on the code.
