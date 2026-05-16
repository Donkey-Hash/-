
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "uartsrxtask.h"
#include "usart.h"
#include "relaysdef.h"
// #include "ltc1867.h"
#include "peristalticpump.h"
#include "valvedef.h"
#include "TestLEDCtr.h"
// #include "bspuart3.h"
#include "replyhost.h"
#include "tempcontroltask.h"
#include "leddef.h"
#include "processcontroltask.h"
#include "ProcAPI.h"
#include "iwdg.h"
#include "FlashUserData.h"
#include "mDebug.h"
#include "rs485.h"
#include "yssyringepump.h"
#include "ysspectro.h"
#include "modbusdata_mb3.h"

#ifndef FIREWARE_VISION
#define FIREWARE_VISION 24088
#endif

extern osSemaphoreId Comm3RxHandle;
extern osSemaphoreId USART3RxHandle;
extern osSemaphoreId USART6RxHandle;
extern osMutexId_t ModbusBusMutexHandle;
extern bool AnalyzerBeWorking;
// extern osSemaphoreId HostRepeatSEMHandle;
// extern osMutexId osCMDdataMutex;

extern osThreadId ProcTaskHandle;
extern osThreadId TempTaskHandle;
extern UCHAR *usCoilBuf;
extern bool TimeoutFlag;
extern bool FlagStir;

extern uint8_t AutoStep1_isWater;

// 外部可调用声明(用于复位ProcessControlTask任务)
extern void ProcessControlTask(void const *argument);
extern const osThreadAttr_t ProcTask_attributes;

void RS232CmdProcess(void);

// extern osThreadId DeterTaskHandle;
// extern HOST_DATA HostData;

bool HaveNewCMDFlag = false;

uint32_t ErrorCode;

uint16_t START_CMD = 0;	// 命令参数

// void ValveControl(uint8_t valve,uint8_t state);
/* 接收屏幕指令任务，通过串口1接收 */
void UartsRxTask(void const *argument)
{
	g_mb_inst = 1;
	pMbInputData->nFirmVersion = FIREWARE_VISION;	// 设置固件版本
	//	H_RS485_RX;
	//	T_RS485_RX;
	
	/*	MB_RTU：指定通信协议为 Modbus RTU。
	 *	pMbHoldData->nLocalAddr：本地设备的 Modbus 地址。
	 *	3：串口端口号（这里改了内层函数，绑定为串口1，参数3无效）。
	 *	100 * pMbHoldData->nBand：波特率，单位为 bps。
	 *	MB_PAR_NONE：无校验位
	 */
	eMBInit(MB_RTU, pMbHoldData->nLocalAddr, 1, 100 * pMbHoldData->nBand, MB_PAR_NONE);	// 初始化ModbusRTU

	//	osDelay(1000);
	Buzzer_OFF		// 关闭蜂鸣器
	eMBEnable();	// 启动 Modbus 通信
	for (;;)
	{
		if (osSemaphoreAcquire(Comm3RxHandle, 500) == osOK) //尝试获取 Comm3RxHandle 信号量，超时时间为500ms
		{
			if (ModbusBusMutexHandle != NULL)
			{
				/* 防止互斥锁异常长期占用导致三路总线全部阻塞并触发 IWDG */
				if (osMutexAcquire(ModbusBusMutexHandle, 200) != osOK)
				{
					LL_IWDG_ReloadCounter(IWDG);
					continue;
				}
			}
			/*  */
			eMBPoll();					// 调用 eMBPoll 函数轮询 Modbus 通信，处理接收到的数据
			if (HaveNewCMDFlag)
			{
				RS232CmdProcess();		// 对上位机发来的指令进行相应操作
				HaveNewCMDFlag = false;	// 表示命令已经被处理
			}
			if (ModbusBusMutexHandle != NULL)
			{
				osMutexRelease(ModbusBusMutexHandle);
			}
		}
		//		LED1_TOGGLE
		LL_IWDG_ReloadCounter(IWDG); // 重载看门口计数器，防止系统复位
		//		osDelay(50);
	}
}


void USART3Task(void const *argument)
{
	g_mb_inst = 3;
	pMbInputData->nFirmVersion = FIREWARE_VISION;	// 设置固件版本
	//	H_RS485_RX;
	//	T_RS485_RX;
	
	/*	MB_RTU：指定通信协议为 Modbus RTU。
	 *	pMbHoldData->nLocalAddr：本地设备的 Modbus 地址。
	 *	3：串口端口号（这里改了内层函数，绑定为串口1，参数3无效）。
	 *	100 * pMbHoldData->nBand：波特率，单位为 bps。
	 *	MB_PAR_NONE：无校验位
	 */
	eMB3Init(MB_RTU, 101, 3, 115200, MB_PAR_NONE);	// 初始化USART3 ModbusRTU

	//	osDelay(1000);
	Buzzer_OFF		// 关闭蜂鸣器
	eMB3Enable();	// 启动 USART3 Modbus 通信
	for (;;)
	{
		if (osSemaphoreAcquire(USART3RxHandle, 500) == osOK)
		{
			if (ModbusBusMutexHandle != NULL)
			{
				if (osMutexAcquire(ModbusBusMutexHandle, 200) != osOK)
				{
					LL_IWDG_ReloadCounter(IWDG);
					continue;
				}
			}
			eMB3Poll();					// 调用 eMB3Poll 处理 USART3 从站通信
			/* 与 USART1 行为保持一致：当写寄存器触发命令标志时，立即处理命令。 */
			if (HaveNewCMDFlag)
			{
				RS232CmdProcess();
				HaveNewCMDFlag = false;
			}
			if (ModbusBusMutexHandle != NULL)
			{
				osMutexRelease(ModbusBusMutexHandle);
			}
		}
		LL_IWDG_ReloadCounter(IWDG); // 重载看门口计数器，防止系统复位
	}
}


void USART6Task(void const *argument)
{
	pMbInputData->nFirmVersion = FIREWARE_VISION;	// 设置固件版本

	/* USART6 作为第三条从站总线（MB6），与 USART1 / USART3 共用同一套寄存器映射与业务回调。 */
	eMB6Init(MB_RTU, pMbHoldData->nLocalAddr, 6, 100 * pMbHoldData->nBand, MB_PAR_NONE);	// 初始化USART6 ModbusRTU

	Buzzer_OFF		// 关闭蜂鸣器
	eMB6Enable();	// 启动 USART6 Modbus 通信
	for (;;)
	{
		if (osSemaphoreAcquire(USART6RxHandle, 500) == osOK)
		{
			if (ModbusBusMutexHandle != NULL)
			{
				if (osMutexAcquire(ModbusBusMutexHandle, 200) != osOK)
				{
					LL_IWDG_ReloadCounter(IWDG);
					continue;
				}
			}
			eMB6Poll();					// 处理 USART6 从站通信
			if (HaveNewCMDFlag)
			{
				RS232CmdProcess();
				HaveNewCMDFlag = false;
			}
			if (ModbusBusMutexHandle != NULL)
			{
				osMutexRelease(ModbusBusMutexHandle);
			}
		}
		LL_IWDG_ReloadCounter(IWDG); // 重载看门口计数器，防止系统复位
	}
}


/*******************************************************************************
* Function Name  	: RS232CmdProcess
* Description    	: RS232命令数据包解析（对串口1发来的命令进行处理）
* Input          	: 接收数据缓存地址

* Output         	:
* Return         	:
*******************************************************************************/
void RS232CmdProcess(void)
{
	pMbHoldData->nCMDTranMoni = 5;
	switch (pMbHoldData->nHostCMD) // 检查上位机指令
	{
	case CMD_SET_BAUD:		// 设置波特率（该功能还未定义）
	case CMD_SET_LOCAL_ADD:	// 设置地址
		pMbHoldData->nHostCMD = CMD_NONE;	
		break;
	case CMD_SLAVE_HALT:
		StopPump();
		CloseValves(0x3FFFF);
		StopTempCtrl(TEMP_CTRL_CH_POOL);
	case CMD_STOP_TEMPER_CTRL: // 0x12, //18 停止温控。对模式1和模式2均有效。
		StopTempCtrl(TEMP_CTRL_CH_POOL);
		pMbHoldData->nHostCMD = CMD_NONE;
		AnalyzerBeWorking = false;
		break;
	case CMD_SAVE_DEV_OPERAT_PARA:
		//				SaveDevOperatPara();
		SaveUserData();
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_SLAVE_RESET:
		pMbHoldData->nHostCMD = CMD_NONE;
		AddErrRecord(SYSTEM_RESET, 0xFE);
		SetBootJumpAppNoWait();
		//				SaveDevOperatPara();
		SaveUserData();
		__disable_irq();
		NVIC_SystemReset();
		break;
	case CMD_SAVE_DATA: //
		pMbHoldData->nHostCMD = CMD_NONE;
		//				SaveDevOperatPara();
		SaveUserData();
		break;
	case CMD_START_NH3N: // 氨氮主流程
		START_CMD = 1;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_START_TP: // TP主流程
		START_CMD = 2;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_START_COD: // COD主流程
		START_CMD = 3;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_START_TN: // TN主流程
		START_CMD = 4;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_START_CODMN: // CODMN主流程
		START_CMD = 5;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_RZSTART_NH3N: // 氨氮主流程	//RZ
		START_CMD = 6;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_RZSTART_TP: // TP主流程
		START_CMD = 7;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_RZSTART_COD: // COD主流程
		START_CMD = 8;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_RZSTART_TN: // TN主流程
		START_CMD = 9;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_RZSTART_CODMN: // CODMN主流程
		START_CMD = 10;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_HCSTART_NH3N: // 氨氮主流程	//HC
		START_CMD = 11;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_HCSTART_TP: // TP主流程（又来一个TP主流程）
		START_CMD = 12;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_HCSTART_COD: // COD主流程
		START_CMD = 13;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_HCSTART_TN: // TN主流程
		START_CMD = 14;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	case CMD_HCSTART_CODMN: // CODMN主流程
		START_CMD = 15;
		pMbHoldData->nHostCMD = CMD_NONE;
		break;
	default:
		if (false == AnalyzerBeWorking)
		{
			pMbHoldData->nCMDTranMoni = 6;
			AnalyzerBeWorking = true;
			osThreadFlagsSet(ProcTaskHandle, COMM_HOST_CMD);
		}
		else
		{
			pMbHoldData->nCMDTranMoni = 7;
		}
		break;
	}
}

void Buzzer_ON_MS(u16 ms)
{
	if(ms <= 0)
	{
		ms = 100;
	}
	Buzzer_ON;
	osDelay(ms);
	Buzzer_OFF;
}


/*******************************************************************************
* Function Name  	: 仪器上电自检
* Description    	: 
* Input          	: 

* Output         	:
* Return         	:
*******************************************************************************/
void Check_Self(void)
{
	if(pMbInputData->nDevBeRunning == 1)	// 如果仪器之前是运行的但被断电打断，直接进入标定检测，继续上次工作状态
	{
		// step0：上电清洗状态位
		pMbHoldData->RW4229 = 21;	// 上电清洗状态
		
		
		// step1：温度自检测
		if(pMbHoldData->fPoolTemp >= 80)	// 如果比色皿温度过高就降温
		{
			// 开风扇
			pMbHoldData->nCombiValve |= (0x00000001 << 10);	// 设置冷却风扇阀的寄存器操作位
			OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
			StopTempCtrl(TEMP_CTRL_CH_POOL);				// 停止温控
			while(1)
			{
				// 这里应该判断比色皿温度是否降到50，达到则跳出循环
				if(pMbHoldData->fPoolTemp <= 50)
				{
					break;
				}
				osDelay(200);
			}
			// 关风扇
			pMbHoldData->nCombiValve |= (0x00000001 << 10);	// 设置冷却风扇阀的寄存器操作位
			CloseValves(pMbHoldData->nCombiValve);			// 关闭对应设置操作为的阀门
		}
		
		// step2：排空管路
		CloseAllValves();				//	先关闭所有阀
		
		// 开阀
		pMbHoldData->nCombiValve = (0x00000001 << 12);			// 设置废液阀的寄存器操作位
		OpenValves(pMbHoldData->nCombiValve);					// 打开对应设置操作为的阀门
		osDelay(500);
		
		// 排水
		pMbHoldData->nStepperMotorSpeed = -1200;			// 设置蠕动泵的速度寄存器位（负数为排液）
		pMbHoldData->nPeristalticPumpParm = 250;		// 设置蠕动泵的转圈数寄存器位
		StopPump();
		osDelay(300);
		StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
		osDelay(500);
		while(!TimeoutFlag)
		{
			osDelay(100);// 等待蠕动泵操作完成
		}
		TimeoutFlag = false; // 清除电机运行完标志位
		osDelay(500);
		
		// 关阀
		pMbHoldData->nCombiValve = (0x00000001 << 12);			// 设置废液阀的寄存器操作位
		CloseValves(pMbHoldData->nCombiValve);					// 关阀	
		osDelay(500);
		
		//pMbHoldData->RW580 = 1;									// 工作流程状态标志(未标志)
		
		// step2: A插入：闷抽比色皿
		pMbHoldData->nStepperMotorSpeed = 1000;			// 设置蠕动泵的速度寄存器位（正数为闷抽比色皿）
		pMbHoldData->nPeristalticPumpParm = 150;		// 设置蠕动泵的转圈数寄存器位
		StopPump();
		osDelay(300);
		StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
		osDelay(500);
		while(!TimeoutFlag)
		{
			osDelay(100);// 等待蠕动泵操作完成
		}
		TimeoutFlag = false; // 清除电机运行完标志位
		//osDelay(500);
		
		// 开阀
		pMbHoldData->nCombiValve = (0x00000001 << 3);			// 设置比色阀的寄存器操作位
		pMbHoldData->nCombiValve |= (0x00000001 << 8);			// 设置消解下阀的寄存器操作位
		pMbHoldData->nCombiValve |= (0x00000001 << 9);			// 设置消解上阀的寄存器操作位
		OpenValves(pMbHoldData->nCombiValve);		// 打开对应设置操作为的阀门
		osDelay(500);
		
		// 正速度排控比色皿
		pMbHoldData->nStepperMotorSpeed = 1000;			// 设置蠕动泵的速度寄存器位（正数为闷抽比色皿）
		pMbHoldData->nPeristalticPumpParm = 150;		// 设置蠕动泵的转圈数寄存器位
		StopPump();
		osDelay(300);
		StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
		osDelay(500);
		while(!TimeoutFlag)
		{
			osDelay(100);		// 等待蠕动泵操作完成
		}
		TimeoutFlag = false; 	// 清除电机运行完标志位
		//osDelay(500);
		
		// 关阀
		pMbHoldData->nCombiValve = (0x00000001 << 3);			// 设置比色阀的寄存器操作位
		pMbHoldData->nCombiValve |= (0x00000001 << 8);			// 设置消解下阀的寄存器操作位
		pMbHoldData->nCombiValve |= (0x00000001 << 9);			// 设置消解上阀的寄存器操作位
		CloseValves(pMbHoldData->nCombiValve);					// 关闭对应设置操作为的阀门
		osDelay(500);
		
		// 开废水阀:检测开哪个阀
		if(AutoStep1_isWater == 1)// 1:蒸馏水标志(首次启动为1，连续工作为0)
		{
			pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
			OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
		}
		else
		{
			pMbHoldData->nCombiValve = (0x00000001 << 12);	// 设置废液阀的寄存器操作位
			OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
		}
		osDelay(300);	// 给0.3s时间开阀
		
		// 排空计量管废液
		pMbHoldData->nStepperMotorSpeed = -1200;			// 设置蠕动泵的速度寄存器位（负数为排废液）
		pMbHoldData->nPeristalticPumpParm = 150;			// 设置蠕动泵的转圈数寄存器位
		StopPump();
		osDelay(300);
		StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
		osDelay(500);
		while(!TimeoutFlag)
		{
			osDelay(100);// 等待蠕动泵操作完成
		}
		TimeoutFlag = false; // 清除电机运行完标志位
		//osDelay(500);
		
		// 关废液阀
		pMbHoldData->nCombiValve = (0x00000001 << 0);		// 设置清洗废液阀的寄存器操作位
		pMbHoldData->nCombiValve |= (0x00000001 << 12);		// 设置废液阀的寄存器操作位
		CloseValves(pMbHoldData->nCombiValve);				// 关闭对应设置操作为的阀门
		osDelay(500);
		
		// step3:排空比色皿(循环3次)
		for(int i = 0; i < 3; i++)
		{
			// 开阀
			pMbHoldData->nCombiValve = (0x00000001 << 3);			// 设置比色阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 8);			// 设置消解下阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 9);			// 设置消解上阀的寄存器操作位
			OpenValves(pMbHoldData->nCombiValve);		// 打开对应设置操作为的阀门
			osDelay(500);
			
			// 抽水
			pMbHoldData->nStepperMotorSpeed = 1000;		// 设置蠕动泵的速度寄存器位（负数为排液）
			pMbHoldData->nPeristalticPumpParm = 150;	// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);		// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; 	// 清除电机运行完标志位
			//osDelay(500);
			
			// 关阀
			pMbHoldData->nCombiValve = (0x00000001 << 3);			// 设置比色阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 8);			// 设置消解下阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 9);			// 设置消解上阀的寄存器操作位
			CloseValves(pMbHoldData->nCombiValve);					// 关闭对应设置操作为的阀门
			osDelay(500);
			
			// 开阀:检测开哪个阀
			if(AutoStep1_isWater == 1)// 1:蒸馏水标志(首次启动为1，连续工作为0)
			{
				pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
				pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
				OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
			}
			else
			{
				pMbHoldData->nCombiValve = (0x00000001 << 12);	// 设置废液阀的寄存器操作位
				OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
			}
			osDelay(300);	// 给0.3s时间开阀
			
			// 排水
			pMbHoldData->nStepperMotorSpeed = -1200;		// 设置蠕动泵的速度寄存器位（负数为排液）
			pMbHoldData->nPeristalticPumpParm = 250;		// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; // 清除电机运行完标志位
			//osDelay(500);
			
			// 关阀
			pMbHoldData->nCombiValve = (0x00000001 << 0);		// 设置清洗废液阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 12);		// 设置废液阀的寄存器操作位
			CloseValves(pMbHoldData->nCombiValve);				// 关闭对应设置操作为的阀门
			osDelay(500);
		}
		
		pMbInputData->nDevBeRunning = 0;
	}
	
}


void RunMainTask(void const *argument) // FREERTOS任务
{
	u16 i = 0;	// 作为任务运行成败返回的标志位
	
	/* 测试RTC */
//	uint8_t h, m, s, day, month, year;
	
	/* 仪器上电后自检 */
	
	CloseAllValves();
	StopPump();
	StopTempCtrl(TEMP_CTRL_CH_POOL);		// 停止温控
	
	//Check_Self();
	
	while (1)
	{
		if(START_CMD > 0)
		{
			Buzzer_ON_MS(150);			// 蜂鸣器提示分析开始
			osDelay(150);
			Buzzer_ON_MS(150);			// 蜂鸣器提示分析开始
			switch(START_CMD)			// 接收用户选择的分析指令
			{
				case 1:					// 氨氮主流程
					START_CMD = 0;		// 清除指令
					i = START_NH3N();	// 调用氨氮主流程
					break;
				case 2:					// 总磷主流程
					START_CMD = 0;		// 清除指令
					i = START_TP();		// 调用总磷主流程
					break;
				case 3:					// COD主流程
					START_CMD = 0;		// 清除指令
					i = START_COD();	// 调用COD主流程
					break;
				case 4:					// 总氮主流程
					START_CMD = 0;      // 清除指令
					i = START_TN();     // 调用总氮主流程
					break;
				case 5:					// CODMN主流程
					START_CMD = 0;      // 清除指令
					i = START_CODMN();  // 调用CODMN主流程
					break;
//				case 6:
//					START_CMD = 0;
//					i = RZSTART_NH3N();
//					break;
//				case 7:
//					START_CMD = 0;
//					i = RZSTART_TP();
//					break;
				case 8:					// ***主流程	
					START_CMD = 0;      // 清除指令
					i = RZSTART_COD();  // 调用***主流程
					break;
//				case 9:
//					START_CMD = 0;
//					i = RZSTART_TN();
//					break;
//				case 10:
//					START_CMD = 0;
//					i = RZSTART_CODMN();
//					break;
				case 11:				// ***主流程	
					START_CMD = 0;	    // 清除指令
					i = HCSTART_NH3N(); // 调用***主流程
					break;
//				case 12:
//					START_CMD = 0;
//					i = HCSTART_TP();
//					break;
				// case 13:
				// 	START_CMD = 0;
				// 	i = HCSTART_COD();
				// 	break;
//				case 14:
//					START_CMD = 0;
//					i = HCSTART_TN();
//					break;
//				case 15:
//					START_CMD = 0;
//					i = HCSTART_CODMN();
//					break;
				default:					// 未知命令
					START_CMD = 0;          // 清除指令
					for(i = 0;i < 3;i ++)   // 蜂鸣器报警提示错误
					{
						Buzzer_ON_MS(300);
						osDelay(300);
					}
					i = 0;
					break;
			}
			if (i) // 有故障
			{
				pMbHoldData->RW580 = 101; // 下位机结束标志
			}

		}
		
		
		
		/* 任务复位检测 */
		if(Check_Reset())									// 检查到运行任务流程需要复位
		{
			// 先检测需要复位的任务状态
			eTaskState eState = eTaskGetState(ProcTaskHandle);
			if(eState == eSuspended || eState == eBlocked)	// 任务处于挂起或阻塞状态，允许复位
			{
				SaveUserData();								// 先保存用户数据
				vTaskDelete(ProcTaskHandle);				// 先删除任务
				osDelay(300);
				ProcTaskHandle = osThreadNew((void*)ProcessControlTask, NULL, &ProcTask_attributes);	// 创建任务
			}
			
		}
		
		
//		/* 测试RTC */
//		h    = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
//		m    = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
//		s    = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));
//		
//		year = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC));
//		month= __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC));
//		day  = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC));
//		
//		pMbInputData->nHourBuild = h;
//		pMbInputData->nMinuteBuild = m;
//		pMbInputData->nSecondBuild = s;
		
		osDelay(1000);
	}
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

u8 GetWaterState(void)	//有液体返回0.缺液体返回1
{
	osDelay(100);
	if(usDiscreteInputBuf[0] & 0x02)
		return 1;
	else
		return 0;
}


void Pump_run(int16_t speed, int16_t maxcircle) // 操作蠕动泵，速度，圈数
{
	StopPump(); // 操作蠕动泵
	osDelay(300);
	pMbHoldData->nStepperMotorSpeed = speed;	   // 速度，大于0就是抽液体，小于0就是排液体
	pMbHoldData->nPeristalticPumpParm = maxcircle; // 圈数
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);
	while (!TimeoutFlag)
		osDelay(100);
	osDelay(500);
}

void Paikongbsm(uint16_t water_flag, uint16_t codmn_flag) // 高锰排空7次，其它3次
{
	uint16_t i = 0, times = 3;
	CloseAllValves(); // 关组合阀
	if (codmn_flag)
		times = 9;
	for (i = 0; i < times; i++)
	{
		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		Pump_run(1300, 150); // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0308); // 关比色阀，消解上+下阀
		osDelay(500);
		if (i < times - 1 && pMbInputData->nLL1AD < pMbHoldData->RW30 * 1.1 && pMbInputData->nLL1AD > pMbHoldData->RW30 * 0.9 && pMbHoldData->RW30 < 1500 && pMbHoldData->RW30 > 1100) // 判断低液位，避免重复排空
		{
			i = times - 1;
		}
		if (water_flag) // 蒸馏水标志
		{
			OpenValves(0x0001); // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
		}
		else
		{
			OpenValves(0x1000); // 开废液阀
		}
		Pump_run(-1500, 180); // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0001);  // 关清洗废液阀
		CloseValves(0x1000);  // 关废液阀
	}
	CloseAllValves(); // 关组合阀
}

void RZPaikongbsm(uint16_t water_flag, uint16_t codmn_flag) // 高锰排空7次，其它3次
{
	uint16_t i = 0, times = 3;
	CloseAllValves(); // 关组合阀
	RUNZE_SETHOLE(10);
	if (codmn_flag)
		times = 9;
	RUNZE_SET_SPEED(200);
	for (i = 0; i < times; i++)
	{
		RUNZE_SETHOLE(10);	//1~10，先采空气1ml
		RUNZE_SET_ML(0.2);	//0~6
		osDelay(1000);


		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		osDelay(500);
		RUNZE_SETHOLE(3);

		// Pump_run(1300, 150); // 命令12，操作蠕动泵，速度，圈数
		RUNZE_SET_ML(6);	//0~6
		// osDelay(5000);
		osDelay(1000);
		CloseValves(0x0308); // 关比色阀，消解上+下阀
		// if (i < times - 1 && pMbInputData->nLL1AD < pMbHoldData->RW30 * 1.1 && pMbInputData->nLL1AD > pMbHoldData->RW30 * 0.9 && pMbHoldData->RW30 < 1500 && pMbHoldData->RW30 > 1100) // 判断低液位，避免重复排空
		// {
		// 	i = times - 1;
		// }
		if (water_flag) // 蒸馏水标志
		{
			RUNZE_SETHOLE(2);
			OpenValves(0x0001); // 开清洗废液阀
			// if (pMbHoldData->RW577)
			// {
			// 	OpenValves(0x1000); // 开废液阀
			// }
		}
		else
		{
			RUNZE_SETHOLE(1);
			OpenValves(0x1000); // 开废液阀
		}
		// Pump_run(-1500, 180); // 命令12，操作蠕动泵，速度，圈数
		RUNZE_SET_ML(0);	//0~6
		CloseValves(0x0001);  // 关清洗废液阀
		CloseValves(0x1000);  // 关废液阀
		osDelay(1000);
	}
	CloseAllValves(); // 关组合阀
	RUNZE_SETHOLE(1);
}

void HCPaikongbsm(uint16_t water_flag, uint16_t codmn_flag) // 高锰排空7次，其它4次
{
	uint16_t i = 0, times = 4;
	CloseAllValves(); // 关组合阀
	HC_SETHOLE(10);
	if (codmn_flag)
		times = 9;
	HC_SET_SPEED(500);
	for (i = 0; i < times; i++)
	{
		HC_SETHOLE(10);	//1~10，先采空气1ml
		HC_SET_ML(0.2);	//0~6
		osDelay(1000);


		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		osDelay(500);
		HC_SETHOLE(3);

		// Pump_run(1300, 150); // 命令12，操作蠕动泵，速度，圈数
		HC_SET_ML(6);	//0~6
		// osDelay(5000);
		osDelay(1000);
		CloseValves(0x0308); // 关比色阀，消解上+下阀
		// if (i < times - 1 && pMbInputData->nLL1AD < pMbHoldData->RW30 * 1.1 && pMbInputData->nLL1AD > pMbHoldData->RW30 * 0.9 && pMbHoldData->RW30 < 1500 && pMbHoldData->RW30 > 1100) // 判断低液位，避免重复排空
		// {
		// 	i = times - 1;
		// }
		if (water_flag) // 蒸馏水标志
		{
			HC_SETHOLE(2);
			OpenValves(0x0001); // 开清洗废液阀
			// if (pMbHoldData->RW577)
			// {
			// 	OpenValves(0x1000); // 开废液阀
			// }
		}
		else
		{
			HC_SETHOLE(1);
			OpenValves(0x1000); // 开废液阀
		}
		// Pump_run(-1500, 180); // 命令12，操作蠕动泵，速度，圈数
		HC_SET_ML(0);	//0~6
		CloseValves(0x0001);  // 关清洗废液阀
		CloseValves(0x1000);  // 关废液阀
		osDelay(1000);
	}
	CloseAllValves(); // 关组合阀
	HC_SETHOLE(1);
}

uint16_t Runxi(void)
{
	uint16_t i = 0, j = 0, k = 0;
	if (pMbHoldData->RW43 == 0) // 润洗次数
		return 0;
	pMbInputData->nStepCode = 7; // 润洗
	osDelay(500);
	while (1)
	{
		CloseAllValves();	   // 关组合阀
		i = pMbHoldData->RW50; // 工作类型
		if (i == 9 || i == 20 || i == 22 || i == 25 || i == 26 || i == 27 || i == 28 || i == 30 || i == 32 || i == 35 || i == 38 || i == 43)
		{
			OpenValves(0x0080); // 开量程阀
			if (pMbHoldData->RW576)
			{
				OpenValves(0x0002); // 开水样阀
			}
		}
		else if (i == 2 || i == 4 || i == 34 || i == 37 || i == 39 || i == 40 || i == 44 || i == 45)
		{
			OpenValves(0x2000); // 核查样阀
		}
		else if (i == 1 || i == 5 || i == 6 || i == 7 || i == 41)
		{
			OpenValves(0x0002); // 开水样阀
		}
		else if (i == 3 || i == 8 || i == 19 || i == 21 || i == 24 || i == 31 || i == 33 || i == 36 || i == 42)
		{
			OpenValves(0x0010); // 蒸馏水阀
		}
		else if(i == 23)	//维护测试
		{
			switch(pMbHoldData->RW44)
			{
				case 0:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					break;
				case 1:
					OpenValves(0x2000); // 核查样阀
					break;
				case 2:
					OpenValves(0x0002); // 开水样阀
					break;
				case 3:
					OpenValves(0x0010); // 蒸馏水阀
					break;
				default:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					break;
			}
		}
		else
		{
			OpenValves(0x0002); // 开水样阀
		}

		// 命令13：
		pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		pMbHoldData->nStepperMotorSpeed = 1300;	 // 速度
		pMbHoldData->nLevelNum = 1;				 // 液位
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves();				 // 关组合阀
		if (pMbInputData->nSysErrorCode) // 718行，报警
		{
			j++;
			if (j >= 3) // 报警次数
			{
				pMbHoldData->RW4230 = 2; // 故障代码=2
				return 1;
			}
		}
		else
		{
			OpenValves(0x0001); // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			Pump_run(-1500, 250); // 操作蠕动泵，速度，圈数
			CloseValves(0x0001);  // 关清洗废液阀
			CloseValves(0x1000);  // 关废液阀
			k++;
			if (k >= pMbHoldData->RW43)
			{
				break;
			}
		}
	}
	// 增加润洗后步骤
	CloseAllValves();
	OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	Pump_run(1300, 100); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0308); // 关比色阀，消解上+下阀
	OpenValves(0x0001);	 // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1500, 200); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();
	return 0;
}

uint16_t RZRunxi(void)
{
	uint16_t i = 0, j = 0, k = 0;
	if (pMbHoldData->RW43 == 0) // 润洗次数
		return 0;
	pMbInputData->nStepCode = 7; // 润洗
	osDelay(500);
	while (1)
	{
		CloseAllValves();	   // 关组合阀
		RUNZE_SET_SPEED(100);
		RUNZE_SETHOLE(10);	//1~10，先采空气1ml
		RUNZE_SET_ML(0.2);	//0~6
		osDelay(1000);

		i = pMbHoldData->RW50; // 工作类型
		if (i == 9 || i == 20 || i == 22 || i == 25 || i == 26 || i == 27 || i == 28 || i == 30 || i == 32 || i == 35 || i == 38 || i == 43)
		{
			OpenValves(0x0080); // 开量程阀
			if (pMbHoldData->RW576)
			{
				OpenValves(0x0002); // 开水样阀
			}
			RUNZE_SETHOLE(7);	//1~10
		}
		else if (i == 2 || i == 4 || i == 34 || i == 37 || i == 39 || i == 40 || i == 44 || i == 45)
		{
			OpenValves(0x2000); // 核查样阀
			RUNZE_SETHOLE(6);	//1~10
		}
		else if (i == 1 || i == 5 || i == 6 || i == 7 || i == 41)
		{
			OpenValves(0x0002); // 开水样阀
			RUNZE_SETHOLE(5);	//1~10
		}
		else if (i == 3 || i == 8 || i == 19 || i == 21 || i == 24 || i == 31 || i == 33 || i == 36 || i == 42)
		{
			OpenValves(0x0010); // 蒸馏水阀
			RUNZE_SETHOLE(4);	//1~10
		}
		else if(i == 23)	//维护测试
		{
			switch(pMbHoldData->RW44)
			{
				case 0:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					RUNZE_SETHOLE(7);	//1~10
					break;
				case 1:
					OpenValves(0x2000); // 核查样阀
					RUNZE_SETHOLE(6);	//1~10
					break;
				case 2:
					OpenValves(0x0002); // 开水样阀
					RUNZE_SETHOLE(5);	//1~10
					break;
				case 3:
					OpenValves(0x0010); // 蒸馏水阀
					RUNZE_SETHOLE(4);	//1~10
					break;
				default:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					RUNZE_SETHOLE(7);	//1~10
					break;
			}
		}
		else
		{
			OpenValves(0x0002); // 开水样阀
			RUNZE_SETHOLE(5);	//1~10
		}

		// 命令13：
		RUNZE_SET_ML(4+0.2);	//0~6
		// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		// pMbHoldData->nStepperMotorSpeed = 1300;	 // 速度
		// pMbHoldData->nLevelNum = 1;				 // 液位
		// pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves();				 // 关组合阀
		// if (pMbInputData->nSysErrorCode) // 718行，报警
		if (GetWaterState())	//有液体返回0.缺液体返回1) // 718行，报警
		{
			RUNZE_SET_ML(6);	//0~6
			j++;
			if (j >= 3) // 报警次数
			{
				pMbHoldData->RW4230 = 2; // 故障代码=2
				return 1;
			}
		}
		else
		{
			OpenValves(0x0001); // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			RUNZE_SETHOLE(2);	//1~10
			RUNZE_SET_SPEED(200);
			RUNZE_SET_ML(0);	//0~6
			// Pump_run(-1500, 250); // 操作蠕动泵，速度，圈数
			CloseValves(0x0001);  // 关清洗废液阀
			CloseValves(0x1000);  // 关废液阀
			k++;
			if (k >= pMbHoldData->RW43)
			{
				break;
			}
		}
		RUNZE_SETHOLE(1);	//1~10
		CloseAllValves();				 // 关组合阀
		RUNZE_SET_ML(0);	//0~6
	}
	// 增加润洗后步骤
	// CloseAllValves();
	// OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	// Pump_run(1300, 100); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x0308); // 关比色阀，消解上+下阀
	// OpenValves(0x0001);	 // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	OpenValves(0x1000); // 开废液阀
	// }
	// Pump_run(-1500, 200); // 命令12，操作蠕动泵，速度，圈数
	RUNZE_SETHOLE(1);	//1~10
	CloseAllValves();
	return 0;
}

uint16_t HCRunxi(void)
{
	uint16_t i = 0, j = 0, k = 0;
	if (pMbHoldData->RW43 == 0) // 润洗次数
		return 0;
	pMbInputData->nStepCode = 7; // 润洗
	osDelay(500);
	while (1)
	{
		CloseAllValves();	   // 关组合阀
		HC_SET_SPEED(200);
		HC_SETHOLE(10);	//1~10，先采空气1ml
		HC_SET_ML(0.2);	//0~6
		osDelay(1000);

		i = pMbHoldData->RW50; // 工作类型
		if (i == 9 || i == 20 || i == 22 || i == 25 || i == 26 || i == 27 || i == 28 || i == 30 || i == 32 || i == 35 || i == 38 || i == 43)
		{
			OpenValves(0x0080); // 开量程阀
			if (pMbHoldData->RW576)
			{
				OpenValves(0x0002); // 开水样阀
			}
			HC_SETHOLE(7);	//1~10
		}
		else if (i == 2 || i == 4 || i == 34 || i == 37 || i == 39 || i == 40 || i == 44 || i == 45)
		{
			OpenValves(0x2000); // 核查样阀
			HC_SETHOLE(6);	//1~10
		}
		else if (i == 1 || i == 5 || i == 6 || i == 7 || i == 41)
		{
			OpenValves(0x0002); // 开水样阀
			HC_SETHOLE(5);	//1~10
		}
		else if (i == 3 || i == 8 || i == 19 || i == 21 || i == 24 || i == 31 || i == 33 || i == 36 || i == 42)
		{
			OpenValves(0x0010); // 蒸馏水阀
			HC_SETHOLE(4);	//1~10
		}
		else if(i == 23)	//维护测试
		{
			switch(pMbHoldData->RW44)
			{
				case 0:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					HC_SETHOLE(7);	//1~10
					break;
				case 1:
					OpenValves(0x2000); // 核查样阀
					HC_SETHOLE(6);	//1~10
					break;
				case 2:
					OpenValves(0x0002); // 开水样阀
					HC_SETHOLE(5);	//1~10
					break;
				case 3:
					OpenValves(0x0010); // 蒸馏水阀
					HC_SETHOLE(4);	//1~10
					break;
				default:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					HC_SETHOLE(7);	//1~10
					break;
			}
		}
		else
		{
			OpenValves(0x0002); // 开水样阀
			HC_SETHOLE(5);	//1~10
		}

		// 命令13：
		HC_SET_ML(4+0.2);	//0~6
		// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		// pMbHoldData->nStepperMotorSpeed = 1300;	 // 速度
		// pMbHoldData->nLevelNum = 1;				 // 液位
		// pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves();				 // 关组合阀
		// if (pMbInputData->nSysErrorCode) // 718行，报警
		if (GetWaterState())	//有液体返回0.缺液体返回1) // 718行，报警
		{
			HC_SET_ML(5);	//0~6
			j++;
			if (j >= 3) // 报警次数
			{
				pMbHoldData->RW4230 = 2; // 故障代码=2
				return 1;
			}
		}
		else
		{
			OpenValves(0x0001); // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			HC_SETHOLE(2);	//1~10
			HC_SET_SPEED(300);
			HC_SET_ML(0);	//0~6
			// Pump_run(-1500, 250); // 操作蠕动泵，速度，圈数
			CloseValves(0x0001);  // 关清洗废液阀
			CloseValves(0x1000);  // 关废液阀
			k++;
			if (k >= pMbHoldData->RW43)
			{
				break;
			}
		}
		HC_SETHOLE(1);	//1~10
		CloseAllValves();				 // 关组合阀
		HC_SET_ML(0);	//0~6
	}
	// 增加润洗后步骤
	// CloseAllValves();
	// OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	// Pump_run(1300, 100); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x0308); // 关比色阀，消解上+下阀
	// OpenValves(0x0001);	 // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	OpenValves(0x1000); // 开废液阀
	// }
	// Pump_run(-1500, 200); // 命令12，操作蠕动泵，速度，圈数
	HC_SETHOLE(1);	//1~10
	CloseAllValves();
	return 0;
}

uint16_t Caiqianpd(void) // 采前判断
{
	uint16_t i = 0;
	while (1)
	{
		if (((pMbHoldData->RW34 + 50) <= pMbInputData->nLL1AD <= (pMbHoldData->RW32 - 50)) && ((pMbHoldData->RW35 + 50) <= pMbInputData->nLL2AD <= (pMbHoldData->RW33 - 50)))
		{
			if (pMbHoldData->RW4229 == 1)
				pMbHoldData->RW4229 = 0;
			if (pMbHoldData->RW31 > 2000 && pMbHoldData->RW30 > 2000)
			{
				pMbHoldData->RW30 = pMbInputData->nLL1AD;
				pMbHoldData->RW31 = pMbInputData->nLL2AD;
				pMbHoldData->RW32 = pMbInputData->nLL1AD * 1.2;
				pMbHoldData->RW33 = pMbInputData->nLL2AD * 1.2;
				pMbHoldData->RW34 = pMbInputData->nLL1AD * 0.5;
				pMbHoldData->RW35 = pMbInputData->nLL2AD * 0.5;
				pMbHoldData->RW579 = 1;
				SaveUserData();
			}
			return 0; // 通过
		}
		else
		{
			pMbHoldData->RW4229 = 1; // 采前液位信号出现异常
			OpenValves(0x1000);		 // 开废液阀
			Pump_run(-1500, 250);	 // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x1000);	 // 关废液阀

			OpenValves(0x0010);	 // 开蒸馏水阀
			Pump_run(1300, 200);	 // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0010); // 关蒸馏水阀

			OpenValves(0x0001); // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			Pump_run(-1500, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0001); // 关清洗废液阀
			if (pMbHoldData->RW577)
			{
				CloseValves(0x1000); // 关废液阀
			}
			if (i++ >= 5)
			{
				break;
			}
		}
	}

	i = 0;
	while (1)
	{
		CheckLevelsAD(); // 108命令
		osDelay(1000);
		if (pMbInputData->nLL1AD >= 1200 && pMbInputData->nLL1AD <= 1400 && pMbInputData->nLL2AD >= 1200 && pMbInputData->nLL2AD <= 1400)
		{
			break;
		}
		else
		{
			if (i++ >= 3)
			{
				pMbHoldData->RW4230 = 1; // 故障代码=1
				return 1;
			}
		}
	}
	pMbHoldData->RW30 = pMbInputData->nLL1AD;
	pMbHoldData->RW31 = pMbInputData->nLL2AD;
	pMbHoldData->RW32 = pMbInputData->nLL1AD * 1.2;
	pMbHoldData->RW33 = pMbInputData->nLL2AD * 1.2;
	pMbHoldData->RW34 = pMbInputData->nLL1AD * 0.5;
	pMbHoldData->RW35 = pMbInputData->nLL2AD * 0.5;
	pMbHoldData->RW579 = 1;
	SaveUserData();
	return 0;
}

uint16_t Caiyang(u16 shangya_flag, u16 TP_flag) // 采样
{
	uint16_t i = 0, j = 0, k = 0;
	pMbInputData->nStepCode = 8;
	osDelay(500);
	while (1)
	{
		if (Caiqianpd()) // 采前判断
		{
			return 1;
		}
		CloseAllValves();	   // 关组合阀
		i = pMbHoldData->RW50; // 工作类型
		if (i == 9 || i == 20 || i == 22 || i == 25 || i == 26 || i == 27 || i == 28 || i == 30 || i == 32 || i == 35 || i == 38 || i == 43)
		{
			OpenValves(0x0080); // 开量程阀
			if (pMbHoldData->RW576)
			{
				OpenValves(0x0002); // 开水样阀
			}
		}
		else if (i == 2 || i == 4 || i == 34 || i == 37 || i == 39 || i == 40 || i == 44 || i == 45)
		{
			OpenValves(0x2000); // 核查样阀
		}
		else if (i == 1 || i == 5 || i == 6 || i == 7 || i == 41)
		{
			OpenValves(0x0002); // 开水样阀
		}
		else if (i == 3 || i == 8 || i == 19 || i == 21 || i == 24 || i == 31 || i == 33 || i == 36 || i == 42)
		{
			OpenValves(0x0010); // 蒸馏水阀
		}
		else if(i == 23)	//维护测试
		{
			switch(pMbHoldData->RW44)
			{
				case 0:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					break;
				case 1:
					OpenValves(0x2000); // 核查样阀
					break;
				case 2:
					OpenValves(0x0002); // 开水样阀
					break;
				case 3:
					OpenValves(0x0010); // 蒸馏水阀
					break;
				default:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					break;
			}
		}
		else
		{
			OpenValves(0x0002); // 开水样阀
		}
		if (TP_flag == 1) // 总磷
		{
			if (pMbHoldData->RW45 == 0 || pMbHoldData->RW45 == 1) // 量程选择
			{
				pMbHoldData->nLevelNum = 2; // 液位
			}
			else
			{
				pMbHoldData->nLevelNum = 1; // 液位
			}
		}
		else
		{
			if (pMbHoldData->RW45 == 0) // 量程选择
			{
				pMbHoldData->nLevelNum = 2; // 液位
			}
			else
			{
				pMbHoldData->nLevelNum = 1; // 液位
			}
		}
		// 命令13：
		pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
		// pMbHoldData->nLevelNum = 1;					//液位
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		if (pMbInputData->nSysErrorCode) // 报警
		{
			CloseAllValves(); // 关组合阀
			j++;
			if (j >= 3) // 报警次数
			{
				pMbHoldData->RW4230 = 3; // 故障代码=3
				return 1;
			}
		}
		else
		{
			// 增加多跑几圈
			Pump_run(100, 20); // 命令12，操作蠕动泵，速度，圈数

			// 下压定位，命令14
			pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
			// pMbHoldData->nLevelNum = 1;					//液位
			pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);

			// 氨氮不用上压定位
			if (shangya_flag)
			{
				// 增加上压定位步骤。
				// 液位下行10圈
				Pump_run(-100, 10); // 命令12，操作蠕动泵，速度，圈数
				// 上行定位，命令15
				pMbHoldData->nPeristalticPumpParm = 200; // 圈数
				pMbHoldData->nStepperMotorSpeed = 30;	 // 速度
				// pMbHoldData->nLevelNum = 1;					//液位
				pMbInputData->nSysErrorCode = DeterminLevelUp(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			}
			CloseAllValves(); // 关组合阀
			if (pMbInputData->nSysErrorCode)
			{
				OpenValves(0x0001); // 开清洗废液阀
				if (pMbHoldData->RW577)
				{
					OpenValves(0x1000); // 开废液阀
				}
				Pump_run(-800, 300); // 操作蠕动泵，速度，圈数
				k++;
				if (k >= 3)
				{
					pMbHoldData->RW4230 = 22; // 故障代码=22
					return 1;
				}
			}
			else
			{
				break;
			}
		}
	}
	CloseAllValves();	  // 关组合阀
	OpenValves(0x0308);	  // 开比色阀，消解上+下阀
	Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0308);  // 关比色阀，消解上+下阀

	// 增压
	Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	// 泄压
	Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数

	CloseAllValves();					  // 关组合阀
	if (pMbInputData->nSysErrorCode == 3) // 采样后清除未采到样报警
	{
		pMbInputData->nSysErrorCode = 0;
	}
	return 0;
}

uint16_t RZCaiyang(u16 shangya_flag, u16 TP_flag) // 采样
{	//TP_flag:	1总磷1ml，2总氮1ml，	3氨氮0.5ml，	4COD2ml，5CODMN2ml
	uint16_t i = 0, j = 0, k = 0;
	float tmp_ml = 0;
	pMbInputData->nStepCode = 8;
	osDelay(500);
	RUNZE_SET_SPEED(60);
	while (1)
	{
		// if (Caiqianpd()) // 采前判断
		// {
		// 	return 1;
		// }
		CloseAllValves();	   // 关组合阀
		RUNZE_SETHOLE(10);	//1~10，先采空气0.2ml
		RUNZE_SET_ML(0.2);	//0~6
		osDelay(1000);
		i = pMbHoldData->RW50; // 工作类型
		if (i == 9 || i == 20 || i == 22 || i == 25 || i == 26 || i == 27 || i == 28 || i == 30 || i == 32 || i == 35 || i == 38 || i == 43)
		{
			OpenValves(0x0080); // 开量程阀
			if (pMbHoldData->RW576)
			{
				OpenValves(0x0002); // 开水样阀
			}
			RUNZE_SETHOLE(7);	//1~10
		}
		else if (i == 2 || i == 4 || i == 34 || i == 37 || i == 39 || i == 40 || i == 44 || i == 45)
		{
			OpenValves(0x2000); // 核查样阀
			RUNZE_SETHOLE(6);	//1~10
		}
		else if (i == 1 || i == 5 || i == 6 || i == 7 || i == 41)
		{
			OpenValves(0x0002); // 开水样阀
			RUNZE_SETHOLE(5);	//1~10
		}
		else if (i == 3 || i == 8 || i == 19 || i == 21 || i == 24 || i == 31 || i == 33 || i == 36 || i == 42)
		{
			OpenValves(0x0010); // 蒸馏水阀
			RUNZE_SETHOLE(4);	//1~10
		}
		else if(i == 23)	//维护测试
		{
			switch(pMbHoldData->RW44)
			{
				case 0:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					RUNZE_SETHOLE(7);	//1~10
					break;
				case 1:
					OpenValves(0x2000); // 核查样阀
					RUNZE_SETHOLE(6);	//1~10
					break;
				case 2:
					OpenValves(0x0002); // 开水样阀
					RUNZE_SETHOLE(5);	//1~10
					break;
				case 3:
					OpenValves(0x0010); // 蒸馏水阀
					RUNZE_SETHOLE(4);	//1~10
					break;
				default:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					RUNZE_SETHOLE(7);	//1~10
					break;
			}
		}
		else
		{
			OpenValves(0x0002); // 开水样阀
			RUNZE_SETHOLE(5);	//1~10
		}
		if (TP_flag == 1) // 总磷
		{
			if (pMbHoldData->RW45 == 0 || pMbHoldData->RW45 == 1) // 量程选择
			{
				pMbHoldData->nLevelNum = 2; // 高液位
			}
			else
			{
				pMbHoldData->nLevelNum = 1; // 低液位
			}
		}
		else
		{
			if (pMbHoldData->RW45 == 0) // 量程选择
			{
				pMbHoldData->nLevelNum = 2; // 液位
			}
			else
			{
				pMbHoldData->nLevelNum = 1; // 液位
			}
		}
		// 命令13：
		// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		// pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
		// // pMbHoldData->nLevelNum = 1;					//液位
		// pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		if(pMbHoldData->nLevelNum == 2) // 高液位4ml
		{
			tmp_ml = 4+0.7;
		}
		else
		{
			if(TP_flag <= 2)
			{
				tmp_ml = 1+0.7;
			}
			else if(TP_flag == 3)
			{
				tmp_ml = 0.5+0.7;
			}
			else
			{
				tmp_ml = 2+0.7;
			}
		}
		RUNZE_SET_ML(tmp_ml);	//0~6

		if (GetWaterState())	//有液体返回0.缺液体返回1) // 718行，报警
		{
			RUNZE_SET_ML(6);	//0~6
			CloseAllValves(); // 关组合阀
			j++;
			if (j >= 3) // 报警次数
			{
				pMbHoldData->RW4230 = 3; // 故障代码=3
				return 1;
			}
		}
		else
		{
			// 增加多跑几圈
			// Pump_run(100, 20); // 命令12，操作蠕动泵，速度，圈数

			// // 下压定位，命令14
			// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			// pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
			// // pMbHoldData->nLevelNum = 1;					//液位
			// pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);

			// // 氨氮不用上压定位
			// if (shangya_flag)
			// {
			// 	// 增加上压定位步骤。
			// 	// 液位下行10圈
			// 	Pump_run(-100, 10); // 命令12，操作蠕动泵，速度，圈数
			// 	// 上行定位，命令15
			// 	pMbHoldData->nPeristalticPumpParm = 200; // 圈数
			// 	pMbHoldData->nStepperMotorSpeed = 30;	 // 速度
			// 	// pMbHoldData->nLevelNum = 1;					//液位
			// 	pMbInputData->nSysErrorCode = DeterminLevelUp(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			// }
			CloseAllValves(); // 关组合阀
			// if (pMbInputData->nSysErrorCode)
			// {
			// 	OpenValves(0x0001); // 开清洗废液阀
			// 	if (pMbHoldData->RW577)
			// 	{
			// 		OpenValves(0x1000); // 开废液阀
			// 	}
			// 	Pump_run(-800, 300); // 操作蠕动泵，速度，圈数
			// 	k++;
			// 	if (k >= 3)
			// 	{
			// 		pMbHoldData->RW4230 = 22; // 故障代码=22
			// 		return 1;
			// 	}
			// }
			// else
			{
				break;
			}
		}
	}

	CloseAllValves();	  // 关组合阀
	RUNZE_SETHOLE(1);	//1~10
	RUNZE_SET_ML(tmp_ml-0.1);	//0~6

	CloseAllValves();	  // 关组合阀
	OpenValves(0x0308);	  // 开比色阀，消解上+下阀
	RUNZE_SETHOLE(3);	//1~10
	RUNZE_SET_ML(0.6);	//0~6
	// Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0308);  // 关比色阀，消解上+下阀

	// // 增压
	// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
	// OpenValves(0x0001);	  // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	OpenValves(0x1000); // 开废液阀
	// }
	// // 泄压
	// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数

	CloseAllValves();					  // 关组合阀
	RUNZE_SETHOLE(1);	//1~10
	RUNZE_SET_ML(0);	//0~6
	if (pMbInputData->nSysErrorCode == 3) // 采样后清除未采到样报警
	{
		pMbInputData->nSysErrorCode = 0;
	}
	return 0;
}

uint16_t HCCaiyang(u16 shangya_flag, u16 TP_flag) // 采样
{	//TP_flag:	1总磷1ml，2总氮1ml，	3氨氮0.5ml，	4COD2ml，5CODMN2ml
	uint16_t i = 0, j = 0, k = 0;
	float tmp_ml = 0;
	pMbInputData->nStepCode = 8;
	osDelay(500);
	HC_SET_SPEED(200);
	while (1)
	{
		// if (Caiqianpd()) // 采前判断
		// {
		// 	return 1;
		// }
		CloseAllValves();	   // 关组合阀
		HC_SETHOLE(10);	//1~10，先采空气1ml
		HC_SET_ML(0.2);	//0~6
		osDelay(1000);
		i = pMbHoldData->RW50; // 工作类型
		if (i == 9 || i == 20 || i == 22 || i == 25 || i == 26 || i == 27 || i == 28 || i == 30 || i == 32 || i == 35 || i == 38 || i == 43)
		{
			OpenValves(0x0080); // 开量程阀
			if (pMbHoldData->RW576)
			{
				OpenValves(0x0002); // 开水样阀
			}
			HC_SETHOLE(7);	//1~10
		}
		else if (i == 2 || i == 4 || i == 34 || i == 37 || i == 39 || i == 40 || i == 44 || i == 45)
		{
			OpenValves(0x2000); // 核查样阀
			HC_SETHOLE(6);	//1~10
		}
		else if (i == 1 || i == 5 || i == 6 || i == 7 || i == 41)
		{
			OpenValves(0x0002); // 开水样阀
			HC_SETHOLE(5);	//1~10
		}
		else if (i == 3 || i == 8 || i == 19 || i == 21 || i == 24 || i == 31 || i == 33 || i == 36 || i == 42)
		{
			OpenValves(0x0010); // 蒸馏水阀
			HC_SETHOLE(4);	//1~10
		}
		else if(i == 23)	//维护测试
		{
			switch(pMbHoldData->RW44)
			{
				case 0:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					HC_SETHOLE(7);	//1~10
					break;
				case 1:
					OpenValves(0x2000); // 核查样阀
					HC_SETHOLE(6);	//1~10
					break;
				case 2:
					OpenValves(0x0002); // 开水样阀
					HC_SETHOLE(5);	//1~10
					break;
				case 3:
					OpenValves(0x0010); // 蒸馏水阀
					HC_SETHOLE(4);	//1~10
					break;
				default:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					HC_SETHOLE(7);	//1~10
					break;
			}
		}
		else
		{
			OpenValves(0x0002); // 开水样阀
			HC_SETHOLE(5);	//1~10
		}
		if (TP_flag == 1) // 总磷
		{
			if (pMbHoldData->RW45 == 0 || pMbHoldData->RW45 == 1) // 量程选择
			{
				pMbHoldData->nLevelNum = 2; // 高液位
			}
			else
			{
				pMbHoldData->nLevelNum = 1; // 低液位
			}
		}
		else
		{
			if (pMbHoldData->RW45 == 0) // 量程选择
			{
				pMbHoldData->nLevelNum = 2; // 液位
			}
			else
			{
				pMbHoldData->nLevelNum = 1; // 液位
			}
		}
		// 命令13：
		// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		// pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
		// // pMbHoldData->nLevelNum = 1;					//液位
		// pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		if(pMbHoldData->nLevelNum == 2) // 高液位4ml
		{
			tmp_ml = 4+0.7;
		}
		else
		{
			if(TP_flag <= 2)
			{
				tmp_ml = 1+0.7;
			}
			else if(TP_flag == 3)
			{
				tmp_ml = 0.5+0.7;
			}
			else
			{
				tmp_ml = 2+0.7;
			}
		}
		HC_SET_ML(tmp_ml);	//0~6

		if (GetWaterState())	//有液体返回0.缺液体返回1) // 718行，报警
		{
			HC_SET_ML(5);	//0~6
			CloseAllValves(); // 关组合阀
			j++;
			if (j >= 3) // 报警次数
			{
				pMbHoldData->RW4230 = 3; // 故障代码=3
				return 1;
			}
		}
		else
		{
			// 增加多跑几圈
			// Pump_run(100, 20); // 命令12，操作蠕动泵，速度，圈数

			// // 下压定位，命令14
			// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			// pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
			// // pMbHoldData->nLevelNum = 1;					//液位
			// pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);

			// // 氨氮不用上压定位
			// if (shangya_flag)
			// {
			// 	// 增加上压定位步骤。
			// 	// 液位下行10圈
			// 	Pump_run(-100, 10); // 命令12，操作蠕动泵，速度，圈数
			// 	// 上行定位，命令15
			// 	pMbHoldData->nPeristalticPumpParm = 200; // 圈数
			// 	pMbHoldData->nStepperMotorSpeed = 30;	 // 速度
			// 	// pMbHoldData->nLevelNum = 1;					//液位
			// 	pMbInputData->nSysErrorCode = DeterminLevelUp(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			// }
			CloseAllValves(); // 关组合阀
			// if (pMbInputData->nSysErrorCode)
			// {
			// 	OpenValves(0x0001); // 开清洗废液阀
			// 	if (pMbHoldData->RW577)
			// 	{
			// 		OpenValves(0x1000); // 开废液阀
			// 	}
			// 	Pump_run(-800, 300); // 操作蠕动泵，速度，圈数
			// 	k++;
			// 	if (k >= 3)
			// 	{
			// 		pMbHoldData->RW4230 = 22; // 故障代码=22
			// 		return 1;
			// 	}
			// }
			// else
			{
				break;
			}
		}
	}

	CloseAllValves();	  // 关组合阀
	HC_SETHOLE(1);	//1~10
	HC_SET_ML(tmp_ml-0.1);	//0~6


	CloseAllValves();	  // 关组合阀
	OpenValves(0x0308);	  // 开比色阀，消解上+下阀
	HC_SETHOLE(3);	//1~10
	HC_SET_ML(0.6);	//0~6
	// Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0308);  // 关比色阀，消解上+下阀

	// // 增压
	// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
	// OpenValves(0x0001);	  // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	OpenValves(0x1000); // 开废液阀
	// }
	// // 泄压
	// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数

	CloseAllValves();					  // 关组合阀
	HC_SETHOLE(1);	//1~10
	HC_SET_ML(0);	//0~6
	if (pMbInputData->nSysErrorCode == 3) // 采样后清除未采到样报警
	{
		pMbInputData->nSysErrorCode = 0;
	}
	return 0;
}

uint16_t Caiyang_CODMN(u16 shangya_flag) // 采样
{
	uint16_t i = 0, j = 0, k = 0;
	u16 count = 0;
	pMbInputData->nStepCode = 8;
	osDelay(500);

	if (pMbHoldData->RW516 > 5) // 范围检查
	{
		pMbHoldData->RW516 = 5;
	}
	if (pMbHoldData->RW45 == 1 && pMbHoldData->RW516 < 1)
	{
		pMbHoldData->RW516 = 1;
	}

	for (count = 0; count < 5; count++) // 高锰采5杯
	{
		while (1)
		{
			if (Caiqianpd()) // 采前判断
			{
				return 1;
			}
			CloseAllValves();	   // 关组合阀
			i = pMbHoldData->RW50; // 工作类型
			if (i == 9 || i == 20 || i == 22 || i == 25 || i == 26 || i == 27 || i == 28 || i == 30 || i == 32 || i == 35 || i == 38 || i == 43)
			{
				if (pMbHoldData->RW45 == 0) // 量程1，采5杯样
				{
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
				}
				else // 量程2，采设置杯数样，再采5-设置杯数的蒸馏水
				{
					if (count < pMbHoldData->RW516)
					{
						OpenValves(0x0080); // 开量程阀
						if (pMbHoldData->RW576)
						{
							OpenValves(0x0002); // 开水样阀
						}
					}
					else
					{
						OpenValves(0x0010); // 蒸馏水阀
					}
				}
			}
			else if (i == 2 || i == 4 || i == 34 || i == 37 || i == 39 || i == 40 || i == 44 || i == 45)
			{
				if (pMbHoldData->RW45 == 0) // 量程1，采5杯样
				{
					OpenValves(0x2000); // 核查样阀
				}
				else // 量程2，采设置杯数样，再采5-设置杯数的蒸馏水
				{
					if (count < pMbHoldData->RW516)
					{
						OpenValves(0x2000); // 核查样阀
					}
					else
					{
						OpenValves(0x0010); // 蒸馏水阀
					}
				}
			}
			else if (i == 1 || i == 5 || i == 6 || i == 7 || i == 41)
			{
				if (pMbHoldData->RW45 == 0) // 量程1，采5杯样
				{
					OpenValves(0x0002); // 开水样阀
				}
				else // 量程2，采设置杯数样，再采5-设置杯数的蒸馏水
				{
					if (count < pMbHoldData->RW516)
					{
						OpenValves(0x0002); // 开水样阀
					}
					else
					{
						OpenValves(0x0010); // 蒸馏水阀
					}
				}
			}
			else if (i == 3 || i == 8 || i == 19 || i == 21 || i == 24 || i == 31 || i == 33 || i == 36 || i == 42)
			{
				OpenValves(0x0010); // 蒸馏水阀
			}
			else if(i == 23)	//维护测试
			{
				if (pMbHoldData->RW45 == 0) // 量程1，采5杯样
				{
					switch(pMbHoldData->RW44)
					{
						case 0:
							OpenValves(0x0080); // 开量程阀
							if (pMbHoldData->RW576)
							{
								OpenValves(0x0002); // 开水样阀
							}
							break;
						case 1:
							OpenValves(0x2000); // 核查样阀
							break;
						case 2:
							OpenValves(0x0002); // 开水样阀
							break;
						case 3:
							OpenValves(0x0010); // 蒸馏水阀
							break;
						default:
							OpenValves(0x0080); // 开量程阀
							if (pMbHoldData->RW576)
							{
								OpenValves(0x0002); // 开水样阀
							}
							break;
					}
				}
				else // 量程2，采设置杯数样，再采5-设置杯数的蒸馏水
				{
					if (count < pMbHoldData->RW516)
					{
						switch(pMbHoldData->RW44)
						{
							case 0:
								OpenValves(0x0080); // 开量程阀
								if (pMbHoldData->RW576)
								{
									OpenValves(0x0002); // 开水样阀
								}
								break;
							case 1:
								OpenValves(0x2000); // 核查样阀
								break;
							case 2:
								OpenValves(0x0002); // 开水样阀
								break;
							case 3:
								OpenValves(0x0010); // 蒸馏水阀
								break;
							default:
								OpenValves(0x0080); // 开量程阀
								if (pMbHoldData->RW576)
								{
									OpenValves(0x0002); // 开水样阀
								}
								break;
						}
					}
					else
					{
						OpenValves(0x0010); // 蒸馏水阀
					}
				}
			}
			else
			{
				OpenValves(0x0002); // 开水样阀
			}

			pMbHoldData->nLevelNum = 2; // 液位

			// 命令13：
			pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			pMbHoldData->nStepperMotorSpeed = 1200;	 // 速度
			// pMbHoldData->nLevelNum = 1;					//液位
			pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			if (pMbInputData->nSysErrorCode) // 报警
			{
				CloseAllValves(); // 关组合阀
				j++;
				if (j >= 3) // 报警次数
				{
					pMbHoldData->RW4230 = 3; // 故障代码=3
					return 1;
				}
			}
			else
			{
				// 增加多跑几圈，高锰改为10圈
				Pump_run(100, 10); // 命令12，操作蠕动泵，速度，圈数

				// 下压定位，命令14
				pMbHoldData->nPeristalticPumpParm = 400; // 圈数
				pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
				// pMbHoldData->nLevelNum = 1;					//液位
				pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);

				// 氨氮不用上压定位
				if (shangya_flag)
				{
					// 增加上压定位步骤。
					// 液位下行10圈
					Pump_run(-100, 10); // 命令12，操作蠕动泵，速度，圈数
					// 上行定位，命令15
					pMbHoldData->nPeristalticPumpParm = 200; // 圈数
					pMbHoldData->nStepperMotorSpeed = 30;	 // 速度
					// pMbHoldData->nLevelNum = 1;					//液位
					pMbInputData->nSysErrorCode = DeterminLevelUp(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
				}
				CloseAllValves(); // 关组合阀
				if (pMbInputData->nSysErrorCode)
				{
					OpenValves(0x0001); // 开清洗废液阀
					if (pMbHoldData->RW577)
					{
						OpenValves(0x1000); // 开废液阀
					}
					Pump_run(-1500, 300); // 操作蠕动泵，速度，圈数
					k++;
					if (k >= 3)
					{
						pMbHoldData->RW4230 = 22; // 故障代码=22
						return 1;
					}
				}
				else
				{
					CloseAllValves();	  // 关组合阀
					OpenValves(0x0308);	  // 开比色阀，消解上+下阀
					Pump_run(-1500, 300); // 命令12，操作蠕动泵，速度，圈数
					CloseValves(0x0308);  // 关比色阀，消解上+下阀
					break;
				}
			}
		}
	}
	// 增压
	Pump_run(-1500, 100); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	// 泄压
	Pump_run(-1500, 100); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();	  // 关组合阀

	CloseAllValves();					  // 关组合阀
	if (pMbInputData->nSysErrorCode == 3) // 采样后清除未采到样报警
	{
		pMbInputData->nSysErrorCode = 0;
	}
	return 0;
}

void Fanhui(void) // 水样/非水样返回
{
	uint16_t i = 0, j = 0;
	i = pMbHoldData->RW50; // 工作类型
	if (i == 9 || i == 20 || i == 22 || i == 25 || i == 26 || i == 27 || i == 28 || i == 30 || i == 32 || i == 35 || i == 38 || i == 43)
	{
		j = pMbHoldData->RW48; // 非水样返回圈数
		if (j)
		{
			OpenValves(0x0080); // 开量程阀
			if (pMbHoldData->RW576)
			{
				OpenValves(0x0002); // 开水样阀
			}
		}
	}
	else if (i == 2 || i == 4 || i == 34 || i == 37 || i == 39 || i == 40 || i == 44 || i == 45)
	{
		j = pMbHoldData->RW48; // 非水样返回圈数
		if (j)
		{
			OpenValves(0x2000); // 核查样阀
		}
	}
	else if (i == 1 || i == 5 || i == 6 || i == 7 || i == 41)
	{
		j = pMbHoldData->RW47; // 水样返回圈数
		if (j)
		{
			OpenValves(0x0002); // 开水样阀
		}
	}
	else if (i == 3 || i == 8 || i == 19 || i == 21 || i == 24 || i == 31 || i == 33 || i == 36 || i == 42)
	{
		j = 0;
		// OpenValves(0x0010); // 蒸馏水阀
	}
	else if(i == 23)	//维护测试
	{
		j = pMbHoldData->RW48; // 非水样返回圈数
		if (j)
		{
			switch(pMbHoldData->RW44)
			{
				case 0:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					break;
				case 1:
					OpenValves(0x2000); // 核查样阀
					break;
				case 2:
					OpenValves(0x0002); // 开水样阀
					break;
				case 3:
					OpenValves(0x0010); // 蒸馏水阀
					break;
				default:
					OpenValves(0x0080); // 开量程阀
					if (pMbHoldData->RW576)
					{
						OpenValves(0x0002); // 开水样阀
					}
					break;
			}
		}
	}
	else
	{
		j = pMbHoldData->RW47; // 水样返回圈数
		if (j)
		{
			OpenValves(0x0002); // 开水样阀
		}
	}
	if (j)
	{
		Pump_run(-1500, j); // 命令12，操作蠕动泵，速度，圈数
	}
	CloseAllValves(); // 173行，关组合阀
}



u16 Caidingbsm(u16 valves, int speedc, int speedd, u16 weizhi, u16 quanshu, u16 add_quanshu) // 采定液体到比色皿
{
	u16 i = 0, j = 0;
	CloseAllValves(); // 关组合阀
	while (1)
	{
		if (Caiqianpd()) // 采前判断
		{
			return 1;
		}
		OpenValves(valves); // 开阀
		// 命令13
		pMbHoldData->nPeristalticPumpParm = 400;  // 圈数
		pMbHoldData->nStepperMotorSpeed = speedc; // 速度
		pMbHoldData->nLevelNum = weizhi;		  // 液位
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		if (pMbInputData->nSysErrorCode)
		{
			CloseAllValves(); // 关组合阀
			i++;
			if (i >= 3)
			{
				switch (pMbInputData->nStepCode) // 运行日志
				{
				case 9:
					pMbHoldData->RW4230 = 4;
					break;
				case 10:
					pMbHoldData->RW4230 = 8;
					break;
				case 11:
					pMbHoldData->RW4230 = 12;
					break;
				case 12:
					pMbHoldData->RW4230 = 14;
					break;
				case 14:
					pMbHoldData->RW4230 = 16;
					break;
				default:
					pMbHoldData->RW4230 = 4;
					break;
				}
				return 1;
			}
		}
		else
		{
			if (add_quanshu)
			{
				// 增加多跑几圈
				Pump_run(100, add_quanshu); // 命令12，操作蠕动泵，速度，圈数
			}
			// 下行判定液位，命令14
			pMbHoldData->nPeristalticPumpParm = 200;  // 圈数
			pMbHoldData->nStepperMotorSpeed = speedd; // 速度
			pMbHoldData->nLevelNum = weizhi;		  // 液位
			pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			CloseAllValves();
			if (pMbInputData->nSysErrorCode)
			{
				j++;
				if (j >= 3)
				{
					switch (pMbInputData->nStepCode) // 运行日志
					{
					case 9:
						pMbHoldData->RW4230 = 5;
						break;
					case 10:
						pMbHoldData->RW4230 = 9;
						break;
					case 11:
						pMbHoldData->RW4230 = 13;
						break;
					case 12:
						pMbHoldData->RW4230 = 15;
						break;
					case 14:
						pMbHoldData->RW4230 = 17;
						break;
					default:
						pMbHoldData->RW4230 = 5;
						break;
					}
					return 1;
				}
			}
			else
			{
				break;
			}
		}
	}
	// 推至比色皿
	CloseAllValves();	  // 关组合阀
	OpenValves(0x0308);	  // 开比色阀，消解上+下阀
	Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();	  // 关组合阀
	// 增压
	Pump_run(-1500, 100); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	// 泻压
	Pump_run(-1500, 100); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();	  // 关组合阀
	return 0;
}

u16 RZCaidingbsm(u8 num,float ml,u8 flag) // 采定液体到比色皿
{	//COD试剂二硫酸flag=1
	u16 i = 0, j = 0;
	RUNZE_SET_SPEED(60);
	while(1)
	{
		CloseAllValves(); // 关组合阀
		RUNZE_SETHOLE(1);	//1~10
		RUNZE_SET_ML(0);
		if(flag == 0)	//正常的采样
		{
			RUNZE_SETHOLE(10);	//1~10，先采空气0.2ml
			RUNZE_SET_ML(0.2);	//0~6
			osDelay(1000);
			
			RUNZE_SETHOLE(num);	//1~10
			RUNZE_SET_ML(ml+0.7);	//0~6
			osDelay(20000);	//COD采试剂二硫酸需要更长延时
			if (GetWaterState())	//有液体返回0.缺液体返回1) // 718行，报警
			{
				RUNZE_SET_ML(6);	//0~6
				j++;
				if (j >= 3) // 报警次数
				{
					return 1;
				}
			}
			else
			{
				RUNZE_SETHOLE(1);	//1~10
				RUNZE_SET_ML(ml+0.6);

				RUNZE_SETHOLE(3);	//1~10
				OpenValves(0x0308);	  // 开比色阀，消解上+下阀
				RUNZE_SET_ML(0.6);

				osDelay(20000);//COD采试剂二硫酸需要更长延时
				CloseAllValves();	  // 关组合阀
				RUNZE_SETHOLE(1);	//1~10
				RUNZE_SET_ML(0);
				break;
			}
		}
		else if(flag == 1)	//COD试剂二硫酸特殊处理
		{
			RUNZE_SETHOLE(10);	//1~10，先采空气0.2ml
			RUNZE_SET_ML(0.2);	//0~6
			osDelay(1000);
			
			RUNZE_SETHOLE(num);	//1~10
			RUNZE_SET_ML(ml+1);	//0~6
			osDelay(20000);	//COD采试剂二硫酸需要更长延时
			if (GetWaterState())	//有液体返回0.缺液体返回1) // 718行，报警
			{
				RUNZE_SET_ML(6);	//0~6
				j++;
				if (j >= 3) // 报警次数
				{
					return 1;
				}
			}
			else
			{
				RUNZE_SETHOLE(1);	//1~10
				RUNZE_SET_ML(ml+0.9);

				RUNZE_SETHOLE(3);	//1~10
				OpenValves(0x0308);	  // 开比色阀，消解上+下阀
				RUNZE_SET_ML(0.9);

				osDelay(20000);//COD采试剂二硫酸需要更长延时
				CloseAllValves();	  // 关组合阀
				RUNZE_SETHOLE(1);	//1~10
				RUNZE_SET_ML(0);
				break;
			}
		}
	}
	//排空
	// RUNZE_SET_SPEED(100);	//1~500

	RUNZE_SETHOLE(10);	//1~10
	RUNZE_SET_ML(5);	//0~6
	
	RUNZE_SETHOLE(3);	//1~10
	OpenValves(0x0308);	  // 开比色阀，消解上+下阀
	RUNZE_SET_ML(0.5);	//0~6

	RUNZE_SETHOLE(1);	//1~10
	RUNZE_SET_ML(0);	//0~6
	CloseAllValves();

	return 0;
}


u16 HCCaidingbsm(u8 num,float ml,u8 flag) // 采定液体到比色皿
{	//COD试剂二硫酸flag=1
	u16 i = 0, j = 0;
	HC_SET_SPEED(100);
	while(1)
	{
		CloseAllValves(); // 关组合阀
		HC_SETHOLE(1);	//1~10
		HC_SET_ML(0);
		if(flag == 0)	//正常的采样
		{
			HC_SETHOLE(10);	//1~10，先采空气0.2ml
			HC_SET_ML(0.2);	//0~6
			osDelay(1000);

			HC_SETHOLE(num);	//1~10
			HC_SET_ML(ml+0.7);	//0~6
			osDelay(20000);
			if (GetWaterState())	//有液体返回0.缺液体返回1) // 718行，报警
			{
				HC_SET_ML(5);	//0~6
				j++;
				if (j >= 3) // 报警次数
				{
					return 1;
				}
			}
			else
			{
				HC_SETHOLE(1);	//1~10
				HC_SET_ML(ml+0.6);

				HC_SETHOLE(3);	//1~10
				OpenValves(0x0308);	  // 开比色阀，消解上+下阀
				HC_SET_ML(0.6);

				osDelay(20000);
				CloseAllValves();	  // 关组合阀
				HC_SETHOLE(1);	//1~10
				HC_SET_ML(0);

				break;
			}
		}
		else if(flag == 1)	//COD试剂二硫酸特殊处理
		{
			HC_SETHOLE(10);	//1~10，先采空气1ml
			HC_SET_ML(0.2);	//0~6
			osDelay(1000);
			
			HC_SETHOLE(num);	//1~10
			HC_SET_ML(ml+1);	//0~6
			osDelay(20000);	//COD采试剂二硫酸需要更长延时
			if (GetWaterState())	//有液体返回0.缺液体返回1) // 718行，报警
			{
				HC_SET_ML(5);	//0~6
				j++;
				if (j >= 3) // 报警次数
				{
					return 1;
				}
			}
			else
			{
				HC_SETHOLE(1);	//1~10
				HC_SET_ML(ml+0.9);


				HC_SETHOLE(3);	//1~10
				OpenValves(0x0308);	  // 开比色阀，消解上+下阀
				HC_SET_ML(0.9);

				osDelay(20000);//COD采试剂二硫酸需要更长延时
				CloseAllValves();	  // 关组合阀
				HC_SETHOLE(1);	//1~10
				HC_SET_ML(0);
				break;
			}
		}
	}
	//排空
	HC_SET_SPEED(200);	//1~500

	HC_SETHOLE(10);	//1~10
	HC_SET_ML(4.5);	//0~6
	
	HC_SETHOLE(3);	//1~10
	OpenValves(0x0308);	  // 开比色阀，消解上+下阀
	HC_SET_ML(0.5);	//0~6

	HC_SETHOLE(1);	//1~10
	HC_SET_ML(0);	//0~6
	CloseAllValves();

	return 0;
}

u16 Chouyetibdw(u16 valves, int speedc, u16 weizhi, u16 quanshu) // 抽液体不定位
{
	u16 i = 0;
	while (1)
	{
		if (Caiqianpd()) // 采前判断
		{
			return 1;
		}
		OpenValves(valves); // 开阀
		// 命令13
		pMbHoldData->nPeristalticPumpParm = 400;  // 圈数
		pMbHoldData->nStepperMotorSpeed = speedc; // 速度
		pMbHoldData->nLevelNum = weizhi;		  // 液位
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves(); // 关组合阀
		if (pMbInputData->nSysErrorCode)
		{
			i++;
			if (i >= 3)
			{
				switch (pMbInputData->nStepCode) // 运行日志
				{
				case 9:
					pMbHoldData->RW4230 = 6;
					break;
				case 10:
					pMbHoldData->RW4230 = 10;
					break;
				default:
					pMbHoldData->RW4230 = 6;
					break;
				}
				return 1;
			}
		}
		else
		{
			break;
		}
	}

	OpenValves(0x0001); // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, quanshu); // 命令12，操作蠕动泵，速度，圈数

	CloseAllValves(); // 关组合阀
	// 增压
	Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	// 泻压
	Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数

	CloseAllValves(); // 关组合阀

	return 0;
}

u16 RZChouyetibdw(u8 num,float ml) // 抽液体不定位
{
	u16 i = 0;
	RUNZE_SET_SPEED(100);
	RUNZE_SETHOLE(1);	//1~10
	RUNZE_SET_ML(0);	//0~6

	RUNZE_SETHOLE(10);	//1~10，先采空气1ml
	RUNZE_SET_ML(1);	//0~6
	osDelay(1000);

	if(num == 3)
	{
		OpenValves(0x0308);	  // 开比色阀，消解上+下阀
	}
	RUNZE_SETHOLE(num);	//1~10
	RUNZE_SET_SPEED(100);
	RUNZE_SET_ML(ml+1);	//0~6

	OpenValves(0x0001); // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	// Pump_run(-1200, quanshu); // 命令12，操作蠕动泵，速度，圈数
	RUNZE_SET_SPEED(200);
	RUNZE_SET_ML(0);	//0~6
	CloseAllValves(); // 关组合阀

	return 0;
}

u16 HCChouyetibdw(u8 num,float ml) // 抽液体不定位
{
	u16 i = 0;
	RUNZE_SET_SPEED(100);
	RUNZE_SETHOLE(1);	//1~10
	RUNZE_SET_ML(0);	//0~6

	RUNZE_SETHOLE(10);	//1~10，先采空气1ml
	RUNZE_SET_ML(1);	//0~6
	osDelay(1000);

	if(num == 3)
	{
		OpenValves(0x0308);	  // 开比色阀，消解上+下阀
	}
	RUNZE_SETHOLE(num);	//1~10
	RUNZE_SET_SPEED(100);
	RUNZE_SET_ML(ml+1);	//0~6

	OpenValves(0x0001); // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	// Pump_run(-1200, quanshu); // 命令12，操作蠕动泵，速度，圈数
	RUNZE_SET_SPEED(200);
	RUNZE_SET_ML(0);	//0~6
	CloseAllValves(); // 关组合阀

	return 0;
}

u16 ChouWaterbdwbsm(void) // 抽蒸馏水不定位到比色皿
{
	u16 i = 0;
	CloseAllValves(); // 关组合阀
	while (1)
	{
		if (Caiqianpd()) // 采前判断
		{
			return 1;
		}
		OpenValves(0x0010); // 蒸馏水阀
		osDelay(500);
		// 命令13
		pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		pMbHoldData->nStepperMotorSpeed = 1300;	 // 速度
		pMbHoldData->nLevelNum = 2;				 // 液位
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		osDelay(500);
		if (pMbInputData->nSysErrorCode)
		{
			CloseAllValves(); // 关组合阀
			i++;
			if (i >= 3)
			{
				pMbHoldData->RW4230 = 18;
				return 1;
			}
		}
		else
		{
			// 推至比色皿
			CloseAllValves();	  // 关组合阀
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1500, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			break;
		}
	}
	return 0;
}

u16 Xishi_NH3N(void) // 稀释
{
	uint16_t i = 0, j = 0;
	switch (pMbHoldData->RW45)
	{
	case 0:
	case 1:
		i = 1;
		break;
	case 2:
		i = 2;
		break;
	case 3:
		i = 3;
		break;
	default:
		i = 1;
		break;
	}
	if (i == 2 || i == 3)
	{
		pMbInputData->nStepCode = 9; // 1次稀释
		osDelay(500);
		// 896,采4ml蒸馏水
		j = Caidingbsm(0x0010, 600, -40, 2, 400, 20); // 采定液体到比色皿
		if (j)
		{
			return 1;
		}
		// 924,从比色皿采低液位丢掉
		j = Chouyetibdw(0x0308, 600, 1, 400); // 抽液体不定位
		if (j)
		{
			return 1;
		}
		// 942,再从比色皿采低液位丢掉
		j = Chouyetibdw(0x0308, 600, 1, 400); // 抽液体不定位
		if (j)
		{
			return 1;
		}

		// 960，抽比色皿
		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		Pump_run(800, 200);	 // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0308); // 关比色阀，消解上+下阀
		osDelay(2000);
		OpenValves(0x0001); // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		osDelay(10000);

		// 1014，计量管下行定位2ml
		// 下行判定液位，命令14
		pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		pMbHoldData->nLevelNum = 1;				 // 液位
		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		osDelay(500);
		while (1)
		{
			osDelay(500);
			pMbInputData->nStepCode = 34; // SetWord(@运行日志@
			osDelay(500);
			// 1034行
			if (pMbInputData->nLL1AD > (pMbHoldData->RW30 * pMbHoldData->nBlankPipeUpTHR / 1000)) // 气泡干扰
			{
				// 下行判定液位，命令14
				pMbHoldData->nPeristalticPumpParm = 200; // 圈数
				pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
				pMbHoldData->nLevelNum = 1;				 // 液位
				pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
				osDelay(500);
				continue;
			}
			else
			{
				break;
			}
		}
		osDelay(1000);
		if (pMbInputData->nSysErrorCode)
		{
			if (pMbInputData->nStepCode == 9)
			{
				pMbHoldData->RW4230 = 7;
				return 1;
			}
		}
		else
		{
			CloseAllValves(); // 关组合阀
			osDelay(1000);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0308);  // 关比色阀，消解上+下阀
			// 增压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			OpenValves(0x0001);	  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			// 泻压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
		}
	}

	if (i == 3) // 1187,量程4，再次稀释
	{
		pMbInputData->nStepCode = 10; // 2次稀释
		osDelay(500);
		// 采1ml蒸馏水
		j = Caidingbsm(0x0010, 600, -40, 1, 400, 20); // 采定液体到比色皿
		// 抽比色皿
		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		Pump_run(800, 200);	 // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0308); // 关比色阀，消解上+下阀
		osDelay(2000);
		OpenValves(0x0001); // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		osDelay(10000);

		// 1274,计量管下行定位1ml
		// 下行判定液位，命令14
		pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		pMbHoldData->nLevelNum = 1;				 // 液位
		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		osDelay(500);
		while (1)
		{
			osDelay(500);
			pMbInputData->nStepCode = 34; // SetWord(@运行日志@
			osDelay(500);
			// 1296行
			if (pMbInputData->nLL1AD > (pMbHoldData->RW30 * pMbHoldData->nBlankPipeUpTHR / 1000)) // 气泡干扰
			{
				// 下行判定液位，命令14
				pMbHoldData->nPeristalticPumpParm = 200; // 圈数
				pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
				pMbHoldData->nLevelNum = 1;				 // 液位
				pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
				osDelay(500);
				continue;
			}
			else
			{
				break;
			}
		}
		osDelay(1000);
		if (pMbInputData->nSysErrorCode)
		{
			if (pMbInputData->nStepCode == 9)
			{
				pMbHoldData->RW4230 = 7;
				return 1;
			}
		}
		else // 1333
		{
			CloseAllValves(); // 关组合阀
			osDelay(1000);
			// 1ml推回比色皿
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0308);  // 关比色阀，消解上+下阀
			// 增压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			OpenValves(0x0001);	  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			// 泻压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
		}
	}
	return 0;
}

u16 HCXishi_NH3N(void) // 稀释
{
	uint16_t i = 0, j = 0;
	switch (pMbHoldData->RW45)
	{
	case 0:
	case 1:
		i = 1;
		break;
	case 2:
		i = 2;
		break;
	case 3:
		i = 3;
		break;
	default:
		i = 1;
		break;
	}
	if (i == 2 || i == 3)
	{
		pMbInputData->nStepCode = 9; // 1次稀释
		osDelay(500);
		// 896,采4ml蒸馏水
		j = HCCaidingbsm(4,4,0); // 采定液体到比色皿

		// j = Caidingbsm(0x0010, 600, -40, 2, 400, 20); // 采定液体到比色皿
		if (j)
		{
			pMbHoldData->RW4230 = 12; // 故障代码
			return 1;
		}
		// 924,从比色皿采低液位丢掉
		j = HCChouyetibdw(3,2); // 抽液体不定位
		if (j)
		{
			return 1;
		}
		// // 942,再从比色皿采低液位丢掉
		// j = Chouyetibdw(0x0308, 600, 1, 400); // 抽液体不定位
		// if (j)
		// {
		// 	return 1;
		// }
		HC_SETHOLE(10);	//1~10，先采空气1ml
		HC_SET_ML(0.5);	//0~6
		osDelay(1000);

		// 960，抽比色皿
		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		HC_SETHOLE(3);	//1~10
		// Pump_run(800, 200);	 // 命令12，操作蠕动泵，速度，圈数
		HC_SET_SPEED(200);
		HC_SET_ML(5);	//0~6
		
		HC_SETHOLE(2);	//1~10
		OpenValves(0x0001); // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		// osDelay(10000);
		HC_SET_ML(1.5);	//0~6

		// 1014，计量管下行定位2ml
		// 下行判定液位，命令14
		// pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		// pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		// pMbHoldData->nLevelNum = 1;				 // 液位
		// pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		// osDelay(500);
		// while (1)
		// {
		// 	osDelay(500);
		// 	pMbInputData->nStepCode = 34; // SetWord(@运行日志@
		// 	osDelay(500);
		// 	// 1034行
		// 	if (pMbInputData->nLL1AD > (pMbHoldData->RW30 * pMbHoldData->nBlankPipeUpTHR / 1000)) // 气泡干扰
		// 	{
		// 		// 下行判定液位，命令14
		// 		pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		// 		pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		// 		pMbHoldData->nLevelNum = 1;				 // 液位
		// 		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		// 		osDelay(500);
		// 		continue;
		// 	}
		// 	else
		// 	{
		// 		break;
		// 	}
		// }
		// osDelay(1000);
		// if (pMbInputData->nSysErrorCode)
		// {
		// 	if (pMbInputData->nStepCode == 9)
		// 	{
		// 		pMbHoldData->RW4230 = 7;
		// 		return 1;
		// 	}
		// }
		// else
		{
			CloseAllValves(); // 关组合阀
			osDelay(1000);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			HC_SETHOLE(3);	//1~10
			// Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			HC_SET_ML(1);	//0~6
			CloseValves(0x0308);  // 关比色阀，消解上+下阀
			// 增压
			// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			// OpenValves(0x0001);	  // 开清洗废液阀
			// if (pMbHoldData->RW577)
			// {
			// 	OpenValves(0x1000); // 开废液阀
			// }
			// 泻压
			// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			HC_SET_ML(0);	//0~6
		}
	}

	if (i == 3) // 1187,量程4，再次稀释
	{
		pMbInputData->nStepCode = 10; // 2次稀释
		osDelay(500);
		// 采0.5ml蒸馏水
		j = HCCaidingbsm(4,0.5,0); // 采定液体到比色皿
		// j = Caidingbsm(0x0010, 600, -40, 1, 400, 20); // 采定液体到比色皿
		if (j)
		{
			pMbHoldData->RW4230 = 12; // 故障代码
			return 1;
		}

		//从比色皿采低液位丢掉
		j = HCChouyetibdw(3, 1.5); // 抽液体不定位
		if (j)
		{
			return 1;
		}

		// 抽比色皿
		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		HC_SETHOLE(3);	//1~10
		HC_SET_SPEED(200);
		HC_SET_ML(4);	//0~6
		// Pump_run(800, 200);	 // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0308); // 关比色阀，消解上+下阀
		osDelay(2000);
		OpenValves(0x0001); // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		HC_SETHOLE(2);	//1~10
		// osDelay(10000);
		HC_SET_ML(1);	//0~6

		// 1274,计量管下行定位1ml
		// 下行判定液位，命令14
		// pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		// pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		// pMbHoldData->nLevelNum = 1;				 // 液位
		// pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		// osDelay(500);
		// while (1)
		// {
		// 	osDelay(500);
		// 	pMbInputData->nStepCode = 34; // SetWord(@运行日志@
		// 	osDelay(500);
		// 	// 1296行
		// 	if (pMbInputData->nLL1AD > (pMbHoldData->RW30 * pMbHoldData->nBlankPipeUpTHR / 1000)) // 气泡干扰
		// 	{
		// 		// 下行判定液位，命令14
		// 		pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		// 		pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		// 		pMbHoldData->nLevelNum = 1;				 // 液位
		// 		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		// 		osDelay(500);
		// 		continue;
		// 	}
		// 	else
		// 	{
		// 		break;
		// 	}
		// }
		// osDelay(1000);
		// if (pMbInputData->nSysErrorCode)
		// {
		// 	if (pMbInputData->nStepCode == 9)
		// 	{
		// 		pMbHoldData->RW4230 = 7;
		// 		return 1;
		// 	}
		// }
		// else // 1333
		{
			CloseAllValves(); // 关组合阀
			osDelay(1000);
			// 1ml推回比色皿
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			HC_SETHOLE(3);	//1~10
			// Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			HC_SET_ML(0.5);	//0~6
			CloseValves(0x0308);  // 关比色阀，消解上+下阀
			// 增压
			// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			// OpenValves(0x0001);	  // 开清洗废液阀
			// if (pMbHoldData->RW577)
			// {
			// 	OpenValves(0x1000); // 开废液阀
			// }
			// 泻压
			// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			HC_SETHOLE(0);	//1~10
			HC_SET_ML(0);	//0~6
		}
	}
	return 0;
}

u16 Xishi_COD(void) // 921,稀释
{
	uint16_t i = 0, j = 0;
	switch (pMbHoldData->RW45)
	{
	case 0:
	case 1:
		i = 1;
		break;
	case 2:
		i = 2;
		break;
	default:
		i = 1;
		break;
	}
	if (i == 2)
	{
		pMbInputData->nStepCode = 9; // 1次稀释
		osDelay(500);
		// 896,采4ml蒸馏水
		j = Caidingbsm(0x0010, 600, -40, 2, 400, 20); // 采定液体到比色皿
		if (j)
		{
			return 1;
		}
		// 924,从比色皿采低液位丢掉
		j = Chouyetibdw(0x0308, 600, 1, 400); // 抽液体不定位
		if (j)
		{
			return 1;
		}
		// // 942,再从比色皿采低液位丢掉
		// j = Chouyetibdw(0x0308, 600, 1, 400); // 抽液体不定位
		// if (j)
		// {
		// 	return 1;
		// }

		// 960，抽比色皿
		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		Pump_run(800, 200);	 // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0308); // 关比色阀，消解上+下阀
		osDelay(2000);
		OpenValves(0x0001); // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		osDelay(10000);

		// 1014，计量管下行定位2ml
		// 下行判定液位，命令14
		pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		pMbHoldData->nLevelNum = 1;				 // 液位
		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		osDelay(500);
		while (1)
		{
			osDelay(500);
			pMbInputData->nStepCode = 34; // SetWord(@运行日志@
			osDelay(500);
			// 1034行
			if (pMbInputData->nLL1AD > (pMbHoldData->RW30 * pMbHoldData->nBlankPipeUpTHR / 1000)) // 气泡干扰
			{
				// 下行判定液位，命令14
				pMbHoldData->nPeristalticPumpParm = 200; // 圈数
				pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
				pMbHoldData->nLevelNum = 1;				 // 液位
				pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
				osDelay(500);
				continue;
			}
			else
			{
				break;
			}
		}
		osDelay(1000);
		if (pMbInputData->nSysErrorCode)
		{
			if (pMbInputData->nStepCode == 9)
			{
				pMbHoldData->RW4230 = 7;
				return 1;
			}
		}
		else
		{
			CloseAllValves(); // 关组合阀
			osDelay(1000);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0308);  // 关比色阀，消解上+下阀
			// 增压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			OpenValves(0x0001);	  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			// 泻压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
		}
	}
	return 0;
}

u16 RZXishi_COD(void) // 921,稀释
{
	uint16_t i = 0, j = 0;
	switch (pMbHoldData->RW45)
	{
	case 0:
	case 1:
		i = 1;
		break;
	case 2:
		i = 2;
		break;
	default:
		i = 1;
		break;
	}
	if (i == 2)
	{
		pMbInputData->nStepCode = 9; // 1次稀释
		osDelay(500);
		// 896,采4ml蒸馏水
		j = RZCaidingbsm(4,4,0); // 采定液体到比色皿
		if (j)
		{
			pMbHoldData->RW4230 = 12; // 故障代码
			return 1;
		}

		// 924,从比色皿采低液位丢掉
		j = RZChouyetibdw(3, 2); // 抽液体不定位
		if (j)
		{
			return 1;
		}

		
		// 960，抽比色皿
		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		RUNZE_SETHOLE(3);	//1~10
		// Pump_run(800, 200);	 // 命令12，操作蠕动泵，速度，圈数
		RUNZE_SET_SPEED(200);
		RUNZE_SET_ML(6);	//0~6

		CloseValves(0x0308); // 关比色阀，消解上+下阀
		// osDelay(2000);


		OpenValves(0x0001); // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		RUNZE_SETHOLE(2);	//1~10
		// osDelay(10000);
		// 1014，计量管下行定位2ml
		RUNZE_SET_ML(2.5);	//0~6
		// 下行判定液位，命令14
		// pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		// pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		// pMbHoldData->nLevelNum = 1;				 // 液位
		// pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		// osDelay(500);
		// while (1)
		// {
		// 	osDelay(500);
		// 	pMbInputData->nStepCode = 34; // SetWord(@运行日志@
		// 	osDelay(500);
		// 	// 1034行
		// 	if (pMbInputData->nLL1AD > (pMbHoldData->RW30 * pMbHoldData->nBlankPipeUpTHR / 1000)) // 气泡干扰
		// 	{
		// 		// 下行判定液位，命令14
		// 		pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		// 		pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		// 		pMbHoldData->nLevelNum = 1;				 // 液位
		// 		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		// 		osDelay(500);
		// 		continue;
		// 	}
		// 	else
		// 	{
		// 		break;
		// 	}
		// }
		osDelay(1000);
		// if (pMbInputData->nSysErrorCode)
		// {
		// 	if (pMbInputData->nStepCode == 9)
		// 	{
		// 		pMbHoldData->RW4230 = 7;
		// 		return 1;
		// 	}
		// }
		// else
		{
			CloseAllValves(); // 关组合阀
			osDelay(1000);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			RUNZE_SETHOLE(3);	//1~10
			// Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			RUNZE_SET_ML(0.5);	//0~6
			CloseValves(0x0308);  // 关比色阀，消解上+下阀
			// 增压
			// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			// OpenValves(0x0001);	  // 开清洗废液阀
			// if (pMbHoldData->RW577)
			// {
			// 	OpenValves(0x1000); // 开废液阀
			// }
			// // 泻压
			// Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			RUNZE_SETHOLE(0);	//1~10
			RUNZE_SET_ML(0);	//0~6
		}
	}
	return 0;
}

u16 Xishi_TN(void) //稀释
{
	uint16_t i = 0, j = 0;
	switch (pMbHoldData->RW45)
	{
	case 0:
	case 1:
		i = 1;
		break;
	case 2:		//量程3才稀释
		i = 2;
		break;
	default:
		i = 1;
		break;
	}
	if (i == 2)
	{
		pMbInputData->nStepCode = 9; // 1次稀释
		osDelay(500);
		// 896,采4ml蒸馏水
		j = Caidingbsm(0x0010, 600, -40, 2, 400, 20); // 采定液体到比色皿
		if (j)
		{
			return 1;
		}
		// 924,从比色皿采低液位丢掉
		j = Chouyetibdw(0x0308, 600, 1, 400); // 抽液体不定位
		if (j)
		{
			return 1;
		}

		// 960，抽比色皿
		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		Pump_run(800, 200);	 // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0308); // 关比色阀，消解上+下阀
		osDelay(2000);
		OpenValves(0x0001); // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		osDelay(10000);

		// 1014，计量管下行定位2ml
		// 下行判定液位，命令14
		pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		pMbHoldData->nLevelNum = 1;				 // 液位
		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		osDelay(500);
		while (1)
		{
			osDelay(500);
			pMbInputData->nStepCode = 34; // SetWord(@运行日志@
			osDelay(500);
			// 1034行
			if (pMbInputData->nLL1AD > (pMbHoldData->RW30 * pMbHoldData->nBlankPipeUpTHR / 1000)) // 气泡干扰
			{
				// 下行判定液位，命令14
				pMbHoldData->nPeristalticPumpParm = 200; // 圈数
				pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
				pMbHoldData->nLevelNum = 1;				 // 液位
				pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
				osDelay(500);
				continue;
			}
			else
			{
				break;
			}
		}
		osDelay(1000);
		if (pMbInputData->nSysErrorCode)
		{
			if (pMbInputData->nStepCode == 9)
			{
				pMbHoldData->RW4230 = 7;
				return 1;
			}
		}
		else
		{
			CloseAllValves(); // 关组合阀
			osDelay(1000);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0308);  // 关比色阀，消解上+下阀
			// 增压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			OpenValves(0x0001);	  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			// 泻压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
		}
	}
	return 0;
}

u16 Xishi_TP(void) // 921,稀释
{
	uint16_t i = 0, j = 0;
	if (pMbHoldData->RW45 == 3) // TP量程4才稀释
	{
		i = 2;
	}
	else
	{
		i = 1;
	}

	if (i == 2)
	{
		pMbInputData->nStepCode = 9; // 1次稀释
		osDelay(500);
		// 896,采4ml蒸馏水
		j = Caidingbsm(0x0010, 600, -40, 2, 400, 20); // 采定液体到比色皿
		if (j)
		{
			return 1;
		}
		// 924,从比色皿采低液位丢掉
		j = Chouyetibdw(0x0308, 600, 1, 400); // 抽液体不定位
		if (j)
		{
			return 1;
		}
		// 942,再从比色皿采低液位丢掉
		j = Chouyetibdw(0x0308, 600, 1, 400); // 抽液体不定位
		if (j)
		{
			return 1;
		}

		// 960，抽比色皿
		OpenValves(0x0308);	 // 开比色阀，消解上+下阀
		Pump_run(800, 200);	 // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0308); // 关比色阀，消解上+下阀
		osDelay(2000);
		OpenValves(0x0001); // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		osDelay(10000);

		// 1014，计量管下行定位2ml
		// 下行判定液位，命令14
		pMbHoldData->nPeristalticPumpParm = 200; // 圈数
		pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
		pMbHoldData->nLevelNum = 1;				 // 液位
		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		osDelay(500);
		while (1)
		{
			osDelay(500);
			pMbInputData->nStepCode = 34; // SetWord(@运行日志@
			osDelay(500);
			// 1034行
			if (pMbInputData->nLL1AD > (pMbHoldData->RW30 * pMbHoldData->nBlankPipeUpTHR / 1000)) // 气泡干扰
			{
				// 下行判定液位，命令14
				pMbHoldData->nPeristalticPumpParm = 200; // 圈数
				pMbHoldData->nStepperMotorSpeed = -40;	 // 速度
				pMbHoldData->nLevelNum = 1;				 // 液位
				pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
				osDelay(500);
				continue;
			}
			else
			{
				break;
			}
		}
		osDelay(1000);
		if (pMbInputData->nSysErrorCode)
		{
			if (pMbInputData->nStepCode == 9)
			{
				pMbHoldData->RW4230 = 7;
				return 1;
			}
		}
		else
		{
			CloseAllValves(); // 关组合阀
			osDelay(1000);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0308);  // 关比色阀，消解上+下阀
			// 增压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			OpenValves(0x0001);	  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			// 泻压
			Pump_run(-1200, 100); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
		}
	}
	return 0;
}

u16 Zhuodubc(void) // 浊度补偿
{
	u16 i = 0, j = 0;
	u32 sum = 0;
	// 1498
	if (pMbHoldData->RW125)
	{
		pMbInputData->nStepCode = 13; // 取比色电压V3
		osDelay(500);
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
		// 命令17
		pMbHoldData->fTemp1Kp = 3800;
		pMbHoldData->fTemp1Ki = 6;
		pMbHoldData->fTemp1Kd = 800;
		pMbHoldData->nTestTemper = 50; // 50度保温
		pMbHoldData->nTestTemperHoldtime = 0;
		ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		while (1)
		{
			osDelay(1000);
			i++;
			if (pMbHoldData->fPoolTemp >= (50 - 0.1))
				break;
			else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
			{
				j = 1;
				// 命令17
				pMbHoldData->fTemp1Kp = 3800;
				pMbHoldData->fTemp1Ki = 6;
				pMbHoldData->fTemp1Kd = 800;
				pMbHoldData->nTestTemper = 50; // 50度保温
				pMbHoldData->nTestTemperHoldtime = 0;
				ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
			}
			else if (i > 60 * 10)
			{
				pMbInputData->nSysErrorCode = 23; // 温度不能上升
				return 1;
			}
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
		i = 0;
		j = 0;
		while (1) // 1563
		{
			osDelay(1000);
			i++;
			if (pMbHoldData->fPoolTemp <= (50 - 0.5))
				break;
			else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
			{
				j = 1;
				// 命令17
				pMbHoldData->fTemp1Kp = 3800;
				pMbHoldData->fTemp1Ki = 6;
				pMbHoldData->fTemp1Kd = 800;
				pMbHoldData->nTestTemper = 50; // 50度保温
				pMbHoldData->nTestTemperHoldtime = 0;
				ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
			}
			else if (i > 60 * 25)
			{
				pMbInputData->nSysErrorCode = 33; // 温度不能下降
				return 1;
			}
			StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
			OpenValves(0x0400);				 // 开风扇
		}
		CloseValves(0x0400);	 // 风扇
		for (i = 0; i < 10; i++) // 1594,取10次平均值
		{
			sum += pMbInputData->nPoolTranLtAD;
			osDelay(200);
		}
		sum /= 10;
		pMbInputData->RW80 = sum; // V3比色电压
	}
	// 命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);

	return 0;
}

void GPY(u16 light)
{
	int16_t temp = 0;
	uint16_t buf[4];

	//命令3，设置光谱仪扫描次数，60次
	pMbHoldData->nSpectroAverageTimes = 60;
	SetAverageTimes();
	osDelay(500);

	//命令4，设置光谱仪积分时间，5
	pMbHoldData->nSpectroIntegralTime = 5;
	SetIntegralTime();
	osDelay(500);

	if(light == 0)
	{
		//命令22，光谱仪关光源
		CloseSpectroLight();
	}
	else
	{
		//命令21，光谱仪开光源
		OpenSpectroLight();
	}
	osDelay(500);

	//命令23，光谱仪启动扫描
	StartSpectroScan();
	osDelay(3000);

	//命令24，光谱仪读数据//数据存入保持寄存器29、30
	temp = ReadSpectroData(buf);
	if (temp >= 0)
	{
		pMbHoldData->nSpectro220AD = buf[0];
		pMbHoldData->nSpectro275AD = buf[1];
	}
	else
	{
		pMbHoldData->nSpectro220AD = temp;
		pMbHoldData->nSpectro275AD = temp;
	}
	osDelay(500);
}

uint16_t START_NH3N(void) // 氨氮主流程.触摸屏主流程170行启动
{
	uint16_t i = 0, j = 0, k = 0;
	u32 sum = 0;
	bool flag = false;
	pMbHoldData->RW580 = 0; // 下位机结束标志
	// 原流程170行开始。2082行结束。主要获取3个比色电压。
	CloseAllValves(); // 173行，关组合阀

	pMbInputData->nStepCode = 1; // SetWord(@运行日志@,0,1);//排空管路
	osDelay(500);
	OpenValves(0x1000);	  // 开废液阀
	Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x1000);  // 关废液阀
	pMbHoldData->RW580 = 1;
	// 227行，排空比色皿
	Paikongbsm(0, 0); // 高锰排空7次，其它3次
	// A插入
	Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0308); // 关比色阀，消解上+下阀

	OpenValves(0x0001); // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}

	pMbHoldData->RW580 = 2;

	// 385行
	i = pMbHoldData->RW50;
	if (i == 8 || i == 19 || i == 21 || i == 24 || i == 29 || i == 31 || i == 33 || i == 42)
	{
		pMbHoldData->RW580 = 3;
		pMbInputData->nStepCode = 2; // SetWord(@运行日志@,0,2);//零点标定清洗计管
		osDelay(500);
		for (i = 0; i < 3; i++)
		{
			// 420行
			CloseValves(0x0010); // 关蒸馏水
			OpenValves(0x0001);	 // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0001);  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				CloseValves(0x1000); // 开废液阀
			}
		}
		Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
		OpenValves(0x0001);	  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0001);  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			CloseValves(0x1000); // 开废液阀
		}
		pMbInputData->nStepCode = 3; // SetWord(@运行日志@,0,3);//零点标定液位校准
		osDelay(500);
		// 530行
		i = 0;
		while (1)
		{
			pMbHoldData->RW580 = 4;
			CheckLevelsAD(); // 108命令
			osDelay(1000);
			if (pMbInputData->nLL1AD >= 1200 && pMbInputData->nLL1AD <= 1400 && pMbInputData->nLL2AD >= 1200 && pMbInputData->nLL2AD <= 1400)
			{
				break;
			}
			else
			{
				if (i++ >= 3)
				{
					pMbHoldData->RW4230 = 1; // 故障代码=1
					return 1;
				}
			}
		}
		pMbHoldData->RW30 = pMbInputData->nLL1AD;
		pMbHoldData->RW31 = pMbInputData->nLL2AD;
		pMbHoldData->RW32 = pMbInputData->nLL1AD * 1.2;
		pMbHoldData->RW33 = pMbInputData->nLL2AD * 1.2;
		pMbHoldData->RW34 = pMbInputData->nLL1AD * 0.5;
		pMbHoldData->RW35 = pMbInputData->nLL2AD * 0.5;
		pMbHoldData->RW579 = 1;
		SaveUserData();
	}
	pMbHoldData->RW580 = 5;

	// 577行
	if (pMbHoldData->RW50 == 1 && pMbHoldData->RW36) // 工作类型==1
	{
		pMbInputData->nStepCode = 4; // 外接泵循环
		osDelay(500);
		OpenValves(0x4000); // 外接泵
		osDelay(pMbHoldData->RW36 * 1000);
		CloseValves(0x4000); // 外接泵
	}
	// 580
	pMbHoldData->fTemp1Kp = 500;
	pMbHoldData->fTemp1Ki = 0.3;
	pMbHoldData->fTemp1Kd = 2000;
	pMbHoldData->nTestTemper = 50;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 50度保温
	pMbHoldData->RW580 = 6;

	// 626行
	if (Caiqianpd()) // 采前判断
	{
		return 1;
	}
	pMbHoldData->RW580 = 7;

	// 632行，润洗2次
	if (Runxi())
	{
		return 1;
	}
	pMbHoldData->RW580 = 8;

	// 798行，采样
	if (Caiyang(0, 0)) // 采样
	{
		return 1;
	}
	pMbHoldData->RW580 = 9;

	Fanhui(); // 水样/其它样返回
	pMbHoldData->RW580 = 10;

	// 877行，稀释
	if (Xishi_NH3N()) // 稀释
	{
		return 1;
	}
	pMbHoldData->RW580 = 11;

	// 1438，第六页
	pMbInputData->nStepCode = 12; // 采试剂一
	osDelay(500);
	i = Caidingbsm(4, 600, -40, 1, 400, 20); // 采定液体到比色皿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 12;

	// 1466
	pMbInputData->nStepCode = 11; // 采蒸馏水
	osDelay(500);
	if (pMbHoldData->RW45 == 0)
	{
		j = 1;
	}
	else
	{
		j = 2;
	}
	i = Caidingbsm(16, 600, -40, j, 400, 20); // 采定液体到比色皿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 13;

	// 1497，浊度补偿
	i = Zhuodubc(); // 浊度补偿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 14;

	// 1612，第七页
	pMbInputData->nStepCode = 14; // 采试剂二
	osDelay(500);
	for (i = 0; i < 2; i++) // 采2次
	{
		j = Caidingbsm(32, 600, -40, 1, 400, 20); // 采定液体到比色皿
		if (j)
		{
			return 1;
		}
		osDelay(1000);
	}
	// 1638试剂二返回
	CloseAllValves();
	OpenValves(32);	   // 试剂二阀
	Pump_run(-600, 2); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();
	pMbHoldData->RW580 = 15;

	// 1657
	pMbInputData->nStepCode = 15; // 静置反应，5分钟
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL);
	// 命令16，为阻塞函数，加热时间结束才会往下走。
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度，5分钟
	pMbHoldData->nTestTemperHoldtime = 5;
	flag = ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	if (flag == false) // 温控失败
	{
		pMbHoldData->RW580 = 16;
		if (pMbHoldData->fPoolTemp < (50 - 1))
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	pMbHoldData->RW580 = 17;

	// 1737，
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	pMbInputData->nStepCode = 22;	 // 冷却
	osDelay(500);
	OpenValves(0x0400); // 开风扇
	i = 0;
	j = 0;
	while (1) //
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (float)49.8)
			break;
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	osDelay(500);
	CloseValves(0x0400); // 风扇
	pMbHoldData->RW580 = 18;

	// 1794,
	pMbInputData->nStepCode = 16; // 取比色电压V2
	osDelay(500);
	i = 0;
	while (1) // 1797
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (float)49.5)
			break;
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
	}
	sum = 0;
	for (i = 0; i < 10; i++) // 1840,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW81 = sum; // V2比色电压
	pMbHoldData->RW580 = 19;

	// 增加保温
	//  命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);

	// 1853
	pMbInputData->nStepCode = 17; // 排反应液
	osDelay(500);
	Paikongbsm(0, 0); // 高锰排空7次，其它3次

	pMbInputData->nStepCode = 18; // 清洗
	osDelay(500);
	CloseAllValves();	   // 关组合阀
	if (pMbHoldData->RW84) // 清洗次数
	{
		k = pMbHoldData->RW84;
	}
	else
	{
		k = 1;
	}
	for (j = 0; j < k; j++)
	{
		for (i = 0; i < 3; i++)
		{
			OpenValves(0x0010); // 蒸馏水阀
			// 命令13：
			pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
			pMbHoldData->nLevelNum = 2;				 // 液位
			pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			CloseAllValves(); // 关组合阀
			osDelay(500);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			osDelay(500);
		}
		Paikongbsm(0, 0); // 高锰排空7次，其它3次
		osDelay(500);
	}
	pMbHoldData->RW580 = 20;
	// 装满比色皿
	CloseAllValves(); // 关组合阀
	for (i = 0; i < 3; i++)
	{
		OpenValves(0x0010); // 蒸馏水阀
		// 命令13：
		pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
		pMbHoldData->nLevelNum = 2;				 // 液位
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves(); // 关组合阀
		osDelay(500);
		OpenValves(0x0308);	  // 开比色阀，消解上+下阀
		Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
		CloseAllValves();	  // 关组合阀
		osDelay(500);
	}

	// 1918
	pMbInputData->nStepCode = 19; // 取比色电压V1
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	// 命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	i = 0;
	j = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp >= (50 - 0.1))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
	OpenValves(0x0400);				 // 开风扇
	i = 0;
	j = 0;
	pMbHoldData->RW580 = 21;
	while (1) // 1985
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (50 - 0.5))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	CloseValves(0x0400); // 风扇
	ProcReadYSADs(10);	 // 命令20
	osDelay(1000);
	sum = 0;
	for (i = 0; i < 10; i++) // 2062,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW82 = sum; // V1比色电压
	pMbInputData->nSysErrorCode = 0;
	osDelay(5000);
	pMbHoldData->RW580 = 100; // 下位机结束标志

	Buzzer_ON_MS(500);
	return 0;
}

uint16_t HCSTART_NH3N(void) // 氨氮主流程.触摸屏主流程170行启动
{
	uint16_t i = 0, j = 0, k = 0;
	u32 sum = 0;
	bool flag = false;
	pMbHoldData->RW580 = 0; // 下位机结束标志
	// 原流程170行开始。2082行结束。主要获取3个比色电压。
	CloseAllValves(); // 173行，关组合阀

	pMbInputData->nStepCode = 1; // SetWord(@运行日志@,0,1);//排空管路
	HC_SETHOLE(0);
	osDelay(2000);
	HC_SETHOLE(1);
	HC_SET_NO();	//关阀。默认是关
	HC_SET_ML(0);
	osDelay(1000);


	// OpenValves(0x1000);	  // 开废液阀
	// Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x1000);  // 关废液阀
	pMbHoldData->RW580 = 1;
	// 227行，排空比色皿
	HCPaikongbsm(0, 0); // 高锰排空7次，其它3次
	// A插入
	// Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	// OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	// Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x0308); // 关比色阀，消解上+下阀

	// OpenValves(0x0001); // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	OpenValves(0x1000); // 开废液阀
	// }
	// Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x0001);  // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	CloseValves(0x1000); // 开废液阀
	// }
	// Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	// OpenValves(0x0001);	  // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	OpenValves(0x1000); // 开废液阀
	// }
	// Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x0001);  // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	CloseValves(0x1000); // 开废液阀
	// }

	pMbHoldData->RW580 = 2;

	// 385行
	// i = pMbHoldData->RW50;
	// if (i == 8 || i == 19 || i == 21 || i == 24 || i == 29 || i == 31 || i == 33 || i == 42)
	// {
	// 	pMbHoldData->RW580 = 3;
	// 	pMbInputData->nStepCode = 2; // SetWord(@运行日志@,0,2);//零点标定清洗计管
	// 	osDelay(500);
	// 	for (i = 0; i < 3; i++)
	// 	{
	// 		// 420行
	// 		CloseValves(0x0010); // 关蒸馏水
	// 		OpenValves(0x0001);	 // 开清洗废液阀
	// 		if (pMbHoldData->RW577)
	// 		{
	// 			OpenValves(0x1000); // 开废液阀
	// 		}
	// 		Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
	// 		CloseValves(0x0001);  // 开清洗废液阀
	// 		if (pMbHoldData->RW577)
	// 		{
	// 			CloseValves(0x1000); // 开废液阀
	// 		}
	// 	}
	// 	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	// 	OpenValves(0x0001);	  // 开清洗废液阀
	// 	if (pMbHoldData->RW577)
	// 	{
	// 		OpenValves(0x1000); // 开废液阀
	// 	}
	// 	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	// 	CloseValves(0x0001);  // 开清洗废液阀
	// 	if (pMbHoldData->RW577)
	// 	{
	// 		CloseValves(0x1000); // 开废液阀
	// 	}
	// 	pMbInputData->nStepCode = 3; // SetWord(@运行日志@,0,3);//零点标定液位校准
	// 	osDelay(500);
	// 	// 530行
	// 	i = 0;
	// 	while (1)
	// 	{
	// 		pMbHoldData->RW580 = 4;
	// 		CheckLevelsAD(); // 108命令
	// 		osDelay(1000);
	// 		if (pMbInputData->nLL1AD >= 1200 && pMbInputData->nLL1AD <= 1400 && pMbInputData->nLL2AD >= 1200 && pMbInputData->nLL2AD <= 1400)
	// 		{
	// 			break;
	// 		}
	// 		else
	// 		{
	// 			if (i++ >= 3)
	// 			{
	// 				pMbHoldData->RW4230 = 1; // 故障代码=1
	// 				return 1;
	// 			}
	// 		}
	// 	}
	// 	pMbHoldData->RW30 = pMbInputData->nLL1AD;
	// 	pMbHoldData->RW31 = pMbInputData->nLL2AD;
	// 	pMbHoldData->RW32 = pMbInputData->nLL1AD * 1.2;
	// 	pMbHoldData->RW33 = pMbInputData->nLL2AD * 1.2;
	// 	pMbHoldData->RW34 = pMbInputData->nLL1AD * 0.5;
	// 	pMbHoldData->RW35 = pMbInputData->nLL2AD * 0.5;
	// 	pMbHoldData->RW579 = 1;
	// 	SaveUserData();
	// }

	//增加柱塞泵介质更换步骤
	HC_SETHOLE(1);	//1~10
	HC_SET_SPEED(200);	//1~500
	HC_SET_ML(0);	//0~6

	HC_SETHOLE(0);	//1~10

	HC_SET_NC();	//开阀
	HC_SET_ML(2);	//0~6
	HC_SET_NO();	//关阀。默认是关
	HC_SETHOLE(1);	//1~10
	HC_SET_ML(0);	//0~6


	HC_SETHOLE(0);	//1~10
	HC_SET_NC();	//开阀
	HC_SET_ML(5);	//0~6
	HC_SET_NO();	//关阀。默认是关
	HC_SETHOLE(1);	//1~10
	HC_SET_ML(0);	//0~6


	pMbHoldData->RW580 = 5;

	// 577行
	if (pMbHoldData->RW50 == 1 && pMbHoldData->RW36) // 工作类型==1
	{
		pMbInputData->nStepCode = 4; // 外接泵循环
		osDelay(500);
		OpenValves(0x4000); // 外接泵
		osDelay(pMbHoldData->RW36 * 1000);
		CloseValves(0x4000); // 外接泵
	}
	// 580
	pMbHoldData->fTemp1Kp = 500;
	pMbHoldData->fTemp1Ki = 0.3;
	pMbHoldData->fTemp1Kd = 2000;
	pMbHoldData->nTestTemper = 50;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 50度保温
	pMbHoldData->RW580 = 6;

	// 626行
	// if (Caiqianpd()) // 采前判断
	// {
	// 	return 1;
	// }
	pMbHoldData->RW580 = 7;

	// 632行，润洗2次
	if (HCRunxi())
	{
		return 1;
	}
	pMbHoldData->RW580 = 8;

	// 798行，采样
	if (HCCaiyang(0, 3)) // 采样
	{
		return 1;
	}
	pMbHoldData->RW580 = 9;

	// Fanhui(); // 水样/其它样返回
	pMbHoldData->RW580 = 10;

	// 877行，稀释
	if (HCXishi_NH3N()) // 稀释
	{
		return 1;
	}
	pMbHoldData->RW580 = 11;

	// 1438，第六页
	pMbInputData->nStepCode = 12; // 采试剂一
	osDelay(500);
	// i = Caidingbsm(4, 600, -40, 1, 400, 20); // 采定液体到比色皿
	i = HCCaidingbsm(8,0.5,0); // 采定液体到比色皿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 12;

	// 1466
	pMbInputData->nStepCode = 11; // 采蒸馏水
	osDelay(500);
	if (pMbHoldData->RW45 == 0)
	{
		// i = Caidingbsm(16, 600, -40, j, 400, 20); // 采定液体到比色皿
		i = HCCaidingbsm(4,0.5,0); // 采定液体到比色皿
		// j = 1;
	}
	else
	{
		i = HCCaidingbsm(4,4,0); // 采定液体到比色皿
		// j = 2;
	}
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 13;

	// 1497，浊度补偿
	i = Zhuodubc(); // 浊度补偿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 14;

	// 1612，第七页
	pMbInputData->nStepCode = 14; // 采试剂二
	osDelay(500);
	// for (i = 0; i < 2; i++) // 采2次
	{
		// j = Caidingbsm(32, 600, -40, 1, 400, 20); // 采定液体到比色皿
		j = HCCaidingbsm(9,1,0); // 采定液体到比色皿
		if (j)
		{
			return 1;
		}
		osDelay(1000);
	}
	// 1638试剂二返回
	// CloseAllValves();
	// OpenValves(32);	   // 试剂二阀
	// Pump_run(-600, 2); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();
	pMbHoldData->RW580 = 15;

	//排空
	HC_SET_SPEED(200);	//1~500
	HC_SETHOLE(1);	//1~10
	HC_SET_ML(0);	//0~6

	HC_SETHOLE(10);	//1~10
	HC_SET_ML(5);	//0~6
	
	HC_SETHOLE(3);	//1~10
	OpenValves(0x0308);	  // 开比色阀，消解上+下阀
	HC_SET_ML(0);	//0~6

	HC_SETHOLE(1);	//1~10
	CloseAllValves();

	// 1657
	pMbInputData->nStepCode = 15; // 静置反应，5分钟
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL);
	// 命令16，为阻塞函数，加热时间结束才会往下走。
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度，5分钟
	pMbHoldData->nTestTemperHoldtime = 5;
	flag = ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	if (flag == false) // 温控失败
	{
		pMbHoldData->RW580 = 16;
		if (pMbHoldData->fPoolTemp < (50 - 1))
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	pMbHoldData->RW580 = 17;

	// 1737，
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	pMbInputData->nStepCode = 22;	 // 冷却
	osDelay(500);
	OpenValves(0x0400); // 开风扇
	i = 0;
	j = 0;
	while (1) //
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (float)49.8)
			break;
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	osDelay(500);
	CloseValves(0x0400); // 风扇
	pMbHoldData->RW580 = 18;

	// 1794,
	pMbInputData->nStepCode = 16; // 取比色电压V2
	osDelay(500);
	i = 0;
	while (1) // 1797
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (float)49.5)
			break;
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
	}
	sum = 0;
	for (i = 0; i < 10; i++) // 1840,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW81 = sum; // V2比色电压
	pMbHoldData->RW580 = 19;

	// 增加保温
	//  命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);

	// 1853
	pMbInputData->nStepCode = 17; // 排反应液
	osDelay(500);
	HCPaikongbsm(0, 0); // 高锰排空7次，其它3次

	pMbInputData->nStepCode = 18; // 清洗
	osDelay(500);
	CloseAllValves();	   // 关组合阀
	if (pMbHoldData->RW84) // 清洗次数
	{
		k = pMbHoldData->RW84;
	}
	else
	{
		k = 1;
	}
	HC_SET_SPEED(200);	//1~500
	HC_SETHOLE(1);	//1~10
	HC_SET_ML(0);	//0~6
	osDelay(1000);
	for (j = 0; j < k; j++)
	{
		for (i = 0; i < 3; i++)
		{
			OpenValves(0x0010); // 蒸馏水阀
			HC_SETHOLE(4);	//1~10
			HC_SET_ML(5);	//0~6
			// 命令13：
			// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			// pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
			// pMbHoldData->nLevelNum = 2;				 // 液位
			// pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			CloseAllValves(); // 关组合阀
			osDelay(500);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			HC_SETHOLE(3);	//1~10
			HC_SET_ML(0);	//0~6
			// Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			osDelay(500);
		}
		HCPaikongbsm(0, 0); // 高锰排空7次，其它3次
		osDelay(500);
	}
	pMbHoldData->RW580 = 20;
	// 装满比色皿
	CloseAllValves(); // 关组合阀
	for (i = 0; i < 2; i++)
	{
		OpenValves(0x0010); // 蒸馏水阀
		HC_SETHOLE(4);	//1~10
		HC_SET_ML(5);	//0~6
		// 命令13：
		// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		// pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
		// pMbHoldData->nLevelNum = 2;				 // 液位
		// pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves(); // 关组合阀
		osDelay(500);
		OpenValves(0x0308);	  // 开比色阀，消解上+下阀
		HC_SETHOLE(3);	//1~10
		HC_SET_ML(0);	//0~6
		// Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
		CloseAllValves();	  // 关组合阀
		osDelay(500);
	}

	// 1918
	pMbInputData->nStepCode = 19; // 取比色电压V1
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	// 命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	i = 0;
	j = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp >= (50 - 0.1))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
	OpenValves(0x0400);				 // 开风扇
	i = 0;
	j = 0;
	pMbHoldData->RW580 = 21;
	while (1) // 1985
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (50 - 0.5))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	CloseValves(0x0400); // 风扇
	ProcReadYSADs(10);	 // 命令20
	osDelay(1000);
	sum = 0;
	for (i = 0; i < 10; i++) // 2062,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW82 = sum; // V1比色电压
	pMbInputData->nSysErrorCode = 0;
	osDelay(5000);
	pMbHoldData->RW580 = 100; // 下位机结束标志

	Buzzer_ON_MS(500);
	return 0;
}

uint16_t START_TP(void) // TP主流程.触摸屏主流程167行启动
{	
	uint16_t i = 0, j = 0, k = 0;
	u32 sum = 0;
	bool flag = false;
	pMbHoldData->RW580 = 0; // 下位机结束标志
	// 原流程170行开始。2082行结束。主要获取3个比色电压。
	CloseAllValves(); // 173行，关组合阀

	pMbInputData->nStepCode = 1; // SetWord(@运行日志@,0,1);//排空管路
	osDelay(500);
	OpenValves(0x1000);	  // 开废液阀（操作对应GPIO口）
	Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x1000);  // 关废液阀
	
	pMbHoldData->RW580 = 1;
	// 227行，排空比色皿
	Paikongbsm(0, 0); // 高锰排空7次，其它3次
	// A插入
	Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0308); // 关比色阀，消解上+下阀

	OpenValves(0x0001); // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}

	pMbHoldData->RW580 = 2;

	// 385行
	i = pMbHoldData->RW50;
	if (i == 8 || i == 19 || i == 21 || i == 24 || i == 29 || i == 31 || i == 33 || i == 42)
	{
		pMbHoldData->RW580 = 3;
		pMbInputData->nStepCode = 2; // SetWord(@运行日志@,0,2);//零点标定清洗计管
		osDelay(500);
		for (i = 0; i < 3; i++)
		{
			// 420行
			CloseValves(0x0010); // 关蒸馏水
			OpenValves(0x0001);	 // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0001);  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				CloseValves(0x1000); // 开废液阀
			}
		}
		Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
		OpenValves(0x0001);	  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0001);  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			CloseValves(0x1000); // 开废液阀
		}
		pMbInputData->nStepCode = 3; // SetWord(@运行日志@,0,3);//零点标定液位校准
		osDelay(500);
		// 530行
		i = 0;
		while (1)
		{
			pMbHoldData->RW580 = 4;
			CheckLevelsAD(); // 108命令
			osDelay(1000);
			if (pMbInputData->nLL1AD >= 1200 && pMbInputData->nLL1AD <= 1400 && pMbInputData->nLL2AD >= 1200 && pMbInputData->nLL2AD <= 1400)
			{
				break;
			}
			else
			{
				if (i++ >= 3)
				{
					pMbHoldData->RW4230 = 1; // 故障代码=1
					return 1;
				}
			}
		}
		pMbHoldData->RW30 = pMbInputData->nLL1AD;
		pMbHoldData->RW31 = pMbInputData->nLL2AD;
		pMbHoldData->RW32 = pMbInputData->nLL1AD * 1.2;
		pMbHoldData->RW33 = pMbInputData->nLL2AD * 1.2;
		pMbHoldData->RW34 = pMbInputData->nLL1AD * 0.5;
		pMbHoldData->RW35 = pMbInputData->nLL2AD * 0.5;
		pMbHoldData->RW579 = 1;
		SaveUserData();
	}
	pMbHoldData->RW580 = 5;

	// 577行
	if (pMbHoldData->RW50 == 1 && pMbHoldData->RW36) // 工作类型==1
	{
		pMbInputData->nStepCode = 4; // 外接泵循环
		osDelay(500);
		OpenValves(0x4000); // 外接泵
		osDelay(pMbHoldData->RW36 * 1000);
		CloseValves(0x4000); // 外接泵
	}
	// 580
	pMbHoldData->fTemp1Kp = 500;
	pMbHoldData->fTemp1Ki = 0.3;
	pMbHoldData->fTemp1Kd = 2000;
	pMbHoldData->nTestTemper = 50;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 50度保温
	pMbHoldData->RW580 = 6;

	// 626行
	if (Caiqianpd()) // 采前判断
	{
		return 1;
	}
	pMbHoldData->RW580 = 7;

	// 632行，润洗2次
	if (Runxi())
	{
		return 1;
	}
	pMbHoldData->RW580 = 8;

	// 798行，采样
	if (Caiyang(0, 1)) // 采样
	{
		return 1;
	}
	if (pMbHoldData->RW45 == 0) // 量程1再采一次
	{
		if (Caiyang(0, 1)) // 采样
		{
			return 1;
		}
	}
	pMbHoldData->RW580 = 9;

	Fanhui(); // 水样/其它样返回
	pMbHoldData->RW580 = 10;

	// 877行，稀释
	if (Xishi_TP()) // 稀释
	{
		return 1;
	}
	pMbHoldData->RW580 = 11;

	// 1438，第六页
	pMbInputData->nStepCode = 12; // 采试剂一
	osDelay(500);
	i = Caidingbsm(4, 600, -40, 1, 400, 20); // 采定液体到比色皿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 12;

	// 1226
	pMbInputData->nStepCode = 21; // 加热消解，120度，5分钟
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL);
	// 命令16，为阻塞函数，加热时间结束才会往下走。
	pMbHoldData->fTemp1Kp = 3600;
	pMbHoldData->fTemp1Ki = 8;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 120; // 165度，15分钟
	pMbHoldData->nTestTemperHoldtime = 5;
	flag = ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	if (flag == false) // 温控失败
	{
		pMbHoldData->RW580 = 16;
		if (pMbHoldData->fPoolTemp < (50 - 1))
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	pMbHoldData->RW580 = 17;

	// 1301
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	pMbInputData->nStepCode = 22;	 // 冷却
	osDelay(500);
	OpenValves(0x0400); // 开风扇
	i = 0;
	j = 0;
	while (1) //
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= 52)
			break;
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	osDelay(500);
	CloseValves(0x0400); // 风扇
	pMbHoldData->RW580 = 18;

	pMbHoldData->fTemp1Kp = 3500;
	pMbHoldData->fTemp1Ki = 2;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 50度保温

	// 1358
	if (pMbHoldData->RW45 != 0) // 量程1不采蒸馏水
	{
		pMbInputData->nStepCode = 11; // 采蒸馏水
		osDelay(500);
		if (pMbHoldData->RW45 == 1) // 量程2采低液位，34采高液位
		{
			j = 1;
		}
		else
		{
			j = 2;
		}
		i = Caidingbsm(16, 600, -40, j, 400, 20); // 采定液体到比色皿
		if (i)
		{
			return 1;
		}
		pMbHoldData->RW580 = 13;
	}

	// 1395，第七页
	pMbInputData->nStepCode = 14; // 采试剂二
	osDelay(500);
	j = Caidingbsm(32, 600, -40, 1, 400, 20); // 采定液体到比色皿
	if (j)
	{
		return 1;
	}
	// 1638试剂二返回
	CloseAllValves();
	if (pMbHoldData->RW48)
	{
		OpenValves(32);					   // 试剂二阀
		Pump_run(-600, pMbHoldData->RW48); // 命令12，操作蠕动泵，速度，圈数
		CloseAllValves();
	}
	pMbHoldData->RW580 = 15;

	// 1497，浊度补偿,COD不用浊度补偿
	i = Zhuodubc(); // 浊度补偿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 14;

	// 1395，第七页
	pMbInputData->nStepCode = 23; // 采试剂三
	osDelay(500);
	j = Caidingbsm(64, 600, -40, 1, 400, 20); // 采定液体到比色皿
	if (j)
	{
		return 1;
	}

	// 1572,
	pMbInputData->nStepCode = 16;	 // 取比色电压V2
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	// 命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	i = 0;
	j = 0;

	while (1)
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp >= (50 - 0.1))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
	OpenValves(0x0400);				 // 开风扇
	i = 0;
	j = 0;
	while (1) // 1563
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (50 - 0.5))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	osDelay(200);
	CloseValves(0x0400); // 风扇
	sum = 0;
	for (i = 0; i < 10; i++) // 1594,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW81 = sum; // V2比色电压

	// 增加保温
	//  命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);

	// 1853
	pMbInputData->nStepCode = 17; // 排反应液
	osDelay(500);
	Paikongbsm(0, 0); // 高锰排空7次，其它3次

	pMbInputData->nStepCode = 18; // 清洗
	osDelay(500);
	CloseAllValves();	   // 关组合阀
	if (pMbHoldData->RW84) // 清洗次数
	{
		k = pMbHoldData->RW84;
	}
	else
	{
		k = 1;
	}
	for (j = 0; j < k; j++)
	{
		for (i = 0; i < 3; i++)
		{
			OpenValves(0x0010); // 蒸馏水阀
			// 命令13：
			pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			pMbHoldData->nStepperMotorSpeed = 1000;	 // 速度
			pMbHoldData->nLevelNum = 2;				 // 液位
			pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			CloseAllValves(); // 关组合阀
			osDelay(500);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			osDelay(500);
		}
		Paikongbsm(0, 0); // 高锰排空7次，其它3次
		osDelay(500);
	}
	pMbHoldData->RW580 = 20;
	// 装满比色皿
	CloseAllValves(); // 关组合阀
	for (i = 0; i < 3; i++)
	{
		OpenValves(0x0010); // 蒸馏水阀
		// 命令13：
		pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		pMbHoldData->nStepperMotorSpeed = 1000;	 // 速度
		pMbHoldData->nLevelNum = 2;				 // 液位
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves(); // 关组合阀
		osDelay(500);
		OpenValves(0x0308);	  // 开比色阀，消解上+下阀
		Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
		CloseAllValves();	  // 关组合阀
		osDelay(500);
	}

	// 1918
	pMbInputData->nStepCode = 19; // 取比色电压V1
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	// 命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	i = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp >= (50 - 0.1))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
	OpenValves(0x0400);				 // 开风扇
	i = 0;
	j = 0;
	pMbHoldData->RW580 = 21;
	while (1) // 1985
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (50 - 0.5))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	CloseValves(0x0400); // 风扇
	ProcReadYSADs(10);	 // 命令20
	osDelay(1000);
	sum = 0;
	for (i = 0; i < 10; i++) // 2062,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW82 = sum; // V1比色电压
	pMbInputData->nSysErrorCode = 0;
	osDelay(5000);
	pMbHoldData->RW580 = 100; // 下位机结束标志

	Buzzer_ON_MS(500);
	return 0;
	
}

uint16_t START_COD(void) // COD主流程.触摸屏主流程167行启动
{
	uint16_t i = 0, j = 0, k = 0;
	u32 sum = 0;
	bool flag = false;
	pMbHoldData->RW580 = 0; // 下位机结束标志
	// 原流程170行开始。2082行结束。主要获取3个比色电压。
	CloseAllValves(); // 173行，关组合阀

	pMbInputData->nStepCode = 1; // SetWord(@运行日志@,0,1);//排空管路
	osDelay(500);
	OpenValves(0x1000);	  // 开废液阀
	Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x1000);  // 关废液阀
	pMbHoldData->RW580 = 1;
	// 227行，排空比色皿
	Paikongbsm(0, 0); // 高锰排空7次，其它3次
	// A插入
	Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0308); // 关比色阀，消解上+下阀

	OpenValves(0x0001); // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}

	pMbHoldData->RW580 = 2;

	// 385行
	i = pMbHoldData->RW50;
	if (i == 8 || i == 19 || i == 21 || i == 24 || i == 29 || i == 31 || i == 33 || i == 42)
	{
		pMbHoldData->RW580 = 3;
		pMbInputData->nStepCode = 2; // SetWord(@运行日志@,0,2);//零点标定清洗计管
		osDelay(500);
		for (i = 0; i < 3; i++)
		{
			// 420行
			CloseValves(0x0010); // 关蒸馏水
			OpenValves(0x0001);	 // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0001);  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				CloseValves(0x1000); // 开废液阀
			}
		}
		Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
		OpenValves(0x0001);	  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0001);  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			CloseValves(0x1000); // 开废液阀
		}
		pMbInputData->nStepCode = 3; // SetWord(@运行日志@,0,3);//零点标定液位校准
		osDelay(500);
		// 530行
		i = 0;
		while (1)
		{
			pMbHoldData->RW580 = 4;
			CheckLevelsAD(); // 108命令
			osDelay(1000);
			if (pMbInputData->nLL1AD >= 1200 && pMbInputData->nLL1AD <= 1400 && pMbInputData->nLL2AD >= 1200 && pMbInputData->nLL2AD <= 1400)
			{
				break;
			}
			else
			{
				if (i++ >= 3)
				{
					pMbHoldData->RW4230 = 1; // 故障代码=1
					return 1;
				}
			}
		}
		pMbHoldData->RW30 = pMbInputData->nLL1AD;
		pMbHoldData->RW31 = pMbInputData->nLL2AD;
		pMbHoldData->RW32 = pMbInputData->nLL1AD * 1.2;
		pMbHoldData->RW33 = pMbInputData->nLL2AD * 1.2;
		pMbHoldData->RW34 = pMbInputData->nLL1AD * 0.5;
		pMbHoldData->RW35 = pMbInputData->nLL2AD * 0.5;
		pMbHoldData->RW579 = 1;
		SaveUserData();
	}
	pMbHoldData->RW580 = 5;

	// 577行
	if (pMbHoldData->RW50 == 1 && pMbHoldData->RW36) // 工作类型==1
	{
		pMbInputData->nStepCode = 4; // 外接泵循环
		osDelay(500);
		OpenValves(0x4000); // 外接泵
		osDelay(pMbHoldData->RW36 * 1000);
		CloseValves(0x4000); // 外接泵
	}
	// 580
	pMbHoldData->fTemp1Kp = 500;
	pMbHoldData->fTemp1Ki = 0.3;
	pMbHoldData->fTemp1Kd = 2000;
	pMbHoldData->nTestTemper = 50;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 50度保温
	pMbHoldData->RW580 = 6;

	// 626行
	if (Caiqianpd()) // 采前判断
	{
		return 1;
	}
	pMbHoldData->RW580 = 7;

	// 632行，润洗2次
	if (Runxi())
	{
		return 1;
	}
	pMbHoldData->RW580 = 8;

	// 798行，采样
	if (Caiyang(0, 0)) // 采样
	{
		return 1;
	}
	pMbHoldData->RW580 = 9;

	Fanhui(); // 水样/其它样返回
	pMbHoldData->RW580 = 10;

	// 877行，稀释
	if (Xishi_COD()) // 稀释
	{
		return 1;
	}
	pMbHoldData->RW580 = 11;

	// 1438，第六页
	pMbInputData->nStepCode = 12; // 采试剂一
	osDelay(500);
	i = Caidingbsm(4, 600, -40, 1, 400, 20); // 采定液体到比色皿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 12;

	// 1466
	pMbInputData->nStepCode = 11; // 采蒸馏水
	osDelay(500);

	i = Chouyetibdw(16, 600, 2, 400); // 抽液体不定位
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 13;

	// // 1497，浊度补偿,COD不用浊度补偿
	// i = Zhuodubc(); // 浊度补偿
	// if (i)
	// {
	// 	return 1;
	// }
	// pMbHoldData->RW580 = 14;

	// 1612，第七页
	pMbInputData->nStepCode = 14; // 采试剂二
	osDelay(500);
	j = Caidingbsm(32, 600, -40, 1, 400, 20); // 采定液体到比色皿
	if (j)
	{
		return 1;
	}
	// 1638试剂二返回
	CloseAllValves();
	OpenValves(32);	   // 试剂二阀
	Pump_run(-600, 2); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();
	pMbHoldData->RW580 = 15;

	// 1657
	pMbInputData->nStepCode = 21; // 加热消解，15分钟
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL);
	// 命令16，为阻塞函数，加热时间结束才会往下走。
	pMbHoldData->fTemp1Kp = 3600;
	pMbHoldData->fTemp1Ki = 9;
	pMbHoldData->fTemp1Kd = 1500;
	pMbHoldData->nTestTemper = 165; // 165度，15分钟
	pMbHoldData->nTestTemperHoldtime = 15;
	flag = ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	if (flag == false) // 温控失败
	{
		pMbHoldData->RW580 = 16;
		if (pMbHoldData->fPoolTemp < (50 - 1))
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	pMbHoldData->RW580 = 17;

	// 1737，
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	pMbInputData->nStepCode = 22;	 // 冷却
	osDelay(500);
	OpenValves(0x0400); // 开风扇
	i = 0;
	j = 0;
	while (1) //
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (float)49.8)
			break;
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	osDelay(500);
	CloseValves(0x0400); // 风扇
	pMbHoldData->RW580 = 18;

	// 1794,
	pMbInputData->nStepCode = 16; // 取比色电压V2
	osDelay(500);
	i = 0;
	while (1) // 1797
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (float)49.5)
			break;
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
	}
	sum = 0;
	for (i = 0; i < 10; i++) // 1840,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW81 = sum; // V2比色电压
	pMbHoldData->RW580 = 19;

	// 增加保温
	//  命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);

	// 1853
	pMbInputData->nStepCode = 17; // 排反应液
	osDelay(500);
	Paikongbsm(0, 0); // 高锰排空7次，其它3次

	pMbInputData->nStepCode = 18; // 清洗
	osDelay(500);
	CloseAllValves();	   // 关组合阀
	if (pMbHoldData->RW84) // 清洗次数
	{
		k = pMbHoldData->RW84;
	}
	else
	{
		k = 1;
	}
	for (j = 0; j < k; j++)
	{
		for (i = 0; i < 3; i++)
		{
			OpenValves(0x0010); // 蒸馏水阀
			// 命令13：
			pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
			pMbHoldData->nLevelNum = 2;				 // 液位
			pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			CloseAllValves(); // 关组合阀
			osDelay(500);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			osDelay(500);
		}
		Paikongbsm(0, 0); // 高锰排空7次，其它3次
		osDelay(500);
	}
	pMbHoldData->RW580 = 20;
	// 装满比色皿
	CloseAllValves(); // 关组合阀
	for (i = 0; i < 3; i++)
	{
		OpenValves(0x0010); // 蒸馏水阀
		// 命令13：
		pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
		pMbHoldData->nLevelNum = 2;				 // 液位
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves(); // 关组合阀
		osDelay(500);
		OpenValves(0x0308);	  // 开比色阀，消解上+下阀
		Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
		CloseAllValves();	  // 关组合阀
		osDelay(500);
	}

	// 1918
	pMbInputData->nStepCode = 19; // 取比色电压V1
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	// 命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	i = 0;
	j = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp >= (50 - 0.1))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
	OpenValves(0x0400);				 // 开风扇
	i = 0;
	j = 0;
	pMbHoldData->RW580 = 21;
	while (1) // 1985
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (50 - 0.5))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	CloseValves(0x0400); // 风扇
	ProcReadYSADs(10);	 // 命令20
	osDelay(1000);
	sum = 0;
	for (i = 0; i < 10; i++) // 2062,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW82 = sum; // V1比色电压
	pMbInputData->nSysErrorCode = 0;
	osDelay(5000);
	pMbHoldData->RW580 = 100; // 下位机结束标志

	Buzzer_ON_MS(500);
	return 0;
}

uint16_t RZSTART_COD(void) // COD主流程.触摸屏主流程167行启动
{
	uint16_t i = 0, j = 0, k = 0;
	u32 sum = 0;
	bool flag = false;
	RZdelayflag = 0;
	pMbHoldData->RW580 = 0; // 下位机结束标志
	// 原流程170行开始。2082行结束。主要获取3个比色电压。
	CloseAllValves(); // 173行，关组合阀


	pMbInputData->nStepCode = 1; // SetWord(@运行日志@,0,1);//排空管路
	RUNZE_SETHOLE(1);	//阀孔复位到孔1
	RUNZE_SET_NO();	//关阀。默认是关
	RUNZE_SET_ML(0);//柱塞泵复位归0
	osDelay(1000);

	// OpenValves(0x1000);	  // 开废液阀
	// Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x1000);  // 关废液阀
	pMbHoldData->RW580 = 1;
	// 227行，排空比色皿
	RZPaikongbsm(0, 0); // 高锰排空7次，其它3次
	// A插入
	// Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	// OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	// Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x0308); // 关比色阀，消解上+下阀

	// OpenValves(0x0001); // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	OpenValves(0x1000); // 开废液阀
	// }
	// Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x0001);  // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	CloseValves(0x1000); // 开废液阀
	// }
	// Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	// OpenValves(0x0001);	  // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	OpenValves(0x1000); // 开废液阀
	// }
	// Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	// CloseValves(0x0001);  // 开清洗废液阀
	// if (pMbHoldData->RW577)
	// {
	// 	CloseValves(0x1000); // 开废液阀
	// }

	pMbHoldData->RW580 = 2;

	// 385行
	// i = pMbHoldData->RW50;
	// if (i == 8 || i == 19 || i == 21 || i == 24 || i == 29 || i == 31 || i == 33 || i == 42)
	// {
	// 	pMbHoldData->RW580 = 3;
	// 	pMbInputData->nStepCode = 2; // SetWord(@运行日志@,0,2);//零点标定清洗计管
	// 	osDelay(500);
	// 	for (i = 0; i < 3; i++)
	// 	{
	// 		// 420行
	// 		CloseValves(0x0010); // 关蒸馏水
	// 		OpenValves(0x0001);	 // 开清洗废液阀
	// 		if (pMbHoldData->RW577)
	// 		{
	// 			OpenValves(0x1000); // 开废液阀
	// 		}
	// 		Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
	// 		CloseValves(0x0001);  // 开清洗废液阀
	// 		if (pMbHoldData->RW577)
	// 		{
	// 			CloseValves(0x1000); // 开废液阀
	// 		}
	// 	}
	// 	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	// 	OpenValves(0x0001);	  // 开清洗废液阀
	// 	if (pMbHoldData->RW577)
	// 	{
	// 		OpenValves(0x1000); // 开废液阀
	// 	}
	// 	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	// 	CloseValves(0x0001);  // 开清洗废液阀
	// 	if (pMbHoldData->RW577)
	// 	{
	// 		CloseValves(0x1000); // 开废液阀
	// 	}
	// 	pMbInputData->nStepCode = 3; // SetWord(@运行日志@,0,3);//零点标定液位校准
	// 	osDelay(500);
	// 	// 530行
	// 	i = 0;
	// 	while (1)
	// 	{
	// 		pMbHoldData->RW580 = 4;
	// 		CheckLevelsAD(); // 108命令
	// 		osDelay(1000);
	// 		if (pMbInputData->nLL1AD >= 1200 && pMbInputData->nLL1AD <= 1400 && pMbInputData->nLL2AD >= 1200 && pMbInputData->nLL2AD <= 1400)
	// 		{
	// 			break;
	// 		}
	// 		else
	// 		{
	// 			if (i++ >= 3)
	// 			{
	// 				pMbHoldData->RW4230 = 1; // 故障代码=1
	// 				return 1;
	// 			}
	// 		}
	// 	}
	// 	pMbHoldData->RW30 = pMbInputData->nLL1AD;
	// 	pMbHoldData->RW31 = pMbInputData->nLL2AD;
	// 	pMbHoldData->RW32 = pMbInputData->nLL1AD * 1.2;
	// 	pMbHoldData->RW33 = pMbInputData->nLL2AD * 1.2;
	// 	pMbHoldData->RW34 = pMbInputData->nLL1AD * 0.5;
	// 	pMbHoldData->RW35 = pMbInputData->nLL2AD * 0.5;
	// 	pMbHoldData->RW579 = 1;
	// 	SaveUserData();
	// }

	//增加柱塞泵介质更换步骤
	RUNZE_SETHOLE(1);	//1~10
	RUNZE_SET_SPEED(100);	//1~500
	RUNZE_SET_ML(0);	//0~6

	RUNZE_SET_NC();	//开阀
	RUNZE_SET_ML(2);	//0~6
	RUNZE_SET_NO();	//关阀。默认是关
	RUNZE_SET_ML(0);	//0~6

	RUNZE_SET_NC();	//开阀
	RUNZE_SET_ML(6);	//0~6
	RUNZE_SET_NO();	//关阀。默认是关
	RUNZE_SET_ML(0);	//0~6

	
	pMbHoldData->RW580 = 5;

	// 577行
	if (pMbHoldData->RW50 == 1 && pMbHoldData->RW36) // 工作类型==1
	{
		pMbInputData->nStepCode = 4; // 外接泵循环
		osDelay(500);
		OpenValves(0x4000); // 外接泵
		osDelay(pMbHoldData->RW36 * 1000);
		CloseValves(0x4000); // 外接泵
	}
	// 580
	pMbHoldData->fTemp1Kp = 500;
	pMbHoldData->fTemp1Ki = 0.3;
	pMbHoldData->fTemp1Kd = 2000;
	pMbHoldData->nTestTemper = 50;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 50度保温
	pMbHoldData->RW580 = 6;

	// 626行
	// if (Caiqianpd()) // 采前判断
	// {
	// 	return 1;
	// }
	pMbHoldData->RW580 = 7;

	// 632行，润洗2次
	if (RZRunxi())
	{
		return 1;
	}
	pMbHoldData->RW580 = 8;

	// 798行，采样
	if (RZCaiyang(0, 4)) // 采样
	{
		return 1;
	}
	pMbHoldData->RW580 = 9;

	// Fanhui(); // 水样/其它样返回
	pMbHoldData->RW580 = 10;

	// 877行，稀释
	if (RZXishi_COD()) // 稀释
	{
		return 1;
	}
	pMbHoldData->RW580 = 11;

	// 1438，第六页
	pMbInputData->nStepCode = 12; // 采试剂一
	osDelay(500);
	i = RZCaidingbsm(8,2,0); // 采定液体到比色皿
	if (i)
	{
		pMbHoldData->RW4230 = 14; // 故障代码
		return 1;
	}
	pMbHoldData->RW580 = 12;

	// 1466
	// pMbInputData->nStepCode = 11; // 采蒸馏水
	// osDelay(500);

	// i = Chouyetibdw(16, 600, 2, 400); // 抽液体不定位
	// if (i)
	// {
	// 	return 1;
	// }
	// pMbHoldData->RW580 = 13;

	// // 1497，浊度补偿,COD不用浊度补偿
	// i = Zhuodubc(); // 浊度补偿
	// if (i)
	// {
	// 	return 1;
	// }
	// pMbHoldData->RW580 = 14;

	// 1612，第七页
	pMbInputData->nStepCode = 14; // 采试剂二
	osDelay(500);
	// j = Caidingbsm(32, 600, -40, 1, 400, 20); // 采定液体到比色皿
	j = RZCaidingbsm(9,2,1);
	if (j)
	{
		pMbHoldData->RW4230 = 16; // 故障代码
		return 1;
	}
	// 1638试剂二返回
	// CloseAllValves();
	// OpenValves(32);	   // 试剂二阀
	// Pump_run(-600, 2); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();
	pMbHoldData->RW580 = 15;

	//排空
	RUNZE_SET_SPEED(100);	//1~500
	RUNZE_SETHOLE(1);	//1~10
	RUNZE_SET_ML(0);	//0~6
	for(i = 0;i < 2;i ++)
	{
		RUNZE_SETHOLE(10);	//1~10
		RUNZE_SET_ML(6);	//0~6
		
		RUNZE_SETHOLE(3);	//1~10
		OpenValves(0x0308);	  // 开比色阀，消解上+下阀
		RUNZE_SET_ML(0);	//0~6
		CloseAllValves();
	}

	RUNZE_SETHOLE(1);	//1~10

	RUNZE_SET_SPEED(100);	//1~500
	// 1657
	pMbInputData->nStepCode = 21; // 加热消解，15分钟
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL);
	// 命令16，为阻塞函数，加热时间结束才会往下走。
	pMbHoldData->fTemp1Kp = 3600;
	pMbHoldData->fTemp1Ki = 9;
	pMbHoldData->fTemp1Kd = 1500;
	pMbHoldData->nTestTemper = 165; // 165度，15分钟
	// pMbHoldData->nTestTemper = 65; // 165度，15分钟
	pMbHoldData->nTestTemperHoldtime = 15;
	flag = ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	if (flag == false) // 温控失败
	{
		pMbHoldData->RW580 = 16;
		if (pMbHoldData->fPoolTemp < (50 - 1))
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	pMbHoldData->RW580 = 17;

	// 1737，
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	pMbInputData->nStepCode = 22;	 // 冷却
	osDelay(500);
	OpenValves(0x0400); // 开风扇
	i = 0;
	j = 0;
	while (1) //
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (float)49.8)
			break;
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	osDelay(500);
	CloseValves(0x0400); // 风扇
	pMbHoldData->RW580 = 18;

	// 1794,
	pMbInputData->nStepCode = 16; // 取比色电压V2
	osDelay(500);
	i = 0;
	while (1) // 1797
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (float)49.5)
			break;
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		if(i > 60 * 5)
		{
			OpenValves(0x0400); // 开风扇
		}
	}
	CloseValves(0x0400); // 风扇
	sum = 0;
	for (i = 0; i < 10; i++) // 1840,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW81 = sum; // V2比色电压
	pMbHoldData->RW580 = 19;

	// 增加保温
	//  命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);

	// 1853
	pMbInputData->nStepCode = 17; // 排反应液
	osDelay(500);
	RZPaikongbsm(0, 0); // 高锰排空7次，其它3次

	pMbInputData->nStepCode = 18; // 清洗
	osDelay(500);
	CloseAllValves();	   // 关组合阀
	if (pMbHoldData->RW84) // 清洗次数
	{
		k = pMbHoldData->RW84;
	}
	else
	{
		k = 1;
	}
	RUNZE_SET_SPEED(200);	//1~500
	RUNZE_SETHOLE(1);	//1~10
	RUNZE_SET_ML(0);	//0~6
	osDelay(1000);
	for (j = 0; j < k; j++)
	{
		for (i = 0; i < 2; i++)	//3改为2
		{
			OpenValves(0x0010); // 蒸馏水阀
			RUNZE_SETHOLE(4);	//1~10
			RUNZE_SET_ML(6);	//0~6
			// 命令13：
			// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			// pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
			// pMbHoldData->nLevelNum = 2;				 // 液位
			// pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			CloseAllValves(); // 关组合阀
			osDelay(500);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			RUNZE_SETHOLE(3);	//1~10
			RUNZE_SET_ML(0);	//0~6

			// Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			osDelay(500);
		}
		RZPaikongbsm(0, 0); // 高锰排空7次，其它3次
		osDelay(500);
	}
	pMbHoldData->RW580 = 20;
	// 装满比色皿
	CloseAllValves(); // 关组合阀
	for (i = 0; i < 2; i++)
	{
		OpenValves(0x0010); // 蒸馏水阀
		RUNZE_SETHOLE(4);	//1~10
		RUNZE_SET_ML(6);	//0~6
		// 命令13：
		// pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		// pMbHoldData->nStepperMotorSpeed = 600;	 // 速度
		// pMbHoldData->nLevelNum = 2;				 // 液位
		// pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves(); // 关组合阀
		osDelay(500);
		OpenValves(0x0308);	  // 开比色阀，消解上+下阀
		RUNZE_SETHOLE(3);	//1~10
		RUNZE_SET_ML(0);	//0~6

		// Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
		CloseAllValves();	  // 关组合阀
		osDelay(500);
	}

	// 1918
	pMbInputData->nStepCode = 19; // 取比色电压V1
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	// 命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	i = 0;
	j = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp >= (50 - 0.1))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
	OpenValves(0x0400);				 // 开风扇
	i = 0;
	j = 0;
	pMbHoldData->RW580 = 21;
	while (1) // 1985
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (50 - 0.5))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	CloseValves(0x0400); // 风扇
	ProcReadYSADs(10);	 // 命令20
	osDelay(1000);
	sum = 0;
	for (i = 0; i < 10; i++) // 2062,取10次平均值
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	pMbInputData->RW82 = sum; // V1比色电压
	pMbInputData->nSysErrorCode = 0;
	osDelay(5000);
	pMbHoldData->RW580 = 100; // 下位机结束标志

	Buzzer_ON_MS(500);
	return 0;
}

uint16_t START_TN(void) // TN主流程.触摸屏主流程167行启动
{
	uint16_t i = 0, j = 0, k = 0;
	bool flag = false;
	pMbHoldData->RW580 = 0; // 下位机结束标志
	// 原流程157行开始。1912行结束。主要获取2个比色电压+6个光谱仪信号
	CloseAllValves(); // 157行，关组合阀

	pMbInputData->nStepCode = 1; // SetWord(@运行日志@,0,1);//排空管路
	osDelay(500);
	OpenValves(0x1000);	  // 开废液阀
	Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x1000);  // 关废液阀
	pMbHoldData->RW580 = 1;
	// 227行，排空比色皿
	Paikongbsm(0, 0); // 高锰排空7次，其它3次
	// A插入
	Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	Pump_run(1000, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0308); // 关比色阀，消解上+下阀

	OpenValves(0x0001); // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 250); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}

	pMbHoldData->RW580 = 2;

	// 357行
	i = pMbHoldData->RW50;
	if (i == 8 || i == 19 || i == 21 || i == 24 || i == 29 || i == 31 || i == 33 || i == 42)
	{
		pMbHoldData->RW580 = 3;
		pMbInputData->nStepCode = 2; // SetWord(@运行日志@,0,2);//零点标定清洗计管
		osDelay(500);
		for (i = 0; i < 3; i++)
		{
			// 397行
			CloseValves(0x0010); // 关蒸馏水
			OpenValves(0x0001);	 // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0001);  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				CloseValves(0x1000); // 开废液阀
			}
		}
		Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
		OpenValves(0x0001);	  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		Pump_run(-1200, 150); // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0001);  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			CloseValves(0x1000); // 开废液阀
		}
		pMbInputData->nStepCode = 3; // SetWord(@运行日志@,0,3);//零点标定液位校准
		osDelay(500);
		// 530行
		i = 0;
		while (1)
		{
			pMbHoldData->RW580 = 4;
			CheckLevelsAD(); // 108命令
			osDelay(1000);
			if (pMbInputData->nLL1AD >= 1200 && pMbInputData->nLL1AD <= 1400 && pMbInputData->nLL2AD >= 1200 && pMbInputData->nLL2AD <= 1400)
			{
				break;
			}
			else
			{
				if (i++ >= 3)
				{
					pMbHoldData->RW4230 = 1; // 故障代码=1
					return 1;
				}
			}
		}
		pMbHoldData->RW30 = pMbInputData->nLL1AD;
		pMbHoldData->RW31 = pMbInputData->nLL2AD;
		pMbHoldData->RW32 = pMbInputData->nLL1AD * 1.2;
		pMbHoldData->RW33 = pMbInputData->nLL2AD * 1.2;
		pMbHoldData->RW34 = pMbInputData->nLL1AD * 0.5;
		pMbHoldData->RW35 = pMbInputData->nLL2AD * 0.5;
		pMbHoldData->RW579 = 1;
		SaveUserData();
	}
	pMbHoldData->RW580 = 5;

	// 552行
	if (pMbHoldData->RW50 == 1 && pMbHoldData->RW36) // 工作类型==1
	{
		pMbInputData->nStepCode = 4; // 外接泵循环
		osDelay(500);
		OpenValves(0x4000); // 外接泵
		osDelay(pMbHoldData->RW36 * 1000);
		CloseValves(0x4000); // 外接泵
	}

	//562
	GPY(0);	//读取光谱仪暗电压
	if(pMbHoldData->nSpectro275AD > 65500 ||pMbHoldData->nSpectro220AD > 65500)
	{
		for(i = 0;i < 5;i ++)
		{
			Buzzer_ON_MS(200);
			osDelay(200);
		}
		GPY(0);	//读取光谱仪暗电压
	}
	pMbInputData->RW426 = pMbHoldData->nSpectro275AD;	//保存暗信号
	pMbInputData->RW427 = pMbHoldData->nSpectro220AD;

	// 618
	pMbHoldData->fTemp1Kp = 500;
	pMbHoldData->fTemp1Ki = 0.3;
	pMbHoldData->fTemp1Kd = 2000;
	pMbHoldData->nTestTemper = 50;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 50度保温
	pMbHoldData->RW580 = 6;

	// 626行
	if (Caiqianpd()) // 采前判断
	{
		return 1;
	}
	pMbHoldData->RW580 = 7;

	// 654行，润洗2次
	if (Runxi())
	{
		return 1;
	}
	pMbHoldData->RW580 = 8;

	// 818行，采样
	if (Caiyang(1, 0)) // 采样
	{
		return 1;
	}
	pMbHoldData->RW580 = 9;

	Fanhui(); // 水样/其它样返回
	pMbHoldData->RW580 = 10;

	// 877行，稀释
	if (Xishi_TN()) // 稀释
	{
		return 1;
	}
	pMbHoldData->RW580 = 11;

	// 1209，第六页
	pMbInputData->nStepCode = 12; // 采试剂一
	osDelay(500);
	i = Caidingbsm(4, 600, -40, 1, 400, 20); // 采定液体到比色皿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 12;

	// 1237
	pMbInputData->nStepCode = 11; // 采蒸馏水
	osDelay(500);
	if(pMbHoldData->RW45 == 0)	//量程1采低液位，其它采高
	{
		j = 1;
	}
	else
	{
		j = 2;
	}
	i = Caidingbsm(16, 600, -40, j, 400, 10); // 采定液体到比色皿
	if (i)
	{
		return 1;
	}

	if(pMbHoldData->RW45 == 1 || pMbHoldData->RW45 == 2)	//量程2 3 再采1管高液位
	{
		i = Caidingbsm(16, 600, -40, 2, 400, 20); // 采定液体到比色皿
		if (i)
		{
			return 1;
		}
	}

	pMbHoldData->RW580 = 13;

	// // 1497，浊度补偿,COD不用浊度补偿
	// i = Zhuodubc(); // 浊度补偿
	// if (i)
	// {
	// 	return 1;
	// }
	// pMbHoldData->RW580 = 14;

	// 1306
	pMbInputData->nStepCode = 14; // 采试剂二
	osDelay(500);
	j = Caidingbsm(32, 600, -40, 1, 400, 20); // 采定液体到比色皿
	if (j)
	{
		return 1;
	}
	// 1638试剂二返回
	CloseAllValves();
	OpenValves(32);	   // 试剂二阀
	Pump_run(-600, 2); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();
	pMbHoldData->RW580 = 15;

	// 1329
	pMbInputData->nStepCode = 21; // 加热消解，10分钟
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL);
	// 命令16，为阻塞函数，加热时间结束才会往下走。
	pMbHoldData->fTemp1Kp = 3600;
	pMbHoldData->fTemp1Ki = 8;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 120; // 120度，10分钟
	pMbHoldData->nTestTemperHoldtime = 10;
	flag = ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	if (flag == false) // 温控失败
	{
		pMbHoldData->RW580 = 16;
		if (pMbHoldData->fPoolTemp < (120 - 1))
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	pMbHoldData->RW580 = 17;

	// 1397，
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	pMbInputData->nStepCode = 22;	 // 冷却
	osDelay(500);
	OpenValves(0x0400); // 开风扇
	i = 0;
	j = 0;
	while (1) //
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (float)52)
			break;
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	osDelay(500);
	CloseValves(0x0400); // 风扇
	pMbHoldData->RW580 = 18;

	// 1470
	pMbInputData->nStepCode = 23; // 采试剂三
	osDelay(500);
	j = Caidingbsm(64, 600, -40, 1, 400, 10); // 采定液体到比色皿
	if (j)
	{
		return 1;
	}
	// 1638试剂三返回
	CloseAllValves();
	if (pMbHoldData->RW48)
	{
		OpenValves(64);					   // 试剂三阀
		Pump_run(-600, pMbHoldData->RW48); // 命令12，操作蠕动泵，速度，圈数
		CloseAllValves();
	}

	//1496
	pMbInputData->nStepCode = 16; // 取比色电压V2
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	//  命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);

	i = 0;
	j = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp >= (50 - 0.1))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
	OpenValves(0x0400);				 // 开风扇
	i = 0;
	j = 0;
	while (1) // 1571
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (50 - 0.3))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	osDelay(200);
	CloseValves(0x0400); // 风扇

	i = 0;
	j = 0;
	while (1) // 1602
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (50 - 0.5))
			break;
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
	}

	GPY(1);
	if(pMbHoldData->nSpectro275AD > 65500 ||pMbHoldData->nSpectro220AD > 65500)
	{
		for(i = 0;i < 5;i ++)
		{
			Buzzer_ON_MS(200);
			osDelay(200);
		}
		GPY(1);	//读取光谱仪暗电压
	}
	pMbInputData->RW428 = pMbHoldData->nSpectro275AD;	//保存反应液信号
	pMbInputData->RW429 = pMbHoldData->nSpectro220AD;

	pMbHoldData->RW580 = 19;

	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	// 增加保温
	//  命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);

	// 1689
	pMbInputData->nStepCode = 17; // 排反应液
	osDelay(500);
	Paikongbsm(0, 0); // 高锰排空7次，其它3次

	pMbInputData->nStepCode = 18; // 清洗
	osDelay(500);
	CloseAllValves();	   // 关组合阀
	if (pMbHoldData->RW84) // 清洗次数
	{
		k = pMbHoldData->RW84;
	}
	else
	{
		k = 1;
	}
	for (j = 0; j < k; j++)
	{
		for (i = 0; i < 3; i++)
		{
			OpenValves(0x0010); // 蒸馏水阀
			// 命令13：
			pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			pMbHoldData->nStepperMotorSpeed = 1000;	 // 速度
			pMbHoldData->nLevelNum = 2;				 // 液位
			pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			CloseAllValves(); // 关组合阀
			osDelay(500);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			osDelay(500);
		}
		Paikongbsm(0, 0); // 高锰排空7次，其它3次
		osDelay(500);
	}
	pMbHoldData->RW580 = 20;
	// 装满比色皿
	CloseAllValves(); // 关组合阀
	for (i = 0; i < 3; i++)
	{
		OpenValves(0x0010); // 蒸馏水阀
		// 命令13：
		pMbHoldData->nPeristalticPumpParm = 400; // 圈数
		pMbHoldData->nStepperMotorSpeed = 1000;	 // 速度
		pMbHoldData->nLevelNum = 2;				 // 液位
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		CloseAllValves(); // 关组合阀
		osDelay(500);
		OpenValves(0x0308);	  // 开比色阀，消解上+下阀
		Pump_run(-1200, 300); // 命令12，操作蠕动泵，速度，圈数
		CloseAllValves();	  // 关组合阀
		osDelay(500);
	}

	// 1918
	pMbInputData->nStepCode = 19; // 取比色电压V1
	osDelay(500);
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	// 命令17
	pMbHoldData->fTemp1Kp = 3800;
	pMbHoldData->fTemp1Ki = 6;
	pMbHoldData->fTemp1Kd = 800;
	pMbHoldData->nTestTemper = 50; // 50度保温
	pMbHoldData->nTestTemperHoldtime = 0;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	i = 0;
	j = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp >= (50 - 0.1))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
	OpenValves(0x0400);				 // 开风扇
	i = 0;
	j = 0;
	pMbHoldData->RW580 = 21;
	while (1) // 1985
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= (50 - 0.5))
			break;
		else if (i > 60 * 5 && pMbHoldData->fPoolTemp < 50 / 2 && j == 0)
		{
			j = 1;
			// 命令17
			pMbHoldData->fTemp1Kp = 3800;
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			pMbHoldData->nTestTemper = 50; // 50度保温
			pMbHoldData->nTestTemperHoldtime = 0;
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		}
		else if (i > 60 * 25)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	CloseValves(0x0400); // 风扇
	GPY(1);
	if(pMbHoldData->nSpectro275AD > 65500 ||pMbHoldData->nSpectro220AD > 65500)
	{
		for(i = 0;i < 5;i ++)
		{
			Buzzer_ON_MS(200);
			osDelay(200);
		}
		GPY(1);	//读取光谱仪暗电压
	}
	pMbInputData->RW430 = pMbHoldData->nSpectro275AD;	//保存反应液信号
	pMbInputData->RW431 = pMbHoldData->nSpectro220AD;
	
	pMbInputData->nSysErrorCode = 0;
	osDelay(5000);
	pMbHoldData->RW580 = 100; // 下位机结束标志

	Buzzer_ON_MS(500);
	return 0;
}

uint16_t START_CODMN(void) // CODMN主流程.触摸屏主流程153行启动
{
	u32 i = 0, j = 0, k = 0;
	u32 sum = 0;
	bool flag = false;
	int32_t last_atti = 0, now_atti = 0;
	u32 voltage_1 = 0, voltage_2 = 0;
	int32_t start_atti = 0, stop_atti = 0, sum_atti = 0;

	pMbHoldData->RW580 = 0; // 下位机结束标志
	// 原流程153行开始。2072行结束。主要获取滴定步数。
	CloseAllValves(); // 173行，关组合阀

	pMbHoldData->nBlankPipeLowTHR = 700; // 空管下限
	pMbHoldData->nBlankPipeUpTHR = 1300; // 空管上限

	pMbInputData->nStepCode = 58; // SetWord(@运行日志@,0,1);//注射泵复位
	osDelay(500);
	CloseValves(0x40000); // 三通阀
	StopYsSyringe();	  // 命令28，注射泵停止。
	osDelay(1000);
	i = 0;
	do
	{
		osDelay(500);
		i++;
		// 命令27，绝对运动
		pMbHoldData->nSyringePumpSpeed = 301;
		pMbHoldData->nSyringePumpMoveTo = -1000;
		SetYsSyringeSpeed();
		osDelay(200);
		MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
		osDelay(200);
		last_atti = (int32_t)ReadPos();
		osDelay(500);
		now_atti = (int32_t)ReadPos();
	} while (now_atti == last_atti && now_atti != 0 && i < 600);

	i = 0;
	while (1) // 归0点
	{
		osDelay(1000);
		i++;
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		if (pMbInputData->nSyringePumpPos < 10 && pMbInputData->nSyringePumpPos >= 0)
		{
			break;
		}
		if (i > 120)
		{
			pMbInputData->nSysErrorCode = 30; // 注射泵复位异常
			return 1;
		}
		if (i > 60 || pMbInputData->nSyringePumpPos == -2)
		{
			// 命令27，绝对运动
			pMbHoldData->nSyringePumpSpeed = 302;
			pMbHoldData->nSyringePumpMoveTo = -1000;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
			osDelay(200);
		}
	}

	OpenValves(0x40000); // 三通阀
	i = 0;
	do
	{
		osDelay(500);
		i++;
		// 命令27，绝对运动
		pMbHoldData->nSyringePumpSpeed = 303;
		pMbHoldData->nSyringePumpMoveTo = 48000;
		SetYsSyringeSpeed();
		osDelay(200);
		MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
		osDelay(200);
		last_atti = (int32_t)ReadPos();
		osDelay(500);
		now_atti = (int32_t)ReadPos();
	} while (now_atti == last_atti && now_atti < 47990 && i < 600);

	osDelay(500);
	while (1) // 272，归48000点
	{
		osDelay(1000);
		i++;
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		if (pMbInputData->nSyringePumpPos > 47990)
		{
			osDelay(1000);
			break;
		}
		if (i > 180)
		{
			pMbInputData->nSysErrorCode = 30; // 注射泵复位异常
			return 1;
		}
		if (i > 120 || pMbInputData->nSyringePumpPos == -2)
		{
			osDelay(1000);
			// 命令27，绝对运动
			pMbHoldData->nSyringePumpSpeed = 304;
			pMbHoldData->nSyringePumpMoveTo = 48000;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
			osDelay(200);
		}
	}
	CloseValves(0x40000); // 三通阀

	pMbInputData->nStepCode = 1; // 排空比色皿
	OpenValves(0x1000);			 // 开废液阀
	Pump_run(-1500, 250);		 // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x1000);		 // 关废液阀
	pMbHoldData->RW580 = 1;
	// 227行，排空比色皿
	Paikongbsm(0, 1); // 高锰排空9次，其它3次
	// A插入
	Pump_run(1300, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	Pump_run(1300, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0308); // 关比色阀，消解上+下阀

	OpenValves(0x0001); // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1500, 250); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}
	Pump_run(-1500, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		OpenValves(0x1000); // 开废液阀
	}
	Pump_run(-1500, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseValves(0x0001);  // 开清洗废液阀
	if (pMbHoldData->RW577)
	{
		CloseValves(0x1000); // 开废液阀
	}

	pMbHoldData->RW580 = 2;

	// 358行
	i = pMbHoldData->RW50;
	if (i == 8 || i == 19 || i == 21 || i == 24 || i == 29 || i == 31 || i == 33 || i == 42)
	{
		pMbHoldData->RW580 = 3;
		pMbInputData->nStepCode = 2; // SetWord(@运行日志@,0,2);//零点标定清洗计管
		osDelay(500);
		for (i = 0; i < 3; i++)
		{
			// 420行
			CloseValves(0x0010); // 关蒸馏水
			OpenValves(0x0001);	 // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				OpenValves(0x1000); // 开废液阀
			}
			Pump_run(-1500, 200); // 命令12，操作蠕动泵，速度，圈数
			CloseValves(0x0001);  // 开清洗废液阀
			if (pMbHoldData->RW577)
			{
				CloseValves(0x1000); // 开废液阀
			}
		}
		Pump_run(-1500, 150); // 命令12，操作蠕动泵，速度，圈数
		OpenValves(0x0001);	  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			OpenValves(0x1000); // 开废液阀
		}
		Pump_run(-1500, 150); // 命令12，操作蠕动泵，速度，圈数
		CloseValves(0x0001);  // 开清洗废液阀
		if (pMbHoldData->RW577)
		{
			CloseValves(0x1000); // 开废液阀
		}
		pMbInputData->nStepCode = 3; // SetWord(@运行日志@,0,3);//零点标定液位校准
		osDelay(500);
		// 530行
		i = 0;
		while (1)
		{
			pMbHoldData->RW580 = 4;
			CheckLevelsAD(); // 108命令
			osDelay(1000);
			if (pMbInputData->nLL1AD >= 1200 && pMbInputData->nLL1AD <= 1400 && pMbInputData->nLL2AD >= 1200 && pMbInputData->nLL2AD <= 1400)
			{
				break;
			}
			else
			{
				if (i++ >= 3)
				{
					pMbHoldData->RW4230 = 1; // 故障代码=1
					return 1;
				}
			}
		}
		pMbHoldData->RW30 = pMbInputData->nLL1AD;
		pMbHoldData->RW31 = pMbInputData->nLL2AD;
		pMbHoldData->RW32 = pMbInputData->nLL1AD * 1.2;
		pMbHoldData->RW33 = pMbInputData->nLL2AD * 1.2;
		pMbHoldData->RW34 = pMbInputData->nLL1AD * 0.5;
		pMbHoldData->RW35 = pMbInputData->nLL2AD * 0.5;
		pMbHoldData->RW579 = 1;
		SaveUserData();
	}
	pMbHoldData->RW580 = 5;

	pMbInputData->nStepCode = 24; // 530，洗滴定管
	for (i = 0; i < 3; i++)
	{
		if (ChouWaterbdwbsm()) // 抽蒸馏水不定位到比色皿
			return 1;
		osDelay(500);
	}
	for (i = 0; i < 3; i++) // 550，设置48000为零点
	{
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		now_atti = pMbInputData->nSyringePumpPos;
		osDelay(500);
		if (now_atti != 0 && now_atti != -2)
		{
			ResetYsSyringeOrigin(); // 命令25，注射泵复位
			osDelay(500);
		}
		else if(now_atti == -2)
		{
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = 1;
			pMbHoldData->nSyringePumpSpeed = 30;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
		else
		{
			break;
		}
	}

	//566
	// 命令26，相对运动
	pMbHoldData->nSyringePumpMove = -3000; // 48000-3000=45000
	pMbHoldData->nSyringePumpSpeed = 30;
	SetYsSyringeSpeed();
	osDelay(200);
	MoveYsSyringe(pMbHoldData->nSyringePumpMove);
	osDelay(200);
	i = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		if (pMbInputData->nSyringePumpPos <= -2990)
		{
			osDelay(1000);
			break;
		}
		if (i > 300)
		{
			pMbInputData->nSysErrorCode = 30; // 注射泵复位异常
			return 1;
		}
		if (i > 60 && pMbInputData->nSyringePumpPos == -2)
		{
			osDelay(200);
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = 1;
			pMbHoldData->nSyringePumpSpeed = 30;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
	}

	if (Caiqianpd()) // 617，采前判断
	{
		return 1;
	}

	// 623，从比色皿抽回一管
	CloseAllValves();
	OpenValves(0x0308); // 开比色阀，消解上+下阀
	// 命令13：
	pMbHoldData->nPeristalticPumpParm = 400; // 圈数
	pMbHoldData->nStepperMotorSpeed = 1300;	 // 速度
	pMbHoldData->nLevelNum = 2;				 // 液位
	pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
	CloseAllValves(); // 关组合阀
	osDelay(500);

	// 666,注射泵运动到48000
	for (i = 0; i < 3; i++) // 设置48000为零点
	{
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		now_atti = pMbInputData->nSyringePumpPos;
		osDelay(500);
		if (now_atti != 0 && now_atti != -2)
		{
			ResetYsSyringeOrigin(); // 命令25，注射泵复位
			osDelay(500);
		}
		else if(now_atti == -2)
		{
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = 1;
			pMbHoldData->nSyringePumpSpeed = 30;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
		else
		{
			break;
		}
	}

	// 683,命令26，相对运动
	pMbHoldData->nSyringePumpMove = 3000; // 48000-3000=45000
	pMbHoldData->nSyringePumpSpeed = 31;
	SetYsSyringeSpeed();
	osDelay(200);
	MoveYsSyringe(pMbHoldData->nSyringePumpMove);
	osDelay(200);
	i = 0;
	j = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		if (pMbInputData->nSyringePumpPos >= 2990)
		{
			break;
		}
		if (pMbInputData->nSyringePumpPos == 0 && i > 120 && j == 0)
		{
			j = 1;
			osDelay(500);
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = 3000;
			pMbHoldData->nSyringePumpSpeed = 31;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
		if (i > 300)
		{
			pMbInputData->nSysErrorCode = 30; // 注射泵复位异常
			return 1;
		}
		if (i > 60 && pMbInputData->nSyringePumpPos == -2)
		{
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = 1;
			pMbHoldData->nSyringePumpSpeed = 30;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
	}
	// 731，排掉计量管
	OpenValves(0x1000);	  // 开废液阀
	Pump_run(-1500, 300); // 命令12
	CloseAllValves();
	Paikongbsm(0, 1); // 高锰排空7次，其它3次

	pMbInputData->nStepCode = 18; // 清洗
	if (pMbHoldData->RW84 > 0)	  // 清洗次数
	{
		for (j = 0; j < pMbHoldData->RW84; j++)
		{
			for (i = 0; i < 6; i++)
			{
				if (ChouWaterbdwbsm()) // 抽蒸馏水不定位到比色皿
					return 1;
			}
			if ((j + 1) >= pMbHoldData->RW84) // 最后一次循环，排空管路
				pMbInputData->nStepCode = 1;  // 排空管路
			Paikongbsm(1, 1);				  // 高锰排空7次，其它3次
		}
	}

	// 811，A插入。负压抽比色皿？
	CloseAllValves();
	Pump_run(1300, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0308);	 // 开比色阀，消解上+下阀
	Pump_run(1300, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();
	OpenValves(0x0001);	  // 清洗废液阀
	Pump_run(-1500, 250); // 命令12，操作蠕动泵，速度，圈数

	// 888，计量管增压再泄压
	CloseAllValves();
	Pump_run(-1500, 150); // 命令12，操作蠕动泵，速度，圈数
	OpenValves(0x0001);	  // 清洗废液阀
	Pump_run(-1500, 150); // 命令12，操作蠕动泵，速度，圈数
	CloseAllValves();

	pMbHoldData->fTemp1Kp = 800;
	pMbHoldData->fTemp1Ki = 2;
	pMbHoldData->fTemp1Kd = 200;
	pMbHoldData->nTestTemper = 50; // 50度保温
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 50度保温
	pMbHoldData->RW580 = 6;

	// 940行
	if (pMbHoldData->RW50 == 1 && pMbHoldData->RW36) // 工作类型==1
	{
		pMbInputData->nStepCode = 4; // 外接泵循环
		osDelay(500);
		OpenValves(0x4000); // 外接泵
		osDelay(pMbHoldData->RW36 * 1000);
		CloseValves(0x4000); // 外接泵
	}

	// 626行
	if (Caiqianpd()) // 采前判断
	{
		return 1;
	}
	pMbHoldData->RW580 = 7;

	// 632行，润洗2次
	if (Runxi())
	{
		return 1;
	}
	pMbHoldData->RW580 = 8;

	// 1143行，采样
	if (Caiyang_CODMN(0)) // 采样
	{
		return 1;
	}

	pMbHoldData->RW580 = 9;

	Fanhui(); // 1218，样/其它样返回
	pMbHoldData->RW580 = 10;

	// 1276
	pMbInputData->nStepCode = 14; // 采试剂二
	osDelay(500);
	i = Caidingbsm(32, 1000, -40, 1, 400, 10); // 采定液体到比色皿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 11;

	// 1300，第六页
	pMbInputData->nStepCode = 12; // 采试剂一
	osDelay(500);
	pMbHoldData->nBlankPipeLowTHR = 940; // 空管下限
	pMbHoldData->nBlankPipeUpTHR = 1060; // 空管上限

	i = Caidingbsm(4, 800, -40, 1, 400, 10); // 采定液体到比色皿
	if (i)
	{
		return 1;
	}
	pMbHoldData->RW580 = 12;

	pMbHoldData->nBlankPipeLowTHR = 700; // 空管下限
	pMbHoldData->nBlankPipeUpTHR = 1300; // 空管上限

	// 开搅拌
	pMbHoldData->nStirWorkTime = 30;   // 搅拌时长
	pMbHoldData->nStirSleepTime = 700; // 停搅拌时长
	FlagStir = true;				   // 开启搅拌

	// 1332
	pMbInputData->nStepCode = 21; // 加热消解，97度，15分钟
	osDelay(500);


	StopTempCtrl(TEMP_CTRL_CH_POOL);
	// 命令16，为阻塞函数，加热时间结束才会往下走。
	pMbHoldData->fTemp1Kp = 4500;
	pMbHoldData->fTemp1Ki = 8;
	pMbHoldData->fTemp1Kd = 2500;
	pMbHoldData->nTestTemper = 97; // 97度，15分钟
	pMbHoldData->nTestTemperHoldtime = 10;
	flag = ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
	if (flag == false) // 温控失败
	{
		pMbHoldData->RW580 = 13;
		if (pMbHoldData->fPoolTemp < (50 - 1))
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	pMbHoldData->RW580 = 14;

	// 1408
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	pMbInputData->nStepCode = 22;	 // 冷却
	osDelay(500);
	OpenValves(0x0400); // 开风扇
	i = 0;
	j = 0;
	while (1) //
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp <= 80)
			break;
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 33; // 温度不能下降
			return 1;
		}
		StopTempCtrl(TEMP_CTRL_CH_POOL); // 停止温控
		OpenValves(0x0400);				 // 开风扇
	}
	osDelay(500);
	CloseValves(0x0400); // 风扇
	pMbHoldData->RW580 = 15;

	pMbHoldData->fTemp1Kp = 4500;
	pMbHoldData->fTemp1Ki = 15;
	pMbHoldData->fTemp1Kd = 300;
	pMbHoldData->nTestTemper = 76;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 76度保温

	// 1474
	pMbInputData->nStepCode = 23; // 采试剂三
	osDelay(500);
	j = Caidingbsm(64, 1000, -40, 1, 400, 10); // 采定液体到比色皿
	if (j)
	{
		return 1;
	}
	// 1638试剂三返回
	CloseAllValves();
	if (pMbHoldData->RW48)
	{
		OpenValves(64);					   // 试剂三阀
		Pump_run(-1200, pMbHoldData->RW48); // 命令12，操作蠕动泵，速度，圈数
		CloseAllValves();
	}

	pMbHoldData->RW580 = 16;

	// 1497
	pMbInputData->nStepCode = 11; // 采蒸馏水
	osDelay(500);
	j = Caidingbsm(16, 1300, -40, 1, 400, 10); // 采定液体到比色皿
	if (j)
	{
		return 1;
	}
	pMbHoldData->RW580 = 17;

	// 1531
	pMbInputData->nStepCode = 25; // 保温
	i = 0;
	j = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		if (pMbHoldData->fPoolTemp >= (76 - 0.2))
			break;
		else if (i > 60 * 10)
		{
			pMbInputData->nSysErrorCode = 23; // 温度不能上升
			return 1;
		}
	}
	osDelay(20000);

	// 1586
	pMbInputData->nStepCode = 26; // 滴定
	pMbHoldData->RW580 = 18;

	ProcReadYSADs(10); // 1565，读取光强值
	sum = 0;
	for (i = 0; i < 10; i++)
	{
		sum += pMbInputData->nPoolTranLtAD;
		osDelay(200);
	}
	sum /= 10;
	voltage_1 = sum; // 滴定前电压
	pMbInputData->RW509 = voltage_1;

 	// 1626,复位。设置48000为零点
	for (i = 0; i < 3; i++) // 设置48000为零点
	{
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		now_atti = pMbInputData->nSyringePumpPos;
		osDelay(500);
		if (now_atti != 0 && now_atti != -2)
		{
			ResetYsSyringeOrigin(); // 命令25，注射泵复位
			osDelay(500);
		}
		else if(now_atti == -2)
		{
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = -1;
			pMbHoldData->nSyringePumpSpeed = 30;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
		else
		{
			break;
		}
	}
	// start_atti = (int32_t)ReadPos(); 
	start_atti = 0;// 滴定起始位置

	// 1643,命令26，相对运动
	pMbHoldData->nSyringePumpMove = -2500; // 48000-3000=45000
	pMbHoldData->nSyringePumpSpeed = 20;
	SetYsSyringeSpeed();
	osDelay(200);
	MoveYsSyringe(pMbHoldData->nSyringePumpMove);
	osDelay(200);
	i = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		if (pMbInputData->nSyringePumpPos <= -2490)
		{
			break;
		}
		if (i > 300)
		{
			pMbInputData->nSysErrorCode = 30; // 注射泵复位异常
			return 1;
		}
		if (i > 60 && pMbInputData->nSyringePumpPos == -2)
		{
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = 1;
			pMbHoldData->nSyringePumpSpeed = 20;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
	}
	pMbHoldData->RW580 = 19;
	osDelay(500);
	// 1704,命令26，相对运动
	pMbHoldData->nSyringePumpMove = -1000; // 48000-3000=45000
	pMbHoldData->nSyringePumpSpeed = 5;
	SetYsSyringeSpeed();
	osDelay(200);
	MoveYsSyringe(pMbHoldData->nSyringePumpMove);
	osDelay(200);
	i = 0;
	while (1)
	{
		osDelay(1000);
		i++;
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		if (pMbInputData->nSyringePumpPos <= -3490)
		{
			break;
		}
		if (i > 300)
		{
			pMbInputData->nSysErrorCode = 30; // 注射泵复位异常
			return 1;
		}
		if (i > 60 && pMbInputData->nSyringePumpPos == -2)
		{
			osDelay(500);
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = -1;
			pMbHoldData->nSyringePumpSpeed = 20;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
	}
	osDelay(500);
	pMbHoldData->RW580 = 20;
	i = 0;
	do
	{
		osDelay(500);
		i++;
		// 命令27，绝对运动
		pMbHoldData->nSyringePumpSpeed = 8;
		pMbHoldData->nSyringePumpMoveTo = -1000;
		SetYsSyringeSpeed();
		osDelay(200);
		MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
		osDelay(200);
		last_atti = (int32_t)ReadPos();
		osDelay(1000);
		now_atti = (int32_t)ReadPos();
	} while (now_atti == last_atti && now_atti != 0  && now_atti != -2 && i < 300);
	i = 0;
	j = 0;
	while (1)
	{
		osDelay(200);
		i++;
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		now_atti = pMbInputData->nSyringePumpPos;
		if (now_atti == 0)
		{
			pMbInputData->nSysErrorCode = 31; //
			return 1;
		}
		if (pMbInputData->nPoolTranLtAD <= (uint16_t)((float)voltage_1 * (float)0.985))
		{
			// StopYsSyringe(); // 命令28，注射泵停止。
			// osDelay(1000);
			break;
		}
		if (i > 300 * 5 && j == 0)
		{
			// 命令27，绝对运动
			pMbHoldData->nSyringePumpSpeed = 8;
			pMbHoldData->nSyringePumpMoveTo = -1000;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
			osDelay(200);
			j = 1;
		}
		if (i > 600 * 5)
		{
			break;
		}
		if (now_atti == -2)
		{
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = 1;
			pMbHoldData->nSyringePumpSpeed = 8;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
	}
	// 1841
	pMbHoldData->RW580 = 21;
	StopYsSyringe(); // 命令28，注射泵停止。
	osDelay(3000);
	i = 0;
	do
	{
		osDelay(500);
		i++;
		// 命令27，绝对运动
		pMbHoldData->nSyringePumpSpeed = 1;
		pMbHoldData->nSyringePumpMoveTo = -1000;
		SetYsSyringeSpeed();
		osDelay(200);
		MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
		osDelay(200);
		last_atti = (int32_t)ReadPos();
		osDelay(1000);
		now_atti = (int32_t)ReadPos();
	} while (now_atti == last_atti && now_atti != 0 && now_atti != -2 && i < 100);
	i = 0;
	j = 0;
	k = 0;
	while (1)
	{
		osDelay(100);
		i++;
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		now_atti = pMbInputData->nSyringePumpPos;
		if (now_atti == 0)
		{
			pMbInputData->nSysErrorCode = 31; //
			return 1;
		}
		if (pMbInputData->nPoolTranLtAD <= (uint16_t)((float)voltage_1 * (float)0.95))
		{
			voltage_2 = pMbInputData->nPoolTranLtAD; // 滴定结束电压
			pMbInputData->RW554 = voltage_2;
			k = 0;
			while (now_atti == -2 && k < 20)
			{
				k ++;
				// 命令26，相对运动
				pMbHoldData->nSyringePumpMove = 1;
				pMbHoldData->nSyringePumpSpeed = 1;
				SetYsSyringeSpeed();
				osDelay(200);
				MoveYsSyringe(pMbHoldData->nSyringePumpMove);
				osDelay(1000);
				now_atti = (int32_t)ReadPos();
				osDelay(500);
			}
			stop_atti = now_atti;
			pMbInputData->RW512 = stop_atti;
			break;
		}
		if (i > 300 * 20 && j == 0)
		{
			// 命令27，绝对运动
			pMbHoldData->nSyringePumpSpeed = 3;
			pMbHoldData->nSyringePumpMoveTo = -1000;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
			osDelay(200);
			j = 1;
		}
		if (i > 600 * 20)
		{
			break;
		}
		if (now_atti == -2)
		{
			// 命令26，相对运动
			pMbHoldData->nSyringePumpMove = 1;
			pMbHoldData->nSyringePumpSpeed = 1;
			SetYsSyringeSpeed();
			osDelay(200);
			MoveYsSyringe(pMbHoldData->nSyringePumpMove);
			osDelay(200);
		}
	}

	pMbHoldData->RW580 = 22;
	// 1929
	sum_atti = start_atti - stop_atti - 3000; // 滴定总步数
	if (sum_atti < 0)
	{
		pMbInputData->nSysErrorCode = 31; //滴定超限
		StopYsSyringe();				 // 命令28，注射泵停止。
		return 1;
	}
	if (sum_atti > 45000)
		sum_atti = 45000;
	pMbInputData->RW514 = sum_atti;

	StopYsSyringe();				 // 命令28，注射泵停止。
	osDelay(1000);
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控
	pMbHoldData->fTemp1Kp = 4500;
	pMbHoldData->fTemp1Ki = 10;
	pMbHoldData->fTemp1Kd = 100;
	pMbHoldData->nTestTemper = 50;
	ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, pMbHoldData->nTestTemper, 60); // 50度保温
	pMbHoldData->RW580 = 23;

	// 1853
	pMbInputData->nStepCode = 17; // 排反应液
	osDelay(500);
	Paikongbsm(0, 1); // 高锰排空7次，其它3次
	pMbHoldData->RW580 = 24;

	pMbInputData->nStepCode = 18; // 清洗
	osDelay(500);
	CloseAllValves();	   // 关组合阀
	if (pMbHoldData->RW84) // 清洗次数
	{
		k = pMbHoldData->RW84;
	}
	else
	{
		k = 1;
	}
	for (j = 0; j < k; j++)
	{
		for (i = 0; i < 5; i++)
		{
			OpenValves(0x0010); // 蒸馏水阀
			// 命令13：
			pMbHoldData->nPeristalticPumpParm = 400; // 圈数
			pMbHoldData->nStepperMotorSpeed = 1300;	 // 速度
			pMbHoldData->nLevelNum = 2;				 // 液位
			pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
			CloseAllValves(); // 关组合阀
			osDelay(500);
			OpenValves(0x0308);	  // 开比色阀，消解上+下阀
			Pump_run(-1500, 300); // 命令12，操作蠕动泵，速度，圈数
			CloseAllValves();	  // 关组合阀
			osDelay(500);
		}
		Paikongbsm(1, 1); // 高锰排空7次，其它3次
		osDelay(500);
	}
	pMbHoldData->RW580 = 25;

	FlagStir = false;				 // 停止搅拌
	StopTempCtrl(TEMP_CTRL_CH_POOL); // 命令18，停止温控

	pMbInputData->nSysErrorCode = 0;
	osDelay(5000);
	pMbHoldData->RW580 = 100; // 下位机结束标志

	Buzzer_ON_MS(500);
	return 0;
}
