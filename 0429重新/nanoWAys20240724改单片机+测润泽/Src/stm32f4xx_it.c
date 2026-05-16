/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "port.h"
#include "datacollecttask.h"
#include "adc.h"
#include "FlashUserData.h"
#include "leddef.h"
#include "ModbusMFC.h"
#include "ModbusMaster.h"
#include "ysspectro.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern USART_TypeDef* pModbusUart;
extern USART_TypeDef* pModbusUart3;
extern USART_TypeDef* pModbusUart6;
extern USART_TypeDef* pMbHostUart;
extern USART_TypeDef* pExDevUart;
//LL_ADs * pLLAD = (LL_ADs*)AD1Buffer;
IN_ADCS_VALULE *pExS = (IN_ADCS_VALULE*)AD1Buffer;
extern IN_ADCS_VALULE InsideADCValue;
IN_ADCS_VALULE_SUM TempADs;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim14;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles PVD interrupt through EXTI line 16.
  */
void PVD_IRQHandler(void)
{
  /* USER CODE BEGIN PVD_IRQn 0 */

  /* USER CODE END PVD_IRQn 0 */
  HAL_PWR_PVD_IRQHandler();
  /* USER CODE BEGIN PVD_IRQn 1 */

  /* USER CODE END PVD_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream1 global interrupt.
  */
void DMA1_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream1_IRQn 0 */
if(LL_DMA_IsActiveFlag_TC1(DMA1))//是否传输完成,注意如果TC2对应数据流2，如果修改了流要跟着修改
	{
		LL_DMA_ClearFlag_TC1(DMA1);//清除传输完成中断标记
		LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);//使能DMA数据流，允许接收下一帧数据	
	}
	
	if(LL_DMA_IsActiveFlag_TE1(DMA1))//是否传输异常,注意如果TC2对应数据流2，如果修改了流要跟着修改
	{
		LED2_ON
		LL_DMA_ClearFlag_TE1(DMA1);//清除异常中断标记
		LL_DMA_DisableIT_TE(DMA1, LL_DMA_STREAM_1);//禁止传输异常中断
		LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_1);//禁止DMA数据流
//		LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_2);
		LED2_OFF
	}
  /* USER CODE END DMA1_Stream1_IRQn 0 */

  /* USER CODE BEGIN DMA1_Stream1_IRQn 1 */

  /* USER CODE END DMA1_Stream1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream3 global interrupt.
  */
void DMA1_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream3_IRQn 0 */
if(LL_DMA_IsActiveFlag_TC3(DMA1))//是否传输完成
	{
//		LED2_ON
		LL_DMA_ClearFlag_TC3(DMA1);//清除传输完成中断标记
		LL_DMA_DisableIT_TC(DMA1, LL_DMA_STREAM_3);//使能传输完成中断
		LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_3);//禁止DMA数据流
#if !APP_USART3_MODBUS_SLAVE_MB3
		LL_USART_EnableIT_TC(pMbHostUart);
#endif
//		LED2_OFF
//		H_RS485_RX
//		LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_4);//使能DMA数据流
	}	
	if(LL_DMA_IsActiveFlag_TE3(DMA1))//是否传输异常
	{
		LED2_ON
		LL_DMA_ClearFlag_TE3(DMA1);//清除异常中断标记
		LL_DMA_DisableIT_TE(DMA1, LL_DMA_STREAM_3);//禁止传输异常中断
		LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_3);//禁止DMA数据流
		LED2_OFF
	}
  /* USER CODE END DMA1_Stream3_IRQn 0 */

  /* USER CODE BEGIN DMA1_Stream3_IRQn 1 */

  /* USER CODE END DMA1_Stream3_IRQn 1 */
}

/**
  * @brief This function handles TIM1 trigger and commutation interrupts and TIM11 global interrupt.
  */
void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_TRG_COM_TIM11_IRQn 0 */
TIMERExpiredISR();
  /* USER CODE END TIM1_TRG_COM_TIM11_IRQn 0 */

  /* USER CODE BEGIN TIM1_TRG_COM_TIM11_IRQn 1 */

  /* USER CODE END TIM1_TRG_COM_TIM11_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
	/* 多总线高负载场景下，USART 可能产生 ORE/FE/NE/PE 等错误标志。
	 * 如果不清除，IRQ 可能会被错误标志持续触发（中断风暴），导致任务无法运行并最终 IWDG 复位。 */
	if (LL_USART_IsActiveFlag_ORE(pModbusUart) || LL_USART_IsActiveFlag_NE(pModbusUart) ||
			LL_USART_IsActiveFlag_FE(pModbusUart) || LL_USART_IsActiveFlag_PE(pModbusUart))
	{
		/* 按参考手册要求：通过读 SR/DR 的序列清除错误标志（LL 宏内部等价实现）。 */
		(void)pModbusUart->SR;
		(void)pModbusUart->DR;
	}
	if(LL_USART_IsActiveFlag_RXNE(pModbusUart))
	{
		LL_USART_ClearFlag_RXNE(pModbusUart);
		prvvUARTRxISR();	   //
	}
	if(LL_USART_IsEnabledIT_TXE(pModbusUart) && LL_USART_IsActiveFlag_TXE(pModbusUart))
	{

		prvvUARTTxReadyISR();  //
	}
  /* USER CODE END USART1_IRQn 0 */
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */
#if APP_USART3_MODBUS_SLAVE_MB3
	if (LL_USART_IsActiveFlag_ORE(pModbusUart3) || LL_USART_IsActiveFlag_NE(pModbusUart3) ||
			LL_USART_IsActiveFlag_FE(pModbusUart3) || LL_USART_IsActiveFlag_PE(pModbusUart3))
	{
		(void)pModbusUart3->SR;
		(void)pModbusUart3->DR;
	}
	if(LL_USART_IsActiveFlag_RXNE(pModbusUart3))
	{
		LL_USART_ClearFlag_RXNE(pModbusUart3);
		prvvUART3RxISR();
	}
	if(LL_USART_IsEnabledIT_TXE(pModbusUart3) && LL_USART_IsActiveFlag_TXE(pModbusUart3))
	{
		prvvUART3TxReadyISR();
	}
#else
	/* USART3 作为外设 Modbus 主站口：DMA + IDLE 作为接收完成标记，TC 作为发送完成标记 */
	if(LL_USART_IsEnabledIT_IDLE(pMbHostUart) && LL_USART_IsActiveFlag_IDLE(pMbHostUart))
	{
		LL_USART_ClearFlag_IDLE(pMbHostUart);
		USART_RxIdleCallback();
	}
	if(LL_USART_IsEnabledIT_TC(pMbHostUart) && LL_USART_IsActiveFlag_TC(pMbHostUart))
	{
		LL_USART_ClearFlag_TC(pMbHostUart);
		LL_USART_DisableIT_TC(pMbHostUart);
		H_RS485_RX
	}
#endif
  /* USER CODE END USART3_IRQn 0 */
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles TIM8 trigger and commutation interrupts and TIM14 global interrupt.
  */
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
  /* USER CODE BEGIN TIM8_TRG_COM_TIM14_IRQn 0 */

  /* USER CODE END TIM8_TRG_COM_TIM14_IRQn 0 */
  HAL_TIM_IRQHandler(&htim14);
  /* USER CODE BEGIN TIM8_TRG_COM_TIM14_IRQn 1 */

  /* USER CODE END TIM8_TRG_COM_TIM14_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt and DAC1/DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
	TIMER6ExpiredISR();
  /* USER CODE END TIM6_DAC_IRQn 0 */
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */
	TIMER3ExpiredISR();
  /* USER CODE END TIM7_IRQn 0 */
  /* USER CODE BEGIN TIM7_IRQn 1 */

  /* USER CODE END TIM7_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream0 global interrupt.
  */
void DMA2_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */
	
  /* USER CODE END DMA2_Stream0_IRQn 0 */

  /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */
//	LED2_ON
	if(LL_DMA_IsActiveFlag_TC0(DMA2))
	{
		LL_DMA_ClearFlag_TC0(DMA2);
  
		if(LL_DMA_CURRENTTARGETMEM0 == LL_DMA_GetCurrentTargetMem(DMA2,LL_DMA_STREAM_0))
		{
			pExS = (IN_ADCS_VALULE*)AD1BufferM1;
		}
		else
		{
			pExS = (IN_ADCS_VALULE*)AD1Buffer;
		}
		
		TempADs = (IN_ADCS_VALULE_SUM){0};
		for(uint8_t i=0;i<MUTI_ADC_SAMPLE;i++)
		{
			TempADs.nAD_IN0 += (pExS + i)->nAD_IN0;
			TempADs.nAD_IN1 += (pExS + i)->nAD_IN1;
			TempADs.nAD_IN7 += (pExS + i)->nAD_IN7;
			TempADs.nAD_IN9 += (pExS + i)->nAD_IN9;
			TempADs.nAD_VTEMP += (pExS + i)->nAD_VTEMP;
//			TempADs.nAD_VREF += (pExS + i)->nAD_VREF;
			TempADs.nAD_VBat += (pExS + i)->nAD_VBat;
		}
		TempADs.nAD_IN0 += (MUTI_ADC_SAMPLE >> 1);
		TempADs.nAD_IN1 += (MUTI_ADC_SAMPLE >> 1);
		TempADs.nAD_IN7 += (MUTI_ADC_SAMPLE >> 1);
		TempADs.nAD_IN9 += (MUTI_ADC_SAMPLE >> 1);
		TempADs.nAD_VTEMP += (MUTI_ADC_SAMPLE >> 1);
//		TempADs.nAD_VREF += (MUTI_ADC_SAMPLE >> 1);
		TempADs.nAD_VBat += (MUTI_ADC_SAMPLE >> 1);
		
		InsideADCValue.nAD_IN0 = TempADs.nAD_IN0 / MUTI_ADC_SAMPLE;
		InsideADCValue.nAD_IN1 = TempADs.nAD_IN1 / MUTI_ADC_SAMPLE;
		InsideADCValue.nAD_IN7 = TempADs.nAD_IN7 / MUTI_ADC_SAMPLE;
		InsideADCValue.nAD_IN9 = TempADs.nAD_IN9 / MUTI_ADC_SAMPLE;
		InsideADCValue.nAD_VTEMP = TempADs.nAD_VTEMP / MUTI_ADC_SAMPLE;
//		InsideADCValue.nAD_VREF = TempADs.nAD_VREF / MUTI_ADC_SAMPLE;
		InsideADCValue.nAD_VBat = TempADs.nAD_VBat / MUTI_ADC_SAMPLE;
		pMbInputData->nPoolTempAD = InsideADCValue.nAD_IN0;
		pMbInputData->nCaveTempAD = InsideADCValue.nAD_IN1;
		pMbInputData->nLL1AD = InsideADCValue.nAD_IN7;
		pMbInputData->nLL2AD = InsideADCValue.nAD_IN9;
	}
//	LED2_OFF
  /* USER CODE END DMA2_Stream0_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream1 global interrupt.
  */
void DMA2_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream1_IRQn 0 */
if(LL_DMA_IsActiveFlag_TC1(DMA2))//是否传输完成,注意如果TC2对应数据流2，如果修改了流要跟着修改
	{
		LL_DMA_ClearFlag_TC1(DMA2);//清除传输完成中断标记
		LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);//使能DMA数据流，允许接收下一帧数据	
	}
	
	if(LL_DMA_IsActiveFlag_TE1(DMA2))//是否传输异常,注意如果TC2对应数据流2，如果修改了流要跟着修改
	{
		LED2_ON
		LL_DMA_ClearFlag_TE1(DMA2);//清除异常中断标记
		LL_DMA_DisableIT_TE(DMA2, LL_DMA_STREAM_1);//禁止传输异常中断
		LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1);//禁止DMA数据流
//		LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_2);
		LED2_OFF
	}
  /* USER CODE END DMA2_Stream1_IRQn 0 */

  /* USER CODE BEGIN DMA2_Stream1_IRQn 1 */

  /* USER CODE END DMA2_Stream1_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream6 global interrupt.
  */
void DMA2_Stream6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream6_IRQn 0 */
if(LL_DMA_IsActiveFlag_TC6(DMA2))//是否传输完成
	{
//		LED2_ON
		LL_DMA_ClearFlag_TC6(DMA2);//清除传输完成中断标记
		LL_DMA_DisableIT_TC(DMA2, LL_DMA_STREAM_6);//使能传输完成中断
		LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_6);//禁止DMA数据流
#if !APP_USART6_MODBUS_SLAVE_MB6
		LL_USART_EnableIT_TC(pExDevUart);
#endif
//		LED2_OFF
//		H_RS485_RX
//		LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_4);//使能DMA数据流
	}	
	if(LL_DMA_IsActiveFlag_TE6(DMA2))//是否传输异常
	{
		LED2_ON
		LL_DMA_ClearFlag_TE6(DMA2);//清除异常中断标记
		LL_DMA_DisableIT_TE(DMA2, LL_DMA_STREAM_6);//禁止传输异常中断
		LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_6);//禁止DMA数据流
		LED2_OFF
	}
  /* USER CODE END DMA2_Stream6_IRQn 0 */

  /* USER CODE BEGIN DMA2_Stream6_IRQn 1 */

  /* USER CODE END DMA2_Stream6_IRQn 1 */
}

/**
  * @brief This function handles USART6 global interrupt.
  */
void USART6_IRQHandler(void)
{
  /* USER CODE BEGIN USART6_IRQn 0 */
#if APP_USART6_MODBUS_SLAVE_MB6
	if (LL_USART_IsActiveFlag_ORE(pModbusUart6) || LL_USART_IsActiveFlag_NE(pModbusUart6) ||
			LL_USART_IsActiveFlag_FE(pModbusUart6) || LL_USART_IsActiveFlag_PE(pModbusUart6))
	{
		(void)pModbusUart6->SR;
		(void)pModbusUart6->DR;
	}
	if(LL_USART_IsActiveFlag_RXNE(pModbusUart6))
	{
		LL_USART_ClearFlag_RXNE(pModbusUart6);
		prvvUART6RxISR();
	}
	if(LL_USART_IsEnabledIT_TXE(pModbusUart6) && LL_USART_IsActiveFlag_TXE(pModbusUart6))
	{
		prvvUART6TxReadyISR();
	}
#else
	if(LL_USART_IsActiveFlag_IDLE(pExDevUart))
	{
		LL_USART_ClearFlag_IDLE(pExDevUart);
		ExDevRxIdleCallback();
	}
	if(LL_USART_IsActiveFlag_TXE(pExDevUart))
	{
		LL_USART_ClearFlag_IDLE(pExDevUart);
		LL_USART_DisableIT_TC(pExDevUart);
	}
#endif
  /* USER CODE END USART6_IRQn 0 */
  /* USER CODE BEGIN USART6_IRQn 1 */

  /* USER CODE END USART6_IRQn 1 */
}

/* USER CODE BEGIN 1 */
/**
  * @brief RTC闹钟时钟
  */
/* USER CODE BEGIN RTC_Alarm_IRQn */

extern uint8_t AutoStep;

void RTC_Alarm_IRQHandler(void)
{
    if (LL_RTC_IsActiveFlag_ALRA(RTC) != 0)
    {
        LL_RTC_ClearFlag_ALRA(RTC); // 清除中断标志
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17); // 清 EXTI 标志（F4 必须）
        // 在这里添加你的准点任务（例如：记录日志、唤醒主程序等）
		
		/* 正式代码 */
		AutoStep = 1;					// 回到第一步
		pMbHoldData->nUpdateData = 0;	// 清空水样计算完成日志
		pMbHoldData->RW50 = 1;			// 开始采水样
		pMbHoldData->nAutoRun = 1;		// 打开采水样样流程
		
		/* 关闭闹钟，等待下一次调用 */
		LL_RTC_DisableWriteProtection(RTC);
		LL_RTC_ALMA_Disable(RTC);
		LL_RTC_DisableIT_ALRA(RTC);       /* 关闭中断 */
		LL_RTC_EnableWriteProtection(RTC);
    }
	
	if (LL_RTC_IsActiveFlag_ALRB(RTC))
    {
        LL_RTC_ClearFlag_ALRB(RTC);					// 清除中断标志
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17);	// 清 EXTI 标志（F4 必须）
        // Alarm B 处理
        /*************** 闹钟B逻辑处理 *****************/
    }
}
/* USER CODE END RTC_Alarm_IRQn */

/* USER CODE END 1 */
