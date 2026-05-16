

#ifndef 	__BSP_UART3_H__
#define 	__BSP_UART3_H__
#include "usart.h"
#include "cmsis_os.h"
#include "gpio.h"
//#include "dma.h"
#include "tim.h"
#include "mtimers.h"





void StartRS232Rx(void);
void UART3_IRQHandler(void);
HAL_StatusTypeDef UART3_Receive_IT(UART_HandleTypeDef *huart);
void osRS232Tx(uint8_t *pData, uint16_t Size);
void SendIntByString(uint8_t channel ,uint16_t data);
#endif

