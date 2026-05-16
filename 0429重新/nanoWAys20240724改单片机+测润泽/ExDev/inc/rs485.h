
#ifndef __RS485_H__
#define __RS485_H__
#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "main.h"
#include "stm32f4xx_hal_uart.h"

#define COMM_RS485PORT		2
//Modbus从设备 RS485发送接收选择引脚 modbus串口屏
#define S_RS485_RX	 	{LL_GPIO_ResetOutputPin(MbSlave_DIR_GPIO_Port,MbSlave_DIR_Pin);}
#define S_RS485_TX	 	{LL_GPIO_SetOutputPin(MbSlave_DIR_GPIO_Port,MbSlave_DIR_Pin);}
//Modbus主设备 RS485发送接收选择引脚 接外部modbus设备
#define H_RS485_RX	 	{LL_GPIO_ResetOutputPin(MbMaster_DIR_GPIO_Port,MbMaster_DIR_Pin);}
#define H_RS485_TX	 	{LL_GPIO_SetOutputPin(MbMaster_DIR_GPIO_Port,MbMaster_DIR_Pin);}
//测试用RS485发送接收选择引脚 接PC机
#define T_RS485_RX	 	{HAL_GPIO_WritePin(UART5_RD_GPIO_Port,UART5_RD_Pin,GPIO_PIN_SET);}
#define T_RS485_TX	 	{HAL_GPIO_WritePin(UART5_RD_GPIO_Port,UART5_RD_Pin,GPIO_PIN_RESET);}

extern UART_HandleTypeDef huart6;

void RS485Puts(char* pdata);
void USART_Configuration(void);
void PutsFloat(float fdata);
void PutsInt(int ndata);

#endif
