
#include "progressbar.h"
#include "replyhost.h"
#include "leddef.h"
#include "FlashUserData.h"
extern osTimerId ReportStepTimerHandle;
//extern osMailQId ReplyHostMailId;


 uint32_t ProcTotalTime = 40*60; //单位：秒
 uint32_t ProcStartTime;
 uint32_t ProcTimeElapsed;
 uint32_t ProcUpdataPeriod = 60000;

bool UpdateBuzyFlag = false;
//static STEP_NAME_CODE CurrentStepName = STEP_RESET;
//static CALIB_MODE CurrentCalibMode = CALIB_UNDO;

//static REPLY_HOST_DATA ProgressBarData;
//REPLY_HOST_DATA UpdateBarName;
//REPLY_HOST_DATA UpdateBarStep;



/*******************************************************************************
* Function Name  	: ProcUpdateStepStatus
* Description    	: 更新进度状态
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//void SetCrrentStepName(STEP_NAME_CODE stepname)
//{
//	ProgressBarData.nData[4] = stepname;  //初始化步骤名称
//}

/*******************************************************************************
* Function Name  	: UpdateStepStatus
* Description    	: 更新进度状态
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//void UpdateStepStatus(void)
//{
//	ProcTimeElapsed = xTaskGetTickCount();
//	ProcTimeElapsed -= ProcStartTime;
//	ProcTimeElapsed /= 10;
//	ProcTimeElapsed /= ProcTotalTime;	
//	pMbHoldData->nStepPercent = ProcTimeElapsed;

//}
/*******************************************************************************
* Function Name  	: UpdateStepStatus
* Description    	: 更新进度状态
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void UpdateStepName(STEP_NAME_CODE stepname)
{
	// pMbInputData->nStepCode = stepname;
	ProcTimeElapsed = xTaskGetTickCount();
	ProcTimeElapsed -= ProcStartTime;
	ProcTimeElapsed /= 10;
	ProcTimeElapsed /= ProcTotalTime;	
	pMbInputData->nStepPercent = ProcTimeElapsed;
}
	



/*******************************************************************************
* Function Name  	: ReportStepTimerCallback
* Description    	: 定时向上位机发进度信息
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ReportStepTimerCallback(void const * argument)  
{
	ProcTimeElapsed = xTaskGetTickCount();
	ProcTimeElapsed -= ProcStartTime;
	pMbInputData->nProcTimeElapsed = ProcTimeElapsed;
	ProcTimeElapsed /= 10;
	ProcTimeElapsed /= ProcTotalTime;	
	if(ProcTimeElapsed > 100)
		ProcTimeElapsed = 100;
	pMbInputData->nStepPercent = ProcTimeElapsed;

}
/*******************************************************************************
* Function Name  	: InitStepStatus
* Description    	: 初始化进度条相关信息
* Input          	:calibMode：标定状态，initstepname：初始进度状态名称，updatecycle：进度条更新周期
									
* Output         	:
* Return         	:
*******************************************************************************/
void InitStepStatus(STEP_NAME_CODE initstepname,uint16_t updatecycle)
{	
	 
	pMbInputData->nStepPercent = 0;
	// pMbInputData->nStepCode = initstepname;	
	
	ProcTotalTime = pMbInputData->nPrecTotalTime;
	ProcUpdataPeriod = updatecycle*1000;	
	ProcStartTime = xTaskGetTickCount();       								//记录起始时刻
	osTimerStart(ReportStepTimerHandle, ProcUpdataPeriod);							//启动定时器 1分钟周期定时

}
/*******************************************************************************
* Function Name  	: StopUpdateStepStatus
* Description    	: 停止进度条更新，同时更新流程总时间
* Input          	:calibMode：标定状态，initstepname：初始进度状态名称，updatecycle：进度条更新周期
									
* Output         	:
* Return         	:
*******************************************************************************/
void StopUpdateStepStatus(void)
{	
	osTimerStop(ReportStepTimerHandle);       //结束更新进度条定时器
	ProcTotalTime = xTaskGetTickCount();     	//记录当前时间
	ProcTotalTime -= ProcStartTime;
	ProcTotalTime += 500 ;  
	ProcTotalTime /= 1000;										//mS -> S     //更新总时间	
	pMbInputData->nPrecTotalTime = ProcTotalTime;
	SaveUserData();
}
/*******************************************************************************
* Function Name  	: StopReportStepTimer
* Description    	: 停止进度条更新，同时更新流程总时间
* Input          	:calibMode：标定状态，initstepname：初始进度状态名称，updatecycle：进度条更新周期
									
* Output         	:
* Return         	:
*******************************************************************************/
void StopReportStepTimer(void)
{	
		osTimerStop(ReportStepTimerHandle);       //结束更新进度条定时器
}



