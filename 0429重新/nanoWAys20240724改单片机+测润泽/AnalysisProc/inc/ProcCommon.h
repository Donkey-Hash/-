
#ifndef __PROC_COMMON_H__
#define __PROC_COMMON_H__
#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "cmsis_os.h"
#include "valvedef.h"
#include "instrumentinfo.h"

//#define  PCB_TEST



typedef bool (*FUN_PROC_HANDLE) (DEVICE_CODE dev,AGENT_CODE agentCode,uint8_t range,uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
extern FUN_PROC_HANDLE ProcHandles[MAX_DEV_NUM];


void ProcInit(DEVICE_CODE dev);
void InitProcHandles(void);


#endif
