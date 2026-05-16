
#ifndef __MODBUS_MFC_H___
#define __MODBUS_MFC_H___
#include <stdint.h>
#include <stdbool.h>
//#include "communicate.h"

//#define MODBUS_MOTOR


typedef enum{
	HOST_MB_MOTOR = 1,
	MB_MFC_HeO2,
	MB_MFC_He,
	MB_MFC_HePurge,
	MB_MFC_Sample
}HOST_MB_DEV;

//typedef struct{
//	uint16_t nFlowPV;
//	uint16_t nFlowSV;	
//	bool bUpdataFlag;
//}MB_MFC_DEV_INFO;


void StopMbMotor(HOST_MB_DEV dev);
void StartMbMotor(HOST_MB_DEV dev,int16_t speed);
void ChangeSpeedMbMotor(HOST_MB_DEV dev,int16_t speed);

void ReadDevResponse(uint8_t* pData);
//void fSetMbMFCFlows(SET_FLOWS_t* pad);
void SetReadMFCCycleTimer(void);
#endif

