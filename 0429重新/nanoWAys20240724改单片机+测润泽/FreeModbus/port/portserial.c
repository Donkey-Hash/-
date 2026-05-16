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

USART_TypeDef* pModbusUart = USART1;// = &huart3;
USART_TypeDef* pModbusUart3 = USART3;
USART_TypeDef* pModbusUart6 = USART6;

void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
	if(xRxEnable)
	{
//		S_RS485_RX;
		LL_USART_EnableIT_RXNE(pModbusUart);
	}
	else
	{
//		S_RS485_TX;
		LL_USART_DisableIT_RXNE(pModbusUart);
	}

	if(xTxEnable)
	{
//		S_RS485_TX;
		/* 确保本次发送能正确等待到“真实的发送完成(TC)” */
		LL_USART_ClearFlag_TC(pModbusUart);
		LL_USART_EnableIT_TXE(pModbusUart);
	}
	else
	{
//		S_RS485_RX;
		LL_USART_DisableIT_TXE(pModbusUart);
	}
}

void vMBPortSerialEnable3( BOOL xRxEnable, BOOL xTxEnable )
{
	/* USART3 在本工程里可能同时被旧的 ModbusMaster(DMA+IDLE) 逻辑使用。
	 * 当启用 MB3(USART3 从站) 时，为避免资源/中断冲突，这里主动关闭 USART3 的 DMA 与 IDLE/TC 中断。 */
	LL_USART_DisableDMAReq_RX(pModbusUart3);
	LL_USART_DisableDMAReq_TX(pModbusUart3);
	LL_USART_DisableIT_IDLE(pModbusUart3);
	LL_USART_DisableIT_TC(pModbusUart3);

	/* 485 收发方向控制：
	 * - 发送时：切到 TX
	 * - 接收/空闲时：切到 RX（默认） */
	if(xTxEnable)
	{
		H_RS485_TX;
		LL_USART_ClearFlag_TC(pModbusUart3);
	}
	else
	{
		H_RS485_RX;
	}

	if(xRxEnable)
	{
		LL_USART_EnableIT_RXNE(pModbusUart3);
	}
	else
	{
		LL_USART_DisableIT_RXNE(pModbusUart3);
	}

	if(xTxEnable)
	{
		LL_USART_EnableIT_TXE(pModbusUart3);
	}
	else
	{
		LL_USART_DisableIT_TXE(pModbusUart3);
	}
}

void vMBPortSerialEnable6( BOOL xRxEnable, BOOL xTxEnable )
{
	/* USART6 在本工程里默认用于外设通信(DMA+IDLE)。
	 * 当启用 MB6(USART6 从站) 时，必须关闭 DMA 与 IDLE/TC 中断，避免与 RTU 定时器收帧机制冲突。 */
	LL_USART_DisableDMAReq_RX(pModbusUart6);
	LL_USART_DisableDMAReq_TX(pModbusUart6);
	LL_USART_DisableIT_IDLE(pModbusUart6);
	LL_USART_DisableIT_TC(pModbusUart6);

	if(xRxEnable)
	{
		LL_USART_EnableIT_RXNE(pModbusUart6);
	}
	else
	{
		LL_USART_DisableIT_RXNE(pModbusUart6);
	}

	if(xTxEnable)
	{
		LL_USART_ClearFlag_TC(pModbusUart6);
		LL_USART_EnableIT_TXE(pModbusUart6);
	}
	else
	{
		LL_USART_DisableIT_TXE(pModbusUart6);
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
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**UART8 GPIO Configuration
  PE0   ------> UART8_RX
  PE1   ------> UART8_TX
  */
  GPIO_InitStruct.Pin = HMI_TX_Pin|HMI_RX_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		
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
  LL_USART_Init(pModbusUart, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(pModbusUart);
  LL_USART_Enable(pModbusUart);
	LL_USART_EnableIT_RXNE(pModbusUart);
	return true;
}

BOOL
xMBPortSerialInit3( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	LL_USART_InitTypeDef USART_InitStruct = {0};
	(void)ucPORT;
	(void)ucDataBits;

	USART_InitStruct.BaudRate = ulBaudRate;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	switch ( eParity )
    {
    case MB_PAR_NONE:
        USART_InitStruct.Parity = LL_USART_PARITY_NONE;
				break;
    case MB_PAR_ODD:
        USART_InitStruct.Parity = LL_USART_PARITY_ODD;
        break;
    case MB_PAR_EVEN:
        USART_InitStruct.Parity = LL_USART_PARITY_EVEN;
        break;
    }
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(pModbusUart3, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(pModbusUart3);
  LL_USART_Enable(pModbusUart3);
	/* MB3 使用 RTU 定时器判断帧结束，不依赖 IDLE/TC/DMA。 */
	LL_USART_DisableDMAReq_RX(pModbusUart3);
	LL_USART_DisableDMAReq_TX(pModbusUart3);
	LL_USART_DisableIT_IDLE(pModbusUart3);
	LL_USART_DisableIT_TC(pModbusUart3);
	H_RS485_RX;
	LL_USART_EnableIT_RXNE(pModbusUart3);
	return TRUE;
}

BOOL
xMBPortSerialInit6( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	LL_USART_InitTypeDef USART_InitStruct = {0};
	(void)ucPORT;
	(void)ucDataBits;

	USART_InitStruct.BaudRate = ulBaudRate;
	USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
	USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	switch ( eParity )
    {
    case MB_PAR_NONE:
        USART_InitStruct.Parity = LL_USART_PARITY_NONE;
				break;
    case MB_PAR_ODD:
        USART_InitStruct.Parity = LL_USART_PARITY_ODD;
        break;
    case MB_PAR_EVEN:
        USART_InitStruct.Parity = LL_USART_PARITY_EVEN;
        break;
    }
	USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
	LL_USART_Init(pModbusUart6, &USART_InitStruct);
	LL_USART_ConfigAsyncMode(pModbusUart6);
	LL_USART_Enable(pModbusUart6);
	/* MB6 使用 RTU 定时器判断帧结束，不依赖 IDLE/TC/DMA。 */
	LL_USART_DisableDMAReq_RX(pModbusUart6);
	LL_USART_DisableDMAReq_TX(pModbusUart6);
	LL_USART_DisableIT_IDLE(pModbusUart6);
	LL_USART_DisableIT_TC(pModbusUart6);
	LL_USART_EnableIT_RXNE(pModbusUart6);
	return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{

	pModbusUart->DR = (ucByte & (uint16_t)0x01FF);
	return TRUE;
}

BOOL
xMBPortSerialPutByte3( CHAR ucByte )
{
	pModbusUart3->DR = (ucByte & (uint16_t)0x01FF);
	return TRUE;
}

BOOL
xMBPortSerialPutByte6( CHAR ucByte )
{
	pModbusUart6->DR = (ucByte & (uint16_t)0x01FF);
	return TRUE;
}
 
BOOL
xMBPortSerialWaitTC( void )
{
//	  while(USART_GetFlagStatus(RS485, USART_FLAG_TC) == RESET){}
//	while(!__HAL_UART_GET_FLAG(pModbusUart, UART_FLAG_TC)){}
	/* 防卡死保护：TC 异常不置位时避免无限 while 导致系统卡死/看门狗复位。
	 * 注意：该函数可能在发送中断上下文被调用，因此这里不能使用 OS 延时。 */
	uint32_t guard = 2000000U;
	while (!LL_USART_IsActiveFlag_TC(pModbusUart) && (guard-- != 0U)) {}
	return (guard != 0U);
}

BOOL
xMBPortSerialWaitTC3( void )
{
	uint32_t guard = 2000000U;
	while (!LL_USART_IsActiveFlag_TC(pModbusUart3) && (guard-- != 0U)) {}
	return (guard != 0U);
}

BOOL
xMBPortSerialWaitTC6( void )
{
	uint32_t guard = 2000000U;
	while (!LL_USART_IsActiveFlag_TC(pModbusUart6) && (guard-- != 0U)) {}
	return (guard != 0U);
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
//	*pucByte = USART_ReceiveData(USART1);
	*pucByte = (uint8_t)(pModbusUart->DR & (uint8_t)0x00FF);
    return TRUE;
}

BOOL
xMBPortSerialGetByte3( CHAR * pucByte )
{
	*pucByte = (uint8_t)(pModbusUart3->DR & (uint8_t)0x00FF);
    return TRUE;
}

BOOL
xMBPortSerialGetByte6( CHAR * pucByte )
{
	*pucByte = (uint8_t)(pModbusUart6->DR & (uint8_t)0x00FF);
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

void prvvUART3TxReadyISR(void)
{
	pxMBFrameCBTransmitterEmpty3();
}

void prvvUART3RxISR(void)
{
	pxMBFrameCBByteReceived3();
}

void prvvUART6TxReadyISR(void)
{
	pxMBFrameCBTransmitterEmpty6();
}

void prvvUART6RxISR(void)
{
	pxMBFrameCBByteReceived6();
}
