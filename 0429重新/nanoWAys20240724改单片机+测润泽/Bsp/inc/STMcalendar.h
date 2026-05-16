
#ifndef __STM_CALENDAR_H__
#define __STM_CALENDAR_H__

#include "main.h"
#define YEAR_BUILD ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
    + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))
 
#define MONTH_BUILD (__DATE__ [2] == 'n' ? 1 \
    : __DATE__ [2] == 'b' ? 2 \
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
    : __DATE__ [2] == 'y' ? 5 \
    : __DATE__ [2] == 'n' ? 6 \
    : __DATE__ [2] == 'l' ? 7 \
    : __DATE__ [2] == 'g' ? 8 \
    : __DATE__ [2] == 'p' ? 9 \
    : __DATE__ [2] == 't' ? 10 \
    : __DATE__ [2] == 'v' ? 11 : 12)
 
#define DAY_BUILD ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
    + (__DATE__ [5] - '0'))
 
#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)

#define HOUR_BUILD ((__TIME__ [0] == ' ' ? 0 : __TIME__ [0] - '0') * 10 \
    + (__TIME__ [1] - '0'))
		
#define MINUTE_BUILD ((__TIME__ [3] == ' ' ? 0 : __TIME__ [3] - '0') * 10 \
    + (__TIME__ [4] - '0'))
		
#define SECOND_BUILD ((__TIME__ [6] == ' ' ? 0 : __TIME__ [6] - '0') * 10 \
    + (__TIME__ [7] - '0'))


typedef struct{
    unsigned short tmSec;   /* seconds after the minute, 0 to 60
                     (0 - 60 allows for the occasional leap second) */
    unsigned short tmMin;   /* minutes after the hour, 0 to 59 */
    unsigned short tmHour;  /* hours since midnight, 0 to 23 */
    unsigned short tmDay;  /* day of the month, 1 to 31 */
    unsigned short tmMonth;   /* months since January, 0 to 11 */
    unsigned short tmYear;  /* years since 1900 */
}DATA_TIME;



void InitSTM32F4Calendar(void);
void RTC_Set(LL_RTC_DateTypeDef *date,LL_RTC_TimeTypeDef *time);
void RTC_Get(LL_RTC_DateTypeDef *date,LL_RTC_TimeTypeDef *time);
unsigned char RTC_Get_Week(unsigned short int year,unsigned char month, unsigned char day);
#endif
