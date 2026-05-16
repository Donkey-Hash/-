
#include "port.h"
#include "mbcrc.h"
#include "ModbusMSMD.h"
#include "ModbusMaster.h"
#include "cmsis_os.h"
//#include "softtimer.h"

extern bool SetMFCFlowFlag;
//extern SET_FLOWS_t tSetFlows;
/******************************************************/
bool ReadMFCFlowFlag = false;

uint8_t MbFlowSetStep = 0;
bool ModbusBusy = false;

//MB_MFC_DEV_INFO MbMFCDevInfo[8];


//SET_FLOWS_t tMFCCurrentSet = {0};
bool MFCUpdateFlag[5];
uint8_t MFCRevError;
//MODBUS_MFC_UPDATE_FLAG MFCUpdateFlag;
/******************************************************/

void StartMbMotor(HOST_MB_DEV dev,int16_t speed)
{
	
	CMDWriteDevReg(dev,1,speed);
	osDelay(100);
//	CMDWriteDevReg(dev,0,1);
}

void StopMbMotor(HOST_MB_DEV dev)
{
	CMDWriteDevReg(dev,0,2);
}

void ChangeSpeedMbMotor(HOST_MB_DEV dev,int16_t speed)
{
	CMDWriteDevReg(dev,1,speed);
}

