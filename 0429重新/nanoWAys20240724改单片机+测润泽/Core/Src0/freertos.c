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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE END Variables */
/* Definitions for DataCollectTask */
osThreadId_t DataCollectTaskHandle;
const osThreadAttr_t DataCollectTask_attributes = {
  .name = "DataCollectTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TempTask */
osThreadId_t TempTaskHandle;
const osThreadAttr_t TempTask_attributes = {
  .name = "TempTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ProcTask */
osThreadId_t ProcTaskHandle;
const osThreadAttr_t ProcTask_attributes = {
  .name = "ProcTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for sModbusTask */
osThreadId_t sModbusTaskHandle;
const osThreadAttr_t sModbusTask_attributes = {
  .name = "sModbusTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for HotMsg */
osMessageQueueId_t HotMsgHandle;
const osMessageQueueAttr_t HotMsg_attributes = {
  .name = "HotMsg"
};
/* Definitions for MotorTimer */
osTimerId_t MotorTimerHandle;
const osTimerAttr_t MotorTimer_attributes = {
  .name = "MotorTimer"
};
/* Definitions for HotHoldTimer */
osTimerId_t HotHoldTimerHandle;
const osTimerAttr_t HotHoldTimer_attributes = {
  .name = "HotHoldTimer"
};
/* Definitions for ReportStepTimer */
osTimerId_t ReportStepTimerHandle;
const osTimerAttr_t ReportStepTimer_attributes = {
  .name = "ReportStepTimer"
};
/* Definitions for osADSMutex */
osMutexId_t osADSMutexHandle;
const osMutexAttr_t osADSMutex_attributes = {
  .name = "osADSMutex"
};
/* Definitions for TempDataMutex */
osMutexId_t TempDataMutexHandle;
const osMutexAttr_t TempDataMutex_attributes = {
  .name = "TempDataMutex"
};
/* Definitions for Comm3Rx */
osSemaphoreId_t Comm3RxHandle;
const osSemaphoreAttr_t Comm3Rx_attributes = {
  .name = "Comm3Rx"
};
/* Definitions for UVdata */
osSemaphoreId_t UVdataHandle;
const osSemaphoreAttr_t UVdata_attributes = {
  .name = "UVdata"
};
/* Definitions for HostMbdata */
osSemaphoreId_t HostMbdataHandle;
const osSemaphoreAttr_t HostMbdata_attributes = {
  .name = "HostMbdata"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDataCollectTask(void *argument);
extern void TempControlTask(void *argument);
extern void ProcessControlTask(void *argument);
extern void UartsRxTask(void *argument);
extern void PumpTimerout(void *argument);
extern void HotHoldTimerCallback(void *argument);
extern void ReportStepTimerCallback(void *argument);

extern void MX_LWIP_Init(void);
extern void MX_USB_DEVICE_Init(void);
extern void MX_USB_HOST_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of osADSMutex */
  osADSMutexHandle = osMutexNew(&osADSMutex_attributes);

  /* creation of TempDataMutex */
  TempDataMutexHandle = osMutexNew(&TempDataMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of Comm3Rx */
  Comm3RxHandle = osSemaphoreNew(1, 1, &Comm3Rx_attributes);

  /* creation of UVdata */
  UVdataHandle = osSemaphoreNew(1, 1, &UVdata_attributes);

  /* creation of HostMbdata */
  HostMbdataHandle = osSemaphoreNew(1, 1, &HostMbdata_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of MotorTimer */
  MotorTimerHandle = osTimerNew(PumpTimerout, osTimerOnce, NULL, &MotorTimer_attributes);

  /* creation of HotHoldTimer */
  HotHoldTimerHandle = osTimerNew(HotHoldTimerCallback, osTimerOnce, NULL, &HotHoldTimer_attributes);

  /* creation of ReportStepTimer */
  ReportStepTimerHandle = osTimerNew(ReportStepTimerCallback, osTimerOnce, NULL, &ReportStepTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of HotMsg */
  HotMsgHandle = osMessageQueueNew (1, sizeof(uint16_t), &HotMsg_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of DataCollectTask */
  DataCollectTaskHandle = osThreadNew(StartDataCollectTask, NULL, &DataCollectTask_attributes);

  /* creation of TempTask */
  TempTaskHandle = osThreadNew(TempControlTask, NULL, &TempTask_attributes);

  /* creation of ProcTask */
  ProcTaskHandle = osThreadNew(ProcessControlTask, NULL, &ProcTask_attributes);

  /* creation of sModbusTask */
  sModbusTaskHandle = osThreadNew(UartsRxTask, NULL, &sModbusTask_attributes);
 

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDataCollectTask */
/**
  * @brief  Function implementing the DataCollectTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDataCollectTask */
__weak void StartDataCollectTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();

  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();

  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN StartDataCollectTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDataCollectTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
