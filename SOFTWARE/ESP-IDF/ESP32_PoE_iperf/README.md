This demo is based on the original iperf demo in esp-idf with adapted configuration for ESP32-PoE.

In order to make it work:
1) Select the port where the board has connected using menuconfig. To do so type inside the console (msys32 or terminal):
make menuconfig

2) Navigate to "Menu flasher config --> Default serial port" select your port.

3) After that flash the board (this may take a while the first time after you change any of the configurations)
make flash

4) Then run the terminal
make monitor

5) Wait to initialize follow the instructions on the terminal:
5.1) Run the ethernet
ethernet start
5.2) Optionally if you want to see more details (IP address, mask, default gateway) type
ethernet macro

6) Run the iperf
6.1) Server mode:
iperf -s -i 1 -t 10
with this you will run it for 10 seconds transfer (more than that will trigger the watchdog reset) and 1 second interval.

6.2) Client mode:
iperf -c SERVER_IP_ADDRESS -i 1


You can check the sample data inside the ESP32-PoE_Data.txt file.

YYYY/MM/DD
2019/06/13
