/*
  * FreeModbus Libary: LPC214X Port
  * Copyright (C) 2007 Tiago Prado Lone <tiago@maxwellbohr.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2007/04/24 23:15:18 wolti Exp $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "rs485.h"
#include "usart.h"
/* ----------------------- Start implementation -----------------------------*/

USART_TypeDef* pModbusUart = UART8;// = &huart3;

void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
	if(xRxEnable)
	{
		S_RS485_RX;
		LL_USART_EnableIT_RXNE(pModbusUart);
	}
	else
	{
		S_RS485_TX;
		LL_USART_DisableIT_RXNE(pModbusUart);
	}

	if(xTxEnable)
	{
		S_RS485_TX;
		LL_USART_EnableIT_TXE(pModbusUart);
	}
	else
	{
		S_RS485_RX;
		LL_USART_DisableIT_TXE(pModbusUart);
	}
}

void
vMBPortClose( void )
{
	LL_USART_DisableIT_TXE(pModbusUart);
	LL_USART_DisableIT_RXNE(pModbusUart);
	LL_USART_Disable(pModbusUart);
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{	
	LL_USART_InitTypeDef USART_InitStruct = {0};
	  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART8);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
  /**UART8 GPIO Configuration
  PE0   ------> UART8_RX
  PE1   ------> UART8_TX
  */
  GPIO_InitStruct.Pin = UART8_RX_Pin|UART8_TX_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);
		
		
	USART_InitStruct.BaudRate = ulBaudRate;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	switch ( eParity )
    {
    case MB_PAR_NONE:
        USART_InitStruct.Parity = LL_USART_PARITY_NONE ;
				break;
    case MB_PAR_ODD:
        USART_InitStruct.Parity = LL_USART_PARITY_ODD ;
        break;
    case MB_PAR_EVEN:
        USART_InitStruct.Parity = LL_USART_PARITY_EVEN ;
        break;
    }	
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(UART8, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(UART8);
  LL_USART_Enable(UART8);
	LL_USART_EnableIT_RXNE(pModbusUart);
	return true;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{

	pModbusUart->DR = (ucByte & (uint16_t)0x01FF);
	return TRUE;
}

BOOL
xMBPortSerialWaitTC( void )
{
//	  while(USART_GetFlagStatus(RS485, USART_FLAG_TC) == RESET){}
//	while(!__HAL_UART_GET_FLAG(pModbusUart, UART_FLAG_TC)){}
		while(!LL_USART_IsActiveFlag_TC(pModbusUart)){}
			return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
//	*pucByte = USART_ReceiveData(USART1);
	*pucByte = (uint8_t)(pModbusUart->DR & (uint8_t)0x00FF);
    return TRUE;
}

/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
	pxMBFrameCBTransmitterEmpty();
}

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
	pxMBFrameCBByteReceived();
}
