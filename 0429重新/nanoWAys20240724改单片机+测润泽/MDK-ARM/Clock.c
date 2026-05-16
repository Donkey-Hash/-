#include "Clock.h"

/*******************************************************************************
* Function Name  	: 设置多个时钟(1-24个)
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void Set_MultiAlarm(UCHAR* coilBuf)
{
	uint8_t i = 0;
	uint32_t alarmCoil = *((uint32_t*)(coilBuf+4));		// 取到定时时刻对应的线圈状态
	//uint32_t alarmCoil = 0x00060000;
	uint8_t currentClock = RTC_GetCurrentHour();		// 先获取当前的小时时间段
	
	/* 获取下一次要定时的时间 */
	if(alarmCoil != 0)									// 判断有没有闹钟需要定时
	{
		for(i = currentClock+1; i < 24; i++)			// 先往后找，看有没有下一次定时
		{
			if(alarmCoil & (0x80000000 >> i)) break;
		}
		
		if(i == 24)
		{
			for(i = 0; i < currentClock; i++)			// 后面没找到，往前找下一次定时时间
			{
				if(alarmCoil & (0x80000000 >> i)) break;
			}
		}
		
		AlarmA_Off();			// 先关闭闹钟
		
		Set_AlarmClock(i);		// 设置定时时间(自动启动)
		
		LL_RTC_ClearFlag_ALRA(RTC); // 清除中断标志
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17); // 清 EXTI 标志（F4 必须）
		
		AlarmA_On();
		
	}
	else
	{
		AlarmA_Off();
	}
	
}
/*******************************************************************************
* Function Name  	: 获取当前的小时段（BCD码转换成二进制）
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
uint8_t RTC_GetCurrentHour(void)
{
    uint8_t bcd = LL_RTC_TIME_GetHour(RTC);
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}
/*******************************************************************************
* Function Name  	: 返回 Alarm A 当前设定的小时值（0-23）
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
uint8_t RTC_GetAlarmHour(void)
{
    uint32_t raw = RTC->ALRMAR;        /* 直接寄存器读最快 */

    /* 提取 BCD 小时字段：HT 在高 4 位，HU 在低 4 位 */
    uint8_t bcdH = ((raw & RTC_ALRMAR_HT_Msk) >> RTC_ALRMAR_HT_Pos) * 10 +
                   ((raw & RTC_ALRMAR_HU_Msk) >> RTC_ALRMAR_HU_Pos);

    return bcdH;                       /* 0-23 */
}

/*******************************************************************************
* Function Name  	: 设置准点闹钟
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void Set_AlarmClock(uint8_t clockTime)
{
	if (clockTime > 23) clockTime = 23;

    /* 转成 BCD */
    uint8_t bcdH = ((clockTime / 10) << 4) | (clockTime % 10);

    LL_RTC_DisableWriteProtection(RTC);

    LL_RTC_ALMA_Disable(RTC);

    /* 设定：小时=bcdH，分=00，秒=00，24h 格式 */
    LL_RTC_ALMA_ConfigTime(RTC, 0, bcdH, 0x00, 0x00);

    /* 掩码：只关心小时，忽略分、秒、日期/星期 */
    LL_RTC_ALMA_SetMask(RTC,
                        LL_RTC_ALMA_MASK_MINUTES |
                        LL_RTC_ALMA_MASK_SECONDS |
                        LL_RTC_ALMA_MASK_DATEWEEKDAY);

    LL_RTC_ALMA_Enable(RTC);
    LL_RTC_EnableIT_ALRA(RTC);   /* 若 NVIC 已开，可省 */

    LL_RTC_EnableWriteProtection(RTC);
}

/*******************************************************************************
* Function Name  	: 打开闹钟
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void AlarmA_On(void)
{
    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_ALMA_Enable(RTC);          /* 打开闹钟硬件 */
    LL_RTC_EnableIT_ALRA(RTC);        /* 打开中断 */
    LL_RTC_EnableWriteProtection(RTC);
}

/*******************************************************************************
* Function Name  	: 关闭闹钟
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void AlarmA_Off(void)
{
    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_ALMA_Disable(RTC);
    LL_RTC_DisableIT_ALRA(RTC);       /* 关闭中断 */
    LL_RTC_EnableWriteProtection(RTC);
}

/*************************Alarm-B******************************/

/*******************************************************************************
* Function Name  	: 设置每周固定时间启动
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void Set_WeeklyAlarm_B(uint8_t weekday, uint8_t hour, uint8_t minute)
{
    uint8_t bcdH = ((hour / 10) << 4) | (hour % 10);
    uint8_t bcdM = ((minute / 10) << 4) | (minute % 10);

    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_ALMB_Disable(RTC);

    /* 选择 Weekday 模式 */
    LL_RTC_ALMB_ConfigTime(RTC,
        LL_RTC_ALMB_DATEWEEKDAYSEL_WEEKDAY,
        bcdH,
        bcdM,
        0x00
    );

    /* 设置星期 */
    LL_RTC_ALMB_SetWeekDay(RTC, weekday);

    /* Mask 秒，只比较 星期 + 时 + 分 */
    LL_RTC_ALMB_SetMask(RTC,
        LL_RTC_ALMB_MASK_SECONDS
    );

    LL_RTC_ClearFlag_ALRB(RTC);
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17);

    LL_RTC_ALMB_Enable(RTC);
    LL_RTC_EnableIT_ALRB(RTC);

    LL_RTC_EnableWriteProtection(RTC);
}
