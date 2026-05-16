#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "stm32f4xx_hal.h"
#include "instrumentinfo.h"
#include "cmsis_os.h"

extern UCHAR *usCoilBuf;	// ±£≥÷œﬂ»¶

void Set_MultiAlarm(UCHAR* coilBuf);
uint8_t RTC_GetCurrentHour(void);
uint8_t RTC_GetAlarmHour(void);
void Set_AlarmClock(uint8_t clockTime);
void AlarmA_On(void);
void AlarmA_Off(void);

#endif
