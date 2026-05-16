
#include <stdbool.h>
#include "usart.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "replyhost.h"
#include "UVSpectrogragh.h"
extern osSemaphoreId UVSEMHandle; 

uint8_t UVRxBuff[64]={0};
uint8_t UVTxBuff[64]={0};


void SendUVOder(void)
{
	uint8_t buf[2] = {0xAA,0x04};
	HAL_UART_Transmit(&huart2,buf,2,100);
}


UV_StatusTypeDef ReadUVData(uint16_t* pData220, uint16_t* pData275)
{
	uint8_t* pbuf;
	uint8_t len;
	HAL_StatusTypeDef ret;
	SendUVOder();
	ret = HAL_UART_Receive(&huart2,UVRxBuff,2,2000);
	if(HAL_OK == ret)
	{
		HAL_UART_Receive_IT(&huart2,UVRxBuff,40);
		if(osSemaphoreWait(UVSEMHandle , 2*60*1000) == osOK)
		{
			pbuf = UVRxBuff;
			len = 40;
			while(len)
			{
				while((*pbuf != 0xAA) && len )
				{
					pbuf ++;
					len --;
				}
				if(len == 0)
					return UV_NO_AA;
				if( 0x00 == XorFF(pbuf+1,7) )
				{
					*pData220 = ( (pbuf[2] << 8) + pbuf[3] );
					*pData275 = ( (pbuf[5] << 8) + pbuf[6] );
					return UV_OK;
				}
				else
				{
					pbuf ++;
					len --;
				}
			}
			return UV_CHECK_ERROR;
		}
		else
			return UV_NO_DATA;
	}
	else
	return UV_NO_RESPONSE;
		
	
	
}

