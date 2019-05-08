#include <stdio.h>
#include "driver/i2c.h"
#include "Olimex_I2C.h"

#if ((defined CONFIG_HW_TS_ENA) && (defined CONFIG_HW_NUNCHUCK_ENA))
#warning Both controllers are enabled and there is a chance of short spikes of the video to occur periodically.
#endif

esp_err_t I2C_Read_Slave(unsigned char Address, i2c_port_t i2c_num, uint8_t* data_rd, size_t size)
{
	if (size == 0)
		return ESP_OK;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ( Address << 1 ) | READ_BIT, ACK_CHECK_EN);
	if (size > 1)
		i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
	i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}

esp_err_t I2C_Write_Slave(unsigned char Address, i2c_port_t i2c_num, uint8_t* data_wr, size_t size)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ( Address << 1 ) | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}

void I2C_Master_Init(int I2C_Num, int SCL_Pin, int SDA_Pin)
{
	#if	(defined CONFIG_HW_NUNCHUCK_ENA) || (defined CONFIG_HW_TS_ENA)
	int i2c_master_port = I2C_Num;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = SDA_Pin;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = SCL_Pin;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
	i2c_param_config(i2c_master_port, &conf);
	i2c_driver_install(i2c_master_port, conf.mode, I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
	#endif
}
