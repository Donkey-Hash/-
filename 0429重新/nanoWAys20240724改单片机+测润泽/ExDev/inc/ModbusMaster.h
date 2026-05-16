

#ifndef	__MODBUS_MASTER_H__
#define __MODBUS_MASTER_H__
#include "rs485.h"
#include "mb.h"
/* ----------------------- Defines ------------------------------------------*/
#define MB_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX     256     /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC     2       /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF      1       /*!< Offset of Modbus-PDU in Ser-PDU. */


typedef enum{
	HOST_MB_YS_SYRINGE = 1,
//	MB_MFC_HeO2,
//	MB_MFC_He,
//	MB_MFC_HePurge,
//	MB_MFC_Sample
}HOST_MB_DEV_ADDR;


typedef struct{
	uint8_t nDevAddr;
	uint8_t nCmd;
	uint16_t	nRegAddr;
	uint16_t	nDataLen;
	uint16_t	nCRC;
	

}MB_MASTER_CMD_3;

void CMDReadDevRegs(uint8_t dev,uint16_t reg,uint16_t len);
uint16_t CMDReadDevReg(uint8_t dev,uint16_t reg);
bool CMDWriteDevReg(uint8_t dev,uint16_t reg,uint16_t pData);
void CMDWriteDevRegs(uint8_t dev,uint16_t reg,uint16_t len,uint16_t* pData);

eMBErrorCode ParseMbData(uint8_t* pData,uint16_t len);
void USART_RxIdleCallback(void);
void USART_DMA_CONFIG(void);

bool MbMasterReadHoldingRegs(uint8_t dev,uint16_t reg,uint16_t len,uint16_t* pdata);
int16_t MbMasterReadInputRegs(uint8_t dev,uint16_t reg,uint16_t len,uint16_t* pdata);
bool MbMasterWriteHoldingReg(uint8_t dev,uint16_t reg,uint16_t data);
bool MbMasterWriteHoldingRegs(uint8_t dev, uint16_t reg, uint16_t len,uint16_t *pdata);
void HostModbusSend(uint8_t *pData, uint16_t length);

#endif
