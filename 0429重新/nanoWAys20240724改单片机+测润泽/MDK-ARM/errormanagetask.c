#include "errormanagetask.h"
#include "FlashUserData.h"

extern void Check_Self(void);


void ErrorCodeManageTask(void *argument)
{
	
	static uint8_t self_check_done = 1;	// 进入上电自检功能
	
	for (;;)
	{
		
		if(self_check_done != 0)
		{
			Check_Self();	// 上电清洗
			
			self_check_done = 0;	// 信号位置0，后面不会进入上电自检
			
			osDelay(100);
		}
		
		if(osSemaphoreAcquire(ErrorCodeSemHandle, osWaitForever) == osOK)
		{
			
			HandleError((nErrorCode)pMbInputData->nFunErrorCode);		// 处理错误代码
			osDelay(100);
		}
		osDelay(100);
	}
}

/*******************************************************************************
* Function Name  	: 发送错误或警告代码
* Description    	: 
* Input          	: 

* Output         	:
* Return         	:
*******************************************************************************/
void SendFunErrorCode(nErrorCode errorcode)
{
	if(errorcode != SYS_OK)
	{
		pMbInputData->nFunErrorCode = errorcode;	// 给对应错误寄存器赋值
		osDelay(200);
		osSemaphoreRelease(ErrorCodeSemHandle);  	// 通知警告或错误处理任务来处理
	}
}


/*******************************************************************************
* Function Name  	: 处理错误或警告代码
* Description    	: 
* Input          	: 

* Output         	:
* Return         	:
*******************************************************************************/
void HandleError(nErrorCode errorcode)	
{
	/***********下面对错误代码进行判断*************/
	
}

