
#include "modbusdata.h"
#include "cpumemmap.h"
#include "cpuflashopt.h"

extern USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];

MODBUS_DATA_t *pMbData = (MODBUS_DATA_t*)usRegHoldingBuf;

//void InitUserData(void)
//{
//	ReadDataFromFlash();
//	if(pMbData->nSaveFlag != 0x55)
//	{
//		pMbData->nBand = 9600;
//		pMbData->nLocalAddr = 10;
//		pMbData->nStartMode = START_APP;
//	}
//}

//void SaveDataToFlash(void)
//{
//	pMbData->nSaveFlag = 0x55;
//	STMFLASH_Write(FLASH_USER_DATA_START_ADDRESS,(uint16_t*)pMbData,sizeof(MODBUS_DATA_t)/2);
//}

//void ReadDataFromFlash(void)
//{
//	STMFLASH_Write(FLASH_USER_DATA_START_ADDRESS,(uint16_t*)pMbData,sizeof(MODBUS_DATA_t)/2);
//}

