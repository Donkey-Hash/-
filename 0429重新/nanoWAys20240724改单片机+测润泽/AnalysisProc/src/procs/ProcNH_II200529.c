
#include "ProcAPI.h"
extern bool DrawTestFlag;

bool ProcNH_II(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime)
{	

	if(false == ProcPreproc(dev))
		return false;
	ProcEmptyPool(dev,CLEAN_WASTE_VALVE);
	ProcUpdateStepStatus(STEP_WASH_WITH_SAMPLE);        //更新进度条名称
	ProcDelay(1);
	if(false == ProcWashPoolWithAgent(dev,agentCode,9) )//样品润洗
		return false;
	if(false == ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9) ) //零点液润洗
		return false;	
	ProcWashTubeWithAgent(dev,agentCode,1,false);       //洗定量管
	ProcUpdateStepStatus((STEP_NAME_CODE)agentCode);						//更新进度条名称
	ProcDelay(1);
	switch(range)
		{
		case 0://0-2
			if(false == ProcAddAgentToPool(dev,agentCode,4,true))return false; //加水样				
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);       //洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,6,true))return false; //加蒸馏水
			ProcAirPool(dev);     //向池子吹气
			ProcEmptyTube(dev,CLEAN_WASTE_VALVE);		//排空定量管
		break;
		case 1://0-10  5.5倍
//			ProcWashTubeWithAgent(dev,agentCode,1,false);						//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,1,false);						//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,1,false);						//洗定量管
			if(false == ProcAddAgentToPool(dev,agentCode,1,true))return false; //加水样				
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);		//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);		//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //加蒸馏水
			ProcAirPool(dev);									//向池子吹气
			ProcEmptyTube(dev,CLEAN_WASTE_VALVE);								//排空定量管
		break;
		case 2://0-50 
//			ProcWashTubeWithAgent(dev,agentCode,1,false);							//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,1,false);							//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,1,false);							//洗定量管
			if(false == ProcAddAgentToPool(dev,agentCode,1,true))return false; //加水样	
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);			//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);	   //洗定量管  		
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,3,true))return false; //加蒸馏水
			ProcAirPool(dev);																							//向池子吹气
			ProcAirPool(dev);																							//向池子吹气
			ProcEmptyTube(dev,CLEAN_WASTE_VALVE);																						//排空定量管
			ProcFillLoop(dev);									 													//定量环定量。
			ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,6);						//洗反应池
		  ProcAirLoopPool(dev);    																			//通过定量环向比色池吹气
			ProcAddAgentLoopPool(dev,AGENT_DISTILLED_WATER,9,true);				//通过定量环向反应池加零点液
		break;
		case 3://0-250
//			ProcWashTubeWithAgent(dev,agentCode,3,false);													//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,3,false);													//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,3,false);													//洗定量管
			if(false == ProcAddAgentToPool(dev,agentCode,6,true))return false; 		//加水样	
			ProcFillLoop(dev);    																								//定量环定量。
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);							//洗定量管
			ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,6); 									//清洗比色池
			ProcAirLoopPool(dev);    																							//通过定量环向比色池吹气
			if(false == ProcAddAgentLoopPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //加零点液,稀释
			ProcAirLoopPool(dev);    																							//通过定量环向比色池吹气
			ProcAirLoopPool(dev);    																							//通过定量环向比色池吹气
			ProcFillLoop(dev);																										//定量环定量
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);							//洗定量管
			ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9);										//清洗比色池
			ProcAirLoopPool(dev);    																							//通过定量环向比色池吹气
			if(false == ProcAddAgentLoopPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //加零点液,稀释
		break;
		default:break;
		}
		if(pumpTime != 0)
			ProcCloseWaterPump(dev);																	  //关水泵
		ProcEmptyTube(dev,CLEAN_WASTE_VALVE);																											//排空定量管
//		ProcPushBackAgent(dev,agentCode);																				//回推水样
		
		ProcUpdateStepStatus(STEP_READ_REF);																							//更新进度条名称				
		ProcReadBlankADs(10);//读AD值
		ProcDelay(10);		
		ProcUpdateStepStatus(STEP_ADD_CHROMOGENIC);																				//更新进度条名称
		ProcDelay(1);
		DrawTestFlag = true;
		ProcPreagentPreWashTube(dev,AGENT_CHROMOGENIC,1,true);
		ProcEmptyTube(dev,REACT_WASTE_VALVE);  //清空定量管
		ProcEmptyTube(dev,REACT_WASTE_VALVE);  //清空定量管
		ProcEmptyTube(dev,REACT_WASTE_VALVE);  //清空定量管
		if(false == ProcAddAgentToPool(dev,AGENT_CHROMOGENIC,1,true))
			return false; //加显色剂
		DrawTestFlag = false;
		ProcAirPool(dev);																											  //向反应池吹气
		ProcPreagentPreWashTube(dev,AGENT_DISTILLED_WATER,1,false);								//洗定量管
		ProcPreagentPreWashTube(dev,AGENT_DISTILLED_WATER,1,false);								//洗定量管
		
		ProcUpdateStepStatus(STEP_ADD_OXIDANT);							//更新进度条名称
		ProcDelay(1);
		DrawTestFlag = true;
		ProcPreagentPreWashTube(dev,AGENT_OXIDANT,1,true);
		if(false == ProcAddAgentToPool(dev,AGENT_OXIDANT,1,true))
			return false; 	//加氧化剂
		DrawTestFlag = false;
		ProcAirPool(dev);
		ProcPreagentPreWashTube(dev,AGENT_DISTILLED_WATER,1,false);								//洗定量管
		ProcPreagentPreWashTube(dev,AGENT_DISTILLED_WATER,1,false);								//洗定量管
		ProcUpdateStepStatus(STEP_REACTING);												//更新进度条名称
		ProcDelay(1);
		ProcTempCtrl(true,TEMP_CTRL_CH_POOL,hotTemp,hotTime);			//开启加热控制

		ProcAirPool(dev);			
																									//向比色池吹气	
		ProcDelay(30);																							
		ProcUpdateStepStatus(STEP_READ_ABS);    				//更新进度条名称
		ProcReadReactADs(10);		
		ProcDelay(5);
		ProcCalculate();    					//通知上位机计算结果
		ProcDelay(5);
		ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);			//更新进度条名称
		ProcDelay(2);
		ProcEmptyPool(dev,REACT_WASTE_VALVE);					//排空反应池		
		ProcDelay(1);
		ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);    //洗定量管
		ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,12);				//清洗反应池
		ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9);				//清洗反应池
		ProcStopUpdateStepStatus();

//		ProcDelay(10);
		return true;
}




