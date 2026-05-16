#include <stdlib.h>
#include <math.h>
#include "ProcAPI.h"
#include "peristalticpump.h"
#include "replyhost.h"
#include "progressbar.h"
#include "UVSpectrogragh.h"
#include "FlashUserData.h"
#include "processcontroltask.h"
#include "syringepump.h"
#include "OPT2000.h"
#include "OOptics.h"
#include "datacollecttask.h"

extern uint16_t BlankRefAD,BlankAbsAD;
extern uint16_t ReactionRefAD,ReactionAbsAD;
extern bool TimeoutFlag;
extern osMutexId_t osADSMutexHandle;

READ_UV_DATA_FUN ReadUVDataFun = ReadOPT2000Data;//ReadOOpticsData;//
FAULT_CODE ProcInitLLAD(void);


void GetADabsReal(uint16_t* pADref,uint16_t* pADtrs,uint16_t nCount)
{
	osDelay(100);
	
	if(osMutexWait(osADSMutexHandle, 500) == osOK)
		{
			PoolLtADsOpt.nSumOfRefAD = 0;
			PoolLtADsOpt.nSumOfTranAD = 0;
			PoolLtADsOpt.nNumOfCollect = nCount;
			if(osMutexRelease(osADSMutexHandle) != osOK)
				CounterADSMutexError++;
		}
		else
			CounterADSMutexError++;
		while(PoolLtADsOpt.nNumOfCollect>0)
			osDelay(100);
		*pADref = (PoolLtADsOpt.nSumOfRefAD + nCount/2) / nCount;
		*pADtrs = (PoolLtADsOpt.nSumOfTranAD + nCount/2) / nCount;
		
}
/*******************************************************************************
* Function Name  : ProFilledWithAgent
* Description    : 试剂预充满定量管
* Input          : 	src:试剂对应的电磁阀; speed:加试剂速度；
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
bool ProcFillTubeWithAgent(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine)
{
	//printf("使用 %dmL 试剂:%d 预充满定量管。\r\n",quanty,agent);
	FAULT_CODE faultCode;
	faultCode = FillTubeWithAgent(dev,agent,quanty,fine);
	if(faultCode != NO_FAULT)
	{
		ReplyFaultInProcess(faultCode);
		return false;
	}
	return true;

}
/*******************************************************************************
* Function Name  	: ProcEmptyTube
* Description    	: 清空定量管中液体
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcEmptyTube(DEVICE_CODE dev,FUN_VALVE_CODE des)
{
	//printf("\r\n排空定量管\r\n");
	SendAir(dev,des,SPEED_AIR_TUBE,MAX_PUSH_LIQUID_CIRCLE);  //清空定量管
}
/*******************************************************************************
* Function Name  : ProcWashTubeWithAgent
* Description    : 清洗定量管
* Input          : 	src:试剂对应的电磁阀; speed:加试剂速度；
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
void ProcWashTubeWithAgent(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine)
{
	//printf("\r\n使用 %dmL 试剂:%d 清洗定量管。\r\n",quanty,agent);
	ProcFillTubeWithAgent(dev,agent,quanty,fine);
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);
}
/*******************************************************************************
* Function Name  : ProcPreagentPreWashTube
* Description    : 试剂润洗定量管，与ProcWashTubeWithAgent不同之处在于，预洗废液排到反应废液里。
* Input          : 	src:试剂对应的电磁阀; speed:加试剂速度；
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
void ProcPreagentPreWashTube(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine)
{
	//printf("\r\n使用 %dmL 试剂:%d 清洗定量管。\r\n",quanty,agent);
	ProcFillTubeWithAgent(dev,agent,quanty,fine);
	ProcEmptyTube(dev,F_VALVE_REACT_WASTE);
}
/*******************************************************************************
* Function Name  	: ProcAirPool
* Description    	:	向反应池吹气
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcAirPool(DEVICE_CODE dev)
{
	//printf("\r\n向比色池吹气。\r\n");
	SendAir(dev,VALVE_GLASS_TANK,SPEED_DEFAULT,11); //向池子吹气
}
/*******************************************************************************
* Function Name  	: ProcAirPool
* Description    	:	向反应池吹气
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcAirLoopPool(DEVICE_CODE dev)
{
	//printf("\r\n通过定量环向比色池吹气。\r\n");
	SendAir(dev,F_VALVE_LOOP_ONLINE_POOL,SPEED_DEFAULT,30); //通过定量环向池子吹气
}
/*******************************************************************************
* Function Name  	: ProcAirPool
* Description    	:	向反应池吹气
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcPushBackAgent(DEVICE_CODE dev,AGENT_CODE agentCode)
{
	//printf("\r\n回推试剂 %d 。\r\n",agentCode);
	SendAir(dev,(FUN_VALVE_CODE)agentCode,SPEED_DEFAULT,10);//水样回推
}
/*******************************************************************************
* Function Name  	: ProcWashPoolWithAgent
* Description    	: 使用试剂agent清洗反应池。
* Input          	:	dev：设备代码，agent：试剂代码，quanty：用量
									
* Output         	:
* Return         	:
*******************************************************************************/
bool ProcWashPoolWithAgent(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty)
{
	FAULT_CODE faultCode;
//	printf("\r\n使用 %dmL 试剂：%d 清洗比色池。\r\n",quanty,agent);
	faultCode = WashPoolWithAgent(dev,agent,quanty);
	if(faultCode != NO_FAULT)
	{
		//printf("\r\n使用试剂：%d 清洗反应池失败！错误代码：%d 。\r\n",agent,faultCode);
		ReplyFaultInProcess(faultCode);
		return false;
	}
	return true;
}
/*******************************************************************************
* Function Name  	: ProcWashPoolWithoutReplyFault
* Description    	: 只用于清洗流程中，用零点液洗比色池。
* Input          	:	dev：设备代码，agent：试剂代码，quanty：用量
									
* Output         	:
* Return         	:
*******************************************************************************/
bool ProcWashPoolWithoutReplyFault(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty)
{
	FAULT_CODE faultCode;

	faultCode = WashPoolWithAgent(dev,agent,quanty);
	if(faultCode != NO_FAULT)
	{

		ReplyFaultInProcess(faultCode);
		return false;
	}
	return true;
}
/*******************************************************************************
* Function Name  	: ProcAddAgentToPool
* Description    	: 向反应池加试剂agent。
* Input          	: dev：设备代码，agent：试剂代码，quanty：用量，单位为试剂驱动器最小单位
										fine：是否需要精确定量
									
* Output         	:
* Return         	:
*******************************************************************************/
bool ProcAddAgentToPool(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine)
{
	FAULT_CODE faultCode;
	//printf("\r\n向比色池加 %umL 的试剂：%d 。\r\n",quanty,agent);
	faultCode = AddAgentToPool(dev,agent,quanty,true);
	if(faultCode != NO_FAULT)
	{	
		//printf("\r\n添加试剂失败！错误代码：%d 。\r\n",faultCode);
		ReplyFaultInProcess(faultCode);
		PoolEmptying(dev,F_VALVE_CLEAN_WASTE);
		return false;
	}
	return true;
}
/*******************************************************************************
* Function Name  	: ProcEmptyPool
* Description    	: 清空反应池
* Input          	:dev：设备代码，des：废液流向
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcEmptyPool(DEVICE_CODE dev,FUN_VALVE_CODE des)
{	
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE); //排空定量管内液体
	if(GetTemperature(TEMP_CH_POOL)>85)
	{
//		ProcUpdateStepStatus(STEP_COOLING);
		ProcTempCtrl(true,TEMP_CTRL_CH_POOL,85,0);
		while(GetTemperature(TEMP_CH_POOL)>85)
			osDelay(5000);
	}
	
	PoolEmptying(dev,des);															//排空比色池
}
/*******************************************************************************
* Function Name  	: ProcDrawPool
* Description    	: 从反应池定量抽水
* Input          	:dev：设备代码，des：废液流向，quanty：抽取量
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcDrawPool(DEVICE_CODE dev,FUN_VALVE_CODE des,uint16_t quanty)
{	
	TrsLiquidByTube(dev,F_VALVE_POOL_ONLY,des,SPEED_REACTION_POOL_SOLUTION,quanty,true);		
//	SendAir(dev,des,SPEED_REACTION_POOL_SOLUTION*2,MAX_PUSH_LIQUID_CIRCLE);
		
}
/*******************************************************************************
* Function Name  	: ProcAddAgentLoopPool
* Description    	: 通过定量环向反应池添加试剂agent
* Input          	: dev：设备代码，agent：试剂代码，quanty：用量，fine：是否需要精确定量
									
* Output         	:
* Return         	:
*******************************************************************************/
bool ProcAddAgentLoopPool(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine)
{
	FAULT_CODE faultCode;
	//printf("\r\n通过定量环向比色池加 %umL 的试剂：%d 。\r\n",quanty,agent);
	faultCode = AddAgentLoopPool(dev,agent,quanty,true);
	if(faultCode != NO_FAULT)
	{	
		//printf("\r\n通过定量环添加试剂失败！错误代码：%d 。\r\n",faultCode);
		ReplyFaultInProcess(faultCode);
		PoolEmptying(dev,F_VALVE_CLEAN_WASTE);
		return false;
	}
	return true;
}
/*******************************************************************************
* Function Name  : ProcFillLoop
* Description    : 注满定量环。从反应池中抽试剂，用液位1定量，粗定量，定量完成后
										将定量管中的试剂排到清洗废液桶，然后排空反应池。调用此函数前
										确保反应池有足够的试剂。
* Input          : 	dev:设备代码; des:排废液功能阀代码
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
bool ProcFillLoop(DEVICE_CODE dev)
{
	FAULT_CODE faultCode;
	//printf("\r\n定量环定量。\r\n");
	faultCode = FillLoop(dev);
	if(faultCode != NO_FAULT)
	{	
		//printf("\r\n定量环定量失败！错误代码：%d 。\r\n",faultCode);
		ReplyFaultInProcess(faultCode);
		return false;
	}
	return true;
}

/*******************************************************************************
* Function Name  	: ProcUpdateStepStatus
* Description    	: 初始化进度条相关信息
* Input          	:calibMode：标定状态，initstepname：初始进度状态名称，updatecycle：进度条更新周期
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcInitStepStatus(DEVICE_CODE dev, STEP_NAME_CODE initstepname,uint16_t updatecycle)
{
	//printf("\r\n初始化进度条...\r\n");
	
	InitStepStatus(initstepname,updatecycle);	
	

	
}
/*******************************************************************************
* Function Name  	: ProcUpdateStepStatus
* Description    	: 更新进度状态
* Input          	:stepname：进度阶段名称代码，详见STEP_NAME_CODE
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcUpdateStepStatus(STEP_NAME_CODE stepname)
{
	//printf("\r\n更新进度状态名称。当前状态：%d\r\n",stepname);
//	AddErrRecord(PROC_UPDATE_STEP_NAME,stepname);
	// pMbInputData->nStepCode = stepname;
	ProcDelay(1);
//	UpdateStepName(stepname);     //将变量传递到全局变量
 	
}
/*******************************************************************************
* Function Name  	: ProcStopUpdateStepStatus
* Description    	: 停止更新进度条，同时更新流程总时间
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcStopUpdateStepStatus(void)
{
	//printf("\r\n结束进度条更新。\r\n");
	AddErrRecord(PROC_END,0x00);
	StopUpdateStepStatus();
}
/*******************************************************************************
* Function Name  	: ProcReportStepTimer
* Description    	: 停止更新进度条.
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcReportStepTimer(void)
{
	//printf("\r\n结束进度条更新。\r\n");
	AddErrRecord(PROC_END,0x00);
	StopReportStepTimer();
}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcReadBlankADs(uint8_t readADTimes)
{
	//printf("\r\n读空白AD值。\r\n");
	GetADabsReal(&pMbHoldData->nBlankRefLtAD,&pMbHoldData->nBlankTranLtAD,readADTimes);
	pMbHoldData->fBlankAbs = log(	(double)pMbHoldData->nBlankRefLtAD /
																(double)pMbHoldData->nBlankTranLtAD );
	osDelay(1000);
	// pMbInputData->nStepCode = STEP_READ_BLANK_AD_DONE;
	osDelay(1000);
//	ReplyHostProcBlankAbs(BlankRefAD,BlankAbsAD);
	//printf("\r\nBlankRefAD:%u,BlankAbsAD:%u.\r\n",BlankRefAD,BlankAbsAD);
//	osDelay(2000);
//	ReplyHostProcBlankAbs(BlankRefAD,BlankAbsAD);
	//printf("\r\nBlankRefAD:%u,BlankAbsAD:%u.\r\n",BlankRefAD,BlankAbsAD);
//	osDelay(2000);
}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcReadReactADs(uint8_t readADTimes)
{
	//printf("\r\n读测量AD值。\r\n");
	GetADabsReal(&pMbHoldData->nReactionRefLtAD,&pMbHoldData->nReactionTranLtAD,readADTimes);	
	pMbHoldData->fReactAbs = log(	(double)pMbHoldData->nReactionRefLtAD /
																(double)pMbHoldData->nReactionTranLtAD);
	osDelay(1000);	
//	pMbHoldData->nStepCode = STEP_READ_REACTION_AD_DANE;
	osDelay(1000);
//	ReplyHostProcReactAbs(ReactionRefAD,ReactionAbsAD);
	//printf("\r\nReactionRefAD:%u,ReactionAbsAD:%u.\r\n",ReactionRefAD,ReactionAbsAD);
	
//	ReplyHostProcReactAbs(ReactionRefAD,ReactionAbsAD);
	//printf("\r\nReactionRefAD:%u,ReactionAbsAD:%u.\r\n",ReactionRefAD,ReactionAbsAD);
//	osDelay(2000);
}
/*******************************************************************************
* Function Name  	: ProcCalculate
* Description    	: 通知上位机计算结果
* Input          	:calibMode：标定模式
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcCalculate(void)
{
//	switch(pMbInputData->nProcCode)
//	{
//		case RROC_AUTO_RROC_CALIBRAT_LOW:
//		case RROC_CALIBRAT_LOW:
//			CalculatLowAbs();
//		break;
//		case RROC_AUTO_RROC_CALIBRAT_HIGH:
//		case RROC_CALIBRAT_HIGH:
//			CalculatHighAbs();
//		break;
//		default:
//			Calculation();
//			break;
//	}
//	
//	osDelay(2000);
}
/*******************************************************************************
* Function Name  	: ProcOpenWaterPump
* Description    	: 开水泵
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcOpenWaterPump(DEVICE_CODE dev)
{
//	printf("\r\n开启水泵。\r\n");
	OpenFunValve(dev,F_VALVE_WATER_PUMP);
}
/*******************************************************************************
* Function Name  	: ProcOpenWaterPump
* Description    	: 开水泵
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcCloseWaterPump(DEVICE_CODE dev)
{
//	printf("\r\n关闭水泵。\r\n");
	CloseFunValve(dev,F_VALVE_WATER_PUMP);
}
/*******************************************************************************
* Function Name  	: ProcDelay
* Description    	: 秒级延时
* Input          	:timeS：需要延时的时长，单位为秒。
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcDelay(uint16_t timeS)
{
	osDelay(timeS*1000);
}
/*******************************************************************************
* Function Name  	: ProcTempCtrl
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/
bool ProcTempCtrl(bool Proc,TEMPER_CTRL_CHANNEL temp_ctrl_ch,uint8_t sv,int16_t holdTime)
{
	FAULT_CODE fault;
	fault = WaitTemperCtrlMsg(1);
//	printf("\r\n开始温度控制，目标温度：%d℃，保持时间：%d分钟。\r\n",sv,holdTime);
//	StartTempControl(Proc,temp_ch,sv,holdTime,mode);			//开启加热控制
	StartOneStageTempCtrl(temp_ctrl_ch,sv,holdTime);
	fault = WaitTemperCtrlMsg(30);
	if(NO_FAULT != fault)     																		//等待加热控制完成
	{
//		printf("\r\n温度控制失败！错误代码：%d 。\r\n",fault);
		ReplyFaultInProcess(fault);
		return false;
	}
	return true;
}
/*******************************************************************************
* Function Name  	: ProcTempCtrl
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/
bool ProcTempCtrlWithoutHoldTime(bool Proc,TEMPER_CTRL_CHANNEL temp_ctrl_ch,uint8_t sv,int16_t holdTime)
{
	FAULT_CODE fault;
	StartOneStageTempCtrl(temp_ctrl_ch,sv,-1);
	fault = WaitTemperCtrlMsg(30);
	if(NO_FAULT != fault)     																		//等待加热控制完成
	{
//		printf("\r\n温度控制失败！错误代码：%d 。\r\n",fault);
		return false;
	}
	return true;
}
/*******************************************************************************
* Function Name  	: ProcResetUV
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/
bool ProcResetUV(void)
{	
//	UV_StatusTypeDef ret;
	bool ret;
  uint16_t abs[1];
	uint16_t ref[1];
	ret = ReadUVDataFun(abs,ref);
	if(true != ret)
	{
//		AddErrRecord(ERROR_SPECTRO_TROUBLE_BLANK,ret);
		ReplyFaultInProcess(FAULT_SPECTRO_TROUBLE);
		return false;
	}
	else
		return true;
}
/*******************************************************************************
* Function Name  	: ProcReadBlankUVData
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/
bool ProcReadBlankUVData(void)
{	
//	UV_StatusTypeDef ret;
	bool ret;
  uint16_t abs[3];
	uint16_t ref[3];
	uint16_t t1,t2,t3;
	
	ret = ReadUVDataFun(abs,ref);
	osDelay(3000);
	ret = ReadUVDataFun(abs + 1,ref + 1);
	osDelay(3000);
	ret = ReadUVDataFun(abs + 2,ref + 2);
	
	t1 = (abs[0] > abs[1])?0:1;
	if(t1 == 0) t2 = 1;
	else t2 = 0;	
	t3 = (abs[t1] < abs[2])?t1:2;
	t1 = (abs[t2] > abs[t3])?t2:t3;
	
	pMbHoldData->nBlankTranLtAD = abs[t1];
	pMbHoldData->nBlankRefLtAD = ref[t1];
	
	if(true != ret)
	{
//		AddErrRecord(ERROR_SPECTRO_TROUBLE_BLANK,ret);
		ReplyFaultInProcess(FAULT_SPECTRO_TROUBLE);
		return false;
	}
	else
	{
		// pMbInputData->nStepCode = STEP_READ_BLANK_AD_DONE;
		osDelay(3000);
		return true;
	}
}
/*******************************************************************************
* Function Name  	: ProcReadBlankUVData
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/
bool ProcReadReactUVData(void)
{
//	UV_StatusTypeDef ret;
	bool ret;
	uint16_t abs[3];
	uint16_t ref[3];
	uint16_t t1,t2,t3;
	
	ret = ReadUVDataFun(abs,ref);
	osDelay(3000);
	ret = ReadUVDataFun(abs + 1,ref + 1);
	osDelay(3000);
	ret = ReadUVDataFun(abs + 2,ref + 2);
	
	t1 = (abs[0] > abs[1])?0:1;
	if(t1 == 0) t2 = 1;
	else t2 = 0;	
	t3 = (abs[t1] < abs[2])?t1:2;
	t1 = (abs[t2] > abs[t3])?t2:t3;
	
	pMbHoldData->nReactionTranLtAD = abs[t1];
	pMbHoldData->nReactionRefLtAD = ref[t1];
	
	if(true != ret)
	{
//		AddErrRecord(ERROR_SPECTRO_TROUBLE,ret);
		ReplyFaultInProcess(FAULT_SPECTRO_TROUBLE);
		return false;
	}
	else
	{
		pMbHoldData->fBlankAbs = log(	(double)pMbHoldData->nBlankRefLtAD ) /
																	(double)pMbHoldData->nReactionRefLtAD;
		pMbHoldData->fReactAbs = log(	(double)pMbHoldData->nBlankTranLtAD/ 
																	(double)pMbHoldData->nReactionTranLtAD );
		// pMbInputData->nStepCode = STEP_READ_REACTION_AD_DANE;
		osDelay(3000);
		return true;
	}	
//	UV_StatusTypeDef ret;
//	ret = ReadUVData(&pMbHoldData->nReactionTranLtAD,&pMbHoldData->nReactionRefLtAD);	
//	if(UV_OK != ret)
//	{
//		AddErrRecord(ERROR_SPECTRO_TROUBLE_BLANK,ret);
//		return false;
//	}
//	else
//	{
//	pMbHoldData->nStepCode = STEP_READ_REACTION_AD_DANE;
//	osDelay(3000);
//	return true;
//	}
}
/*******************************************************************************
* Function Name  	: ProcReadBlankUVData
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/
//	uint16_t refdark,trandark;
//	uint16_t refzero,tranzero;
//bool ProcReadBlankOOpUVData(void)
//{	
//	UV_StatusTypeDef ret  = UV_OK;


//	OOpGetWaveData(220,&tranzero);
//	OOpGetWaveData(275,&refzero);
//	
////	OOpGetZeroData(220,&tranzero);
////	OOpGetZeroData(275,&refzero);
//	
//	OOpGetDarkCur(220,&trandark);
//	OOpGetDarkCur(275,&refdark);

////	pMbHoldData->nRefDarkCurAD = refdark;
////	pMbHoldData->nTranDarkCurAD = trandark;
//	pMbHoldData->nBlankTranLtAD = tranzero - trandark;
//	pMbHoldData->nBlankRefLtAD = refzero - refdark;
//	
//	
//	if(UV_OK != ret)
//	{
//		AddErrRecord(ERROR_SPECTRO_TROUBLE_BLANK,ret);
//		ReplyFaultInProcess(FAULT_SPECTRO_TROUBLE);
//		return false;
//	}
//	else
//	{
//		pMbHoldData->nStepCode = STEP_READ_BLANK_AD_DONE;
//		osDelay(3000);
//		return true;
//	}
//}
/*******************************************************************************
* Function Name  	: ProcReadBlankUVData
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/
//uint16_t refreact,tranreact;
//bool ProcReadReactOOpUVData(void)
//{
//	UV_StatusTypeDef ret = UV_OK;

//	

//	OOpGetWaveData(220,&tranreact);
//	OOpGetWaveData(275,&refreact);
//	pMbHoldData->nReactionTranLtAD = tranreact - trandark;//pMbHoldData->nTranDarkCurAD;
//	pMbHoldData->nReactionRefLtAD = refreact - refdark;//pMbHoldData->nRefDarkCurAD;
//		
//	
//	if(UV_OK != ret)
//	{
//		AddErrRecord(ERROR_SPECTRO_TROUBLE,ret);
//		ReplyFaultInProcess(FAULT_SPECTRO_TROUBLE);
//		return false;
//	}
//	else
//	{
//		pMbHoldData->nStepCode = STEP_READ_REACTION_AD_DANE;
//		osDelay(3000);
//		return true;
//	}	
////	UV_StatusTypeDef ret;
////	ret = ReadUVData(&pMbHoldData->nReactionTranLtAD,&pMbHoldData->nReactionRefLtAD);	
////	if(UV_OK != ret)
////	{
////		AddErrRecord(ERROR_SPECTRO_TROUBLE_BLANK,ret);
////		return false;
////	}
////	else
////	{
////	pMbHoldData->nStepCode = STEP_READ_REACTION_AD_DANE;
////	osDelay(3000);
////	return true;
////	}
//}
/*******************************************************************************
* Function Name  	: ProcReadBlankUVData
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/
////uint16_t refreact,tranreact;
//void ProcPreHeatOOpUV(void)
//{
//	OOpGetWaveData(220,&tranreact);
//	OOpGetWaveData(275,&refreact);

//}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
uint16_t ProcGetReactAD(uint16_t nCount)
{
	//printf("\r\n读测量AD值。\r\n");
	GetADabsReal(&ReactionRefAD,&ReactionAbsAD,nCount);
  return 	ReactionAbsAD;
}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcSendBlankADs(void)
{		
//	pMbHoldData->nBlankRefLtAD = ref;
//	pMbHoldData->nBlankTranLtAD = abs;
	// pMbInputData->nStepCode = STEP_READ_BLANK_AD_DONE;

}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcSendRectADs(void)
{		
//	pMbHoldData->nBlankRefLtAD = ref;
//	pMbHoldData->nBlankTranLtAD = abs;
	// pMbInputData->nStepCode = STEP_READ_REACTION_AD_DANE;

}
/*******************************************************************************
* Function Name  	: ProcPreproc
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/

bool ProcPreproc(DEVICE_CODE dev)//,FUN_VALVE_CODE wasteValve)
{
	FAULT_CODE faultCode = NO_FAULT;

	pMbHoldData->nMaintainTimePast = 0;
	pMbHoldData->nTimeToSV = 0;
	pMbInputData->nSysErrorCode = 0;
	pMbHoldData->fCaveTemp = pMbHoldData->fPoolTemp;
	CloseAllValves();
	AddErrRecord(PROC_START,00);
	
	ProcInitStepStatus(dev,STEP_RESET,60);  					//初始化进度条相关信息,进度条更新时间为60秒	
		ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
//	faultCode = ProcInitLLAD();         //校准一次液位定量对管
	if(NO_FAULT != faultCode)
	{
		ReplyFaultInProcess(faultCode);
		AddErrRecord(PROC_LL_INIT_FALSE,0x00);
		return false;
	}
	
//	ProcEmptyPool(dev,wasteValve);								//排空比色池	
	return true;

}
/*******************************************************************************
* Function Name  	: ProcReadBlankUVData
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcTitraPump(DEVICE_CODE dev,FUN_VALVE_CODE funValveCode,uint16_t times)
{
	while(times--)
	{
		OpenFunValve(dev,funValveCode);
		osDelay(150);
		CloseFunValve(dev,funValveCode);
		osDelay(100);
	}
}
/*******************************************************************************
* Function Name  	: ProcTitration
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/

bool ProcTitration(DEVICE_CODE dev,uint16_t initAD)//,uint32_t* pTitrationTime)
{
	uint16_t pAD;
	uint16_t dAD;
	uint16_t count = 0;
	uint16_t delay = 150;
	uint16_t thrldCount = 0;
	
	while(1)
	{
		OpenFunValve(dev,F_VALVE_OXIDANT);
		osDelay(150);
		CloseFunValve(dev,F_VALVE_OXIDANT);
		count++;
		
		pMbHoldData->nBlankRefLtAD = count;// % 1000;
		osDelay(delay);
		pAD = ProcGetReactAD(1);
		pMbHoldData->nReactionTranLtAD = pAD;
		dAD = (pAD > initAD?(pAD-initAD):(initAD-pAD));
		if(dAD >= 500 && dAD < 1000)
			{
				delay = 550;
				thrldCount = 0;
			}
			else if (dAD >= 1000 && dAD < 1500)
				{
					delay = 950;
					thrldCount = 0;
//					ProcStopHotting(TEMP_CTRL_CH_POOL);
				}
				else if( dAD >= 1500 )
					{
						do{
							pAD = ProcGetReactAD(1);
							pMbHoldData->nReactionTranLtAD = pAD;
							dAD = (pAD > initAD?(pAD-initAD):(initAD-pAD));
						if(dAD >= 1500)
							thrldCount++;
						else
						{
							thrldCount = 0;
							break;
						}
						}while(thrldCount < 3);
						if(++thrldCount >= 3)
							{	
//								pMbHoldData->nReactionTranLtAD = pAD;								
//								CloseFunValve(dev,TITRATION_PUMP);
								pMbHoldData->nBlankTranLtAD = (uint16_t)(Temp1PV *10 + 0.5f);
								CloseFunValve(dev,F_VALVE_STIR_MOTOR);
								ProcStopHotting(TEMP_CTRL_CH_POOL);
//								*pTitrationTime = count;//timepast;// /1000;
								return true;
							}
					}
		if(count > 1500)
			{
//				*pTitrationTime = 0;
				CloseFunValve(dev,F_VALVE_STIR_MOTOR);
				ProcStopHotting(TEMP_CTRL_CH_POOL);
//				*pTitrationTime = count;
				ReplyFaultInProcess(TITRATION_FAIL);
				return false;
			}
	}
}
/*******************************************************************************
* Function Name  	: ProcTitration
* Description    	: 启动温度控制
* Input          	:Proc：true：流程控制调用，false：设备调试调用；temp_ch：温度通道；sv：设定温度；holdTime：设定温度维持时间
                   mode：温度控制方式，详见TEMPER_CTRL_MODE。									
* Output         	:
* Return         	:
*******************************************************************************/

bool ProcTitrationBySyringe(DEVICE_CODE dev,uint16_t initAD)//,uint32_t* pTitrationTime)
{
	uint16_t pAD;
	uint16_t dAD;
//	uint16_t count = 0;
	uint16_t delay = 0;
	uint16_t thrldCount = 0;
	uint16_t dispenseleft = 0;
	uint16_t sumdispense = 0;
	uint16_t dul = 5;
	
	SetSyringeValveSwitch(dev,F_VALVE_OXIDANT);
	SyringePumpAspirateFull();
	dispenseleft = SYRINGE_VOLUME;
	osDelay(500);
	SetSyringeValveSwitch(dev,F_VALVE_POOL_SYRINGE);
	pMbInputData->nPoolTranLtBackAD = 0xFFFF;
	pMbHoldData->nReactionRefLtAD = ProcGetReactAD(10);
	initAD = pMbHoldData->nReactionRefLtAD;
	while(1)
	{
		if(dispenseleft == 0)
		{
			SetSyringeValveSwitch(dev,F_VALVE_OXIDANT);
			SyringePumpAspirateFull();
			dispenseleft = SYRINGE_VOLUME;
			osDelay(500);
			SetSyringeValveSwitch(dev,F_VALVE_POOL_SYRINGE);
		}
		
		SyringePumpDispense(dul);
		sumdispense += dul;
		dispenseleft -= dul;
//		OpenFunValve(dev,F_VALVE_OXIDANT);
//		osDelay(150);
//		CloseFunValve(dev,F_VALVE_OXIDANT);
		
		
//		count++;
		
		pMbHoldData->nBlankRefLtAD = sumdispense;// % 1000;
		osDelay(delay);
		pAD = ProcGetReactAD(1);
		pMbHoldData->nReactionTranLtAD = pAD;
		dAD = (pAD > initAD?(pAD-initAD):(initAD-pAD));
		if(dAD >= 500 && dAD < 1000)
			{
				delay = 250;
				thrldCount = 0;
			}
			else if (dAD >= 1000 && dAD < 1500)
				{
					delay = 1500;
					thrldCount = 0;
					dul = 5;
//					ProcStopHotting(TEMP_CTRL_CH_POOL);
				}
				else if( dAD >= 1500 )
					{
						pMbInputData->nPoolTranLtBackAD = pAD;
						do{
								pAD = ProcGetReactAD(1);
								pMbHoldData->nReactionTranLtAD = pAD;
								dAD = (pAD > initAD?(pAD-initAD):(initAD-pAD));
								if(dAD >= 1500)
								{
									thrldCount++;
									if(thrldCount == 1)pMbInputData->nPoolTranLtBackAD = pAD;
								}
								else
								{
									pMbInputData->nPoolTranLtBackAD = 0;
									thrldCount = 0;
									break;
								}
							}while(thrldCount < 3);
						if(++thrldCount >= 3)
							{	
//								pMbHoldData->nReactionTranLtAD = pAD;								
//								CloseFunValve(dev,TITRATION_PUMP);
								pMbInputData->nPoolTranLtBackAD = 0xFFFF;
								pMbHoldData->nBlankTranLtAD = (uint16_t)(Temp1PV *10 + 0.5f);
								CloseFunValve(dev,F_VALVE_STIR_MOTOR);
								ProcStopHotting(TEMP_CTRL_CH_POOL);
//								*pTitrationTime = count;//timepast;// /1000;
								return true;
							}
					}
					else
					{
						delay = 0;
						thrldCount = 0;
						dul = 5;
					}
		if(sumdispense > 7500)
			{
//				*pTitrationTime = 0;
				CloseFunValve(dev,F_VALVE_STIR_MOTOR);
				ProcStopHotting(TEMP_CTRL_CH_POOL);
//				*pTitrationTime = count;
				ReplyFaultInProcess(TITRATION_FAIL);
				return false;
			}
	}
}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcReplyTitrationTime(void)
{
	
//	pMbHoldData->nReactionRefLtAD = (uint16_t)(TitrationTime / 1000);
//	pMbHoldData->nReactionTranLtAD = (uint16_t)TitrationTime;// % 1000;
	// pMbInputData->nStepCode = STEP_READ_BLANK_AD_DONE;//STEP_READ_REACTION_AD_DANE;

	osDelay(2000);
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
                   							
* Output         	:
* Return         	:
*******************************************************************************/
void ProcEmptyPoolWithPump(DEVICE_CODE dev,FUN_VALVE_CODE des,uint16_t timeSec)
{
	OpenFunValve(dev,des);
	ProcDelay(timeSec);
	CloseFunValve(dev,des);
	
}

/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
                   							
* Output         	:
* Return         	:
*******************************************************************************/
void ProcStopHotting(TEMPER_CTRL_CHANNEL temp_ctrl_ch)
{
	StopHotting(temp_ctrl_ch);
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
                   							
* Output         	:
* Return         	:
*******************************************************************************/
void ProcOptFunValve(DEVICE_CODE dev,FUN_VALVE_CODE des,uint16_t timeSec)
{
	OpenFunValve(dev,des);
	ProcDelay(timeSec);
	CloseFunValve(dev,des);
	
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
                   							
* Output         	:
* Return         	:
*******************************************************************************/
void ProcOptPumpPush(uint16_t speed,uint8_t maxcircle)
{
	StartPump(PUSH_LIQUID,speed,maxcircle);
	
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
                   							
* Output         	:
* Return         	:
*******************************************************************************/
void ProcOptPumpDraw(uint16_t speed,uint8_t maxcircle)
{
	StartPump(DRAW_LIQUID,speed,maxcircle);	
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
                   							
* Output         	:
* Return         	:
*******************************************************************************/
void ProcForcedEmptyPool(DEVICE_CODE dev,FUN_VALVE_CODE des,uint8_t drawcircle,uint8_t pushcircle,uint8_t times)
{
//	SPEED_REACTION_POOL_SOLUTION
	for(int i=0;i<times;i++)
	{
		OpenFunValve(dev,F_VALVE_POOL_ONLY);
		osDelay(500);
		StartPump(DRAW_LIQUID,SPEED_REACTION_POOL_SOLUTION,drawcircle);
		while(!TimeoutFlag)
			osDelay(10);
		CloseFunValve(dev,F_VALVE_POOL_ONLY);
		osDelay(500);
		SendAir(dev,des,SPEED_REACTION_POOL_SOLUTION,pushcircle);
	}
	
}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcReadV3ADs(uint8_t readADTimes)
{
	//printf("\r\n读空白AD值。\r\n");
	
	GetADabsReal(&pMbHoldData->nBlankRefLtAD,&pMbHoldData->nBlankTranLtAD,readADTimes);
//	pMbHoldData->fBlankAbs = log(	(double)pMbHoldData->nBlankRefLtAD /
//																(double)pMbHoldData->nBlankTranLtAD );
	osDelay(1000);
	// pMbInputData->nStepCode = STEP_READ_BLANK_AD_DONE;
	osDelay(1000);

}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcReadV2ADs(uint8_t readADTimes)
{
	//printf("\r\n读空白AD值。\r\n");
	
	GetADabsReal(&pMbHoldData->nBlankRefLtAD,&pMbHoldData->nReactionRefLtAD,readADTimes);
//	pMbHoldData->fBlankAbs = log(	(double)pMbHoldData->nBlankRefLtAD /
//																(double)pMbHoldData->nBlankTranLtAD );
	osDelay(1000);
	// pMbInputData->nStepCode = STEP_READ_BLANK_AD_DONE;
	osDelay(1000);

}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcReadV1ADs(uint8_t readADTimes)
{
	//printf("\r\n读空白AD值。\r\n");
	
	GetADabsReal(&pMbHoldData->nBlankRefLtAD,&pMbHoldData->nReactionTranLtAD,readADTimes);
//	pMbHoldData->fBlankAbs = log(	(double)pMbHoldData->nBlankRefLtAD /
//																(double)pMbHoldData->nBlankTranLtAD );
	osDelay(1000);
	// pMbInputData->nStepCode = STEP_READ_BLANK_AD_DONE;
	osDelay(1000);
;
}
/*******************************************************************************
* Function Name  	: ProcReadADs
* Description    	: 读取AD值
* Input          	: 读取次数
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcReadYSADs(uint8_t readADTimes)
{
	//printf("\r\n读空白AD值。\r\n");
	
	GetADabsReal(&pMbHoldData->nBlankRefLtAD,&pMbHoldData->nBlankTranLtAD,readADTimes);
//	pMbHoldData->fBlankAbs = log(	(double)pMbHoldData->nBlankRefLtAD /
//																(double)pMbHoldData->nBlankTranLtAD );
	osDelay(1000);
	// pMbInputData->nStepCode = STEP_READ_BLANK_AD_DONE;
	osDelay(1000);

}
/*******************************************************************************
* Function Name  	: ProcCalculate
* Description    	: 通知上位机计算结果
* Input          	:calibMode：标定模式
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcCalculateYS(void)
{
//	float ftemp;
//	float re;
//	switch(pMbInputData->nProcCode)
//	{
//		case RROC_AUTO_RROC_CALIBRAT_LOW:
//		case RROC_CALIBRAT_LOW:
//			pMbHoldData->fMeasAbs	= log10((double)pMbHoldData->nReactionTranLtAD / (double)pMbHoldData->nReactionRefLtAD);//log10()
//			ftemp = pMbHoldData->fKturbidity * ((double)pMbHoldData->nReactionTranLtAD / (double)pMbHoldData->nBlankTranLtAD);//log10()
//			pMbHoldData->fMeasAbs -= ftemp;
//			pMbHoldData->fCalibLowAbs = pMbHoldData->fMeasAbs;
//		break;
//		case RROC_AUTO_RROC_CALIBRAT_HIGH:
//		case RROC_CALIBRAT_HIGH:
//			pMbHoldData->fMeasAbs	= log10((double)pMbHoldData->nReactionTranLtAD / (double)pMbHoldData->nReactionRefLtAD);//log10()
//			ftemp = pMbHoldData->fKturbidity * ((double)pMbHoldData->nReactionTranLtAD / (double)pMbHoldData->nBlankTranLtAD);//log10()
//			pMbHoldData->fMeasAbs -= ftemp;
//			pMbHoldData->fCalibHighAbs = pMbHoldData->fMeasAbs;
//		break;
//		default:
//			pMbHoldData->fMeasAbs	= log10((double)pMbHoldData->nReactionTranLtAD / (double)pMbHoldData->nReactionRefLtAD);//log10()
//			ftemp = pMbHoldData->fKturbidity * ((double)pMbHoldData->nReactionTranLtAD / (double)pMbHoldData->nBlankTranLtAD);//log10()
//			pMbHoldData->fMeasAbs -= ftemp;
//			re = (pMbHoldData->fMeasAbs) * pMbHoldData->fCalibKValue + pMbHoldData->fCalibBValue;
//			if(re < 0)
//			{
//				float ran = (float)(rand() & 0x00FF);
//				ran /= 256;
//				re = ran * 0.1;
//			}			
//			pMbHoldData->fMeasureResult = re;
//			break;
//	}
//	
//	osDelay(2000);
}
