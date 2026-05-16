
#include "instrumentinfo.h"
#include "peristalticpump.h"
#include "syringepump.h"
#include "modbusdata.h"
#include "FlashUserData.h"
#include "replyhost.h"
extern MODBUS_INPUT_REG_t *pMbInputData;


/***************************各种液体对应的抽取速度*****************************/
const uint16_t PumpAgentSpeed[70]	= 
{
	0,0,0,0,0,0,0 
	,SPEED_SAMPLE                     //7
	,SPEED_DISTILLED_WATER		
	,SPEED_STANDARD_SOLUTION				//										//SS:Standard solution			标液
	,SPEED_MERCUCY_SULFATE					//										//MS：Mercury sulfate				硫酸汞
	,SPEED_POTASSIUM_DICHROMATE			//										//PD:potassium dichromate 	重铬酸钾
	,SPEED_SULPHURIC_ACID						//										//SA:sulphuric acid       	硫酸
	,SPEED_LYE											//									//碱液
	,SPEED_RELEASE_AGENT						//					//释放剂
	,SPEED_NEUTRALIZER							//						//中和液
	,SPEED_BUFFERS									//								//缓冲剂
	,SPEED_CHROMOGENIC							//						//显色剂
	,SPEED_ABSORBENT								//							//吸收液
	,SPEED_EXTRACT									//								//萃取液
	,SPEED_MARSKING_AGENT						//				//掩蔽剂
	,SPEED_CLEANERS									//							//清洗剂
	,SPEED_DIGESTION								//							//消解液
	,SPEED_OXIDANT									//								//氧化剂
	,SPEED_REGULATOR								//							//调节剂
	,SPEED_REDUCER									//								//25还原剂
	,SPEED_COATING_FILM 											// // 26,                    	//汞膜液
	,SPEED_REFERENCE 												// // 27,													//参比液
	,SPEED_LOOP_ONLINE_POOL									// // 28,				//反应池串入定量环////		600 // 26,												//汞膜液阀
	,SPEED_LOOP_VALVE_OUT									//29
	,0,0,0,0
	,SPEED_VERIFY_SS                                 //34标液核查。为了和以前程序兼容
	,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	,SPEED_RECOVER 													// // 50,														    //加标回收、质控水样//28
	,SPEED_LOW_SS 														// // 51,															//低浓度标液，用于低点核查
	,SPEED_VERIFY_BLIND_SAMPLE 							// // 52, 												//盲样核查
	,SPEED_VERIFY_DISTILLED_WATER 						// // 53,										//蒸馏水/零点液核查
	,SPEED_VERIFY_SS 												// // 54,																	//标液核查
	,SPEED_VERIFY_SPAN 											// // 55,
  ,SPEED_DEFAULT,SPEED_DEFAULT,SPEED_DEFAULT //56,57,58
	,SPEED_REACTION_POOL_SOLUTION							//59
	,SPEED_DEFAULT                            //60
	,SPEED_AGENT_1
	,SPEED_AGENT_2
	,SPEED_AGENT_3
	,SPEED_AGENT_4
	,SPEED_AGENT_5                         //65
	
};

/***************************测试设备功能阀继电器信息****************************/
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_None[MAX_FUN_VALVE_NUM] =  {	
	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube},										//水样阀
	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube},										//零点液
	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube},										//标液阀
	
	{F_VALVE_POTASSIUM_DICHROMATE,					RELAY_5,								TrsLiquidByTube},										//重铬酸钾
	{F_VALVE_MERCUCY_SULFATE,								RELAY_6,								TrsLiquidByTube},										//硫酸汞
	{F_VALVE_SULPHURIC_ACID,								RELAY_7,								TrsLiquidByTube},										//浓硫酸

	{F_VALVE_VERIFY_SS, 										RELAY_1,								TrsLiquidByTube},										//空气阀	
	{F_VALVE_FAN,														RELAY_10,								TrsLiquidByTube},										//风扇	
	{F_VALVE_POOL_ONLY,											RELAY_9 | RELAY_11,								TrsLiquidByTube},										//反应池高压阀	
	
	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube},										//清洗废液阀	
	{F_VALVE_REACT_WASTE,										RELAY_2 | RELAY_13,			TrsLiquidByTube},										//反应废液阀
	{F_VALVE_WATER_PUMP,										RELAY_15,								TrsLiquidByTube},										//水泵
	
	{F_VALVE_CAVE_FAN,											RELAY_14,								TrsLiquidByTube},										//水泵
	{F_VALVE_NONE},
};

/***************************DEV_COD_CR功能阀继电器信息****************************/
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_CODCr[MAX_FUN_VALVE_NUM] =  {
	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube,	380},										//水样阀
	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube,	380},										//零点液
	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube,	380},										//标液阀
	
	{F_VALVE_POTASSIUM_DICHROMATE,					RELAY_5,								TrsLiquidByTube,	200},										//重铬酸钾
	{F_VALVE_MERCUCY_SULFATE,								RELAY_6,								TrsLiquidByTube,	380},										//硫酸汞
	{F_VALVE_SULPHURIC_ACID,								RELAY_7,								TrsLiquidByTube,	100},										//浓硫酸

	{F_VALVE_VERIFY_SS, 										RELAY_1,								TrsLiquidByTube,	500},										//空气阀	
	{F_VALVE_FAN,														RELAY_10,								TrsLiquidByTube,	500},										//风扇	
	{F_VALVE_POOL_ONLY,											RELAY_9 | RELAY_11,			TrsLiquidByTube,	500},										//反应池高压阀	
	
	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube,	380},										//清洗废液阀	
	{F_VALVE_REACT_WASTE,										RELAY_2 | RELAY_13,			TrsLiquidByTube,	380},										//反应废液阀
	{F_VALVE_WATER_PUMP,										RELAY_15,								TrsLiquidByTube,	500},										//水泵
	{F_VALVE_CAVE_FAN,											RELAY_14,								TrsLiquidByTube,	500},										//水泵
	{F_VALVE_NONE},
};
/***************************DEV_COD_CL功能阀继电器信息****************************/
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_CODCL[MAX_FUN_VALVE_NUM] =  {
	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube},										//水样阀
	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube},										//零点液
	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube},										//标液阀
	
	{F_VALVE_POTASSIUM_DICHROMATE,					RELAY_5,								TrsLiquidByTube},										//重铬酸钾
	{F_VALVE_MERCUCY_SULFATE,								RELAY_6,								TrsLiquidByTube},										//硫酸汞
	{F_VALVE_SULPHURIC_ACID,								RELAY_7,								TrsLiquidByTube},										//浓硫酸
	
	{F_VALVE_VERIFY_SS, 										RELAY_1,								TrsLiquidByTube},										//空气阀
	{F_VALVE_FAN,														RELAY_10,								TrsLiquidByTube},										//风扇	
	{F_VALVE_POOL_ONLY,											RELAY_9 | RELAY_11,			TrsLiquidByTube},										//反应池高压阀
	
	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube},										//清洗废液阀
	{F_VALVE_REACT_WASTE,										RELAY_2 | RELAY_13,			TrsLiquidByTube},										//反应废液阀
	{F_VALVE_WATER_PUMP,										RELAY_15,								TrsLiquidByTube},										//水泵	
	{F_VALVE_CAVE_FAN,											RELAY_14,								TrsLiquidByTube},										//水泵
	{F_VALVE_NONE},
};
/***************************DEV_COD_Mn功能阀继电器信息****************************/
//const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_CODMn[MAX_FUN_VALVE_NUM] =  {
//	{F_VALVE_STIR_MOTOR, 													RELAY_1,								TrsLiquidByTube},										//搅拌电机
//	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube},										//清洗废液阀
//	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube},										//水样阀
//	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube},										//标液阀
//	{F_VALVE_DIGESTION,											RELAY_5,								TrsLiquidByTube},										//消解液
//	{F_VALVE_OXIDANT,												RELAY_7,								TrsLiquidByTitration},										//氧化剂
//	{F_VALVE_NEUTRALIZER,										RELAY_6,								TrsLiquidByTube},										//中和液
//	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube},										//零点液
//	{F_VALVE_POOL_ONLY,											RELAY_9,								TrsLiquidByTube},										//反应池高压阀
////	{TITRATION_PUMP,											RELAY_10,								TrsLiquidByTube},									//滴定泵
//	{F_VALVE_WATER_PUMP,										RELAY_11,								TrsLiquidByTube},									//水泵
//	{F_VALVE_REACT_WASTE,										RELAY_12,								TrsLiquidByTube},									//反应废液阀
//};
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_CODMn[MAX_FUN_VALVE_NUM] =  {
	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube},										//水样阀
	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube},										//零点液
	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube},										//标液阀
	
	{F_VALVE_DIGESTION,											RELAY_5,								TrsLiquidByTube},										//消解液
	{F_VALVE_OXIDANT,												SYRINGE_VALVE_A,				TrsLiquidBySyringe},										//氧化剂
	{F_VALVE_NEUTRALIZER,										RELAY_6,								TrsLiquidByTube},										//中和液	
	
	{F_VALVE_STIR_MOTOR, 													RELAY_1,								TrsLiquidByTube},										//搅拌电机	
	{F_VALVE_LOOP_ONLINE_POOL,							RELAY_6 | RELAY_7,								TrsLiquidByTube},			//阀10:反应池串入定量环	
	{F_VALVE_POOL_SYRINGE,									SYRINGE_VALVE_B,				TrsLiquidBySyringe},										//反应池高压阀
//	{TITRATION_PUMP,											RELAY_10,								TrsLiquidByTube},									//滴定泵	
	{F_VALVE_POOL_ONLY,											RELAY_9,								TrsLiquidByTube},										//反应池高压阀
	
	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube},										//清洗废液阀
	{F_VALVE_REACT_WASTE,										RELAY_2 | RELAY_13,								TrsLiquidByTube},									//反应废液阀	
	{F_VALVE_WATER_PUMP,										RELAY_15,								TrsLiquidByTube},										//水泵		
	{F_VALVE_CAVE_FAN,											RELAY_14,								TrsLiquidByTube},										//水泵
	{F_VALVE_NONE},
};

//const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_CODMnOF[MAX_FUN_VALVE_NUM] =  {
//	{F_VALVE_STIR_MOTOR, 													RELAY_1,								TrsLiquidByTube},										//搅拌电机
//	{F_VALVE_OF_SAMPLE_SRC,									RELAY_3,								TrsLiquidByTube},										//水样阀
//	{F_VALVE_OF_SAMPLE_POOL,								RELAY_2|RELAY_6,				TrsLiquidByTube},										//标液阀
//	{F_VALVE_OF_DISTILLED_WATER_SRC,				RELAY_3|RELAY_4,				TrsLiquidByTube},										//消解液
//	{F_VALVE_OXIDANT,												SYRINGE_VALVE_A,				TrsLiquidBySyringe},										//氧化剂
//	{F_VALVE_OF_DISTILLED_WATER_POOL,				RELAY_2|RELAY_6,				TrsLiquidByTube},										//中和液
//	{F_VALVE_OF_NEUTRALIZER_SRC,						RELAY_7,								TrsLiquidByTube},										//零点液
//	{F_VALVE_OF_NEUTRALIZER_POOL,						RELAY_8|RELAY_9,				TrsLiquidByTube},										//反应池高压阀
//	{F_VALVE_POOL_SYRINGE,									SYRINGE_VALVE_B,				TrsLiquidBySyringe},										//反应池高压阀
//	{F_VALVE_WATER_PUMP,										RELAY_15,								TrsLiquidByTube},									//水泵
//	{F_VALVE_REACT_WASTE,										RELAY_12,								TrsLiquidByTube},									//反应废液阀
//	{F_VALVE_OF_DIGESTION_SRC_POOL,					RELAY_5,								TrsLiquidByTube},			//
//	{F_VALVE_FAN,														RELAY_10,								TrsLiquidByTube},
//	{F_VALVE_NONE},
//};
/***************************DEV_TP功能阀继电器信息****************************/
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_TP[MAX_FUN_VALVE_NUM] =  {
	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube},										//水样阀
	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube},										//零点液
	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube},										//标液阀	
	{F_VALVE_OXIDANT,												RELAY_5,								TrsLiquidByTube},										//氧化剂
	{F_VALVE_REDUCER,												RELAY_6,								TrsLiquidByTube},										//还原剂
	{F_VALVE_CHROMOGENIC,										RELAY_7,								TrsLiquidByTube},										//显色剂
	
	{F_VALVE_POOL_ONLY,											RELAY_9 | RELAY_11,								TrsLiquidByTube},										//反应池高压阀
	{F_VALVE_VERIFY_SS, 													RELAY_1,								TrsLiquidByTube},										//空气阀	
	{F_VALVE_FAN,														RELAY_10,								TrsLiquidByTube},									//风扇	
	
	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube},										//清洗废液阀
	{F_VALVE_REACT_WASTE,										RELAY_2 | RELAY_13,			TrsLiquidByTube},				//反应废液阀
	{F_VALVE_WATER_PUMP,										RELAY_15,								TrsLiquidByTube},									//水泵
	{F_VALVE_CAVE_FAN,											RELAY_14,								TrsLiquidByTube},										//水泵
	{F_VALVE_NONE},
};
/***************************DEV_HCHO功能阀继电器信息****************************/
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_HCHO[MAX_FUN_VALVE_NUM] =  {
	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube},										//水样阀
	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube},										//零点液
	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube},										//标液阀
	
	{F_VALVE_OXIDANT,												RELAY_5,								TrsLiquidByTube},										//氧化剂
	{F_VALVE_REDUCER,												RELAY_6,								TrsLiquidByTube},										//还原剂
	{F_VALVE_CHROMOGENIC,										RELAY_7,								TrsLiquidByTube},										//显色剂
	
	{F_VALVE_AIR, 													RELAY_1,								TrsLiquidByTube},										//空气阀
	{F_VALVE_POOL_ONLY,											RELAY_9,								TrsLiquidByTube},										//反应池高压阀
	{F_VALVE_FAN,														RELAY_10,								TrsLiquidByTube},									//风扇	
	
	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube},										//清洗废液阀
	{F_VALVE_REACT_WASTE,										RELAY_2 | RELAY_12,			TrsLiquidByTube},				//反应废液阀
	{F_VALVE_WATER_PUMP,										RELAY_15,								TrsLiquidByTube},									//水泵
	{F_VALVE_NONE},
};
/***************************DEV_NH_II功能阀继电器信息****************************/
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_NH_II[MAX_FUN_VALVE_NUM] =  {
	{F_VALVE_SAMPLE,										RELAY_2,											TrsLiquidByTube},		//水样阀
	{F_VALVE_DISTILLED_WATER,						RELAY_5,											TrsLiquidByTube},		//零点液
	{F_VALVE_STANDARD_SOLUTION,					RELAY_8,											TrsLiquidByTube},		//标液阀	
	
	{F_VALVE_AGENT_1,										RELAY_3,											TrsLiquidByTube},		//试剂1
	{F_VALVE_AGENT_2,										RELAY_6,											TrsLiquidByTube},		//试剂2
	{F_VALVE_AGENT_3,										RELAY_7,											TrsLiquidByTube},		//试剂3
	{F_VALVE_AGENT_4,										RELAY_16,											TrsLiquidByTube},		//试剂4
	{F_VALVE_AGENT_5,										RELAY_17,											TrsLiquidByTube},		//试剂5
	
	{F_VALVE_VERIFY_SS, 								RELAY_14,											TrsLiquidByTube},		//核查标液
	{F_VALVE_POOL_ONLY,									RELAY_4 |RELAY_9 | RELAY_10,	TrsLiquidByTube},		//比色池
	
	{F_VALVE_CLEAN_WASTE,								RELAY_1,											TrsLiquidByTube},		//清洗废液
	{F_VALVE_REACT_WASTE, 							RELAY_13,											TrsLiquidByTube},		//反应废液阀
	{F_VALVE_WATER_PUMP,								RELAY_15,											TrsLiquidByTube},		//采水泵
	{F_VALVE_CAVE_FAN,									RELAY_12,											TrsLiquidByTube},		//机箱风扇
	{F_VALVE_FAN,												RELAY_11,											TrsLiquidByTube},		//散热风扇	
	{F_VALVE_NONE},
	
};
/***************************DEV_ECHM功能阀继电器信息****************************/
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_ECHM[MAX_FUN_VALVE_NUM] =  {
	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube},										//水样阀
	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube},										//零点液
	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube},										//标液阀
	
	{F_VALVE_REGULATOR,											RELAY_5,								TrsLiquidByTube},										//消解液
	{F_VALVE_BUFFERS,												RELAY_6,								TrsLiquidByTube},										//氧化剂
	{F_VALVE_MARSKING_AGENT,								RELAY_7,								TrsLiquidByTube},										//中和液
	
	{F_VALVE_AIR, 													RELAY_1,								TrsLiquidByTube},										//空气阀
	{F_VALVE_POOL_ONLY,											RELAY_9,								TrsLiquidByTube},										//反应池高压阀	
	
	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube},										//清洗废液阀
	{F_VALVE_REACT_WASTE,										RELAY_2 | RELAY_13,								TrsLiquidByTube},									//反应废液阀
	{F_VALVE_WATER_PUMP,										RELAY_18,								TrsLiquidByTube},									//水泵
	{F_VALVE_NONE},
};
/***************************DEV_TN功能阀继电器信息****************************/
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_TN[MAX_FUN_VALVE_NUM] =  {
	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube},										//水样阀
	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube},										//零点液
	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube},										//标液阀	
	
	{F_VALVE_DIGESTION,											RELAY_5,								TrsLiquidByTube},										//消解液
	{F_VALVE_OXIDANT,												RELAY_6,								TrsLiquidByTube},										//氧化剂
	{F_VALVE_NEUTRALIZER,										RELAY_7,								TrsLiquidByTube},										//中和液
	
	{F_VALVE_VERIFY_SS, 										RELAY_1,								TrsLiquidByTube},										//空气阀
	{F_VALVE_POOL_ONLY,											RELAY_9 | RELAY_11,								TrsLiquidByTube},										//反应池高压阀
	{F_VALVE_FAN,														RELAY_10,								TrsLiquidByTube},									//风扇
	
	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube},										//清洗废液阀
	{F_VALVE_REACT_WASTE,										RELAY_2 | RELAY_13,								TrsLiquidByTube},										//反应废液阀
	{F_VALVE_WATER_PUMP,										RELAY_15,								TrsLiquidByTube},									//水泵	
	{F_VALVE_CAVE_FAN,											RELAY_18,								TrsLiquidByTube},										//水泵
	{F_VALVE_NONE},
};
/***************************DEV_TNr功能阀继电器信息****************************/
const FUN_VALVE_RELARS_INFO FunValveRelaysInfo_TNR[MAX_FUN_VALVE_NUM] =  {
	
	{F_VALVE_SAMPLE,												RELAY_3,								TrsLiquidByTube},										//水样阀
	{F_VALVE_DISTILLED_WATER,								RELAY_8,								TrsLiquidByTube},										//零点液
	{F_VALVE_STANDARD_SOLUTION,							RELAY_4,								TrsLiquidByTube},										//标液阀	
	
	{F_VALVE_OXIDANT,												RELAY_5,								TrsLiquidByTube},										//氧化剂
	{F_VALVE_CHROMOGENIC,										RELAY_6,								TrsLiquidByTube},										//显色剂
	{F_VALVE_SULPHURIC_ACID,								RELAY_7,								TrsLiquidByTube},										//硫酸
	
	{F_VALVE_VERIFY_SS, 													RELAY_1,								TrsLiquidByTube},										//空气阀
	{F_VALVE_POOL_ONLY,											RELAY_9 | RELAY_11,								TrsLiquidByTube},										//反应池高压阀
	{F_VALVE_FAN,														RELAY_10,								TrsLiquidByTube},									//风扇
	
	{F_VALVE_CLEAN_WASTE, 									RELAY_2,								TrsLiquidByTube},										//清洗废液阀
	{F_VALVE_REACT_WASTE,										RELAY_2 | RELAY_13,								TrsLiquidByTube},										//反应废液阀
	{F_VALVE_WATER_PUMP,										RELAY_15,								TrsLiquidByTube},									//水泵	
	{F_VALVE_CAVE_FAN,											RELAY_14,								TrsLiquidByTube},										//水泵
	{F_VALVE_NONE},
};

/***************************把各设备功能阀继电器信息添加到下面数组对应的位置，没有功能阀继电器信息的设备置零****************************/



FUN_VALVE_RELARS_INFO *pDevsValveRelayInfo[MAX_DEV_NUM] = {0};
//{
//	(FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_None,          								//测试设备  	//0
//	(FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_CODCr,          								//铬法COD  		//1
//	(FUN_VALVE_RELARS_INFO*)0,																								//紫外COD  		//2
//	(FUN_VALVE_RELARS_INFO*)0,																								//电化学COD  	//3
//	(FUN_VALVE_RELARS_INFO*)0,																								//I型氨氮  		//4
//	(FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_TP,														//总磷  			//5
//	(FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_TN,														//总氮  			//6
//	(FUN_VALVE_RELARS_INFO*)0,																								//总锰  			//7
//	(FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_NH_II,													//II型氨氮  	//8
//	(FUN_VALVE_RELARS_INFO*)0,																								//六价铬  		//9
//	(FUN_VALVE_RELARS_INFO*)0,																								//总铬  			//10
//	(FUN_VALVE_RELARS_INFO*)0,																								//镍  				//11
//	(FUN_VALVE_RELARS_INFO*)0,																								//地表水砷  	//12
//	(FUN_VALVE_RELARS_INFO*)0,																								//污染源砷  	//13
//	(FUN_VALVE_RELARS_INFO*)0,																								//总铁  			//14
//	(FUN_VALVE_RELARS_INFO*)0,																								//总铜  			//15
//	(FUN_VALVE_RELARS_INFO*)0,																								//镉  				//16
//	(FUN_VALVE_RELARS_INFO*)0,																								//总汞  			//17
//	(FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_CODMn,													//CODmm  			//18
//	(FUN_VALVE_RELARS_INFO*)0,																								//挥发酚  		//19
//	(FUN_VALVE_RELARS_INFO*)0,																								//铅  				//20
//	(FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_HCHO,													//甲醛  			//21
//	(FUN_VALVE_RELARS_INFO*)0,																								//锌  				//22
//	(FUN_VALVE_RELARS_INFO*)0,																								//电化学  		//23
//	(FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_CODMnOF,												//氰化物  		//24
//	
//};

 /*******************************************************************************
* Function Name  : GetRelays
* Description    : 通过功能阀代码查找继电器组合
* Input          : dev：设备代码，funCode：功能阀代码
* Output         : 无
* Return         : 继电器组合
*******************************************************************************/
static uint32_t GetRelays(DEVICE_CODE dev,FUN_VALVE_CODE funCode)
{
	uint8_t i;
	uint32_t relays;
	FUN_VALVE_RELARS_INFO *pa = pDevsValveRelayInfo[dev];
	for(i=0;i<MAX_FUN_VALVE_NUM;i++)
	if(pa[i].nValveFunCode == funCode)
	{
		relays = pa[i].nRelays;
		return relays;
	}
	return RELAY_NONE;
}
 /*******************************************************************************
* Function Name  : GetRelays
* Description    : 通过功能阀代码查找继电器组合
* Input          : dev：设备代码，funCode：功能阀代码
* Output         : 无
* Return         : 继电器组合
*******************************************************************************/
TRANS_LIQUID_FUN GetLiquidFun(DEVICE_CODE dev,FUN_VALVE_CODE funCode)
{
	uint8_t i;
	TRANS_LIQUID_FUN fun;
	FUN_VALVE_RELARS_INFO *pa = pDevsValveRelayInfo[dev];
	for(i=0;i<MAX_FUN_VALVE_NUM;i++)
	if(pa[i].nValveFunCode == funCode)
	{
		fun = pa[i].nFun;
		return fun;
	}
	return NULL;
}
 /*******************************************************************************
* Function Name  : OpenFunValve
* Description    : 打开功能阀
* Input          : dev：设备代码，funCode：功能阀代码
* Output         : 无
* Return         : 继电器组合
*******************************************************************************/
 bool OpenFunValve(DEVICE_CODE dev,FUN_VALVE_CODE funCode)
{
	uint32_t relays = GetRelays(dev,funCode);
	pMbHoldData->nFunVavleCode = funCode;
	if(funCode == ALL_FUN_VALVE_CODE)
	{
		OpenAllValves();
		return true;
	}
	
	if(relays == 0x00)
		return false;
	else
	{
	  OpenValves(relays);
		return true;
	}
}
/*******************************************************************************
* Function Name  : CloseFunValve
* Description    : 关闭功能阀
* Input          : dev：设备代码，funCode：功能阀代码
* Output         : 无
* Return         : 继电器组合
*******************************************************************************/
 bool CloseFunValve(DEVICE_CODE dev,FUN_VALVE_CODE funCode)
{
	uint32_t relays = GetRelays(dev,funCode);
	pMbHoldData->nFunVavleCode = F_VALVE_NONE;
	if(funCode == ALL_FUN_VALVE_CODE)
	{
		CloseAllValves();
		return true;
	}
	if(relays == 0x00)
		return false;
	else
	{
	  CloseValves(relays);
		return true;
	}
}
/*******************************************************************************
* Function Name  	: InitValveRelayInfo
* Description    	: 初始化功能阀配置
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void InitValveRelayInfo(void)
{	
	pDevsValveRelayInfo[DEV_NONE] = (FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_None;
	pDevsValveRelayInfo[DEV_COD_CR] = (FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_CODCr;
	pDevsValveRelayInfo[DEV_COD_CL] = (FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_CODCL;
	pDevsValveRelayInfo[DEV_NH_II] = (FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_NH_II;
	pDevsValveRelayInfo[DEV_TN] = (FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_TN;
	pDevsValveRelayInfo[DEV_TP] = (FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_TP;
	pDevsValveRelayInfo[DEV_TNR] = (FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_TNR;
	pDevsValveRelayInfo[DEV_COD_Mn] = (FUN_VALVE_RELARS_INFO*)FunValveRelaysInfo_CODMn;

}

/*******************************************************************************
* Function Name  	: GetFunValveList
* Description    	: 通过modbus 输入寄存器向屏幕传递功能阀代码。
                    屏幕同功能阀代码，读取设定好的功能阀名称，然后进行显示
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
// void GetFunValveList(DEVICE_CODE dev)
// {
// 	uint8_t i;
// 	FUN_VALVE_RELARS_INFO *pa = pDevsValveRelayInfo[dev];
// 	uint16_t *pMbFunValve = &pMbInputData->nFunValve1;
// 	for(i=0;i<MAX_FUN_VALVE_NUM;i++)
// 	{
// //		if(pa[i].nValveFunCode == F_VALVE_NONE)
// //			return;
// 		pMbFunValve[i] = pa[i].nValveFunCode;
// 	}
// }



/*******************************************************************************
* Function Name  : GetDevOperatPara
* Description    : 获取设备信息，通过modbus寄存器对外开放
* Input          : 无
* Output         : 无
* Return         : 
*******************************************************************************/
//void SaveDevOperatPara(void)
//{
//	if(pMbHoldData->nWorkRange > 3)
//		pMbHoldData->nWorkRange = 0;
//	uint16_t rang = pMbHoldData->nWorkRange ; 
//	ThisDevOperatPara.nLastWorkRange = rang;
//	ThisDevOperatPara.nReactionTime = pMbHoldData->nHotHoldTime;
//	ThisDevOperatPara.nReactionTemp = pMbHoldData->nHotTemper;
//	ThisDevOperatPara.nWorkRange1Set = pMbHoldData->nRange1Set;
//	ThisDevOperatPara.nWorkRange2Set = pMbHoldData->nRange2Set;
//	ThisDevOperatPara.nWorkRange3Set = pMbHoldData->nRange3Set;
//	ThisDevOperatPara.nWorkRange4Set = pMbHoldData->nRange4Set;
//	ThisDevOperatPara.nWaterPumpTime = pMbHoldData->nWaterPumpHoldTime;	
//	ThisDevOperatPara.RangesCurvePara[rang] = *(CALIBRATION_PARA*)&(pMbHoldData->fHighStdConcent) ;
//	ThisDevOperatPara.fTemp1Kp = pMbHoldData->fTemp1Kp;
//	ThisDevOperatPara.fTemp1Ki = pMbHoldData->fTemp1Ki;
//	ThisDevOperatPara.fTemp1Kd = pMbHoldData->fTemp1Kd;
//	ThisDevOperatPara.nTempCtrlMaxUk = pMbHoldData->nTempCtrlMaxUk;
//	
//	HAL_StatusTypeDef status;
//	status = SaveDevParaDataToFlash(ThisDevOperatPara.nThisDevCode);
//	if(status != HAL_OK)
//	{
//		pMbInputData->nFunErrorCode = status;
//		ReplyFaultInProcess(FAULT_SAVE_DEV_PARA);
//	}
//	
//	
//}

 /*******************************************************************************
* Function Name  : SwitchDev
* Description    : 切换设备
* Input          : 无
* Output         : 无
* Return         : 
*******************************************************************************/
//void SwitchDev(void)
//{
////	if(pMbHoldData->nDevCode != ThisDevOperatPara.nThisDevCode)
////	{
////		HAL_StatusTypeDef status;
////		status = SaveDevParaDataToFlash(ThisDevOperatPara.nThisDevCode);
////		if(status != HAL_OK)
////		{
////			pMbInputData->nFunErrorCode = status;
////			ReplyFaultInProcess(FAULT_SAVE_DEV_PARA);
////		}
////		else
////		{
////			GetDevParaDataFromFlash(pMbHoldData->nDevCode);
////		}
////	}
//}
 /*******************************************************************************
* Function Name  : SwitchRange
* Description    : 切换量程
* Input          : 无
* Output         : 无
* Return         : 
*******************************************************************************/
//void SwitchRange(void)
//{
//	if(pMbHoldData->nWorkRange != ThisDevOperatPara.nLastWorkRange )
//	{
//		if(pMbHoldData->nWorkRange > 3)
//		{
//			pMbHoldData->nWorkRange = ThisDevOperatPara.nLastWorkRange;   //如果量程输入错误，使用上一个量程
//			return;
//		}
//		ThisDevOperatPara.nLastWorkRange = pMbHoldData->nWorkRange;
//		*(CALIBRATION_PARA*)&(pMbHoldData->fHighStdConcent) = ThisDevOperatPara.RangesCurvePara[ThisDevOperatPara.nLastWorkRange];
//		HAL_StatusTypeDef status;
//		status = SaveDevParaDataToFlash(ThisDevOperatPara.nThisDevCode);
//		if(status != HAL_OK)
//		{
//			pMbInputData->nFunErrorCode = status;
//			ReplyFaultInProcess(FAULT_SAVE_DEV_PARA);
//		}
//	}
//}

