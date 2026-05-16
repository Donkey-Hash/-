

#include "ProcAPI.h"

bool ProcTN(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime)
{
	USART2Init(115200);
//	OOpInit(5,220,1);
	if(false == ProcPreproc(dev))		return false;
	ProcEmptyPool(dev,CLEAN_WASTE_VALVE);	
	if(false == ProcResetUV())	
		return false ;
	
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);  ////更新进度条,样品润洗
	ProcDelay(1);
	if(false == ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,12) ) //蒸馏水清洗  
		return false;

	if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,12,true))//加蒸馏水
		return false;
	
	
	ProcUpdateStepStatus(STEP_READ_REF);//读空白吸光度   220 275
	if(false == ProcReadBlankUVData())
		return false ;	
//	if(false == ProcReadBlankOOpUVData())
//		return false ;
	ProcEmptyPool(dev,CLEAN_WASTE_VALVE);															//排空比色池	
	
	ProcUpdateStepStatus((STEP_NAME_CODE)agentCode);//更新进度条,加水样
	ProcDelay(1);
	switch(range)
	{
		case 0:
			ProcWashTubeWithAgent(dev,agentCode,3,false);				
			ProcWashTubeWithAgent(dev,agentCode,3,false);				
			if(false == ProcAddAgentToPool(dev,agentCode,4,true)) //加水样            3mL改为4mL-志强190427
				return false; 
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,6,true)) //加零点液
				return false;

			break;
		case 1:                                                    //增加0-25量程-志强190427
			ProcWashTubeWithAgent(dev,agentCode,3,false);				//洗定量管  
			ProcWashTubeWithAgent(dev,agentCode,3,false);				//洗定量管  
			if(false == ProcAddAgentToPool(dev,agentCode,1,true)) //加水样
				return false;
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管  3mL改为1mL-志强190427
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true)) //加蒸馏水
			   return false; //加蒸馏水
			break;
		case 2:
		    ProcWashTubeWithAgent(dev,agentCode,3,false);				//洗定量管
			ProcWashTubeWithAgent(dev,agentCode,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,agentCode,1,true)) //加水样     
			return false;
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管  3mL改为1mL-志强190427
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //加蒸馏水
			ProcDrawPool(dev,CLEAN_WASTE_VALVE,7);                //9改成7 -志强20201212
			ProcAirPool(dev);
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管  3mL改为1mL-志强190427
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,7,true))return false; //加蒸馏水   //9改成7 -志强20201212
			break;
			
	}
		
	if(pumpTime != 0)
	{
		CloseFunValve(dev,WATER_PUMP_VALVE);//关水泵
	}

	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_OXIDANT);
	ProcDelay(1);
	
	if(false == ProcAddAgentToPool(dev,AGENT_OXIDANT,1,true))//加氧化剂
		return false;
	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管 
	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管
	
	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_DIGESTION);
	ProcDelay(1);	
	if(false == ProcAddAgentToPool(dev,AGENT_DIGESTION,1,true))//加消解液
		return false;
	
	ProcAirPool(dev); //向池子吹气
	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);				//洗定量管  3mL改为1mL-志强090427
	
	ProcUpdateStepStatus(STEP_HOTTING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,hotTemp,hotTime);//开启加热控制

	ProcUpdateStepStatus(STEP_COOLING);
	ProcDelay(1);
	ProcTempCtrl(true,TEMP_CTRL_CH_POOL,50,0); //冷却控制

	ProcUpdateStepStatus((STEP_NAME_CODE)AGENT_NEUTRALIZER);
	ProcDelay(1);	
	if(false == ProcAddAgentToPool(dev,AGENT_NEUTRALIZER,1,true))//加中和剂  3mL改为1mL-志强090427
		return false;
	ProcAirPool(dev); //向池子吹气
	ProcUpdateStepStatus(STEP_COOLING);
	ProcDelay(1);
	//ProcTempCtrl(true,TEMP_CTRL_CH_POOL,50,hotTime); //冷却控制   
	
	ProcUpdateStepStatus(STEP_READ_ABS);
	ProcDelay(2);
	ProcReadReactUVData();	
//	ProcReadReactOOpUVData();
	ProcCalculate();    					//通知上位机计算结果
		
	ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);			//更新进度条名称
	ProcDelay(1);
	ProcEmptyPool(dev,CLEAN_WASTE_VALVE);					//排空反应池		
	ProcDelay(1);
	ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);    //洗定量管                 
	if(false == ProcAddAgentToPool(dev,AGENT_NEUTRALIZER,1,false))return false;//加中和剂    3mL改1 -志强090427
	ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,13);				//清洗反应池         12mL改13 -志强090427
	ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,15);				//清洗反应池
	ProcStopUpdateStepStatus();

	ProcDelay(10);
	return true;
}




