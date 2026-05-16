
#ifndef __REPLY_HOST_H__
#define __REPLY_HOST_H__
#include "cmsis_os.h"
#include "instrumentinfo.h"
#define COMM_3_TX_MAX_TIMES		3					//每条数据尝试重发次数。

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
typedef enum{
	REPLY_DATA_ACK,
	REPLY_PROC_DATA,
	REPLY_PROC_DONE

}HOST_DATA_TYPE;
typedef enum{
	REPLY_HOST_START,
	REPLY_HOST_ACK_OK,
	REPLY_HOST_ACK_FAULT,
}REPLY_HOST_ACK_STATE;
//typedef struct{
//	HOST_DATA_TYPE	 nType;
//	REPLY_HOST_ACK_STATE nSendStat;
//	int8_t  nTryTimes;
//	int8_t	nReSendCount;
//	uint8_t nDataLenth;
//	uint8_t nData[9];
//}REPLY_HOST_DATA;




uint8_t XorFF(uint8_t* pdata,uint8_t nlen);





//void ReplyHostSyn(void);
//uint8_t ReplyHost4Bytes(REPLY_HOST_DATA* pReplyHostData,uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4);
//uint8_t ReplyHost7Bytes(REPLY_HOST_DATA* pReplyHostData,uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4,uint8_t data5,uint8_t data6,uint8_t data7);
void ProcDone(void);

uint8_t ReadFirmVersion(void);
//uint8_t ReplyFaultInFlowCtr(FAULT_CODE fault);
//uint8_t ReplySuccessInFlowCtr(void);
//uint8_t ReplyFaultInTempCtr(FAULT_CODE fault);
//uint8_t ReplyCtrDoneInTempCtr(void);
//uint8_t ReplyHotDataInTempCtr(uint16_t temp,uint16_t time);

uint8_t ReplyHotDataInProcess(uint16_t temp,uint16_t time);
uint8_t ReplyHostProcReactAbs(uint16_t ref,uint16_t abs);
uint8_t ReplyHostProcBlankAbs(uint16_t ref,uint16_t abs);  							//参比吸光度
uint8_t ReplyFaultInProcess(FAULT_CODE fault);

void Calculation(void);
void FittingCurve(void);
void CalculatLowAbs(void);
void CalculatHighAbs(void);
uint8_t ReplyAbsADsInADTest(uint16_t ADref,uint16_t ADabs,uint8_t temp);
uint8_t ReplyLLADsInADTest(uint16_t ADll1,uint16_t ADll2);




#endif
