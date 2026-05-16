
#include <stdbool.h>
#include "usart.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "replyhost.h"
#include "OPT2000.h"
extern osSemaphoreId UVdataHandle; 

uint8_t OPTRxBuff[64]={0};
uint8_t OPTTxBuff[64]={0};
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t OPTCheckSum(uint8_t* pdata,uint16_t len)
{
	uint8_t sum = 0;
	while(len--)
	{
		sum += *(pdata++);
	}
	sum &= 0xFF;
	return sum;
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void StartOPT(uint16_t integtime)
{
//	uint8_t buf[2] = {0xAA,0x04};
	OPTTxBuff[0] = 0xAA;
	OPTTxBuff[1] = 0x55;
	OPTTxBuff[2] = 0x00;
	OPTTxBuff[3] = 0x06;
	OPTTxBuff[4] = 0x1E;
	OPTTxBuff[5] = (integtime >> 8) & 0xFF;
	OPTTxBuff[6] = integtime & 0xFF;
//	*(uint16_t*)(OPTTxBuff + 5) = integtime;
	OPTTxBuff[7] = OPTCheckSum(OPTTxBuff + 2,5);
//	HAL_UART_Transmit(&huart6,OPTTxBuff,8,100);
	
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void StartOPTNoLight(uint16_t integtime)
{
//	uint8_t buf[2] = {0xAA,0x04};
	OPTTxBuff[0] = 0xAA;
	OPTTxBuff[1] = 0x55;
	OPTTxBuff[2] = 0x00;
	OPTTxBuff[3] = 0x06;
	OPTTxBuff[4] = 0x2F;
	OPTTxBuff[5] = (integtime >> 8) & 0xFF;
	OPTTxBuff[6] = integtime & 0xFF;
//	*(uint16_t*)(OPTTxBuff + 5) = integtime;
	OPTTxBuff[7] = OPTCheckSum(OPTTxBuff + 2,5);
//	HAL_UART_Transmit(&huart6,OPTTxBuff,8,100);
	
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
bool ReadOPT2000Data_t(uint16_t* pData220, uint16_t* pData275,uint16_t integtime)
{
	uint8_t* pbuf;
	uint8_t len;
//	HAL_StatusTypeDef ret;
	
	StartOPT(integtime);
//	HAL_UART_Receive_IT(&huart6,OPTRxBuff,10);
	if(osSemaphoreAcquire(UVdataHandle , 1000) == osOK)
	{
		pbuf = OPTRxBuff;
			len = 10;
			while(len)
			{
				while((*pbuf != 0x55) && len )
				{
					pbuf ++;
					len --;
				}
				if(len == 0)
					return false;
				if( pbuf[8] == OPTCheckSum(pbuf+1,7) )
				{
					*pData220 = ( (pbuf[4] << 8) + pbuf[5] );
					*pData275 = ( (pbuf[6] << 8) + pbuf[7] );
					return true;
				}
				else
				{
					pbuf ++;
					len --;
				}
			}
			return false;
//		}
//		else
//			return OPT_NO_DATA;
	}
	else
	{
//		HAL_UART_Abort_IT(&huart6);
		return false;
	}	
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
bool ReadOPTDarkData_t(uint16_t* pDark220, uint16_t* pDark275,uint16_t integtime)
{
	uint8_t* pbuf;
	uint8_t len;
//	HAL_StatusTypeDef ret;
	
	StartOPTNoLight(integtime);
//	HAL_UART_Receive_IT(&huart6,OPTRxBuff,10);
	if(osSemaphoreAcquire(UVdataHandle , 1000) == osOK)
	{
		pbuf = OPTRxBuff;
			len = 10;
			while(len)
			{
				while((*pbuf != 0x55) && len )
				{
					pbuf ++;
					len --;
				}
				if(len == 0)
					return false;
				if( pbuf[8] == OPTCheckSum(pbuf+1,7) )
				{
					*pDark220 = ( (pbuf[4] << 8) + pbuf[5] );
					*pDark275 = ( (pbuf[6] << 8) + pbuf[7] );
					return true;
				}
				else
				{
					pbuf ++;
					len --;
				}
			}
			return false;
//		}
//		else
//			return OPT_NO_DATA;
	}
	else
	{
//		HAL_UART_Abort_IT(&huart6);
		return false;
	}	
}

/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
bool ReadOPTDarkData(uint16_t* pDark220, uint16_t* pDark275)
{
	uint16_t data220,data275;
	uint32_t sum220 = 0;
	uint32_t sum275 = 0;
	for(int i = 0;i < 10;i++)
	{
		ReadOPTDarkData_t(&data220,&data275,100);
		sum220 += data220;
		sum275 += data275;
	}
	sum220 += 5;
	sum275 += 5;
	*pDark220 = sum220 / 10;
	*pDark275 = sum275 / 10;
	return true;
}
/*******************************************************************************
* Function Name  	: 
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
bool ReadOPT2000Data(uint16_t* pData220, uint16_t* pData275)
{
	uint16_t dark220,dark275;
	uint16_t data220,data275;
	uint32_t sum220 = 0;
	uint32_t sum275 = 0;
	
	ReadOPTDarkData(&dark220,&dark275);
	
	for(int i = 0;i < 10;i++)
	{
		ReadOPT2000Data_t(&data220,&data275,50);
		sum220 += data220;
		sum275 += data275;
	}
	sum220 += 5;
	sum275 += 5;
	*pData220 = sum220 / 10 - dark220;
	*pData275 = sum275 / 10 - dark275;
	return true;
}
