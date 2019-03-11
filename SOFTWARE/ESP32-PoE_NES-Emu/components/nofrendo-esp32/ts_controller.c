#include <stdio.h>
#include "driver/i2c.h"

#include "ts_controller.h"

static esp_err_t i2c_example_master_read_slave(i2c_port_t i2c_num, uint8_t* data_rd, size_t size)
{
	#if	(defined	CONFIG_HW_TS_CLK) && (defined CONFIG_HW_TS_DAT)
	if (size == 0)
		return ESP_OK;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
	if (size > 1)
		i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
	i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
	#else
	return 0;
	#endif
}

#if 0	// left just in case we need write function but commented to avoid annoying warnings
static esp_err_t i2c_example_master_write_slave(i2c_port_t i2c_num, uint8_t* data_wr, size_t size)
{
	#if	(defined	CONFIG_HW_TS_CLK) && (defined CONFIG_HW_TS_DAT)
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
	#else
	return 0;
	#endif
}
#endif

static void i2c_example_master_init()
{
	#if	(defined	CONFIG_HW_TS_CLK) && (defined CONFIG_HW_TS_DAT)
	int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
	i2c_param_config(i2c_master_port, &conf);
	i2c_driver_install(i2c_master_port, conf.mode, I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
	#endif
}

// empirically derived data: MinX = 307; MaxX = 7759; MinY = 541; MaxY = 7767
// also needs to change the X and Y axis
#define	MAX_X	7450
#define	MAX_Y	7200

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


int Read_Touch_Screen (int *X, int *Y)
{
	//#define	PRINT_BYTES
	unsigned char Data[5]={0, 0, 0, 0, 0};
	i2c_example_master_read_slave(I2C_EXAMPLE_MASTER_NUM, Data, 5);
	#ifdef	PRINT_BYTES
	for (int i=0; i<5; i++)
		printf ("Byte[%d] = %d (0x%x);\t", i, Data[i], Data[i]);
	printf ("\n");
	#endif
	if (Data[0] != 0x81)	// if screen is touched 0th byte will have value 0x81
		return 0;
	*X = ((int)Data[2]<<8) | Data[1];
	*Y = ((int)Data[4]<<8) | Data[3];


	/*
	// detecting what are the min and max values on both axis
	static int MinX=10000, MaxX=0, MinY=10000, MaxY = 0;
	if (*X < MinX) MinX = *X;
	if (*X > MaxX) MaxX = *X;
	if (*Y < MinY) MinY = *Y;
	if (*Y > MaxY) MaxY = *Y;
	printf ("MinX = %d; MaxX = %d; MinY = %d; MaxY = %d\n", MinX, MaxX, MinY, MaxY);
	*/
	return 1;
}

void Calculate_XY (int *X, int *Y)
{
	*X = *X - 307;
	*Y = *Y - 541;
	*Y = MAX_Y - *Y;
	*X = (*X) ^ (*Y);
	*Y = (*X) ^ (*Y);
	*X = (*X) ^ (*Y);
}

int Detect_Buttons (int X, int Y)
{
	int Result = 0xffff;

	Result &= ~(Check_Pressed (Button_Start  , X, Y)<<BUTTON_START);
	Result &= ~(Check_Pressed (Button_Select , X, Y)<<BUTTON_SELECT);
	Result &= ~(Check_Pressed (Button_ButtonA, X, Y)<<BUTTON_BUTTON_A);
	Result &= ~(Check_Pressed (Button_ButtonB, X, Y)<<BUTTON_BUTTON_B);

	Result &= ~(Check_Pressed (Button_Up   , X, Y)<<BUTTON_UP);
	Result &= ~(Check_Pressed (Button_Down , X, Y)<<BUTTON_DOWN);
	Result &= ~(Check_Pressed (Button_Left , X, Y)<<BUTTON_LEFT);
	Result &= ~(Check_Pressed (Button_Right, X, Y)<<BUTTON_RIGHT);

	if (Check_Pressed(Combo_Up_Left   , X, Y)) Result &= ~((1<<BUTTON_UP)    | (1<<BUTTON_LEFT));
	if (Check_Pressed(Combo_Up_Right  , X, Y)) Result &= ~((1<<BUTTON_UP)    | (1<<BUTTON_RIGHT));
	if (Check_Pressed(Combo_Up_ButA   , X, Y)) Result &= ~((1<<BUTTON_UP)    | (1<<BUTTON_BUTTON_A));
	if (Check_Pressed(Combo_Up_ButB   , X, Y)) Result &= ~((1<<BUTTON_UP)    | (1<<BUTTON_BUTTON_B));

	if (Check_Pressed(Combo_Down_Left , X, Y)) Result &= ~((1<<BUTTON_DOWN)  | (1<<BUTTON_LEFT));
	if (Check_Pressed(Combo_Down_Right, X, Y)) Result &= ~((1<<BUTTON_DOWN)  | (1<<BUTTON_RIGHT));
	if (Check_Pressed(Combo_Down_ButA , X, Y)) Result &= ~((1<<BUTTON_DOWN)  | (1<<BUTTON_BUTTON_A));
	if (Check_Pressed(Combo_Down_ButB , X, Y)) Result &= ~((1<<BUTTON_DOWN)  | (1<<BUTTON_BUTTON_B));

	if (Check_Pressed(Combo_Left_ButA , X, Y)) Result &= ~((1<<BUTTON_LEFT)  | (1<<BUTTON_BUTTON_A));
	if (Check_Pressed(Combo_Left_ButB , X, Y)) Result &= ~((1<<BUTTON_LEFT)  | (1<<BUTTON_BUTTON_B));
	if (Check_Pressed(Combo_Right_ButA, X, Y)) Result &= ~((1<<BUTTON_RIGHT) | (1<<BUTTON_BUTTON_A));
	if (Check_Pressed(Combo_Right_ButB, X, Y)) Result &= ~((1<<BUTTON_RIGHT) | (1<<BUTTON_BUTTON_B));

	if (Result != 0xffff)
	{
		//printf ("Result = %x\n", Result);
		printf ("Buttons pressed: ");
		if ((~Result) & (1<<BUTTON_LEFT))     printf ("Left, ");
		if ((~Result) & (1<<BUTTON_RIGHT))    printf ("Right, ");
		if ((~Result) & (1<<BUTTON_UP))       printf ("Up, ");
		if ((~Result) & (1<<BUTTON_DOWN))     printf ("Down, ");

		if ((~Result) & (1<<BUTTON_START))    printf ("Start, ");
		if ((~Result) & (1<<BUTTON_SELECT))   printf ("Select, ");
		if ((~Result) & (1<<BUTTON_BUTTON_A)) printf ("ButA, ");
		if ((~Result) & (1<<BUTTON_BUTTON_B)) printf ("ButB, ");
		printf ("\n");
	}

	return Result;
}

int Check_Pressed (Rectangle Button, int X, int Y)
{
	if ((X>Button.MinX) && (X<Button.MaxX) && (Y>Button.MinY) && (Y<Button.MaxY))
		return 1;
	else
		return 0;
}

void TS_Init ()
{
	i2c_example_master_init();
}

int TS_Read_Input ()
{
	int Result=0xffff;
	int X=0, Y=0;
	if (Read_Touch_Screen(&X, &Y))
	{
		//printf ("X = %4.d (0x%4.x);\tY = %4.d (0x%4.x)\n", X, X, Y, Y);
		Calculate_XY (&X, &Y);
		//printf ("X = %4.d (0x%4.x);\tY = %4.d (0x%4.x)\n\n", X, X, Y, Y);
		Result = Detect_Buttons (X, Y);
	}

	return Result;
}
