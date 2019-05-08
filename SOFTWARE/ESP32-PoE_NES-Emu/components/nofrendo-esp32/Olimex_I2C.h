#ifndef	_OLIMEX_I2C_H
#define	_OLIMEX_I2C_H

// I2C macros
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           /*!< I2C master clock frequency */

#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

esp_err_t I2C_Read_Slave(unsigned char Address, i2c_port_t i2c_num, uint8_t* data_rd, size_t size);

esp_err_t I2C_Write_Slave(unsigned char Address, i2c_port_t i2c_num, uint8_t* data_wr, size_t size);

void I2C_Master_Init(int I2C_Num, int SCL_Pin, int SDA_Pin);

#endif
