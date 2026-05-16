
#ifndef __MODBUS_MFC_H___
#define __MODBUS_MFC_H___
#include <stdint.h>
#include <stdbool.h>
//#include "communicate.h"




typedef enum{
	MB_MFC_CH4 = 1,
	MB_MFC_HeO2,
	MB_MFC_He,
	MB_MFC_HePurge,
	MB_MFC_Sample
}MB_MFC_DEV;

typedef struct{
	uint16_t nFlowPV;
	uint16_t nFlowSV;	
	bool bUpdataFlag;
}MB_MFC_DEV_INFO;




void ReadDevResponse(uint8_t* pData);
//void fSetMbMFCFlows(SET_FLOWS_t* pad);
void SetReadMFCCycleTimer(void);
#endif

