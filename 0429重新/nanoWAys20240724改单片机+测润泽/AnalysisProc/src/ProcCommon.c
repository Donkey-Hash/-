
#include "ProcCommon.h"
#include "ProcAPI.h"
#include "peristalticpump.h"
#include "replyhost.h"
#include "tempcontroltask.h"
#include "processcontroltask.h"
#include "TestLEDCtr.h"
#include "instrumentdef.h"
extern FUN_VALVE_RELARS_INFO *pDevsValveRelayInfo[MAX_DEV_NUM];
FUN_PROC_HANDLE ProcHandles[MAX_DEV_NUM];

uint8_t CurrentStep = 0;

uint16_t BlankRefAD,BlankAbsAD; //
uint16_t ReactionRefAD,ReactionAbsAD;
uint16_t LL1AD,LL2AD;
uint16_t LL1ADBlank,LL2ADBlank;
float LL1InitCurrent = 10;
float LL2InitCurrent = 10;


/**********************************将流程处理函数在此处显示声明*****************/
extern bool ProcTest(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
extern bool ProcCODCr(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
extern bool ProcCODCl(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
extern bool ProcCODMn(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
extern bool ProcNH_II(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
extern bool ProcTN(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
extern bool ProcTP(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
extern bool ProcCODMnOverflow(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
extern bool ProcTNR(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime); //总氮-间苯二酚标签TNR 2019-05-19


/*******************************************************************************
* Function Name  	: InitProcHandles
* Description    	: 将设备的流程处理函数添加到函数组
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void InitProcHandles(void)
{
	ProcHandles[DEV_NONE] = ProcTest;
	ProcHandles[DEV_COD_CR] = ProcCODCr;
	ProcHandles[DEV_COD_CL] = ProcCODCl;
	ProcHandles[DEV_NH_II] = ProcNH_II;
	ProcHandles[DEV_TN] = ProcTN;
	ProcHandles[DEV_TP] = ProcTP; // 总磷流程
	ProcHandles[DEV_COD_Mn] = ProcCODMn;
	ProcHandles[DEV_TNR] = ProcTNR;
}

/*******************************************************************************
* Function Name  	: ProcInit
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcInit(DEVICE_CODE dev)
{
	osDelay(500);
	ProcPreproc(dev);
//	ProcInitStepStatus(dev,STEP_RESET,60);//初始化进度条相关信息,进度条更新时间为60
//	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE); //排空定量管内液体
	if(GetTemperature(TEMP_CH_POOL)>85)
	{
		ProcUpdateStepStatus(STEP_COOLING);
		ProcTempCtrl(true,TEMP_CTRL_CH_POOL,85,0);
		while(GetTemperature(TEMP_CH_POOL)>85)
			osDelay(5000);
	}
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);
	ProcEmptyPool(dev,F_VALVE_CLEAN_WASTE);
	ProcWashPoolWithoutReplyFault(dev,AGENT_DISTILLED_WATER,9);
	ProcReportStepTimer();
	ProcDelay(2);
	ProcDone();
}
/*******************************************************************************
* Function Name  	: ProcInit
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcClean(DEVICE_CODE dev)
{
	ProcInitStepStatus(dev,STEP_RESET,60);//初始化进度条相关信息,进度条更新时间为60
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE); //排空定量管内液体
	if(GetTemperature(TEMP_CH_POOL)>85)
	{
		ProcUpdateStepStatus(STEP_COOLING);
		ProcTempCtrl(true,TEMP_CTRL_CH_POOL,85,0);
		while(GetTemperature(TEMP_CH_POOL)>85)
			osDelay(5000);
	}
	ProcUpdateStepStatus(STEP_WASH_DETECTION_POOL);
	ProcEmptyPool(dev,F_VALVE_CLEAN_WASTE);
	ProcWashPoolWithoutReplyFault(dev,AGENT_DISTILLED_WATER,9);
	ProcReportStepTimer();
	ProcDelay(2);
	ProcDone();
}





/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/

bool ProcTest(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime)
{		
//	AGENT_CODE agentCode = AGENT_SAMPLE;
	ProcDelay(3);
	if(false == ProcPreproc(dev))
		return false;
	ProcEmptyPool(dev,F_VALVE_CLEAN_WASTE);
	ProcDelay(1);
	ProcUpdateStepStatus(STEP_PUMP_WATER);
	ProcDelay(1);	
	ProcUpdateStepStatus(STEP_WASH_WITH_SAMPLE);
	ProcDelay(1);	
	ProcUpdateStepStatus((STEP_NAME_CODE)agentCode);
	ProcDelay(1);
	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_MERCUCY_SULFATE);
	ProcDelay(1);
	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_POTASSIUM_DICHROMATE);
	ProcDelay(1);
	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_SULPHURIC_ACID);
	ProcDelay(1);	
	ProcUpdateStepStatus(STEP_HOTTING);
	ProcDelay(1);
	ProcUpdateStepStatus(STEP_COOLING);
	ProcDelay(1);
	ProcUpdateStepStatus(STEP_READ_ABS);
	ProcDelay(1);
	ProcReadReactADs(10);		
	ProcCalculate();    					//通知上位机计算结果	
	ProcDelay(2);
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);	
	ProcReportStepTimer();//(); //结束进度条更新
	ProcDelay(1);
	return true;
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/

bool ProcPreFill(DEVICE_CODE dev)
{		

	FUN_VALVE_RELARS_INFO *pa = pDevsValveRelayInfo[dev];
	int i;
	
	ProcInitStepStatus(dev,STEP_RESET,60);//初始化进度条相关信息,进度条更新时间为60秒	
	ProcDelay(2);
	for(i=0;i<MAX_FUN_VALVE_NUM;i++)
	{
		if(pa[i].nValveFunCode >= F_VALVE_MERCUCY_SULFATE && pa[i].nValveFunCode <= F_VALVE_REDUCER) 
		{
			ProcUpdateStepStatus((STEP_NAME_CODE)pa[i].nValveFunCode);
//			ProcDelay(2);	
			ProcWashTubeWithAgent(dev,(AGENT_CODE)pa[i].nValveFunCode,3,false);
			ProcDelay(1);	
		}
	}	
	ProcReportStepTimer(); //结束进度条更新
	ProcDelay(2);
	return true;
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
FAULT_CODE ProcInitLLAD(void)
{
	uint8_t mmstate = 0;
	LL1AD = 0;
	LL2AD = 0;
	SetLL1LEDCurrent(LL1InitCurrent);
//	SetLL2LEDCurrent(0);
	while(1)//LL1AD )
	{
		osDelay(500);
		LL1AD = pMbInputData->nLL1AD;
		if(LL1AD < LL_BLANK_AD_MIN)
		{
			if(LL1InitCurrent > I_MAX_LED)
				return LL1_LIGHT_FAULT;
			if(mmstate == 2)
				break;
			LL1InitCurrent += 0.1f;
			pMbHoldData->nLL1LdCur = LL1InitCurrent * 10 + 0.5f;
			SetLL1LEDCurrent(LL1InitCurrent);
			mmstate = 1;
		}
		else if(LL1AD > LL_BLANK_AD_MAX)
		{
			if(LL1InitCurrent < 0)
				return LL1_LIGHT_FAULT;
			if(mmstate == 1)
				break;
			LL1InitCurrent -= 0.1f;
			pMbHoldData->nLL1LdCur = LL1InitCurrent * 10 + 0.5f;
			SetLL1LEDCurrent(LL1InitCurrent);
			mmstate = 2;
		}
		else
			break;
	}
	osDelay(500);
	LL1AD = pMbInputData->nLL1AD;
	LL1ADBlank = LL1AD;
	pMbInputData->nLL1BlankAD = LL1ADBlank;
	pMbHoldData->nLL1LdCur = LL1InitCurrent * 10 + 0.5f;
//	SetLL1LEDCurrent(0);
	SetLL2LEDCurrent(LL2InitCurrent);
	mmstate = 0;   //电流调整方向清零
	while(1)
	{
		osDelay(500);
		LL2AD = pMbInputData->nLL2AD;
		if(LL2AD < LL_BLANK_AD_MIN)
		{
			if(LL2InitCurrent > I_MAX_LED)
				return LL2_LIGHT_FAULT;
			if(mmstate == 2)
				break;
			LL2InitCurrent += 0.1f;
			pMbHoldData->nLL2LdCur = LL2InitCurrent * 10 +0.5f;
			SetLL2LEDCurrent(LL2InitCurrent);
			mmstate = 1;
		}
		else if(LL2AD > LL_BLANK_AD_MAX)
		{
			if(LL2InitCurrent < 0)
				return LL2_LIGHT_FAULT;
			if(mmstate == 1)
				break;
			LL2InitCurrent -= 0.1f;
			pMbHoldData->nLL2LdCur = LL2InitCurrent * 10 +0.5f;
			SetLL2LEDCurrent(LL2InitCurrent);
			mmstate = 2;
		}
		else
			break;
	}
	osDelay(500);
	LL2AD = pMbInputData->nLL2AD;
	LL2ADBlank = LL2AD;
	pMbInputData->nLL2BlankAD = LL2AD;
	pMbHoldData->nLL2LdCur = LL2InitCurrent * 10 +0.5f;
	SetLL1LEDCurrent(LL1InitCurrent);
//	taskENTER_CRITICAL();
	SaveUserData();
//	taskEXIT_CRITICAL();
	return NO_FAULT;
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
PUMP_LIQUID_ERROR CheckLevelsAD(void)
{
	uint8_t mmstate = 0;
	LL1AD = 0;
	LL2AD = 0;
	SetLL1LEDCurrent(LL1InitCurrent);
//	SetLL2LEDCurrent(0);
	while(1)//LL1AD )
	{
		osDelay(500);
		LL1AD = pMbInputData->nLL1AD;
		if(LL1AD < LL_BLANK_AD_MIN)
		{
			if(LL1InitCurrent > I_MAX_LED)
				return PUMP_LIQUID_CHECK_LEVEL1_ERROR;
			if(mmstate == 2)
				break;
			LL1InitCurrent += 0.1f;
			pMbHoldData->nLL1LdCur = LL1InitCurrent * 10 + 0.5f;
			SetLL1LEDCurrent(LL1InitCurrent);
			mmstate = 1;
		}
		else if(LL1AD > LL_BLANK_AD_MAX)
		{
			if(LL1InitCurrent < 0)
				return PUMP_LIQUID_CHECK_LEVEL1_ERROR;
			if(mmstate == 1)
				break;
			LL1InitCurrent -= 0.1f;
			pMbHoldData->nLL1LdCur = LL1InitCurrent * 10 + 0.5f;
			SetLL1LEDCurrent(LL1InitCurrent);
			mmstate = 2;
		}
		else
			break;
	}
	osDelay(500);
	LL1AD = pMbInputData->nLL1AD;
	LL1ADBlank = LL1AD;
	pMbInputData->nLL1BlankAD = LL1ADBlank;
	pMbHoldData->nLL1LdCur = LL1InitCurrent * 10 + 0.5f;
//	SetLL1LEDCurrent(0);
	SetLL2LEDCurrent(LL2InitCurrent);
	mmstate = 0;   //电流调整方向清零
	while(1)
	{
		osDelay(500);
		LL2AD = pMbInputData->nLL2AD;
		if(LL2AD < LL_BLANK_AD_MIN)
		{
			if(LL2InitCurrent > I_MAX_LED)
				return PUMP_LIQUID_CHECK_LEVEL2_ERROR;
			if(mmstate == 2)
				break;
			LL2InitCurrent += 0.1f;
			pMbHoldData->nLL2LdCur = LL2InitCurrent * 10 +0.5f;
			SetLL2LEDCurrent(LL2InitCurrent);
			mmstate = 1;
		}
		else if(LL2AD > LL_BLANK_AD_MAX)
		{
			if(LL2InitCurrent < 0)
				return PUMP_LIQUID_CHECK_LEVEL2_ERROR;
			if(mmstate == 1)
				break;
			LL2InitCurrent -= 0.1f;
			pMbHoldData->nLL2LdCur = LL2InitCurrent * 10 +0.5f;
			SetLL2LEDCurrent(LL2InitCurrent);
			mmstate = 2;
		}
		else
			break;
	}
	osDelay(500);
	LL2AD = pMbInputData->nLL2AD;
	LL2ADBlank = LL2AD;
	pMbInputData->nLL2BlankAD = LL2AD;
	pMbHoldData->nLL2LdCur = LL2InitCurrent * 10 +0.5f;
	SetLL1LEDCurrent(LL1InitCurrent);
//	taskENTER_CRITICAL();
	SaveUserData();
//	taskEXIT_CRITICAL();
	return PUMP_LIQUID_OK;
}
