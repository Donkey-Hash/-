/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "datacollecttask.h"
// #include "fmc.h"
#include "mtimers.h"
#include "TestLEDCtr.h"
#include "ADS1220.h"
#include "FlashUserData.h"
#include "leddef.h"
#include "adc.h"
#include "STMcalendar.h"
// #include "SEGGER_RTT.h"
void OpenStir(void);
void CloseStir(void);

bool FlagStir = false;
uint32_t StirCtrlCycle = 0;
uint32_t StirCtrlCount = 0;

uint16_t IO1InputFilter = 0;
uint16_t IO2InputFilter = 0;
void StartStirCtrlTask(void *argument)
{

	for (;;)
	{

		osDelay(10); // 时间控制精度 10ms
		IO1InputFilter <<= 1;
		IO1InputFilter |= LL_GPIO_IsInputPinSet(X1_GPIO_Port, X1_Pin);
		if ((IO1InputFilter & 0x03) == 0x03)
		{
			usDiscreteInputBuf[0] |= 0x01;
		}
		else if ((IO1InputFilter & 0x03) == 0x00)
		{
			usDiscreteInputBuf[0] &= 0xFE;
		}
		IO2InputFilter <<= 1;
		IO2InputFilter |= LL_GPIO_IsInputPinSet(X2_GPIO_Port, X2_Pin);
		if ((IO2InputFilter & 0x03) == 0x03)
		{
			usDiscreteInputBuf[0] |= 0x02;
		}
		else if ((IO2InputFilter & 0x03) == 0x00)
		{
			usDiscreteInputBuf[0] &= 0xFD;
		}

		if (FlagStir)
		{
			StirCtrlCycle = (pMbHoldData->nStirSleepTime + pMbHoldData->nStirWorkTime) / 10;
			StirCtrlCount += 10;
			if (StirCtrlCount < pMbHoldData->nStirSleepTime / 10)
				CloseStir();
			else
				OpenStir();
			if (StirCtrlCount >= StirCtrlCycle)
			{
				StirCtrlCount = 0;
				CloseStir();
			}
		}
		else
		{
			CloseStir();
		}
	}
}
/*******************************************************************************
* Function Name  	: SendTemperCtrlMsg
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void OpenStir(void)
{
	if (pMbHoldData->nStirValve > 19 || pMbHoldData->nStirValve < 1)
		pMbHoldData->nStirValve = 18;
	OpenValveX((VALVE_NAME)pMbHoldData->nStirValve);
}

/*******************************************************************************
* Function Name  	: SendTemperCtrlMsg
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void CloseStir(void)
{
	if (pMbHoldData->nStirValve > 19 || pMbHoldData->nStirValve < 1)
		pMbHoldData->nStirValve = 18;
	CloseValveX((VALVE_NAME)pMbHoldData->nStirValve);
}
