#ifndef	_TS_CONTROLLER_H
#define	_TS_CONTROLLER_H


// I2C macros
#if	(defined	CONFIG_HW_TS_CLK) && (defined CONFIG_HW_TS_DAT)
#define I2C_EXAMPLE_MASTER_SCL_IO          CONFIG_HW_TS_CLK /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO          CONFIG_HW_TS_DAT /*!< gpio number for I2C master data  */
#else
#define I2C_EXAMPLE_MASTER_SCL_IO          0
#define I2C_EXAMPLE_MASTER_SDA_IO          0
#endif
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           /*!< I2C master clock frequency */

#define ESP_SLAVE_ADDR                     0x4d             /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */


// Buttons number for event selection. Check the "ev" array inside video_audio.c file
#define	BUTTON_START	3
#define	BUTTON_SELECT	0
#define	BUTTON_UP		4
#define	BUTTON_DOWN		6
#define	BUTTON_LEFT		7
#define	BUTTON_RIGHT	5
#define	BUTTON_BUTTON_A	13
#define	BUTTON_BUTTON_B	14

struct _Rectangle
{
	int MinX, MaxX, MinY, MaxY;	// values of the Min/Max X/Y
};
typedef	struct _Rectangle Rectangle;

int Read_Touch_Screen (int *X, int *Y);
int Check_Pressed (Rectangle Button, int X, int Y);
void Calculate_XY (int *X, int *Y);
int Detect_Buttons (int X, int Y);
void TS_Init ();
int TS_Read_Input ();

#endif
