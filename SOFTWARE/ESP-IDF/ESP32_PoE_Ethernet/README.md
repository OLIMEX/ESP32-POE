This is a simple demo for Olimex ESP32-PoE revision B board.

The demo sets up Ethernet connection and acquires IP address. It is made with ESP-IDF.

In order to get it the software you should:

1) Setup the ESP-IDF development framework. You can download and install it by following the instructions here  https://github.com/espressif/esp-idf

2) Start: "msys32/mingw32.exe" and navigate to this project's directory.

3) Connect ESP32-PoE to the USB and specify the board's USB port in the ESP-IDF menu:

3.1) "make menuconfig"
3.2) Navigate to: Serial flasher config --> Default serial port --> <here select proper USB port>
3.3) Save the settings.

4) Compile this example and upload it to the board with command: "make flash"

5) Wait until firmware is flashed and open terminal on the ESP32-PoE port.

6) Press enter to start the demo and wait until it is initialized. Your network needs DHCP.

7) When you get IP address you can ping the printed IP.

2018/09/14
