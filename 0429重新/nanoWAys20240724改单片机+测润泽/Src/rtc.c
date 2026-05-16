/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  LL_RTC_InitTypeDef RTC_InitStruct = {0};
  LL_RTC_TimeTypeDef RTC_TimeStruct = {0};
  LL_RTC_DateTypeDef RTC_DateStruct = {0};

  if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
  {
    FlagStatus pwrclkchanged = RESET;
    /* Update LSE configuration in Backup Domain control register */
    /* Requires to enable write access to Backup Domain if necessary */
    if (LL_APB1_GRP1_IsEnabledClock (LL_APB1_GRP1_PERIPH_PWR) != 1U)
    {
      /* Enables the PWR Clock and Enables access to the backup domain */
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
      pwrclkchanged = SET;
    }
    if (LL_PWR_IsEnabledBkUpAccess () != 1U)
    {
      /* Enable write access to Backup domain */
      LL_PWR_EnableBkUpAccess();
      while (LL_PWR_IsEnabledBkUpAccess () == 0U)
      {
      }
    }
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
    LL_RCC_LSE_Enable();

   /* Wait till LSE is ready */
    while(LL_RCC_LSE_IsReady() != 1)
    {
    }
  LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    /* Restore clock configuration if changed */
    if (pwrclkchanged == SET)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_PWR);
    }
  }

  /* Peripheral clock enable */
  LL_RCC_EnableRTC();

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC and set the Time and Date
  */
  RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
  RTC_InitStruct.AsynchPrescaler = 127;
  RTC_InitStruct.SynchPrescaler = 255;
  LL_RTC_Init(RTC, &RTC_InitStruct);
  LL_RTC_SetAsynchPrescaler(RTC, 127);
  LL_RTC_SetSynchPrescaler(RTC, 255);

  /** Initialize RTC and set the Time and Date
  */
  if(LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0) != 0x32F2){

  RTC_TimeStruct.Hours = 0;
  RTC_TimeStruct.Minutes = 0;
  RTC_TimeStruct.Seconds = 0;
  LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_TimeStruct);
  RTC_DateStruct.WeekDay = LL_RTC_WEEKDAY_MONDAY;
  RTC_DateStruct.Month = LL_RTC_MONTH_JANUARY;
  RTC_DateStruct.Day = 0x1;
  RTC_DateStruct.Year = 0;
  LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_DateStruct);
  LL_RTC_BAK_SetRegister(RTC,LL_RTC_BKP_DR0,0x32F2);
  }
  /* USER CODE BEGIN RTC_Init 2 */
	/* USER CODE BEGIN RTC_Init 2 */
	
	/* 2. 配置 Alarm A：每天每小时的 20 分 0 秒触发一次 */
	LL_RTC_DisableWriteProtection(RTC);           /* 允许写 RTC 寄存器 */
	
	LL_RTC_ALMA_Disable(RTC);                     /* 先关 Alarm A，确保安全修改 */
	//while (LL_RTC_IsActiveFlag_ALRAW(RTC)) { }  /* 等待硬件完成写入 */
	
	/* 设置时间/日期掩码，实现“只关心分钟”的效果 */
	LL_RTC_ALMA_ConfigTime(RTC,       	 /* ① */
							LL_RTC_ALMA_TIME_FORMAT_PM,          /* ② 24h 制，AM/PM 无意义，填 0 */
							0x00,       						/* ③ 小时：00（被掩码忽略时随意） */
							0x13,       						/* ④ 分钟：40 */
							0x00);      						/* ⑤ 秒钟：00 */
	
	LL_RTC_ALMA_SetMask(RTC,
						LL_RTC_ALMA_MASK_HOURS |
						LL_RTC_ALMA_MASK_SECONDS |
						LL_RTC_ALMA_MASK_DATEWEEKDAY);
	
	LL_RTC_ALMA_Enable(RTC);                      /* 重新使能 Alarm A */
	LL_RTC_EnableIT_ALRA(RTC);                    /* 允许 Alarm A 中断 */
	
	LL_RTC_EnableWriteProtection(RTC);            /* 恢复写保护 */
	
	/* 1. 使能 SYSCFG 时钟（EXTI 需要） */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	
	/* 2. 允许 EXTI17 产生中断（只做一次即可） */
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_17);        /* 中断线 17 使能 */
	LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_17);/* 上升沿触发（高电平有效） */
	
	/* 1. 使能 RTC 全局中断 */
	NVIC_SetPriority(RTC_Alarm_IRQn, 0);
	NVIC_EnableIRQ(RTC_Alarm_IRQn);
	
  /* USER CODE END RTC_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
