
#ifndef	__UARTS_TX_TASK_H__
#define	__UARTS_TX_TASK_H__
#include "stm32f1xx_hal.h"
#define USER_SIGNEL_COMM_1_TX 0x0001
#define USER_SIGNEL_COMM_2_TX 0x0002
#define USER_SIGNEL_COMM_3_TX 0x0004


typedef struct {
	uint8_t nIdleBuff;
	uint8_t nTxBuff[2][10];	
}COMM3_TX_BUFF;

void UartsTxTask(void const * argument);

#endif
