

#ifndef 	__BSP_UART1_H__
#define 	__BSP_UART1_H__
#include "usart.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "dma.h"
#include "tim.h"
#include "mtimers.h"





void StartUart1Rx(void);
void UART1_IRQHandler(void);
HAL_StatusTypeDef UART1_Receive_IT(UART_HandleTypeDef *huart);
void osUart1Tx(uint8_t *pData, uint16_t Size);
void SendIntByString(uint8_t channel ,uint16_t data);
#endif

