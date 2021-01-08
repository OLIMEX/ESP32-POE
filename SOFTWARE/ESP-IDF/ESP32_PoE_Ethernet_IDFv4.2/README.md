# Ethernet Example

## Overview

This example is based on the original example of Espressif "ethernet\basic" and adapted to function with Olimex boards ESP32-PoE and ESP32-PoE-ISO. It demonstrates basic usage of `Ethernet driver` together with `tcpip_adapter`. The work flow of the example could be as follows:

1. Install Ethernet driver
2. Send DHCP requests and wait for a DHCP lease
3. If get IP address successfully, then you will be able to ping the device

If you have a new Ethernet application to go (for example, connect to IoT cloud via Ethernet), try this as a basic template, then add your own code.

## How to use example

### Hardware Required

To run this example, it's recommended that you have one of the Olimex boards [ESP32-PoE](https://www.olimex.com/Products/IoT/ESP32/ESP32-POE/) or [ESP32-PoE-ISO](https://www.olimex.com/Products/IoT/ESP32/ESP32-POE-ISO/). This example should also work for 3rd party ESP32 board as long as it's integrated with a supported Ethernet PHY chip. Up until now, ESP-IDF supports up to four Ethernet PHY: `LAN8720`, `IP101`, `DP83848` and `RTL8201`, additional PHY drivers should be implemented by users themselves.

Besides that, `esp_eth` component can drive third-party Ethernet module which integrates MAC and PHY and provides common communication interface (e.g. SPI, USB, etc).This example will take the **DM9051** as an example, illustrating how to install the Ethernet driver in the same manner. In this project by default this feature is disabled. If you want to enable it:
```
idf.py menuconfig
```
then option and navigate to "Component config -> Ethernet" and check the "Support SPI to Ethernet Module" and then in that submenu check "Use DM9051".

#### Pin Assignment

| GPIO   | RMII Signal | Notes        |
| ------ | ----------- | ------------ |
| GPIO21 | TX_EN       | EMAC_TX_EN   |
| GPIO19 | TX0         | EMAC_TXD0    |
| GPIO22 | TX1         | EMAC_TXD1    |
| GPIO25 | RX0         | EMAC_RXD0    |
| GPIO26 | RX1         | EMAC_RXD1    |
| GPIO27 | CRS_DV      | EMAC_RX_DRV  |
| GPIO17 | CLK         | EMAC_CLK_OUT |

* SMI (Serial Management Interface) wiring is not fixed. You may need to changed it according to your board schematic. By default they're connected as follows:

| GPIO   | SMI Signal  | Notes         |
| ------ | ----------- | ------------- |
| GPIO23 | MDC         | Output to PHY |
| GPIO18 | MDIO        | Bidirectional |

* PHY chip has a reset pin, but is not connected to the ESP32-PoE(-ISO) and thus it is left as -1 value.
### Configure the project

```
idf.py menuconfig
```

In the Example Configuration menu:

Choose the kind of Ethernet under Ethernet Type.
If Internal EMAC is selected:
Choose PHY device under Ethernet PHY Device, by default, ESP32-PoE(-ISO) has an LAN8710A on board but it works fine with LAN8720 selected from the menuconfig.
Set GPIO number used by SMI signal under SMI MDC GPIO number and SMI MDIO GPIO number respectively.
If DM9051 Module is selected:
Set SPI specific configuration, including SPI host number, GPIO number and clock rate.
Set GPIO number used by PHY chip reset under PHY Reset GPIO number, you may have to change the default value according to your board schematic. PHY hardware reset can be disabled by set this value to -1.
Set PHY address under PHY Address, you may have to change the default value according to your board schematic.
For this project values are as follows:
|   Value  |         Option        |
| -------- | -------------------   |
| Internal |     Ethernet Type     |
| LAN8720  |  Ethernet PHY Device  |
|  GPIO23  |      SMI MDC GPIO     |
|  GPIO18  |     SMI MDIO GPIO     |
|    -1    | PHY Reset GPIO number |
|     0    |      PHY Address      |


In the Component config > Ethernet menu:

Under Support ESP32 internal EMAC controller sub-menu:
In the PHY interface, select Reduced Media Independent Interface (RMII), ESP-IDF currently only support RMII mode.
In the RMII clock mode, select one of the source that RMII clock (50MHz) comes from: Input RMII clock from external or Output RMII clock from internal.
If Output RMII clock from internal is enabled, you also have to set the GPIO number that used to output the RMII clock, under RMII clock GPIO number. In this case, you can set the GPIO number to 16 or 17.
If Output RMII clock from GPIO0 (Experimental!) is also enabled, then you have no choice but GPIO0 to output the RMII clock.
In Amount of Ethernet DMA Rx buffers and Amount of Ethernet DMA Tx buffers, you can set the amount of DMA buffers used for Tx and Rx.
Under Support SPI to Ethernet Module sub-menu, select the SPI module that you used for this example. Currently ESP-IDF only supports DM9051.
For this project values are as follows:
|   Value   |       Option       |
| --------- | -------------------|
|   RMII    |   PHY Interface    |
|  Output   |  RMII clock mode   |
| unchecked |  Output RMII clock |
|  GPIO17   |  clock PGIO number |
|    512    |   DMA buffer Size  |
|    10     |   DMA Rx buffers   |
|    10     |   DMA Tx buffers   |

### Build, Flash, and Run

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT build flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```bash
I (394) eth_example: Ethernet Started
I (3934) eth_example: Ethernet Link Up
I (3934) eth_example: Ethernet HW Addr 30:ae:a4:c6:87:5b
I (5864) tcpip_adapter: eth ip: 192.168.2.151, mask: 255.255.255.0, gw: 192.168.2.2
I (5864) eth_example: Ethernet Got IP Address
I (5864) eth_example: ~~~~~~~~~~~
I (5864) eth_example: ETHIP:192.168.2.151
I (5874) eth_example: ETHMASK:255.255.255.0
I (5874) eth_example: ETHGW:192.168.2.2
I (5884) eth_example: ~~~~~~~~~~~
```

Now you can ping your ESP32 in the terminal by entering `ping 192.168.2.151` (it depends on the actual IP address you get).

