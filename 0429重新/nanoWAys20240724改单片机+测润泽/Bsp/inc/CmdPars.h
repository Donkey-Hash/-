
#ifndef __CMD_PARS_H__
#define __CMD_PARS_H__
#include "valvedef.h"
#include "cmsis_os.h"
#include "instrumentinfo.h"

#define FIRM_VERSION     1

#define COMM_3_TX_MAX_TIMES		3

//#define SPEED_SAMPLE			400
#define QUANTITY_TEST			1


#define	USER_SIGNEL_COMM_CMD			0x0001
#define COMM_CMD_FLOW_CTRL			0x0001
#define COMM_CMD_PROC_HANDLE			0x0002


typedef struct{
	uint8_t nCMD;
	uint8_t nData1;
	uint8_t nData2;
	uint8_t nData3;
	uint8_t nData4;
	uint8_t nData5;
	uint8_t nData6;
	uint8_t nData7;
}HOST_DATA;

typedef struct{
	uint8_t nSynCode;
	uint8_t nEnd;
}REPLY_HOST_SYN;

typedef struct{
	uint8_t nSynCode;
	uint8_t nData1;
	uint8_t nData2;
	uint8_t nData3;
	uint8_t nData4;
	uint8_t nXor;

}REPLY_HOST_DATA_4_BYTES;

typedef struct{
	uint8_t nSynCode;
	uint8_t nData1;
	uint8_t nData2;
	uint8_t nData3;
	uint8_t nData4;
	uint8_t nData5;
	uint8_t nData6;
	uint8_t nData7;
	uint8_t nXor;

}REPLY_HOST_DATA_7_BYTES;


extern HOST_DATA HostData;
extern osMutexId osCMDdataMutex;



uint8_t XorFF(uint8_t* pdata,uint8_t nlen);





void ReplyHostSyn(void);
uint8_t ReplyHost4Bytes(uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4);
//void ReplyHost7BytesNoBlock(uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4,uint8_t data5,uint8_t data6);
uint8_t ReplyHost7Bytes(uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4,uint8_t data5,uint8_t data6);
//void ReplyHostProcNoBlock(uint8_t data1,uint16_t data2,uint16_t data3,uint8_t data4);
void ProcDone(void);

uint8_t ReadFirmVersion(void);
uint8_t ReplyFaultInFlowCtr(FAULT_CODE fault);
uint8_t ReplySuccessInFlowCtr(void);
uint8_t ReplyFaultInTempCtr(FAULT_CODE fault);
uint8_t ReplyCtrDoneInTempCtr(void);
uint8_t ReplyHotDataInTempCtr(uint16_t temp,uint16_t time);

uint8_t ReplyHotDataInProcess(uint16_t temp,uint16_t time);
uint8_t ReplyHostProcCalculatAbs(uint16_t ref,uint16_t abs);
uint8_t ReplyFaultInProcess(FAULT_CODE fault);

uint8_t Calculation(void);
uint8_t FittingCurve(void);
uint8_t CalculatLowAbs(void);
uint8_t CalculatHighAbs(void);
uint8_t ReplyAbsADsInADTest(uint16_t ADref,uint16_t ADabs,uint8_t temp);
uint8_t ReplyLLADsInADTest(uint16_t ADll1,uint16_t ADll2);
uint8_t SendStepInfo(uint8_t preStep,uint8_t allSteps,STEP_NAME_CODE stepName,CALIB_MODE test);



#endif
