

#include "ProcAPI.h"

bool ProcCODMnOverflow(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime)
{
	USART2Init(9600);
	InitSyringeTitrationPumpParam();
	if(false == ProcPreproc(dev))
		return false;

	
	ProcEmptyPoolWithPump(dev,REACT_WASTE_VALVE,30);	
//	OVERFLOW_SAMPLE_SRC, 											//溢流，抽取水样通路
//	OVERFLOW_SAMPLE_POOL,											//溢流，水样进反应池通路
//	OVERFLOW_DISTILLED_WATER_SRC,							//溢流，抽取零点液通路
//	OVERFLOW_DISTILLED_WATER_POOL,						//溢流，抽取零点液通路
//	OVERFLOW_NEUTRALIZER_SRC,									//
//	OVERFLOW_NEUTRALIZER_POOL,								//
//	OVERFLOW_DIGESTION_SRC_POOL,
	ProcUpdateStepStatus(STEP_WASH_WITH_SAMPLE);
	ProcOptFunValve(dev,OVERFLOW_SAMPLE_SRC,180);
//	ProcOptPumpPush(600,30);
	ProcDelay(60);
	ProcOptFunValve(dev,OVERFLOW_SAMPLE_POOL,30);
	ProcDelay(1);
	ProcOptFunValve(dev,STIR_MOTOR,20);
	ProcDelay(1);
	ProcEmptyPoolWithPump(dev,REACT_WASTE_VALVE,30);	
	ProcDelay(1);
	
	ProcUpdateStepStatus(STEP_ADD_SAMPLE);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_SAMPLE_SRC,180);
	ProcOptPumpPush(600,30);
	ProcDelay(60);
	ProcOptFunValve(dev,OVERFLOW_SAMPLE_POOL,30);
	ProcDelay(1);
	ProcOptFunValve(dev,STIR_MOTOR,20);
	ProcDelay(1);
	OpenFunValve(dev,STIR_MOTOR);
	
	ProcUpdateStepStatus(STEP_ADD_DIGESTION);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_DIGESTION_SRC_POOL,8);
	
	ProcUpdateStepStatus(STEP_ADD_OXIDANT);
	ProcDelay(1);
	if(false == ProcAddAgentToPool(dev,AGENT_OXIDANT,3500,true))//氧化剂
		return false;
	
	ProcUpdateStepStatus(STEP_HOTTING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,hotTemp,hotTime);//开启加热控制
	
	ProcUpdateStepStatus(STEP_ADD_NEUTRALIZER);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_NEUTRALIZER_SRC,10);
	ProcDelay(1);
	ProcOptFunValve(dev,FAN_VALVE,30);
	ProcDelay(5);
	ProcOptFunValve(dev,OVERFLOW_NEUTRALIZER_POOL,20);
	ProcDelay(1);
	
	ProcUpdateStepStatus(STEP_HOTTING);
	ProcTempCtrlWithoutHoldTime(true,TEMP_CTRL_CH_POOL,80,TEMPER_CTRL_HOT);
	ProcDelay(10);
	OpenFunValve(dev,STIR_MOTOR);
	ProcUpdateStepStatus(STEP_TITRATING);
	ProcDelay(3);
	
	if(false == ProcTitrationBySyringe(dev,pMbHoldData->nReactionRefLtAD) )       //滴定
		return false;
	ProcSendBlankADs();
	ProcDelay(2);	
	ProcSendRectADs();
	ProcDelay(2);	
	ProcCalculate();    					//通知上位机计算结果		
	ProcDelay(2);
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);
	ProcDelay(1);
	
	CloseFunValve(dev,STIR_MOTOR);
	ProcEmptyPoolWithPump(dev,REACT_WASTE_VALVE,40);
	
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_DISTILLED_WATER_SRC,20);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_DISTILLED_WATER_POOL,20);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_DISTILLED_WATER_SRC,20);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_DISTILLED_WATER_POOL,20);
	ProcDelay(1);
	OpenFunValve(dev,STIR_MOTOR);
	CloseFunValve(dev,STIR_MOTOR);
	ProcEmptyPoolWithPump(dev,REACT_WASTE_VALVE,40);	
	
	ProcOptFunValve(dev,OVERFLOW_DISTILLED_WATER_SRC,20);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_DISTILLED_WATER_POOL,20);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_DISTILLED_WATER_SRC,20);
	ProcDelay(1);
	ProcOptFunValve(dev,OVERFLOW_DISTILLED_WATER_POOL,20);
	ProcDelay(1);
	OpenFunValve(dev,STIR_MOTOR);
	CloseFunValve(dev,STIR_MOTOR);
	ProcEmptyPoolWithPump(dev,REACT_WASTE_VALVE,40);	
	
	ProcStopUpdateStepStatus();
	ProcDelay(10);
	return true;
}




