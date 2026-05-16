
#ifndef __PROC_API_H__
#define __PROC_API_H__
#include <stdio.h>
#include "instrumentinfo.h"
#include "tempcontroltask.h"
//#include "ltc1867.h"
//#include "bspuart2.h"
#include "syringepump.h"
#include "OOptics.h"
#include "FlashUserData.h"

bool ProcFillTubeWithAgent(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine);
void ProcEmptyTube(DEVICE_CODE dev,FUN_VALVE_CODE des);
void ProcWashTubeWithAgent(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine);
void ProcPreagentPreWashTube(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine);
void ProcAirPool(DEVICE_CODE dev);
void ProcAirLoopPool(DEVICE_CODE dev);
void ProcPushBackAgent(DEVICE_CODE dev,AGENT_CODE agentCode);
void ProcEmptyPool(DEVICE_CODE dev,FUN_VALVE_CODE des);
void ProcDrawPool(DEVICE_CODE dev,FUN_VALVE_CODE des,uint16_t quanty);
bool ProcWashPoolWithAgent(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty);
bool ProcWashPoolWithoutReplyFault(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty);
bool ProcAddAgentToPool(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine);
bool ProcAddAgentLoopPool(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine);
bool ProcFillLoop(DEVICE_CODE dev);
void GetADWithBackground(uint16_t* pADref,uint16_t* pADtrs,uint16_t nCount);
void GetADabsReal(uint16_t* pADref,uint16_t* pADtrs,uint16_t nCount);
//void ReadADabs(uint16_t* pADref,uint16_t* pADtrs,int N);
bool ProcTempCtrl(bool Proc,TEMPER_CTRL_CHANNEL temp_ctrl_ch,uint8_t sv,int16_t holdTime);
bool ProcTempCtrlWithoutHoldTime(bool Proc,TEMPER_CTRL_CHANNEL temp_ctrl_ch,uint8_t sv,int16_t holdTime);
void ProcUpdateStepStatus(STEP_NAME_CODE stepname);
void ProcInitStepStatus(DEVICE_CODE dev, STEP_NAME_CODE initstepname,uint16_t updatecycle);
void ProcStopUpdateStepStatus(void);
void ProcReportStepTimer(void);
void ProcReadBlankADs(uint8_t readADTimes);
void ProcReadReactADs(uint8_t readADTimes);
void ProcCalculate(void);
void ProcOpenWaterPump(DEVICE_CODE dev);
void ProcCloseWaterPump(DEVICE_CODE dev);
void ProcDelay(uint16_t timeS);

bool ProcResetUV(void);
bool ProcReadBlankUVData(void);
bool ProcReadReactUVData(void);
bool ProcPreproc(DEVICE_CODE dev);//,FUN_VALVE_CODE wasteValve);

FAULT_CODE ProcInitLLAD(void);
PUMP_LIQUID_ERROR CheckLevelsAD(void);
uint16_t ProcGetReactAD(uint16_t nCount);
void ProcSendBlankADs(void);
void ProcSendRectADs(void);

void ProcTitraPump(DEVICE_CODE dev,FUN_VALVE_CODE funValveCode,uint16_t times);
bool ProcTitration(DEVICE_CODE dev,uint16_t initAD);//,uint32_t* pTitrationTime);
bool ProcTitrationBySyringe(DEVICE_CODE dev,uint16_t initAD);
void ProcReplyTitrationTime(void);
void ProcEmptyPoolWithPump(DEVICE_CODE dev,FUN_VALVE_CODE des,uint16_t timeSec);


void ProcStopHotting(TEMPER_CTRL_CHANNEL temp_ctrl_ch);
void ProcOptFunValve(DEVICE_CODE dev,FUN_VALVE_CODE des,uint16_t timeSec);
void ProcOptPumpPush(uint16_t speed,uint8_t maxcircle);
void ProcOptPumpDraw(uint16_t speed,uint8_t maxcircle);

bool ProcReadBlankOOpUVData(void);         //20190118
bool ProcReadReactOOpUVData(void);//20190118
void ProcPreHeatOOpUV(void);

void ProcForcedEmptyPool(DEVICE_CODE dev,FUN_VALVE_CODE des,uint8_t drawcircle,uint8_t pushcircle,uint8_t times);
void ProcReadV1ADs(uint8_t readADTimes);
void ProcReadV2ADs(uint8_t readADTimes);
void ProcReadV3ADs(uint8_t readADTimes);
void ProcCalculateYS(void);
void ProcReadYSADs(uint8_t readADTimes);
#endif

