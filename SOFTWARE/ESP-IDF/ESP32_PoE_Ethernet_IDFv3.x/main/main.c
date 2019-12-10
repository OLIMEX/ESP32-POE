#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"

#include <esp_types.h>



#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_vfs_fat.h"
#include "esp_task_wdt.h"
#include "esp_eth.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "rom/uart.h"


#include "ethernet/olimex_ethernet.h"
#include "eth_phy/phy_lan8720.h"

#define TAG "OLIMEX"

#define UART_FLUSH()        while(uart_rx_one_char(&ch) == ESP_OK)
#define UART_WAIT_KEY()     while(uart_rx_one_char(&ch) != ESP_OK) \
                                vTaskDelay(1 / portTICK_PERIOD_MS)

void app_main(void)
{
	uint8_t ch;
	uint16_t timeout_ms;

	printf("\nESP32-PoE Ethernet Demo. Press \033[1mENTER\033[0m to start.\n\n");

	printf("-------------------------------------\n");
	UART_WAIT_KEY();
	while (1)
	{
		printf("\n\033[1m========== Ethernet Demo ==========\033[0m\n");
		printf("Plug Ethernet cable and press Enter\n");
		UART_WAIT_KEY();
		printf("Start...........................................");
		fflush(stdout);
		UART_FLUSH();

		timeout_ms = 1000;
		while(--timeout_ms && uart_rx_one_char(&ch) != ESP_OK)
			vTaskDelay(1 / portTICK_PERIOD_MS);
		if(timeout_ms)
		{
			printf("[ \033[35m------\033[0m ]\n");
		}
		else
		{
			printf("[ \033[32mCOMPLETE\033[0m ]\n");
			testEthernet();
		}
		UART_FLUSH();
		printf("Press any key to start again!\n");
		UART_WAIT_KEY();
	}

    while(1)
        vTaskDelay(1000 / portTICK_PERIOD_MS);
}
