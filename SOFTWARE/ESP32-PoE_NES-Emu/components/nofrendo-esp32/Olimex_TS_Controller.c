#include <stdio.h>
#include "driver/i2c.h"

#include "Olimex_I2C.h"
#include "Olimex_TS_Controller.h"
#ifdef	CONFIG_HW_TS_ENA

// empirically derived data: MinX = 307; MaxX = 7759; MinY = 541; MaxY = 7767
// also needs to change the X and Y axis
#define	MAX_X	7450
#define	MAX_Y	7200
#define	OFFSET_X	307
#define	OFFSET_Y	541

const Rectangle
//                 MinX  MaxX, MinY  MaxY
Button_Start    = {   0, 1000, 6500, 7500},
Button_Select   = {6500, 7500, 6500, 7500},
Button_Up       = {3250, 4250, 6500, 7500},
Button_Down     = {3250, 4250,    0, 1000},
Button_Left     = {   0, 1000, 3250, 4250},
Button_Right    = {6500, 7500, 3250, 4250},
Button_ButtonA  = {   0, 1000,    0, 1000},
Button_ButtonB  = {6500, 7500,    0, 1000},

// up + left	1300 5400
// up + right	5400 5200
// up + A		1500 4400
// up + B		5400 3400
// down + left	2100 1800
// down + right	5100 2200
// down + A		2000  700
// down + B		5100  700
// left + A		 500 1700
// left + B		4000 1600
// right + A	2600 2300
// right + B	6800 2300
Combo_Up_Left    = {1300-500, 1300+500, 5400-500, 5400+500},
Combo_Up_Right   = {5400-500, 5400+500, 5200-500, 5200+500},
Combo_Up_ButA    = {1500-500, 1500+500, 4400-500, 4400+500},
Combo_Up_ButB    = {5400-500, 5400+500, 3400-500, 3400+500},

Combo_Down_Left  = {2100-500, 2100+500, 1800-500, 1800+500},
Combo_Down_Right = {5100-500, 5100+500, 2200-500, 2200+500},
Combo_Down_ButA  = {2000-500, 2000+500,  700-500,  700+500},
Combo_Down_ButB  = {5100-500, 5100+500,  700-500,  700+500},

Combo_Left_ButA  = { 500-500,  500+500, 1700-500, 1700+500},
Combo_Left_ButB  = {4000-500, 4000+500, 1600-500, 1600+500},
Combo_Right_ButA = {2600-500, 2600+500, 2300-500, 2300+500},
Combo_Right_ButB = {6800-500, 6800+500, 2300-500, 2300+500};


int TS_Read_I2C_Data (unsigned char Data[])
{
	//#define	PRINT_BYTES
	I2C_Read_Slave(TS_I2C_ADDR, TS_MASTER_NUM, Data, 5);
	#ifdef	PRINT_BYTES
	for (int i=0; i<5; i++)
		printf ("Byte[%d] = %d (0x%x);\t", i, Data[i], Data[i]);
	printf ("\n");
	#endif
	if (Data[0] != 0x81)	// if screen is touched 0th byte will have value 0x81
		return 0;

	return 1;
}

Coordinates TS_Calculate_Data (unsigned char InputData[])
{
	Coordinates OutputData;
	// convert I2C data into TS coordinates
	OutputData.X = ((int)InputData[2]<<8) | InputData[1];
	OutputData.Y = ((int)InputData[4]<<8) | InputData[3];

	// fix the offset
	OutputData.X  = OutputData.X - OFFSET_X;
	OutputData.Y  = OutputData.Y - OFFSET_Y;
	
	// fix the direction of the value
	OutputData.Y  = MAX_Y - OutputData.Y;
	if (OutputData.Y < 0)
		OutputData.Y = 0;

	// fix the axis
	OutputData.X  = OutputData.X  ^ OutputData.Y;
	OutputData.Y  = OutputData.X  ^ OutputData.Y;
	OutputData.X  = OutputData.X  ^ OutputData.Y;

	return OutputData;
}

int TS_Detect_Buttons (Coordinates Data)
{
	int Result = 0xffff;

	Result &= ~(TS_Check_Pressed (Button_Start  , Data.X, Data.Y)<<TS_BUTTON_START);
	Result &= ~(TS_Check_Pressed (Button_Select , Data.X, Data.Y)<<TS_BUTTON_SELECT);
	Result &= ~(TS_Check_Pressed (Button_ButtonA, Data.X, Data.Y)<<TS_BUTTON_A);
	Result &= ~(TS_Check_Pressed (Button_ButtonB, Data.X, Data.Y)<<TS_BUTTON_B);

	Result &= ~(TS_Check_Pressed (Button_Up     , Data.X, Data.Y)<<TS_BUTTON_UP);
	Result &= ~(TS_Check_Pressed (Button_Down   , Data.X, Data.Y)<<TS_BUTTON_DOWN);
	Result &= ~(TS_Check_Pressed (Button_Left   , Data.X, Data.Y)<<TS_BUTTON_LEFT);
	Result &= ~(TS_Check_Pressed (Button_Right  , Data.X, Data.Y)<<TS_BUTTON_RIGHT);

	if (TS_Check_Pressed(Combo_Up_Left , Data.X, Data.Y))Result&=~((1<<TS_BUTTON_UP)|(1<<TS_BUTTON_LEFT));
	if (TS_Check_Pressed(Combo_Up_Right, Data.X, Data.Y))Result&=~((1<<TS_BUTTON_UP)|(1<<TS_BUTTON_RIGHT));
	if (TS_Check_Pressed(Combo_Up_ButA , Data.X, Data.Y))Result&=~((1<<TS_BUTTON_UP)|(1<<TS_BUTTON_A));
	if (TS_Check_Pressed(Combo_Up_ButB , Data.X, Data.Y))Result&=~((1<<TS_BUTTON_UP)|(1<<TS_BUTTON_B));

	if (TS_Check_Pressed(Combo_Down_Left , Data.X, Data.Y))Result&=~((1<<TS_BUTTON_DOWN)|(1<<TS_BUTTON_LEFT));
	if (TS_Check_Pressed(Combo_Down_Right, Data.X, Data.Y))Result&=~((1<<TS_BUTTON_DOWN)|(1<<TS_BUTTON_RIGHT));
	if (TS_Check_Pressed(Combo_Down_ButA , Data.X, Data.Y))Result&=~((1<<TS_BUTTON_DOWN)|(1<<TS_BUTTON_A));
	if (TS_Check_Pressed(Combo_Down_ButB , Data.X, Data.Y))Result&=~((1<<TS_BUTTON_DOWN)|(1<<TS_BUTTON_B));

	if (TS_Check_Pressed(Combo_Left_ButA , Data.X, Data.Y))Result&=~((1<<TS_BUTTON_LEFT) |(1<<TS_BUTTON_A));
	if (TS_Check_Pressed(Combo_Left_ButB , Data.X, Data.Y))Result&=~((1<<TS_BUTTON_LEFT) |(1<<TS_BUTTON_B));
	if (TS_Check_Pressed(Combo_Right_ButA, Data.X, Data.Y))Result&=~((1<<TS_BUTTON_RIGHT)|(1<<TS_BUTTON_A));
	if (TS_Check_Pressed(Combo_Right_ButB, Data.X, Data.Y))Result&=~((1<<TS_BUTTON_RIGHT)|(1<<TS_BUTTON_B));

	if (Result != 0xffff)
	{
		//printf ("Result = %x\n", Result);
		printf ("Buttons pressed: ");
		if ((~Result) & (1<<TS_BUTTON_LEFT))     printf ("Left, ");
		if ((~Result) & (1<<TS_BUTTON_RIGHT))    printf ("Right, ");
		if ((~Result) & (1<<TS_BUTTON_UP))       printf ("Up, ");
		if ((~Result) & (1<<TS_BUTTON_DOWN))     printf ("Down, ");

		if ((~Result) & (1<<TS_BUTTON_START))    printf ("Start, ");
		if ((~Result) & (1<<TS_BUTTON_SELECT))   printf ("Select, ");
		if ((~Result) & (1<<TS_BUTTON_A))        printf ("ButA, ");
		if ((~Result) & (1<<TS_BUTTON_B))        printf ("ButB, ");
		printf ("\n");
	}

	return Result;
}

int TS_Check_Pressed (Rectangle Button, int X, int Y)
{
	if ((X>Button.MinX) && (X<Button.MaxX) && (Y>Button.MinY) && (Y<Button.MaxY))
		return 1;
	else
		return 0;
}

void TS_Init ()
{
	I2C_Master_Init(TS_MASTER_NUM, TS_MASTER_SCL_IO, TS_MASTER_SDA_IO);
}

int TS_Read_Input ()
{
	int Result=0xffff;
	unsigned char Data[5];
	//int X=0, Y=0;
	if (TS_Read_I2C_Data(Data))
		Result = TS_Detect_Buttons (TS_Calculate_Data (Data));

	return Result;
}
#endif	//#ifdef	CONFIG_HW_TS_ENA
