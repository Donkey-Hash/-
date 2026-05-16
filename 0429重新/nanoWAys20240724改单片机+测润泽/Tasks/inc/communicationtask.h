
#ifndef	__COMMUNICATION_TASK_H__
#define	__COMMUNICATION_TASK_H__
#include "stm32f1xx_hal.h"
#include "stdbool.h"
extern bool HostResponseFlag;


void CommunicationTask(void const * argument);
void RS232CmdProcess(uint8_t* rev);

#endif
