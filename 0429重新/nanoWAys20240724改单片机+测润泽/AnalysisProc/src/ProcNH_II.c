
#include "ProcAPI.h"
extern bool DrawTestFlag;

bool ProcNH_II(DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime) //命令代码 试剂代码 范围 加热温度 加热时间 水泵时间
{	

	if(false == ProcPreproc(dev))
		return false;
	ProcEmptyPool(dev,F_VALVE_CLEAN_WASTE);//排空反应池
	ProcUpdateStepStatus(STEP_PUMP_WATER);//更新进度条名称：开水泵
	if(pumpTime != 0)
	{
		OpenFunValve(dev,F_VALVE_WATER_PUMP);//开水泵
		ProcDelay(pumpTime*60);//延时60*秒
	}
	ProcUpdateStepStatus(STEP_WASH_WITH_SAMPLE);        //更新进度条名称：润洗
//	ProcDelay(1);//延时1秒
//	if(false == ProcWashPoolWithAgent(dev,agentCode,6) )//样品润洗
//		return false;//未采到样品报警
//	ProcAddAgentLoopPool(dev,AGENT_DISTILLED_WATER,3,false);//加高位蒸馏水到反应池
//	if(false == ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,12) ) //零点液润洗
//		return false;	
	ProcWashTubeWithAgent(dev,agentCode,3,false);       //洗定量管
	ProcUpdateStepStatus((STEP_NAME_CODE)agentCode);						//更新进度条名称
	ProcDelay(1);
	switch(range)
		{
		case 0://0-2
			if(false == ProcAddAgentToPool(dev,agentCode,6,true))return false; //加2个高位水样
			else
				ProcPushBackAgent(dev,agentCode);//水样回推
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);       //洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);				//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,1,true))return false; //加1个低位蒸馏水
			ProcAirPool(dev);     //向池子吹气
			ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);		//排空定量管
		break;
		case 1://0-10  5.5倍
		
		
//			ProcWashTubeWithAgent(dev,agentCode,1,false);						//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,1,false);						//洗定量管
			if(false == ProcAddAgentToPool(dev,agentCode,1,true))return false; //加1个低位水样		
			else
				ProcPushBackAgent(dev,agentCode);//水样回推		
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);		//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);		//洗定量管
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,6,true))return false; //加2个高位水样蒸馏水
			ProcAirPool(dev);									//向池子吹气
			ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);								//排空定量管
		break;
		case 2://0-50 
//			ProcWashTubeWithAgent(dev,agentCode,1,false);							//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,1,false);							//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,1,false);							//洗定量管
			if(false == ProcAddAgentToPool(dev,agentCode,2,true))return false; //加水样	增加1毫升水样20200529
			else
				ProcPushBackAgent(dev,agentCode);//水样回推
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,1,false);			//洗定量管
			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);	   //洗定量管  		
			if(false == ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,6,true))return false; //加蒸馏水增加3ml纯水20200529
			ProcAirPool(dev);																							//向池子吹气
			ProcAirPool(dev);																							//向池子吹气
			ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);																						//排空定量管
			ProcFillLoop(dev);									 													//定量环定量。
			ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9);						//洗反应池
		  ProcAirLoopPool(dev);    																			//通过定量环向比色池吹气
			ProcAddAgentLoopPool(dev,AGENT_DISTILLED_WATER,9,true);				//通过定量环向反应池加零点液
		break;
		case 3://0-250
//			ProcWashTubeWithAgent(dev,agentCode,3,false);													//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,3,false);													//洗定量管
//			ProcWashTubeWithAgent(dev,agentCode,3,false);		//洗定量管							
			if(false == ProcAddAgentLoopPool(dev,agentCode,6,true))return false; 		//加水样	增加3毫升水样2020529
			else
				ProcPushBackAgent(dev,agentCode);//水样回推
			if(false == ProcAddAgentLoopPool(dev,AGENT_DISTILLED_WATER,3,true))return false; //加蒸馏水
			ProcAirLoopPool(dev);																							//向池子吹气
//			ProcAirPool(dev);																							//向池子吹气
            ProcDelay(10);
			ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);																						//排空定量管
			ProcFillLoop(dev);    																								//定量环定量。
//     		ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);							//洗定量管
			ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9); 									//清洗比色池
			ProcAirLoopPool(dev);    																							//通过定量环向比色池吹气
			if(false == ProcAddAgentLoopPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //通过定量环向反应池加零点液
			ProcAirLoopPool(dev);    																							//通过定量环向比色池吹气
//			ProcAirLoopPool(dev);    																							//通过定量环向比色池吹气
			ProcDelay(10);
			ProcFillLoop(dev);																										//定量环定量
//			ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);							//洗定量管
			ProcWashPoolWithAgent(dev,AGENT_DISTILLED_WATER,9);										//清洗比色池
			ProcAirLoopPool(dev);    																							//通过定量环向比色池吹气
			if(false == ProcAddAgentLoopPool(dev,AGENT_DISTILLED_WATER,9,true))return false; //通过定量环向反应池加零点液
		break;
		default:break;
		}
		if(pumpTime != 0)
			ProcCloseWaterPump(dev);																	  //关水泵
		ProcEmptyTube(dev,F_VALVE_CLEAN_WASTE);																								//排空定量管
//		ProcPushBackAgent(dev,agentCode);																				//回推水样
		
	
		ProcUpdateStepStatus(STEP_ADD_CHROMOGENIC);																				//更新进度条名称
		ProcDelay(1);
		DrawTestFlag = true;
		ProcPreagentPreWashTube(dev,AGENT_1 ,1,true);//试剂一润洗
		ProcEmptyTube(dev,F_VALVE_REACT_WASTE);  //清空定量管
		ProcEmptyTube(dev,F_VALVE_REACT_WASTE);  //清空定量管
//		ProcEmptyTube(dev,F_VALVE_REACT_WASTE);  //清空定量管
		if(false == ProcAddAgentToPool(dev,AGENT_1,1,true))
			return false; //加试剂一
		DrawTestFlag = false;
		ProcAirPool(dev);																											  //向反应池吹气
		ProcPreagentPreWashTube(dev,AGENT_DISTILLED_WATER,1,false);								//洗定量管
		ProcPreagentPreWashTube(dev,AGENT_DISTILLED_WATER,1,false);								//洗定量管
	
		ProcUpdateStepStatus(STEP_READ_REF);																							//更新进度条名称
		ProcReadV3ADs(10);//读AD值V3	
     	ProcDelay(10);
	
		ProcUpdateStepStatus(STEP_ADD_OXIDANT);							//更新进度条名称
		ProcDelay(1);
		DrawTestFlag = true;
		ProcPreagentPreWashTube(dev,AGENT_2,1,true);   //试剂二润洗
		if(false == ProcAddAgentToPool(dev,AGENT_2,1,true))
			return false; 	//加试剂二
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
		ProcReadV2ADs(10);	//读反应信号V2	
		ProcDelay(5);
//	    ProcCalculate();    					//通知上位机计算结果
		ProcDelay(5);
		ProcUpdateStepStatus(STEP_WASH_REACTION_POOL);			//更新进度条名称
		ProcDelay(2);
		ProcForcedEmptyPool(dev,F_VALVE_REACT_WASTE,5,6,6);
		ProcEmptyPool(dev,F_VALVE_REACT_WASTE);					//排空反应池		
		ProcDelay(1);
//		ProcWashTubeWithAgent(dev,AGENT_DISTILLED_WATER,3,false);    //洗定量管
		ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true);				//加零点液
		ProcEmptyPool(dev,F_VALVE_CLEAN_WASTE);//排空反应池
		ProcAddAgentToPool(dev,AGENT_DISTILLED_WATER,9,true);				//加零点液
		ProcReadV1ADs(10);     //读V1
		ProcCalculateYS();     //计算吸光度和测量结果，对于高标和低标流程是计算高标低标吸光度，测试流程不仅计算测量吸光度，还计算测量结果
//		ProcEmptyPool(dev,F_VALVE_REACT_WASTE);	//排空反应池
		ProcStopUpdateStepStatus();

//    pMbHoldData->fMeasureResult = 40;
		return true;
}




