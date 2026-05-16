
#include "STMcalendar.h"

#define RTC_BKP_DATE_TIME_UPDTATED ((uint32_t)0x32F2)

unsigned char const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表   
unsigned char RTC_Get_Week(unsigned short int year,unsigned char month, unsigned char day)
{
	unsigned short int temp2;
	unsigned char yearH, yearL;

	yearH = year/100; yearL = year%100;
	
	// 如果为21世纪,年份数加100
	if (yearH>19) yearL+=100;
	
	// 所过闰年数只算1900年之后的
	temp2 = yearL + yearL/4;
	temp2 = temp2 % 7;
	temp2 = temp2 + day + table_week[month-1];
	
	if (yearL%4 == 0&&month<3) temp2--;
	
	return(temp2 % 7);
}

void InitSTM32F4Calendar(void)
{
	LL_RTC_InitTypeDef RTC_InitStruct = {0};
  LL_RTC_TimeTypeDef RTC_TimeStruct = {0};
  LL_RTC_DateTypeDef RTC_DateStruct = {0};
	
	if(LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR1) != 0x32F2)	// 如果备份寄存器不是0x32F2，说明从未设置过RTC，进行设置
		{
			/* 设置时间和日期 */
			RTC_TimeStruct.Hours = HOUR_BUILD;
			RTC_TimeStruct.Minutes = MINUTE_BUILD;
			RTC_TimeStruct.Seconds = SECOND_BUILD;
			LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_TimeStruct);
		
			RTC_DateStruct.Month = MONTH_BUILD;
			RTC_DateStruct.Year = YEAR_BUILD - 2000;
			RTC_DateStruct.Day = DAY_BUILD;
			RTC_DateStruct.WeekDay = 00;// RTC_Get_Week(RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Day);
			
			/* 初始化RTC */ 
			LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_DateStruct);
			
			/* 写入备份寄存器，表示 RTC 初始化成功 */
			LL_RTC_BAK_SetRegister(RTC,LL_RTC_BKP_DR1,0x32F2);
		}
}

/* 手动设置RTC时间和日期 */
void RTC_Set(LL_RTC_DateTypeDef *date,LL_RTC_TimeTypeDef *time)
{	
	LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, time);
	LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, date);	
	LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, RTC_BKP_DATE_TIME_UPDTATED);
}

/* 获取当前日期和时间 */
void RTC_Get(LL_RTC_DateTypeDef *date,LL_RTC_TimeTypeDef *time)
{
	time->Hours   = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
	time->Minutes = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
	time->Seconds = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));
	
	date->Year  = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC));
	date->Month = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC));
	date->Day   = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC));
}


