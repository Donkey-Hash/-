
#ifndef	__UARTS_RX_TASK_H__
#define	__UARTS_RX_TASK_H__
#include "stm32f4xx_hal.h"
#include "stdbool.h"
extern bool HostResponseFlag;

#ifndef u8
#define u8 uint8_t
#endif
#ifndef u16
#define u16 uint16_t
#endif
#ifndef u32
#define u32 uint32_t
#endif

void UartsRxTask(void const * argument);

uint16_t START_NH3N(void);
uint16_t START_TP(void);
uint16_t START_COD(void);
uint16_t START_TN(void);
uint16_t START_CODMN(void);

uint16_t RZSTART_COD(void);

uint16_t HCSTART_NH3N(void);

void RunMainTask(void const *argument);

#endif
