/*******************************************************************************
 * 文件名:
 * 描  述:
 * 固件库版本:		version V0.0.01
 * 日  期:    		2023-01-05
 * 作  者:	   		myy qq:109226701
 *******************************************************************************/
#include <string.h>
#include "main.h"
#include "port.h"
#include "mbcrc.h"
#include "cmsis_os.h"
#include "usart.h"
#include "leddef.h"
#include "FlashUserData.h"
#include "ysspectro.h"

#define MB_SER_PDU_SIZE_MIN 4	/*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX 256 /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC 2	/*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF 0	/*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF 1	/*!< Offset of Modbus-PDU in Ser-PDU. */

/*
 * NOTE:
 * USART6 同时被“MB6(Modbus-RTU 从站)”与“光谱仪外设通信(DMA+IDLE)”复用会冲突。
 * 为了保证 USART6 的 Modbus 通信稳定，这里在 APP_USART6_MODBUS_SLAVE_MB6 使能时
 * 直接屏蔽（注释掉/禁用）光谱仪通信相关实现，所有接口返回失败/空数据且不占用 USART6 DMA/中断资源。
 */
#if APP_USART6_MODBUS_SLAVE_MB6
USART_TypeDef *pExDevUart = USART6;

void ExDevDMAConfig(void) {}
void ExDevRxIdleCallback(void) {}

bool OpenSpectroLight(void) { return false; }
bool CloseSpectroLight(void) { return false; }
bool StartSpectroScan(void) { return false; }
int16_t ReadSpectroData(uint16_t *pdata)
{
	if (pdata != NULL)
	{
		pdata[0] = 0;
		pdata[1] = 0;
	}
	return -100;
}
bool SetAverageTimes(void) { return false; }
bool SetIntegralTime(void) { return false; }

#else
// UART_HandleTypeDef* pMbHuart;
extern osSemaphoreId_t UVdataHandle;
USART_TypeDef *pExDevUart = USART6;

// static uint16_t ResponeTimeout =   200;
uint8_t ExDevRxBuff[MB_SER_PDU_SIZE_MAX];
uint8_t ExDevTxBuff[32];
static uint16_t ExDevDMARxSize;
/*==============================================================================*/
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void ExDevDMAConfig(void)
{
#if APP_USART6_MODBUS_SLAVE_MB6
	/* USART6 复用为 MB6(Modbus-RTU 从站) 时，禁止启用外设 DMA+IDLE 接收链路，避免与 MB6 中断/定时器机制冲突。 */
	return;
#endif
	LL_USART_EnableIT_IDLE(pExDevUart);
	LL_USART_EnableDMAReq_RX(pExDevUart);

	LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)(&pExDevUart->DR)); // LL_USART_DMA_GetRegAddr(USART1->DR));
	LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)ExDevRxBuff);
	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_1, MB_SER_PDU_SIZE_MAX);

	LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_1); // 使能传输完成中断
	LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_1); // 使能传输异常中断
	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);

	osSemaphoreAcquire(UVdataHandle, 1);
}
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void ExDevRxIdleCallback(void)
{
	uint8_t cnt;

	//		LED2_ON
	LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_1); //

	ExDevDMARxSize = MB_SER_PDU_SIZE_MAX - LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_1);
	osSemaphoreRelease(UVdataHandle);
	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_1, MB_SER_PDU_SIZE_MAX);
	LL_USART_ClearFlag_IDLE(pExDevUart);
	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);
	//		LED2_OFF
}

/*******************************************************************************
 * 函数名: ExDevSend
 * 功  能: 设置定时器
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void ExDevSend(uint8_t *pData, uint16_t length)
{

	LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_6, (uint32_t)pData);
	LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_6, (uint32_t)(&pExDevUart->DR));
	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_6, length);

	LL_USART_EnableDMAReq_TX(pExDevUart);
	LL_DMA_ClearFlag_TC4(DMA2);
	LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_6); // 使能传输完成中断
	LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_6); // 使能传输异常中断
	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_6);
}

/*==============================================================================*/
/*==============================================================================*/

/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
int16_t ExDevReadHoldingRegs(uint8_t dev, uint16_t reg, uint16_t len, uint16_t *pdata)
{
	uint16_t crc;
	int j = 0;
	ExDevTxBuff[j++] = dev;
	ExDevTxBuff[j++] = 0x03;
	ExDevTxBuff[j++] = (reg >> 8) & 0xFF;
	ExDevTxBuff[j++] = reg & 0xFF;
	ExDevTxBuff[j++] = (len >> 8) & 0xFF;
	ExDevTxBuff[j++] = len & 0xFF;
	crc = usMBCRC16(ExDevTxBuff, 6);
	ExDevTxBuff[j++] = crc & 0xFF;
	ExDevTxBuff[j++] = (crc >> 8) & 0xFF;
	memset(ExDevRxBuff, 0, sizeof(ExDevRxBuff));
	osSemaphoreAcquire(UVdataHandle, 1);
	ExDevSend(ExDevTxBuff, j);
	if (osSemaphoreAcquire(UVdataHandle, 500) == osOK)
	{
		if (ExDevRxBuff[0] == dev)
		{
			crc = *(uint16_t *)(ExDevRxBuff + 3 + 2 * len);
			if (crc == usMBCRC16(ExDevRxBuff, 3 + 2 * len))
			{
				for (int i = 0; i < len; i++)
				{
					pdata[i] = ((uint16_t)ExDevRxBuff[3 + 2 * i] << 8);
					pdata[i] += ExDevRxBuff[4 + 2 * i];
				}
			}
			else
			{
				return -1;
			}
		}
		else
		{
			//
			return -2;
		}
	}
	else
	{
		return -3;
	}
}
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
bool ExDevCMDWriteDevReg(uint8_t dev, uint16_t reg, uint16_t data)
{
	uint16_t crc;
	int i = 0;
	ExDevTxBuff[i++] = dev;
	ExDevTxBuff[i++] = 0x06;
	ExDevTxBuff[i++] = (reg >> 8) & 0xFF;
	ExDevTxBuff[i++] = reg & 0xFF;
	ExDevTxBuff[i++] = (data >> 8) & 0xFF;
	ExDevTxBuff[i++] = data & 0xFF;
	crc = usMBCRC16(ExDevTxBuff, i);
	ExDevTxBuff[i++] = crc & 0xFF;
	ExDevTxBuff[i++] = (crc >> 8) & 0xFF;
	memset(ExDevRxBuff, 0, sizeof(ExDevRxBuff));
	osSemaphoreAcquire(UVdataHandle, 1); // 清除信号量
	ExDevSend(ExDevTxBuff, i);
	if (osSemaphoreAcquire(UVdataHandle, 500) == osOK)
	{
		if (ExDevRxBuff[0] == dev)
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
}

/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void ExDevCMDReadDevRegs(uint8_t dev, uint16_t reg, uint16_t len)
{
}

/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void ExDevCMDWriteDevRegs(uint8_t dev, uint16_t reg, uint16_t len, uint16_t *pData)
{
}
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/

/*==============================================================================*/
/*==============================================================================*/
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
bool OpenSpectroLight(void)
{
	return ExDevCMDWriteDevReg(1, 0x0B, 0x01);
}
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
bool CloseSpectroLight(void)
{
	return ExDevCMDWriteDevReg(1, 0x0B, 0x00);
}
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
bool StartSpectroScan(void)
{
	return ExDevCMDWriteDevReg(1, 0x00, 0x06);
}
/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
int16_t ReadSpectroData(uint16_t *pdata)
{

	return ExDevReadHoldingRegs(1, 0x20, 2, pdata);
}

/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
bool SetAverageTimes(void)
{
	return ExDevCMDWriteDevReg(1, 0x05, pMbHoldData->nSpectroAverageTimes);
}

/*******************************************************************************
 * 函数名:
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
bool SetIntegralTime(void)
{
	long temp = pMbHoldData->nSpectroIntegralTime * 1000;
	uint16_t hreg = (uint16_t)(temp >> 16 & 0xFFFF);
	uint16_t lreg = (uint16_t)(temp & 0xFFFF);
	ExDevCMDWriteDevReg(1, 0x03, hreg);
	ExDevCMDWriteDevReg(1, 0x04, lreg);
}

#endif /* APP_USART6_MODBUS_SLAVE_MB6 */
