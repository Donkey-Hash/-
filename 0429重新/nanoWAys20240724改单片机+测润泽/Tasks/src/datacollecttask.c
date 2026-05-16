/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Desiption        : Code for freertos applications
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
#include "modbusdata_mb3.h"
// #include "SEGGER_RTT.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern bool TimeoutFlag;
extern osMutexId_t osADSMutexHandle;
extern DMA_HandleTypeDef hdma_adc1;

IN_ADCS_VALULE InsideADCValue;		// 内部ADC读取到的数字量结构体
uint16_t ExADCsValue[6];			// 采集外部AD值
POOL_LT_ADS_OPT PoolLtADsOpt = {0}; // 比色池ADC运算
uint32_t CounterADSMutexError = 0;	// AD操作互斥信号量冲突计数
bool FlagAbsLightOn = false;		// 测量灯开关信号
uint16_t tRefBackAD = 0;			// 记录参比背景值
uint16_t tTransBackAD = 0;			// 记录透射背景值
uint16_t tRefAD = 0;				// 参比ad瞬时值
uint16_t tTransAD = 0;				// 透射AD瞬时值
int sumref = 0;						// 参比值累加
int sumtrs = 0;						// 透射值累加
uint32_t SumRefBackAD = 0;
uint32_t SumTransBackAD = 0;

uint16_t AD1Buffer[AD_BUFF_SIZE];	// ADC DMA buffer0 //用于双buffer DMA
uint16_t AD1BufferM1[AD_BUFF_SIZE]; // ADC DMA buffer1
// uint16_t AD2Buffer[AD_BUFF_SIZE];
// uint16_t AD3Buffer[AD_BUFF_SIZE];
// uint16_t AD4Buffer[AD_BUFF_SIZE];
/* USER CODE END Variables */
/* Definitions for defaultTask */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void ReadADs(uint16_t *pAD, uint16_t nCount);
void ResultIOut(void);
/* USER CODE END FunctionPrototypes */

extern void TempControlTask(void *argument);
extern void ProcessControlTask(void *argument);
extern void UartsRxTask(void *argument);
extern void PumpTimerout(void *argument);
extern void HotHoldTimerCallback(void *argument);
extern void ReportStepTimerCallback(void *argument);

extern void MX_LWIP_Init(void);
extern void MX_USB_DEVICE_Init(void);
extern void MX_USB_HOST_Init(void);

void StartADCwithDoubleBuferDMA(void);
void DataConv(IN_ADCS_VALULE *padcs);
void Get_RealTime(LL_RTC_DateTypeDef *date,LL_RTC_TimeTypeDef *time);

/* 初始化日历 */
LL_RTC_DateTypeDef Date = {0};	// 创建年月日结构体
LL_RTC_TimeTypeDef Time = {0};	// 创建时分秒结构体

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */

char JScopeBuf[1024];
unsigned TempWrite;
unsigned TempReadback;

#define SUM_CNT	20	//平均值次数

void StartDataCollectTask(void *argument)
{
	uint16_t i = 0;

	/* init code for LWIP */
	//  MX_LWIP_Init();

	/* init code for USB_DEVICE */
	//  MX_USB_DEVICE_Init();

	/* init code for USB_HOST */
	//  MX_USB_HOST_Init();
	/* USER CODE BEGIN StartDefaultTask */

	OpenLLLED();					// 打开LED指示灯
	OpenAbsorbLED();				// 打开吸光灯
	InitSTM32F4Calendar();			// 初始化日历时间（RTC时钟）
	osDelay(50);
	Buzzer_OFF						// 关闭蜂鸣器
	StartADCwithDoubleBuferDMA();	// 打开ADC_DMA双缓存模式采集数据
	LL_SPI_Enable(SPI2);			// 打开 SPI2 进行通信
	ADS1220Config();				// ADS1220模数转换器芯片初始化配置
	//	SEGGER_RTT_Init();
	//	SEGGER_RTT_ConfigDownBuffer(0, "RTTDOWN", NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
	//	SEGGER_RTT_ConfigUpBuffer(1, "JScope_i2i2i2i2", JScopeBuf, 2048, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

	for (;;)
	{
		osDelay(10);
		pMb3InputData->MeasurementCycle  = pMbHoldData->nClockTime; //测量周期	--整数浮点数
		//		LED1_ON
		Get_RealTime(&Date, &Time);		// 获取实时时间
		ReadADs(ExADCsValue, 4);	// 读外部采集的AD值
		//		SEGGER_RTT_printf(0, ",%d,%d\r\n",ExADCsValue[0],ExADCsValue[1]);
		//		LED1_OFF
		if (FlagAbsLightOn)			// 如果测量灯打开
		{
			tRefAD = ExADCsValue[LED_REF_CH];	// 比色池参比电压
			tTransAD = ExADCsValue[LED_ABS_CH];	// 比色电压
			// CloseAbsorbLED(); // 手动注释掉，防止比色灯闪烁。
			LED2_OFF
			FlagAbsLightOn = false;
			sumref += (int)(tRefAD - tRefBackAD);		// 累加比色池参比值，减去背景值 tRefBackAD
			sumtrs += (int)(tTransAD - tTransBackAD);	// 累加比色值，减去背景值 tTransBackAD
			i++;
			//			SEGGER_RTT_printf(0, ",%d,%d,%d,%d\r\n",tRefAD,tRefBackAD,tTransAD,tTransBackAD);
			if (i == SUM_CNT)	// 当达到一定次数20次，取平均
			{
				/* 将数据处理并储存到保持寄存器中 */
				pMbInputData->nPoolRefLtAD = 1; // ((sumref + 5 )  / 10);
				pMbInputData->nPoolTranLtAD = ((sumtrs + 5) / SUM_CNT);
				pMbInputData->nPoolRefLtBackAD = 1; // (SumRefBackAD + 5)/ 10;
				pMbInputData->nPoolTranLtBackAD = (SumTransBackAD + 5) / SUM_CNT;
				//				SEGGER_RTT_printf(0, ",%d",pMbInputData->nPoolRefLtAD);
				//				SEGGER_RTT_printf(0, ",%d\r\n",pMbInputData->nPoolTranLtAD );
				if (PoolLtADsOpt.nNumOfCollect > 0)
				{
					if (osMutexWait(osADSMutexHandle, 500) == osOK)
					{
						PoolLtADsOpt.nSumOfRefAD += pMbInputData->nPoolTranLtBackAD;
						PoolLtADsOpt.nSumOfTranAD += pMbInputData->nPoolTranLtAD;
						PoolLtADsOpt.nNumOfCollect--;
						if (osMutexRelease(osADSMutexHandle) != osOK)
							CounterADSMutexError++;
					}
					else
						CounterADSMutexError++;
				}
				pMbHoldData->fAbsorbance = log((double)(pMbInputData->nPoolRefLtAD) / (double)(pMbInputData->nPoolTranLtAD));	// 计算吸光度 = log10(V1/V2)
				//				SEGGER_RTT_printf(0, ",Abs: %.6f\r\n",pMbHoldData->fAbsorbance);
				LED1_TOGGLE	// LED翻转
				i = 0;		// 清空循环标志位
				sumref = 0;	// 清空参数
				sumtrs = 0;
				SumRefBackAD = 0;
				SumTransBackAD = 0;
				
				ResultIOut();				// DAC输出
				DataConv(&InsideADCValue);	// 将ADC数字量数据转换成真实数据写入到寄存器：InsideADCValue的值在DMA2中断进行赋值
				OpenAbsorbLED();			// 设置吸光灯电流并打开
			}
		}
		else
		{
			// tRefBackAD = ExADCsValue[LED_REF_CH];	//注释掉，不需要暗电压
			// tTransBackAD = ExADCsValue[LED_ABS_CH];
			tRefBackAD = 0;
			tTransBackAD = 0;
			SumRefBackAD += tRefBackAD;
			SumTransBackAD += tTransBackAD;
			// OpenAbsorbLED();	注释掉
			LED2_ON
			FlagAbsLightOn = true;
		}
		//    osDelay(1000);LL_GPIO_IsInputPinSet
	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/*******************************************************************************
 * Function Name  : 获取实时时间写入寄存器
 * Description    : 
 * Input          : 
 * Output         : 
 * Return         : 
 *******************************************************************************/
void Get_RealTime(LL_RTC_DateTypeDef *date,LL_RTC_TimeTypeDef *time)
{
	RTC_Get(date, time);		// 获取实时时间
	
	pMbInputData->nYearBuild = date->Year;
	pMbInputData->nMonthBuild = date->Month;
	pMbInputData->nDayBuild = date->Day;
	
	pMbInputData->nHourBuild = time->Hours;
	pMbInputData->nMinuteBuild = time->Minutes;
	pMbInputData->nSecondBuild = time->Seconds;
}

/*******************************************************************************
 * Function Name  : LTCReadADx
 * Description    : 对透光和参照AD值进行同时连续读取。
 * Input          : 通道号，连续读取次数
 * Output         : None
 * Return         : 对应通道AD值
 *******************************************************************************/
// uint16_t adabs[32],adref[32];
//	uint32_t adsum[6];
// int16_t ResultAds[20];
void ReadADs(uint16_t *pAD, uint16_t nCount)
{

	//	pAD[0] = (ReadADC(3,nCount) ) / nCount;
	//	pAD[1] = (ReadADC(2,nCount) ) / nCount;
	ReadADCs(pAD, nCount);
}
/*******************************************************************************
* Function Name  	:
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
#define STD_D_OUT_H 4050
#define STD_D_OUT_L 100

void ResultIOut(void) // 电压单位mV
{
	float temp;
	float rangset;
	uint16_t Dh;
	//	MCP4725_WriteData_Voltage(pMbHoldData->nRange1Set);
	//	if(pMbHoldData->nWorkRange > 3)
	//	{
	//		LL_DAC_ConvertData12RightAligned(DAC, LL_DAC_CHANNEL_1,0);
	//		LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
	//	}
	//	else
	{
		if (*pvrAmaxOut)
		{
			LL_DAC_ConvertData12RightAligned(DAC, LL_DAC_CHANNEL_1, STD_D_OUT_H);
			LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
			return;
		}
		if (*pvrAminOut)
		{
			LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, STD_D_OUT_L);
			LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
			return;
		}

		if (*pvrCurrenttest)
		{
			temp = pMbHoldData->fCurrentTestSet;
		}
		else
		{
			temp = pMbHoldData->fMeasureResult;	// 使用测量结果
		}
		rangset = pMbHoldData->fWorkRangeSet;	// 获取工作范围设置值
		temp /= rangset;
		temp *= 16;
		temp += (4 - pMbHoldData->fAminOut);
		temp *= (STD_D_OUT_H - STD_D_OUT_L);
		temp /= (pMbHoldData->fAmaxOut - pMbHoldData->fAminOut);
		temp += STD_D_OUT_L;
		if (temp < 0)
			temp = 0;
		Dh = (uint16_t)(temp + 0.5);
		if (Dh > 0xfff)
			Dh = 0xfff;
		LL_DAC_ConvertData12RightAligned(DAC, LL_DAC_CHANNEL_1, Dh);	// 将Dh转换为12位右对齐数据
		LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);							// 启用DAC通道1
	}
}
/*******************************************************************************
 * Function Name  : StartADCwithDoubleBuferDMA
 * Description    : 启动ADC DMA 双缓存方式
 * Input          : 无
 * Output         : None
 * Return         : 无
 *******************************************************************************/
void StartADCwithDoubleBuferDMA(void)
{
	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, AD_BUFF_SIZE);
	LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_0, LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA));
	LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)AD1Buffer);
	LL_DMA_SetMemory1Address(DMA2, LL_DMA_STREAM_0, (uint32_t)AD1BufferM1);
	LL_DMA_EnableDoubleBufferMode(DMA2, LL_DMA_STREAM_0);
	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
	LL_DMA_ClearFlag_TC0(DMA2);
	LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);

	LL_ADC_Enable(ADC1);
	LL_ADC_REG_StartConversionSWStart(ADC1);
	LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
}
/*******************************************************************************
 * Function Name  : DataConv
 * Description    : ADC 的数据转换
 * Input          : 无
 * Output         : None
 * Return         : 无
 *******************************************************************************/
void DataConv(IN_ADCS_VALULE *padcs)
{
	IN_ADCS_VALULE adcs = *padcs;
	float temp = padcs->nAD_VTEMP;
	temp *= 3.3;
	temp /= 4095;
	temp -= 0.76;
	temp /= 0.0025;
	temp += 25;

	pMbInputData->fCPUTemp = temp;
	//	pMbHoldData->fCaveTemp = temp; //使用CPU温度作为机箱温度
	temp = padcs->nAD_VBat * 2;
	temp *= 3.3;
	temp /= 4095;
	pMbInputData->fVbat = temp;
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
