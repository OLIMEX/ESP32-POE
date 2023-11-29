#ifndef _DS3231_H_
#define _DS3231_H_

#define DS3231_ADDRESS   ((unsigned char)0x68)

void I2C_SendDataToSlave(unsigned char Address, unsigned char Register, unsigned char Data);
unsigned char I2C_ReadDataFromSlave(unsigned char Address, unsigned char Register);

// registers map
typedef enum
{
    SECONDS         = 0x00,
    MINUTES         = 0x01,
    HOURS           = 0x02,
    DAY_OF_WEEK     = 0x03,
    DATE            = 0x04,
    MONTH           = 0x05,
    YEAR            = 0x06,

    ALARM1_SECONDS  = 0x07,
    ALARM1_MINUTES  = 0x08,
    ALARM1_HOURS    = 0x09,
    ALARM1_DAY_DATE = 0x0A,

    ALARM2_MINUTES  = 0x0B,
    ALARM2_HOURS    = 0x0C,
    ALARM2_DAY_DATE = 0x0D,

    CTRL_REGISTER   = 0x0E,
    STAT_REGISTER   = 0x0F,

    AGING_OFFSET    = 0x10,
    TEMPERATURE_H   = 0x11,
    TEMPERATURE_L   = 0x12,

    NUMBER_OF_REGS  = 0x13
}DS3231_Registers;

// masking for the BCD to int conversion
#define SECONDS_MASK    0x7F
#define MINUTES_MASK    0x7F
#define HOURS_MASK      0x3F
#define DOTW_MASK       0x07
#define DATE_MASK       0x3F
#define MONTH_MASK      0x1F
#define YEAR_MASK       0xFF

// alarm 1 and 2 settings
// for details refer to DS3231 datasheet, the table with Alarm mask bits
// since alarm 2 doesn't have a seconds settings it will be triggered on 00 seconds
#define ALARM_EVERY_SECOND 0x0F    // Alarm once per second
#define ALARM_EVERY_MINUTE 0x0E    // Alarm when seconds match
#define ALARM_EVERY_HOUR   0x0C    // Alarm when minutes and seconds match
#define ALARM_EVERY_DAY    0x08    // Alarm when hours, minutes and seconds match
#define ALARM_EVERY_DATE   0x00    // Alarm when date, hours, minutes and seconds match
#define ALARM_EVERY_DOTW   0x40    // Alarm when day of the week, hours, minutes and seconds match


typedef enum
{
    SUNDAY         = 0,
    MONDAY         = 1,
    TUESDAY        = 2,
    WEDNESDAY      = 3,
    THURSDAY       = 4,
    FRIDAY         = 5,
    SATURDAY       = 6,
    NUMBER_OF_DAYS = 7
}DS3231_DOTW;
extern const char DaysOfTheWeek[NUMBER_OF_DAYS][10];


unsigned char BCDtoInt(unsigned char BCD);
unsigned char InttoBCD(unsigned char Int);

class DS3231
{
private:
	unsigned char Read_Buff[NUMBER_OF_REGS];
public:
	int Century, Year, Month, Date, Day, Hours, Minutes, Seconds;
	double Temp;
	DS3231 ();
	void UpdateData ();
	
	void SetTime (int Hours, int Minutes, int Seconds);
	void SetDate (int Year, int Month, int Date, int DayOfTheWeek);
	void SetAlarm1 (int Hours, int Minutes, int Seconds, int Day_Date, int Setting);
	void SetAlarm2 (int Hours, int Minutes, int Day_Date, int Setting);

	int CheckCentury ();
	int CheckAlarm1 ();
	int CheckAlarm2 ();

	int GetYear ();
	int GetMonth ();
	int GetDate ();
	int GetDayOfTheWeek ();
	int GetHours ();
	int GetMinutes ();
	int GetSeconds ();
	double GetTemperature ();
};

#endif
