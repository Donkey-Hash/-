

#include "ProcAPI.h"

bool ProcTP(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime)
{

	if(false == ProcPreproc(dev))
		return false;
	
	ProcEmptyPool(dev,F_VALVE_CLEAN_WASTE);								//排空比色池	
	ProcUpdateStepStatus(STEP_PUMP_WATER);
	if(pumpTime != 0)
	{
		ProcUpdateStepStatus(STEP_PUMP_WATER);
		OpenFunValve(dev,F_VALVE_WATER_PUMP);				//开水泵
		ProcDelay(pumpTime*60);
	}

	ProcUpdateStepStatus(STEP_WASH_WITH_SAMPLE);  ////更新进度条,样品润洗
	ProcDelay(1);
	
	if(false == ProcWashPoolWithAgent(dev,agentCode,9) )//样品润洗                           190103  样品润洗与零点润洗调换顺序
		return false;	
	if(false == ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,12) ) //零点液润洗
		return false;	
//  ProcWashTubeWithAgent(dev,agentCode,3,false);				//洗定量管
	ProcUpdateStepStatus((STEP_NAME_CODE)agentCode);//更新进度条,加水样
	ProcDelay(1);
	switch(range)
	{
		case 0:
			if(false == ProcAddAgentToPool(dev,agentCode,9,true)) //加水样
				return false;
			else
				ProcPushBackAgent(dev,agentCode);//水样回推
			ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);//清定量管				
			break;
		case 1:
			if(false == ProcAddAgentToPool(dev,agentCode,3,true)) return false;//加水样
		else
				ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);//清定量管
				ProcPushBackAgent(dev,agentCode);//水样回推
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,8,true))return false; //加蒸馏水	
			break;
		case 2:
			if(false == ProcAddAgentToPool(dev,agentCode,2,true)) return false;//加水样
			else
				ProcPushBackAgent(dev,agentCode);//水样回推
			ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);//清定量管
				
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //加蒸馏水	
			break;
		case 3:
			if(false == ProcAddAgentToPool(dev,agentCode,2,true)) return false;//加水样
			else
				ProcPushBackAgent(dev,agentCode);//水样回推
			ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);//清定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //加蒸馏水	
		  ProcDrawPool(dev,F_VALVE_CLEAN_WASTE,8);
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
		  if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //加蒸馏水	
			break;
		default:
			break;
			
	}
	if(pumpTime != 0)
	{
		CloseFunValve(dev,F_VALVE_WATER_PUMP);//关水泵
	}

	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_OXIDANT);
	ProcDelay(1);

	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcPreagentPreWashTube(dev,AGENT_OXIDANT,1,false);	
	if(false == ProcAddAgentToPool(dev,AGENT_OXIDANT,1,true))//加氧化剂
		return false;
	ProcAirPool(dev); //向池子吹气
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE); //清定量管
	

	
	ProcUpdateStepStatus(STEP_HOTTING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,hotTemp,hotTime);//开启加热控制

	ProcUpdateStepStatus(STEP_COOLING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,70,0); //冷却控制
	
  ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_REDUCER);
	ProcDelay(1);

	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcPreagentPreWashTube(dev,AGENT_REDUCER,1,false);	
	if(false == ProcAddAgentToPool(dev,AGENT_REDUCER,1,true))//加还原剂
		return false;
	ProcAirPool(dev); //向池子吹气
	ProcAirPool(dev); //向池子吹气
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE); //清定量管
	
	ProcUpdateStepStatus(STEP_HOTTING);
  ProcTempCtrl(true,TEMP_CTRL_CH_POOL,70,1);//开启加热控制
	
	ProcUpdateStepStatus(STEP_READ_REF);																							//更新进度条名称		
//////	ProcDelay(60);
	ProcReadBlankADs(10);	
	
	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_CHROMOGENIC);	
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcPreagentPreWashTube(dev,AGENT_CHROMOGENIC,1,false);	
	if(false == ProcAddAgentToPool(dev,AGENT_CHROMOGENIC,1,true))return false; //加显色剂
	ProcAirPool(dev);																											  //向反应池吹气
	
	ProcUpdateStepStatus(STEP_HOTTING);
  ProcTempCtrl(true,TEMP_CTRL_CH_POOL,72,3);//开启加热控制
	

	ProcUpdateStepStatus(STEP_READ_ABS);
	ProcDelay(20);

	ProcReadReactADs(10);		
	ProcCalculate();    					//通知上位机计算结果
		
	ProcDelay(2);
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);
	ProcDelay(1);
	ProcForcedEmptyPool(dev,F_VALVE_REACT_WASTE,5,6,6);
	ProcEmptyPool(dev,F_VALVE_REACT_WASTE);
	

	ProcDelay(1);
	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
	ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,12);

	ProcStopUpdateStepStatus();
	ProcDelay(10);
	return true;
}




