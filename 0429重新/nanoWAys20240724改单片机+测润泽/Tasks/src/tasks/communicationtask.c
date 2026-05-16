
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "uartsrxtask.h"
#include "usart.h"
#include "relaysdef.h"
#include "ltc1867.h"
#include "peristalticpump.h"
#include "valvedef.h"
#include "tlc5620.h"
#include "bspuart3.h"
#include "CmdPars.h"

#ifndef FIREWARE_VISION
#define FIREWARE_VISION		0x0200
#endif
extern osSemaphoreId HostSEMHandle;

extern osThreadId CommTaskHandle;
extern osThreadId ProcTaskHandle;
extern osThreadId TempTaskHandle;
extern osThreadId DeterTaskHandle;

bool HostResponseFlag = false;




void UartsRxTask(void const * argument)
{
	StartRS232Rx();											//启动数据接收
	  for(;;)
  {
		if(osSemaphoreWait(HostSEMHandle , 5000) == osOK)
		{
			if(huart3.RxXferCount == 0)									//接收到一个完整数据包
			{
				RS232CmdProcess(RS232RxBuff);	
			}
			else if(huart3.RxXferCount == 8)						
			{
				if((RS232RxBuff[0] == 0xAA) && (RS232RxBuff[1] == 0x66) )				//接收到应答数据
				{
					HostResponseFlag = true;
				}
			}
			StartRS232Rx();									//启动数据接收
		}
  }
}

/*******************************************************************************
* Function Name  	: RS232CmdProcess
* Description    	: RS232命令数据包解析
* Input          	: 接收数据缓存地址
									
* Output         	:
* Return         	:
*******************************************************************************/
void RS232CmdProcess(uint8_t* rev)
{
	if(rev[0] != 0xAA)
		return;
	else if(rev[9] != XorFF(rev+1,8))			//校验
		return;
	else																	//数据解析成功
	{
		if(osMutexWait(osCMDdataMutex, osWaitForever) == osOK)
			HostData = *(HOST_DATA*)(rev+1);											//数据安全转移至公共buff
		if(osMutexRelease(osCMDdataMutex) != osOK)
		{};
		osSignalSet(ProcTaskHandle,USER_SIGNEL_COMM_CMD);				//通知命令处理进程
		ReplyHostSyn();
	}
	
}



