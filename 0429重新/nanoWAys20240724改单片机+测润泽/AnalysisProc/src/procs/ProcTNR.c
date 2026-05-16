

#include "ProcAPI.h"

bool ProcTNR(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime) //总氮-间苯二酚标签TNR 2019-05-19
{
//	AGENT_CODE agentCode = AGENT_SAMPLE;
	if(false == ProcPreproc(dev))
		return false;
	
	ProcEmptyPool(dev,F_VALVE_CLEAN_WASTE);								//排空比色池	
	if(pumpTime != 0)
	{
		ProcUpdateStepStatus(STEP_PUMP_WATER);
		OpenFunValve(dev,F_VALVE_WATER_PUMP);				//开水泵
		ProcDelay(pumpTime*60);
	}
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);
	ProcDelay(1);
//	ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9);
	ProcUpdateStepStatus(STEP_WASH_WITH_SAMPLE);  ////更新进度条,样品润洗
	ProcDelay(1);
//	if(false == ProcWashPoolWithAgent(dev,range==0?agentCode:AGENT_SAMPLE,range==0?3:6) ) //样品润洗
//		return false;
//	if(false == ProcWashPoolWithAgent(dev,range==0?agentCode:AGENT_SAMPLE,range==0?3:6) )
//		return false;
	
	if(false == ProcWashPoolWithAgent(dev,agentCode,9) )//样品润洗                           190103  样品润洗与零点润洗调换顺序  
		return false;	
	if(false == ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,12) ) //零点液润洗
		return false;	

	ProcUpdateStepStatus((STEP_NAME_CODE)agentCode);//更新进度条,加水样
	ProcDelay(1);
	switch(range)
	{
		case 0:  //TNR 0-5 20190519
			if(false == ProcAddAgentToPool(dev,agentCode,4,true)) //加水样  4mL -20190519     
				return false;
//			else
//				ProcPushBackAgent(dev,agentCode);//水样回推
			break;
		case 1: //TNR 0-20 20190519
			if(false == ProcAddAgentToPool(dev,agentCode,1,true)) return false;//加水样 1mL 20190519
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,3,true)) return false; //加蒸馏水	3mL 20190519
			break;
		//case 2:
		//	if(false == ProcAddAgentToPool(dev,agentCode,1,true)) return false;//加水样
		//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管
		//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
		//	if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //加蒸馏水	
		//	break;
		//case 3:
		//	if(false == ProcAddAgentToPool(dev,agentCode,1,true)) return false;//加水样
		//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管
		//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
		//	if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //加蒸馏水	
		//  ProcDrawPool(dev,F_VALVE_CLEAN_WASTE,8);
		//	ProcAirPool(dev); //向池子吹气                                   		//190416添加
		//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
		//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
		//  if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,8,true))return false; //加蒸馏水	
		//	break;
		default:
			break;
			
	}
	if(pumpTime != 0)
	{
		CloseFunValve(dev,F_VALVE_WATER_PUMP);//关水泵
	}

	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_OXIDANT);//加消解液 20190520
	ProcDelay(1);
	ProcPreagentPreWashTube(dev,AGENT_OXIDANT,1,true);								//190429 增加
	//ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	if(false == ProcAddAgentToPool(dev,AGENT_OXIDANT,1,true))//加氧化剂  对应消解液20190520
		return false;
	ProcAirPool(dev); //向池子吹气
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE); //清定量管
	

	
	ProcUpdateStepStatus(STEP_HOTTING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,hotTemp,hotTime);//开启加热控制

	ProcUpdateStepStatus(STEP_COOLING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,70,0); //冷却控制
	

	//ProcUpdateStepStatus(STEP_READ_REF);																							//更新进度条名称		
	//ProcDelay(30);
	//ProcReadBlankADs(10);	
	
	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_CHROMOGENIC);	//加硫酸20190519
	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//纯水洗定量管 20190519
	//ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	if(false == ProcAddAgentToPool(dev,AGENT_CHROMOGENIC,7,true))return false; //加显色剂-对应加硫酸 7mL 20190519
	ProcAirPool(dev);																											  //向反应池吹气
  
    ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_REDUCER); //加间苯二酚
	ProcDelay(1);
	ProcPreagentPreWashTube(dev,AGENT_REDUCER,1,true);        //190429 增加
	//ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);  //清空定量管
	if(false == ProcAddAgentToPool(dev,AGENT_REDUCER,1,true))//加还原剂  对应间苯二酚显色剂	20190519
		return false;
	ProcAirPool(dev); //向池子吹气
	ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE); //清定量管
		
	
	ProcUpdateStepStatus(STEP_HOTTING);
    ProcTempCtrl(true,TEMP_CTRL_CH_POOL,70,2);//开启加热控制
	

	ProcUpdateStepStatus(STEP_READ_ABS);
	ProcDelay(20);

	ProcReadReactADs(10);		
	ProcCalculate();    					//通知上位机计算结果
	
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,50,0); //冷却控制 降温到50度后排液20190520
		
	ProcDelay(2);
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);
	ProcDelay(1);
	ProcEmptyPool(dev,F_VALVE_REACT_WASTE);
	

	ProcDelay(1);
//	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管 190416 注销
	ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,3);       //190416 增加
	ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,12);

	ProcStopUpdateStepStatus();
	ProcDelay(10);
	return true;
}




