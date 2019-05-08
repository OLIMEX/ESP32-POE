#ifndef	_OLIMEX_TS_CONTROLLER_H
#define	_OLIMEX_TS_CONTROLLER_H

#ifdef	CONFIG_HW_TS_ENA
#define TS_I2C_ADDR	0x4D
#define TS_MASTER_NUM               CONFIG_HW_TS_I2C_NUM	/*!< I2C port number for master dev */
#define TS_MASTER_SCL_IO            CONFIG_HW_TS_CLK		/*!< gpio number for I2C master clock */
#define TS_MASTER_SDA_IO            CONFIG_HW_TS_DAT		/*!< gpio number for I2C master data  */

// Buttons number for event selection. Check the "ev" array inside video_audio.c file
#if	(CONFIG_TS_PLAYER == 1)
	#define	TS_BUTTON_START		0
	#define	TS_BUTTON_SELECT	1
	#define	TS_BUTTON_A			2
	#define	TS_BUTTON_B			3
	#define	TS_BUTTON_UP		4
	#define	TS_BUTTON_DOWN		5
	#define	TS_BUTTON_LEFT		6
	#define	TS_BUTTON_RIGHT		7
#else
	#define	TS_BUTTON_START		8
	#define	TS_BUTTON_SELECT	9
	#define	TS_BUTTON_A			10
	#define	TS_BUTTON_B			11
	#define	TS_BUTTON_UP		12
	#define	TS_BUTTON_DOWN		13
	#define	TS_BUTTON_LEFT		14
	#define	TS_BUTTON_RIGHT		15
#endif

struct _Rectangle
{
	int MinX, MaxX, MinY, MaxY;	// values of the Min/Max X/Y
};
typedef	struct _Rectangle Rectangle;

struct _Coordinates
{
	int X, Y;
};
typedef	struct _Coordinates Coordinates;

int TS_Read_I2C_Data (unsigned char Data[]);
int TS_Check_Pressed (Rectangle Button, int X, int Y);
Coordinates TS_Calculate_Data (unsigned char InputData[]);
int TS_Detect_Buttons (Coordinates Data);
void TS_Init ();
int TS_Read_Input ();
#endif	// #ifdef CONFIG_HW_TS_ENA

#endif	// #ifndef	_OLIMEX_TS_CONTROLLER_H
