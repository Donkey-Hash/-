
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
//#include "stm32f1xx_hal.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbrtu.h"
#include "mbframe.h"

#include "mbcrc.h"
#include "mbport.h"

//#include "cmsis_os.h"
//#include "modbusdata.h"
#include "mbconfig.h"

//extern osSemaphoreId MbPollHandle; 
//extern MODBUS_HOLDING_REG_t *pMbHoldData;

/* ----------------------- Defines ------------------------------------------*/
#define MB_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX     256     /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC     2       /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF      1       /*!< Offset of Modbus-PDU in Ser-PDU. */
/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
    STATE_RX_INIT,              /*!< Receiver is in initial state. */
    STATE_RX_IDLE,              /*!< Receiver is in idle state. */
    STATE_RX_RCV,               /*!< Frame is beeing received. */
    STATE_RX_ERROR              /*!< If the frame is invalid. */
} eMBRcvState;

typedef enum
{
    STATE_TX_IDLE,              /*!< Transmitter is in idle state. */
    STATE_TX_XMIT               /*!< Transmitter is in transfer state. */
} eMBSndState;

extern UCHAR    ucMBAddress;
extern peMBFrameSend peMBFrameSendCur;



uint8_t Xor(uint8_t* pdata,uint8_t nlen)
{
	uint8_t temp = 0x00;
	while(nlen--)
		temp ^= (*pdata++);
	return temp;
}


#if MB_FUNC_PSEUDO_MASTER_SEND > 0

eMBErrorCode
eMBFuncPseudoMasterSend( unsigned char* pucFrame, USHORT usLength )
{
    eMBErrorCode    eStatus = MB_ENOERR;
		eStatus = peMBFrameSendCur( ucMBAddress, pucFrame, usLength );
    return eStatus;
}
#endif
