


#ifndef	__INSTRUMENT_INFO_H__
#define	__INSTRUMENT_INFO_H__
#include "valvedef.h"
#include "instrumentdef.h"
typedef DrawLiquidERROR (* TRANS_LIQUID_FUN)
				( DEVICE_CODE dev,FUN_VALVE_CODE src,FUN_VALVE_CODE des,uint16_t speed,uint16_t quanty,bool fine);
/***************试剂与电磁阀对应表*************************/
//typedef struct{
//	VALVE_NAME 	nValve;
//	AGENT_CODE 	nAgentCode;
////	uint8_t			nQuantity;
//}AGENT_SRC_VALVE_INFO;

/*************功能阀与继电器对应表*************************/
typedef struct{
	FUN_VALVE_CODE 	nValveFunCode;      //电磁阀组合要实现的功能阀代码
	uint32_t 	nRelays;               //每一位表示一个继电器	
	TRANS_LIQUID_FUN nFun;
	uint16_t  nLLTHR;
	uint8_t 	nMaxPullCircles;
	uint8_t		nMaxPushCircles;
	
}FUN_VALVE_RELARS_INFO;

typedef struct {
	float fHighStdConcent;   //高标试剂浓度
	float fLowStdConcent;			//低标试剂浓度
	float fHighAbs;						//高标吸光度
	float fLowAbs;						//低标吸光度
	float fK;									//标定曲线K值(斜率)
	float fB;									//标定曲线B值(截距)
}CALIBRATION_PARA;

/* 用户参数结构体 */
typedef struct{
	DEVICE_CODE nThisDevCode;
	uint16_t nLastWorkRange;
	uint16_t nReactionTemp;
	uint16_t nReactionTime;
	uint16_t nWaterPumpTime;	
	uint16_t nWorkRange1Set;
	uint16_t nWorkRange2Set;
	uint16_t nWorkRange3Set;
	uint16_t nWorkRange4Set;	
	uint16_t nTempCtrlMaxUk;
	CALIBRATION_PARA RangesCurvePara[4];	
	float								fTemp1Kp;								//73
	float								fTemp1Ki;								//75
	float								fTemp1Kd;								//77
}DEV_OPERAT_PARA;

//typedef struct{
////	uint8_t		nAllSteps;
////	AGENT_SRC_VALVE_INFO *pAgentValve;
//	FUN_VALVE_RELARS_INFO* pFunValveRelayInfo;
////	STEP_NAME_CODE *pStepNames;	
//}DEVICE_INFO;
//typedef struct{
//	uint8_t		nNumValves;
//	FUN_VALVE_CODE *pFunValve;	
//}DEV_VALVE_INFO;

extern const uint16_t PumpAgentSpeed[70];

TRANS_LIQUID_FUN GetLiquidFun(DEVICE_CODE dev,FUN_VALVE_CODE funCode);
bool OpenFunValve(DEVICE_CODE dev,FUN_VALVE_CODE funCode);
bool CloseFunValve(DEVICE_CODE dev,FUN_VALVE_CODE funCode);

void InitValveRelayInfo(void);
void GetFunValveList(DEVICE_CODE dev);
void GetDevOperatPara(void);
void SaveDevOperatPara(void);
void SwitchRange(void);
void SwitchDev(void);
#endif


