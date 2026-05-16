

#ifndef 	__BSP_UART1_H__
#define 	__BSP_UART1_H__
#include "usart.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "dma.h"
#include "tim.h"
#include "mtimers.h"





void StartUart2Rx(void);
void UART2_IRQHandler(void);
HAL_StatusTypeDef UART2_Receive_IT(UART_HandleTypeDef *huart);
void osUart2Tx(uint8_t *pData, uint16_t Size);
void SendString(char* pdata);
void SendIntByString(uint8_t channel ,uint16_t data);
void SendLTCADsString(uint16_t refAD ,uint16_t absAD);
#endif

