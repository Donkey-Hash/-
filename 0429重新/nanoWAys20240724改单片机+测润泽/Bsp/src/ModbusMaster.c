/*******************************************************************************
 * 文件名:
 * 描  述:
 * 固件库版本:		version V0.0.01
 * 日  期:    		2016-03-10
 * 作  者:	   		myy qq:109226701
 *******************************************************************************/
#include <stdbool.h>
#include <string.h>
#include "port.h"
#include "mbcrc.h"
#include "cmsis_os.h"
#include "usart.h"
#include "leddef.h"

#include "ModbusMFC.h"
#include "ModbusMaster.h"

// UART_HandleTypeDef* pMbHuart;
extern osSemaphoreId_t HostMbdataHandle;
USART_TypeDef *pMbHostUart = USART3;

// uint16_t ResponeTimeout =   200;
volatile uint8_t MbRxBuff[128];
volatile uint8_t MbTxBuff[64];
uint16_t DMAUsart2RxSize;
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void USART_DMA_CONFIG(void)
{
#if APP_USART3_MODBUS_SLAVE_MB3
	/* USART3 已被配置为 MB3(Modbus-RTU 从站) 口时，禁止再启用 DMA+IDLE 主站接收模式，
	 * 否则会与 FreeModbus 的 USART3 中断入口/方向控制产生冲突。 */
	LL_USART_DisableDMAReq_RX(pMbHostUart);
	LL_USART_DisableDMAReq_TX(pMbHostUart);
	LL_USART_DisableIT_IDLE(pMbHostUart);
	LL_USART_DisableIT_TC(pMbHostUart);
	H_RS485_RX
	return;
#else
	LL_USART_EnableIT_IDLE(pMbHostUart);
	LL_USART_EnableDMAReq_RX(pMbHostUart);

	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_1, (uint32_t)(&pMbHostUart->DR)); // LL_USART_DMA_GetRegAddr(USART1->DR));
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_1, (uint32_t)MbRxBuff);
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_1, MB_SER_PDU_SIZE_MAX);
	//	LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_1);
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_1); // 使能传输完成中断
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_1); // 使能传输异常中断
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);

	osSemaphoreAcquire(HostMbdataHandle, 100);
	// LL_USART_DMA_GetRegAddr(USART1->DR));
	//	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_3, (uint32_t)MbRxBuff);
	//	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_3, MB_SER_PDU_SIZE_MAX);
#endif
}
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void USART_RxIdleCallback(void)
{
#if APP_USART3_MODBUS_SLAVE_MB3
	/* MB3 模式下不使用 USART3 的 IDLE 中断作为接收完成标记。 */
	return;
#else
	uint8_t cnt;

	//		LED2_ON
	LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_1); //

	DMAUsart2RxSize = MB_SER_PDU_SIZE_MAX - LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_1);
	osSemaphoreRelease(HostMbdataHandle);
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_1, MB_SER_PDU_SIZE_MAX);
	LL_USART_ClearFlag_IDLE(pMbHostUart);
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);
	//		LED2_OFF
#endif
}

/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void HostModbusSend(uint8_t *pData, uint16_t length);

/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/

uint16_t CMDReadDevReg(uint8_t dev, uint16_t reg)
{
#if APP_USART3_MODBUS_SLAVE_MB3
	(void)dev;
	(void)reg;
	return 0;
#else
	uint16_t crc;
	int j = 0;
	MbTxBuff[j++] = dev;
	MbTxBuff[j++] = 0x03;
	MbTxBuff[j++] = (reg >> 8) & 0xFF;
	MbTxBuff[j++] = reg & 0xFF;
	MbTxBuff[j++] = 0x00;
	MbTxBuff[j++] = 0x01;
	crc = usMBCRC16(MbTxBuff, 6);
	MbTxBuff[j++] = crc & 0xFF;
	MbTxBuff[j++] = (crc >> 8) & 0xFF;
	memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(MbTxBuff, j);
	//	HAL_UART_Receive_IT(pMbHuart,(uint8_t*)MbRxBuff,7);
	if (osSemaphoreAcquire(HostMbdataHandle, 100) == osOK)
	{
		if (MbRxBuff[0] == dev)
		{
			crc = *(uint16_t *)(MbRxBuff + 5);
			if (crc == usMBCRC16(MbRxBuff, 5))
			{
				crc = ((uint16_t)MbRxBuff[3] << 8);
				crc += MbRxBuff[4];
			}
		}
		else
		{
			crc = 0;
		}
		return crc;
	}
	else
	{
		//		UART_EndReceive_IT(pMbHuart);
		return 0x00;
	}
#endif
}

/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/

bool MbMasterReadHoldingRegs(uint8_t dev, uint16_t reg, uint16_t len, uint16_t *pdata)
{
#if APP_USART3_MODBUS_SLAVE_MB3
	(void)dev;
	(void)reg;
	(void)len;
	(void)pdata;
	return false;
#else
	uint16_t crc;
	int j = 0;
	MbTxBuff[j++] = dev;
	MbTxBuff[j++] = 0x03;
	MbTxBuff[j++] = (reg >> 8) & 0xFF;
	MbTxBuff[j++] = reg & 0xFF;
	MbTxBuff[j++] = (len >> 8) & 0xFF;
	MbTxBuff[j++] = len & 0xFF;
	crc = usMBCRC16(MbTxBuff, 6);
	MbTxBuff[j++] = crc & 0xFF;
	MbTxBuff[j++] = (crc >> 8) & 0xFF;
	memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(MbTxBuff, j);
	if (osSemaphoreAcquire(HostMbdataHandle, 500) == osOK)
	{
		if (MbRxBuff[0] == dev)
		{
			crc = *(uint16_t *)(MbRxBuff + 3 + 2 * len);
			if (crc == usMBCRC16(MbRxBuff, 3 + 2 * len))
			{
				for (int i = 0; i < len; i++)
				{
					pdata[i] = ((uint16_t)MbRxBuff[3 + 2 * i] << 8);
					pdata[i] += MbRxBuff[4 + 2 * i];
				}
				return true;
			}
			else
				return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
#endif
}
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/

int16_t MbMasterReadInputRegs(uint8_t dev, uint16_t reg, uint16_t len, uint16_t *pdata)
{
#if APP_USART3_MODBUS_SLAVE_MB3
	(void)dev;
	(void)reg;
	(void)len;
	(void)pdata;
	return -3;
#else
	uint16_t crc;
	uint16_t rcrc;
	int j = 0;
	int i = 0;
	MbTxBuff[j++] = dev;
	MbTxBuff[j++] = 0x04;
	MbTxBuff[j++] = (reg >> 8) & 0xFF;
	MbTxBuff[j++] = reg & 0xFF;
	MbTxBuff[j++] = (len >> 8) & 0xFF;
	MbTxBuff[j++] = len & 0xFF;
	crc = usMBCRC16(MbTxBuff, 6);
	MbTxBuff[j++] = crc & 0xFF;
	MbTxBuff[j++] = (crc >> 8) & 0xFF;
	memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(MbTxBuff, j);
	if (osSemaphoreAcquire(HostMbdataHandle, 500) == osOK)
	{
		if (MbRxBuff[0] == dev)
		{
			crc = *(uint16_t *)(MbRxBuff + 3 + 2 * len);
			rcrc = usMBCRC16(MbRxBuff, 3 + 2 * len);
			if (crc == rcrc)
			{
				for (int i = 0; i < len; i++)
				{
					pdata[i] = ((uint16_t)MbRxBuff[3 + 2 * i] << 8);
					pdata[i] += MbRxBuff[4 + 2 * i];
				}
				return 0;
			}
			else
				return -1; // 校验错误
		}
		else
		{
			return -2; // 收到数据地址错误
		}
	}
	else
	{
		return -3; // 应答超时
	}
#endif
}
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/

bool MbMasterWriteHoldingReg(uint8_t dev, uint16_t reg, uint16_t data)
{
#if APP_USART3_MODBUS_SLAVE_MB3
	(void)dev;
	(void)reg;
	(void)data;
	return false;
#else
	uint16_t crc;
	int i = 0;
	MbTxBuff[i++] = dev;
	MbTxBuff[i++] = 0x06;
	MbTxBuff[i++] = (reg >> 8) & 0xFF;
	MbTxBuff[i++] = reg & 0xFF;
	MbTxBuff[i++] = (data >> 8) & 0xFF;
	MbTxBuff[i++] = data & 0xFF;
	crc = usMBCRC16(MbTxBuff, i);
	MbTxBuff[i++] = crc & 0xFF;
	MbTxBuff[i++] = (crc >> 8) & 0xFF;
	memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(MbTxBuff, i);
	//	HAL_UART_Receive_IT(pMbHuart,(uint8_t*)MbRxBuff,8);
	if (osSemaphoreAcquire(HostMbdataHandle, 500) == osOK)
	{
		if (MbRxBuff[0] == dev)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		//		UART_EndReceive_IT(pMbHuart);
		return false;
	}
#endif
}

/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
bool MbMasterWriteHoldingRegs(uint8_t dev, uint16_t reg, uint16_t len, uint16_t *pdata)
{
#if APP_USART3_MODBUS_SLAVE_MB3
	(void)dev;
	(void)reg;
	(void)len;
	(void)pdata;
	return false;
#else
	uint16_t i;
	uint16_t TxCount = 0;
	uint16_t crc = 0;
	MbTxBuff[TxCount++] = dev;		/* 从站地址 */
	MbTxBuff[TxCount++] = 0x10;		/* 功能码 */
	MbTxBuff[TxCount++] = reg >> 8; /* 寄存器地址 高字节 */
	MbTxBuff[TxCount++] = reg;		/* 寄存器地址 低字节 */
	MbTxBuff[TxCount++] = len >> 8; /* 寄存器(16bits)个数 高字节 */
	MbTxBuff[TxCount++] = len;		/* 低字节 */
	MbTxBuff[TxCount++] = len << 1; /* 数据个数 */

	for (i = 0; i < len; i++)
	{
		MbTxBuff[TxCount++] = (pdata[i] >> 8) & 0xFF;
		; /* 后面的数据长度 */
		MbTxBuff[TxCount++] = (pdata[i]) & 0xFF;
		; /* 后面的数据长度 */
	}
	crc = usMBCRC16((uint8_t *)&MbTxBuff, TxCount);
	MbTxBuff[TxCount++] = crc;		/* crc 低字节 */
	MbTxBuff[TxCount++] = crc >> 8; /* crc 高字节 */
	memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend((uint8_t *)&MbTxBuff, TxCount);
	if (osSemaphoreAcquire(HostMbdataHandle, 500) == osOK)
	{
		if (MbRxBuff[0] == dev)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		//		UART_EndReceive_IT(pMbHuart);
		return false;
	}
#endif
}

/*******************************************************************************
 * 函数名: SetRS232RXsfTimer
 * 功  能: 设置定时器
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void HostModbusSend(uint8_t *pData, uint16_t length)
{
#if APP_USART3_MODBUS_SLAVE_MB3
	(void)pData;
	(void)length;
	H_RS485_RX
	return;
#else
	H_RS485_TX
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_3, (uint32_t)pData);
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_3, (uint32_t)(&pMbHostUart->DR));
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_3, length);

	LL_USART_EnableDMAReq_TX(pMbHostUart);
	LL_DMA_ClearFlag_TC4(DMA1);
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_3); // 使能传输完成中断
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_3); // 使能传输异常中断
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_3);

	//	H_RS485_RX
#endif
}


