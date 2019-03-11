// Copyright (c) 2017 Olimex Ltd.
//
// GNU GENERAL PUBLIC LICENSE
//    Version 3, 29 June 2007
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_event.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_eth.h"

#include "rom/ets_sys.h"
#include "rom/gpio.h"

#include "soc/dport_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"

#include "tcpip_adapter.h"
#include "nvs_flash.h"
#include "driver/gpio.h"


#include "eth_phy/phy_lan8720.h"
#include "olimex_ethernet.h"

#define PIN_SMI_MDC   23
#define PIN_SMI_MDIO  18
#define PIN_PHY_POWER 12

static void phy_device_power_enable_via_gpio(bool enable)
{
	if (!enable)
		phy_lan8720_default_ethernet_config.phy_power_enable(false);

	gpio_pad_select_gpio(PIN_PHY_POWER);
	gpio_set_direction(PIN_PHY_POWER,GPIO_MODE_OUTPUT);
	gpio_set_level(PIN_PHY_POWER, (int)enable);

	// Allow the power up/down to take effect, min 300us
	vTaskDelay(1);

	if (enable)
		phy_lan8720_default_ethernet_config.phy_power_enable(true);
}

static void eth_gpio_config_rmii(void)
{
    // RMII data pins are fixed:
    // TXD0 = GPIO19
    // TXD1 = GPIO22
    // TX_EN = GPIO21
    // RXD0 = GPIO25
    // RXD1 = GPIO26
    // CLK == GPIO0
    phy_rmii_configure_data_interface_pins();
    // MDC is GPIO 23, MDIO is GPIO 18
    phy_rmii_smi_configure_pins(PIN_SMI_MDC, PIN_SMI_MDIO);
}

void eth_task(void *pvParameter)
{
    tcpip_adapter_ip_info_t ip;
    memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1) {

        vTaskDelay(2000 / portTICK_PERIOD_MS);

        if (tcpip_adapter_get_ip_info(ESP_IF_ETH, &ip) == 0) {
            ESP_LOGI(ETHERNET_TAG, "~~~~~~~~~~~");
            ESP_LOGI(ETHERNET_TAG, "ETHIP:"IPSTR, IP2STR(&ip.ip));
            ESP_LOGI(ETHERNET_TAG, "ETHPMASK:"IPSTR, IP2STR(&ip.netmask));
            ESP_LOGI(ETHERNET_TAG, "ETHPGW:"IPSTR, IP2STR(&ip.gw));
            ESP_LOGI(ETHERNET_TAG, "~~~~~~~~~~~");
        }
    }
}

uint8_t initEthernet()
{
	eth_config_t config = phy_lan8720_default_ethernet_config;
	esp_err_t ret = ESP_OK;

	/* Initialize adapter */
	tcpip_adapter_init();
	esp_event_loop_init(NULL, NULL);

	/* Set the PHY address in the example configuration */
	config.phy_addr = 0;
	config.gpio_config = eth_gpio_config_rmii;
	config.tcpip_input = tcpip_adapter_eth_input;
	config.phy_power_enable = phy_device_power_enable_via_gpio;

	/* Chanege clock mode */
	config.clock_mode = ETH_CLOCK_GPIO17_OUT;

	/* Initialize ethernet */
	ret = esp_eth_init(&config);
	if(ret != ESP_OK)
		return ret;

	/* Enable ethernet */
	return esp_eth_enable();

}

uint8_t testEthernet()
{
    tcpip_adapter_ip_info_t ip;
    uint8_t timeout = 0;

    /* Clear ip info */
    memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));



    /* Initialize ethernet */
    printf("Initialize LAN..................................");
    fflush(stdout);
    if(initEthernet() != ESP_OK) {
        printf("[ \033[31mERROR\033[0m ]\n");
        esp_eth_disable();
        return ESP_FAIL;
    }
    printf("[ \033[32mCOMPLETE\033[0m ]\n");

    /* Wait for IP */
    printf("Receiving IP address............................");
    fflush(stdout);
    while(timeout++ < 10) {
        if (tcpip_adapter_get_ip_info(ESP_IF_ETH, &ip) == 0)
            if(ip.gw.addr == 0x0100a8c0)
                break;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if(ip.gw.addr == 0x0100a8c0)
        printf("[ \033[32mCOMPLETE\033[0m ]\n");
    else
        printf("[ \033[31mERROR\033[0m ]\n");

	// SPP +
	int x = ip.ip.addr, A[4];
	A[0] = (x >>  0) & 0xFF;
	A[1] = (x >>  8) & 0xFF;
	A[2] = (x >> 16) & 0xFF;
	A[3] = (x >> 24) & 0xFF;
	printf ("IP address: %d.%d.%d.%d\n", A[0], A[1], A[2], A[3]);

    //esp_eth_disable();
    return ESP_OK;
}
