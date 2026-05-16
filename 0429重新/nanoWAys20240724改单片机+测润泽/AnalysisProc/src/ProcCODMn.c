

#include "ProcAPI.h"

bool ProcCODMn(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime)
{
//	USART2Init(9600);
	InitSyringeTitrationPumpParam();
	if(false == ProcPreproc(dev))
		return false;

	
	ProcEmptyPoolWithPump(dev,F_VALVE_REACT_WASTE,30);
	ProcUpdateStepStatus(STEP_PUMP_WATER);
if(pumpTime != 0)
	{
		ProcUpdateStepStatus(STEP_PUMP_WATER);
		OpenFunValve(dev,F_VALVE_WATER_PUMP);				//开水泵
		ProcDelay(pumpTime*60);
	}	
//	ProcUpdateStepStatus(STEP_WASH_WITH_SAMPLE);  ////更新进度条,样品润洗
	ProcDelay(1);
//	if(false == ProcWashPoolWithAgent(dev,agentCode,6) ) //样品润洗
//		return false;
//	if(false == ProcWashPoolWithAgent(dev,agentCode,6) )
//		return false;
	pMbInputData->nPoolTranLtBackAD = 0;
	pMbInputData->nPoolRefLtBackAD = 0;
//	if(false == ProcAddAgentToPool(dev,agentCode,6,true))  //  加水样
//		return false;
//	ProcAirPool(dev); 
//	ProcEmptyPoolWithPump(dev,F_VALVE_REACT_WASTE,20);     //排空
//	if(false == ProcAddAgentToPool(dev,agentCode,6,true))  //加水样
//		return false;
//	ProcAirPool(dev); 
//	ProcEmptyPoolWithPump(dev,F_VALVE_REACT_WASTE,20);     //排空
	
//	if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,12,true))  //
//		return false;
//	ProcAirPool(dev); 
//	ProcEmptyPoolWithPump(dev,F_VALVE_REACT_WASTE,30);

	ProcUpdateStepStatus((STEP_NAME_CODE)agentCode);//更新进度条,加水样
	ProcDelay(1);
	switch(range)
	{
		case 0:
			if(false == ProcAddAgentToPool(dev,agentCode,21,true)) //加水样
				return false;
			else
				ProcPushBackAgent(dev,agentCode);//水样回推
			break;
		case 1:
			break;
		case 2:
			break;			
	}
	if(pumpTime != 0)
	{
		CloseFunValve(dev,F_VALVE_WATER_PUMP);//关水泵
	}

	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_DIGESTION);
	ProcDelay(1);
	if(false == ProcAddAgentToPool(dev,AGENT_DIGESTION,3,true))//消解液
		return false;
//	ProcAirPool(dev); //向池子吹气
	OpenFunValve(dev,F_VALVE_STIR_MOTOR);
	ProcDelay(5);
	CloseFunValve(dev,F_VALVE_STIR_MOTOR);
	
	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false); //洗定量管

	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);
//	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);//清定量管
	
	
	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_OXIDANT);
	ProcDelay(1);
	
	OpenFunValve(dev,F_VALVE_STIR_MOTOR);
	ProcDelay(10);

	if(false == ProcAddAgentToPool(dev,AGENT_OXIDANT,3500,true))//氧化剂
		return false;
//	ProcTitraPump(dev,TITRATION_PUMP,1000);      //滴定泵加氧化剂
	
	
//	OpenFunValve(dev,(FUN_VALVE_CODE)AGENT_OXIDANT);
//	ProcDelay(140);
//	CloseFunValve(dev,(FUN_VALVE_CODE)AGENT_OXIDANT);
//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false); //洗定量管
//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);
	
//	CloseFunValve(dev,F_VALVE_STIR_MOTOR);
	

//	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE); //清定量管
	

	
	ProcUpdateStepStatus(STEP_HOTTING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,hotTemp,hotTime);//开启加热控制

	ProcUpdateStepStatus(STEP_COOLING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,80,0); //冷却控制

	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_NEUTRALIZER);
	ProcDelay(1);
//	ProcTitraPump(dev,F_VALVE_NEUTRALIZER,600);



	if(false == ProcAddAgentToPool(dev,AGENT_NEUTRALIZER,3,true))  //中和液F_VALVE_NEUTRALIZER
		return false;
//	ProcWashTubeWithAgent(dev,AGENT_NEUTRALIZER,1,false);							//洗定量管
//	ProcAirLoopPool(dev);    																			//通过定量环向比色池吹气
	
 																			//通过定量环向比色池吹气
	
	
//	OpenFunValve(dev,(FUN_VALVE_CODE)AGENT_NEUTRALIZER);
//	ProcDelay(65);
//	CloseFunValve(dev,(FUN_VALVE_CODE)AGENT_NEUTRALIZER);
	
	ProcUpdateStepStatus(STEP_HOTTING);
	ProcTempCtrlWithoutHoldTime(true,TEMP_CTRL_CH_POOL,70,TEMPER_CTRL_HOT);
	ProcDelay(10);
	OpenFunValve(dev,F_VALVE_STIR_MOTOR);
//  uint32_t titrationtime;
	ProcUpdateStepStatus(STEP_TITRATING);
	ProcDelay(3);
//	uint16_t initAD = ProcGetReactAD();
//	pMbHoldData->nBlankTranLtAD = (uint16_t)(Temp1PV *10 + 0.5);
//	pMbHoldData->nPoolTranLtBackAD = 0xFFFF;
//	pMbHoldData->nReactionRefLtAD = ProcGetReactAD(10);
	if(false == ProcTitrationBySyringe(dev,pMbHoldData->nReactionRefLtAD) )       //滴定
		return false;
	ProcSendBlankADs();
//	uint16_t endAD = ProcGetReactAD();
	ProcDelay(2);
	
	ProcSendRectADs();
	ProcDelay(2);
	
	ProcCalculate();    					//通知上位机计算结果
		
	ProcDelay(2);
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);
	ProcDelay(1);
	
	ProcEmptyPoolWithPump(dev,F_VALVE_REACT_WASTE,30);


	
//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);
//	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	
	
//	if(false == ProcAddAgentToPool(dev,AGENT_NEUTRALIZER,1,true))  //中和液
//		return false;


OpenFunValve(dev,F_VALVE_STIR_MOTOR);
	if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,27,true))  //
		return false;
	ProcEmptyPoolWithPump(dev,F_VALVE_REACT_WASTE,40);
	
	if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,27,true))  //
		return false;

	
	ProcEmptyPoolWithPump(dev,F_VALVE_REACT_WASTE,40);
	CloseFunValve(dev,F_VALVE_STIR_MOTOR);
//	ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9);

	ProcStopUpdateStepStatus();
	ProcDelay(10);
	return true;
}




