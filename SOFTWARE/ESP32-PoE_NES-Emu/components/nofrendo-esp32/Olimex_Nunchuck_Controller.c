#include <stdio.h>
#include "driver/i2c.h"

#include "Olimex_I2C.h"
#include "Olimex_Nunchuck_Controller.h"
#ifdef	CONFIG_HW_NUNCHUCK_ENA

static unsigned char NunchuckInitData[2] = {0xF0, 0x55};
int Nunchuck_Read_I2C_Data (unsigned char Data[])
{
	//#define	PRINT_BYTES
	unsigned char Dummy = 0;
	int ret;
	I2C_Write_Slave(NUNCHUCK_I2C_ADDR, NUNCHUCK_MASTER_NUM, &Dummy, 1);
	ret = !I2C_Read_Slave(NUNCHUCK_I2C_ADDR, NUNCHUCK_MASTER_NUM, Data, 6);
	if (ret && (Data[0] == 0xff) && (Data[1] == 0xff) && (Data[2] == 0xff) && (Data[3] == 0xff) && (Data[4] == 0xff) && (Data[5] == 0xff))	// if we successfully read data but all bytes are 0xFF it means the nunchuck is not initialized
		I2C_Write_Slave(NUNCHUCK_I2C_ADDR, NUNCHUCK_MASTER_NUM, NunchuckInitData, 2);
	#ifdef	PRINT_BYTES
	for (int i=0; i<6; i++)
		printf ("Byte[%d] = %d (0x%x);\t", i, Data[i], Data[i]);
	printf ("\n");
	#endif
	return ret;
}

Nunchuck Nunchuck_Calculate_Data (unsigned char InputData[])
{
	Nunchuck OutputData;
	OutputData.JoyX = InputData[0];
	OutputData.JoyY = InputData[1];
	OutputData.AccX = ((int)InputData[2]<<2) | (InputData[5]>>6);
	OutputData.AccY = ((int)InputData[3]<<2) | (InputData[5]>>4);
	OutputData.AccZ = ((int)InputData[4]<<2) | (InputData[5]>>2);
	OutputData.ButZ = !(InputData[5] & 1);
	OutputData.ButC = !(InputData[5] & 2);
	//printf ("JoyX = %3.d; JoyY = %3.d; AccX = %3.d; AccY = %3.d; AccZ = %3.d; ButC = %d; ButZ = %d\n", OutputData.JoyX, OutputData.JoyY, OutputData.AccX, OutputData.AccY, OutputData.AccZ, OutputData.ButC, OutputData.ButZ);
	return OutputData;
}

int Nunchuck_Detect_Buttons (Nunchuck Data)
{
	int Result = 0xffff;

	static int Released = 1;
	if (!gpio_get_level(BOARD_BUTTON) && Released)
	{
		int Timeout = 500;
		Released = 0;
		while ((!gpio_get_level(BOARD_BUTTON)) && Timeout)
		{
			printf ("Timeout: %d\n", Timeout);
			Timeout--;
		}
		if (Timeout)	// short time press
			Result &= ~(1<<NUNCHUCK_BUTTON_SELECT);
		else	// long time press
			Result &= ~(1<<NUNCHUCK_BUTTON_START);
	}
	if (gpio_get_level(BOARD_BUTTON))
		Released = 1;

	if (Data.ButC) Result &= ~(1<<NUNCHUCK_BUTTON_A);
	if (Data.ButZ) Result &= ~(1<<NUNCHUCK_BUTTON_B);

	if (Data.JoyY > 220) Result &= ~(1<<NUNCHUCK_BUTTON_UP);
	if (Data.JoyY < 30)  Result &= ~(1<<NUNCHUCK_BUTTON_DOWN);
	if (Data.JoyX < 30)  Result &= ~(1<<NUNCHUCK_BUTTON_LEFT);
	if (Data.JoyX > 220) Result &= ~(1<<NUNCHUCK_BUTTON_RIGHT);
	return Result;
}

void Nunchuck_Init ()
{
	I2C_Master_Init(NUNCHUCK_MASTER_NUM, NUNCHUCK_MASTER_SCL_IO, NUNCHUCK_MASTER_SDA_IO);
	I2C_Write_Slave(NUNCHUCK_I2C_ADDR, NUNCHUCK_MASTER_NUM, NunchuckInitData, 2);

	gpio_pad_select_gpio(BOARD_BUTTON);
	gpio_set_direction(BOARD_BUTTON, GPIO_MODE_INPUT);
}

int Nunchuck_Read_Input ()
{
	unsigned char I2C_Data[6];
	int Result=0xffff;
	if (Nunchuck_Read_I2C_Data(I2C_Data))
		Result = Nunchuck_Detect_Buttons (Nunchuck_Calculate_Data (I2C_Data));
	return Result;
}
#endif	// #ifdef	CONFIG_HW_NUNCHUCK_ENA
