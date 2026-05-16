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

osThreadId_t RunMainTaskHandle;
const osThreadAttr_t RunMainTask_attributes = {
  .name = "RunMainTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};

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
  .stack_size = 256 * 8,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for sModbusTask */
osThreadId_t sModbusTaskHandle;
const osThreadAttr_t sModbusTask_attributes = {
  .name = "sModbusTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};

/* 双总线Modebus任务适配 */
osThreadId_t USART3TaskHandle;
const osThreadAttr_t USART3Task_attributes = {
  .name = "USART3Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t USART6TaskHandle;
const osThreadAttr_t USART6Task_attributes = {
  .name = "USART6Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for StirCtrlTask */
osThreadId_t StirCtrlTaskHandle;
const osThreadAttr_t StirCtrlTask_attributes = {
  .name = "StirCtrlTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Definitions for ErrorCodeManageTask */
osThreadId_t ErrorCodeManageTaskHandle;
const osThreadAttr_t ErrorCodeManageTask_attributes = {
  .name = "ErrorCodeManageTask",
  .stack_size = 256 * 4,          /* 按需求调整，这里给 256 字 */
  .priority = (osPriority_t) osPriorityNormal,
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

/* Definitions for ModbusBusMutex */
osMutexId_t ModbusBusMutexHandle;
const osMutexAttr_t ModbusBusMutex_attributes = {
  .name = "ModbusBusMutex"
};
/* Definitions for Comm3Rx */
osSemaphoreId_t Comm3RxHandle;
const osSemaphoreAttr_t Comm3Rx_attributes = {
  .name = "Comm3Rx"
};
osSemaphoreId_t USART3RxHandle;
const osSemaphoreAttr_t USART3Rx_attributes = {
  .name = "USART3Rx"
};
osSemaphoreId_t USART6RxHandle;
const osSemaphoreAttr_t USART6Rx_attributes = {
  .name = "USART6Rx"
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
/* Definitions for HotMsg */
osSemaphoreId_t HotMsgHandle;
const osSemaphoreAttr_t HotMsg_attributes = {
  .name = "HotMsg"
};

osSemaphoreId_t ErrorCodeSemHandle;
const osSemaphoreAttr_t ErrorCodeSem_attributes = {
  .name = "ErrorCodeSem"
};


/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* Definitions for SampleTimeOut */
osTimerId_t SampleTimeOutHandle;
const osTimerAttr_t SampleTimeOut_attributes = {
  .name = "SampleTimeOut"
};

/* USER CODE END FunctionPrototypes */

void StartDataCollectTask(void *argument);
extern void TempControlTask(void *argument);
extern void ProcessControlTask(void *argument);
extern void UartsRxTask(void *argument);
extern void StartStirCtrlTask(void *argument);
extern void PumpTimerout(void *argument);
extern void HotHoldTimerCallback(void *argument);
extern void ReportStepTimerCallback(void *argument);
extern void SampleTimeOutCallback(void *argument);

extern void ErrorCodeManageTask(void *argument);

extern void RunMainTask(void *argument);

/* 双modbus总线 */
extern void USART3Task(void *argument);
extern void USART6Task(void *argument);

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

  /* creation of ModbusBusMutex */
  ModbusBusMutexHandle = osMutexNew(&ModbusBusMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of Comm3Rx */
  Comm3RxHandle = osSemaphoreNew(1, 1, &Comm3Rx_attributes);

  /* creation of USART3Rx */
  USART3RxHandle = osSemaphoreNew(1, 0, &USART3Rx_attributes);

  /* creation of USART6Rx */
  USART6RxHandle = osSemaphoreNew(1, 0, &USART6Rx_attributes);

  /* creation of UVdata */
  UVdataHandle = osSemaphoreNew(1, 1, &UVdata_attributes);

  /* creation of HostMbdata */
  HostMbdataHandle = osSemaphoreNew(1, 1, &HostMbdata_attributes);

  /* creation of HotMsg */
  HotMsgHandle = osSemaphoreNew(1, 1, &HotMsg_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  
  /* 在 MX_FREERTOS_Init() 里 */
	ErrorCodeSemHandle = osSemaphoreNew(1, 0, &ErrorCodeSem_attributes);  // 初始计数为0

  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of MotorTimer */
  MotorTimerHandle = osTimerNew(PumpTimerout, osTimerOnce, NULL, &MotorTimer_attributes);

  /* creation of HotHoldTimer */
  HotHoldTimerHandle = osTimerNew(HotHoldTimerCallback, osTimerOnce, NULL, &HotHoldTimer_attributes);

  /* creation of ReportStepTimer */
  ReportStepTimerHandle = osTimerNew(ReportStepTimerCallback, osTimerPeriodic, NULL, &ReportStepTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  // 创建定时器（假设使用单次模式，4小时后触发一次）
  SampleTimeOutHandle = osTimerNew(SampleTimeOutCallback, osTimerOnce, NULL, &SampleTimeOut_attributes);
  /* USER CODE END RTOS_TIMERS */

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

	/* USART3 只能在“主站”与“从站(MB3)”二选一，避免资源/中断冲突。 */
#if APP_USART3_MODBUS_SLAVE_MB3
	USART3TaskHandle = osThreadNew(USART3Task, NULL, &USART3Task_attributes);
#endif

	/* USART6：当作为 MB6(Modbus-RTU 从站) 时，使用独立实例栈，可与 USART1/MB3 并行运行。 */
#if APP_USART6_MODBUS_TASK_ENABLE
	USART6TaskHandle = osThreadNew(USART6Task, NULL, &USART6Task_attributes);
#endif
	
  /* creation of StirCtrlTask */
  StirCtrlTaskHandle = osThreadNew(StartStirCtrlTask, NULL, &StirCtrlTask_attributes);
 
   RunMainTaskHandle = osThreadNew(RunMainTask, NULL, &RunMainTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
  /* creation of ErrorCodeManageTask */
	ErrorCodeManageTaskHandle = osThreadNew(ErrorCodeManageTask, NULL, &ErrorCodeManageTask_attributes);
	
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

