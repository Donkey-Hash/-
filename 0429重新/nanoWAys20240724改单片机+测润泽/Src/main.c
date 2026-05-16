/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "cmsis_os.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "iwdg.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "leddef.h"
#include "FlashUserData.h"
#include "errormanagetask.h"
#include "Clock.h"
#include "STMcalendar.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void _sys_exit(int x) 
{ 
    x = x; 
} 

/**
  * ��������: �ض���c�⺯��printf��USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
#if APP_USART3_MODBUS_SLAVE_MB3
int fputc(int ch, FILE *f)
{
	(void)f;
	return ch;
}

int fgetc(FILE * f)
{
	(void)f;
	return -1;
}
#else
int fputc(int ch, FILE *f)
{ 
	while(!LL_USART_IsActiveFlag_TXE(USART3)){}//�ȴ�����Ϊ��
	LL_USART_TransmitData8(USART3,(uint8_t)ch);	
	while(LL_USART_IsActiveFlag_TC(USART3)==RESET){}//�ȴ��������
	return ch;
}

/**
  * ��������: �ض���c�⺯��getchar,scanf��USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
int fgetc(FILE * f)
{
	while(!LL_USART_IsActiveFlag_RXNE(USART3)){}  //RXNEΪ1ʱ����һ�����ѽ��ղ��ɶ�ȡ	
	return LL_USART_ReceiveData8(USART3);
}
#endif

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	MX_DMA_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_TIM6_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM7_Init();
  MX_TIM11_Init();
  MX_SPI3_Init();
  MX_TIM3_Init();
  MX_RTC_Init();
  MX_USART3_UART_Init();
  MX_DMA_Init();
  MX_USART6_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
//	LL_IWDG_DisableWriteAccess(IWDG);
	LL_RCC_ClearResetFlags();			//�����λ��־λ
	LL_USART_DeInit(USART1);			// ���³�ʼ������1
	Buzzer_ON							// ������������ʾϵͳ��
	InitUserData();						// �ܳ�������ǰ�ȳ�ʼ��Modbus��Ȧ������
	
	/* ���Դ��������Լ����� */
	if((pMbHoldData->nReagent1Volume <= 50)||(pMbHoldData->nReagent2Volume <= 50)||(pMbHoldData->nReagent3Volume <= 50))
	{
		pMbHoldData->nReagent1Volume = 500;		// �Լ�1����Ϊ500ml
		pMbHoldData->nReagent2Volume = 500;		// �Լ�2����Ϊ500ml
		pMbHoldData->nReagent3Volume = 500;		// �Լ�3����Ϊ500ml
		pMbHoldData->nWaterVolume = 500;		// ����ˮ�������Ϊ500ml
		pMbInputData->nFunErrorCode = REAGENT1_EMPTY;
	}
	/************************************************/

	pMbHoldData->nResetRun = 0;					// ��λ��־=0
	pMbInputData->nFunErrorCode = SYS_OK;		// �ϵ���մ������
	
	/* ���Դ���:����ģʽ */
	pMbHoldData->nStartMode = 2;				// ����(ģʽ����Ϊ����ģʽ)
	pMbHoldData->nClockTime = 60;				// ��ˮ����ʱ1��Сʱ
	//pMbHoldData->nControlledStep = CMD_PERISTALTIC_PUMP_OPT;
	//pMbHoldData->nStepperMotorSpeed = 1200;
	//pMbHoldData->nPeristalticPumpParm = 400;
	
	/* ����RTC�Ƿ��� */
//	/* 1. ȷ�� EXTI Line17 ������ */
//	if (!(EXTI->IMR & EXTI_IMR_MR17))
//		pMbHoldData->RW50 = 56;
//	
//	/* 2. ȷ�� RTC Alarm A ��ʹ�� */
//	if (!(RTC->CR & RTC_CR_ALRAE))
//		pMbHoldData->RW50 = 57;
//	
//	/* 3. ȷ�� NVIC ��ʹ�� */
//	if (!(NVIC->ISER[(uint32_t)(RTC_Alarm_IRQn >> 5)] & (1 << (RTC_Alarm_IRQn & 0x1F))))
//		pMbHoldData->RW50 = 58;
	
	AlarmA_Off();	// �ȹر�����
	
	/* ���Դ��� */
	pMbHoldData->nCombiValve = 0;
	pMbHoldData->nValveCtl = 0;
	//pMbInputData->nDevBeRunning = 0;	// ������������
	//Set_MultiAlarm(usCoilBuf);		// �����¸�����
	
	//LL_USART_TransmitData8(USART3, 'A');	// ���Դ���3
	
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_5)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_LSI_Enable();

   /* Wait till LSI is ready */
  while(LL_RCC_LSI_IsReady() != 1)
  {

  }
  LL_PWR_EnableBkUpAccess();
  LL_RCC_LSE_Enable();

   /* Wait till LSE is ready */
  while(LL_RCC_LSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8, 336, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(168000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
