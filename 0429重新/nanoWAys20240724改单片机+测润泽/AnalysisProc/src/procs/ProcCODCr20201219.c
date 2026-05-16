

#include "ProcAPI.h"

bool ProcCODCr(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime)
{
	if(false == ProcPreproc(dev))
		return false;
	ProcEmptyPool(dev,F_VALVE_CLEAN_WASTE);
	

	if(pumpTime != 0)
	{
		ProcUpdateStepStatus(STEP_PUMP_WATER);
		OpenFunValve(dev,F_VALVE_WATER_PUMP);				//开水泵
		ProcDelay(pumpTime*60);
	}
	ProcUpdateStepStatus(STEP_WASH_WITH_SAMPLE);  ////更新进度条,样品润洗
	ProcDelay(1);
	if(false == ProcWashPoolWithAgent(dev,agentCode,6) ) 
		return false;
	if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,false)) 
		return false;
	ProcUpdateStepStatus(STEP_READ_REF);																							//更新进度条名称			
	ProcReadBlankADs(10);//读AD值
	ProcDelay(2);
	ProcEmptyPool(dev,F_VALVE_CLEAN_WASTE);
//	if(false == ProcWashPoolWithAgent(dev,range==0?agentCode:AGENT_SAMPLE,range==0?3:6) ) //样品润洗
//		return false;
//	if(false == ProcWashPoolWithAgent(dev,range==0?agentCode:AGENT_SAMPLE,range==0?3:6) )
//		return false;

	ProcUpdateStepStatus((STEP_NAME_CODE)agentCode);//更新进度条,加水样
	ProcDelay(1);
	switch(range)
	{
		case 0:
			if(false == ProcAddAgentToPool(dev,agentCode,5,true)) //加水样
				return false;
			else
				ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);//清定量管
//				ProcPushBackAgent(dev,agentCode);//水样回推
			break;
		case 1:
			if(false == ProcAddAgentToPool(dev,agentCode,4,true)) //加水样   
				return false;
			else
				ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);//清定量管
				ProcPushBackAgent(dev,agentCode);//水样回推
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);			//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);	   //洗定量管  		
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,1,true))return false; //加蒸馏水 4
			break;
		case 2:
			if(false == ProcAddAgentToPool(dev,agentCode,1,true)) //加水样   
				return false;
			else
				ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);//清定量管
				ProcPushBackAgent(dev,agentCode);//水样回推
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);			//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);	   //洗定量管  		
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,4,true))return false; //加蒸馏水 4
			break;
			
	}
	if(pumpTime != 0)
	{
		CloseFunValve(dev,F_VALVE_WATER_PUMP);//关水泵
	}
	
		

	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_MERCUCY_SULFATE);
	ProcDelay(1);
	if(false == ProcAddAgentToPool(dev,AGENT_MERCUCY_SULFATE,1,true))//加硫酸汞
		return false;
	ProcAirPool(dev); //向池子吹气
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);//清定量管
	
	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_POTASSIUM_DICHROMATE);
	ProcDelay(1);
	if(false == ProcAddAgentToPool(dev,AGENT_POTASSIUM_DICHROMATE,1,true))//加重铬酸钾
		return false;
	ProcAirPool(dev); //向池子吹气
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE); //清定量管
	
	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_SULPHURIC_ACID);
	ProcDelay(1);
		
//	if(false == ProcAddAgentToPool(dev,AGENT_SULPHURIC_ACID,1,true))  //加硫酸
//		return false;
//	ProcDelay(1);
	if(false == ProcAddAgentToPool(dev,AGENT_SULPHURIC_ACID,3,true))//加硫酸
		return false;
	
	ProcAirPool(dev);   //向池子吹气
	ProcAirPool(dev);   //向池子吹气
	ProcAirPool(dev);   //向池子吹气
	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);							//洗定量管
	ProcUpdateStepStatus(STEP_HOTTING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,hotTemp,hotTime);//开启加热控制

	ProcUpdateStepStatus(STEP_COOLING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,90,0); //冷却控制
	ProcAirPool(dev);   //向池子吹气
	ProcAirPool(dev);   //向池子吹气
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,90,3); //冷却控制
	ProcUpdateStepStatus(STEP_READ_ABS);
	ProcDelay(1);

	ProcReadReactADs(10);		
	ProcCalculate();    					//通知上位机计算结果
		
	ProcDelay(2);
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);
	ProcDelay(1);
	ProcForcedEmptyPool(dev,F_VALVE_REACT_WASTE,7,8,6);
	ProcEmptyPool(dev,F_VALVE_REACT_WASTE);
	
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);
	ProcDelay(1);
	ProcFillTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9);
	ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9);
	ProcStopUpdateStepStatus();
	ProcDelay(10);
	return true;
}




