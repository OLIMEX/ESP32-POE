#ifndef	_OLIMEX_NUNCHUCK_CONTROLLER_H
#define	_OLIMEX_NUNCHUCK_CONTROLLER_H


#ifdef	CONFIG_HW_NUNCHUCK_ENA
#define NUNCHUCK_I2C_ADDR	0x52
#define NUNCHUCK_MASTER_NUM         CONFIG_HW_NUNCHUCK_I2C_NUM	/*!< I2C port number for master dev */
#define NUNCHUCK_MASTER_SCL_IO      CONFIG_HW_NUNCHUCK_CLK		/*!< gpio number for I2C master clock */
#define NUNCHUCK_MASTER_SDA_IO      CONFIG_HW_NUNCHUCK_DAT		/*!< gpio number for I2C master data  */

#define	BOARD_BUTTON	34

// Buttons number for event selection. Check the "ev" array inside video_audio.c file
#if	(CONFIG_NUNCHUCK_PLAYER == 1)
	#define	NUNCHUCK_BUTTON_START	0
	#define	NUNCHUCK_BUTTON_SELECT	1
	#define	NUNCHUCK_BUTTON_A		2
	#define	NUNCHUCK_BUTTON_B		3
	#define	NUNCHUCK_BUTTON_UP		4
	#define	NUNCHUCK_BUTTON_DOWN	5
	#define	NUNCHUCK_BUTTON_LEFT	6
	#define	NUNCHUCK_BUTTON_RIGHT	7
#else
	#define	NUNCHUCK_BUTTON_START	8
	#define	NUNCHUCK_BUTTON_SELECT	9
	#define	NUNCHUCK_BUTTON_A		10
	#define	NUNCHUCK_BUTTON_B		11
	#define	NUNCHUCK_BUTTON_UP		12
	#define	NUNCHUCK_BUTTON_DOWN	13
	#define	NUNCHUCK_BUTTON_LEFT	14
	#define	NUNCHUCK_BUTTON_RIGHT	15
#endif

struct _Nunchuck
{
	int JoyX, JoyY, AccX, AccY, AccZ, ButC, ButZ;
};
typedef struct _Nunchuck Nunchuck;

int Nunchuck_Read_I2C_Data (unsigned char Data[]);
Nunchuck Nunchuck_Calculate_Data (unsigned char InputData[]);
int Nunchuck_Detect_Buttons (Nunchuck Data);
void Nunchuck_Init ();
int Nunchuck_Read_Input ();

#endif	// #ifdef CONFIG_HW_NUNCHUCK_ENA

#endif	// #ifdef _OLIMEX_NUNCHUCK_CONTROLLER_H
