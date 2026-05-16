
#include <stdbool.h>
#include "usart.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "replyhost.h"
#include "OOptics.h"
extern osSemaphoreId UVSEMHandle; 

char UVRxBuff[64]={0};
char UVTxBuff[64]={0};



/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
uint8_t CharToHex(char ch)
{
	uint8_t hex;
	
	if(ch >= '0' && ch <= '9')
		hex = ch - '0';
	else if(ch >= 'A' && ch <= 'F')
		hex = ch - 'A' + 0x0A;
	else if(ch >= 'a' && ch <= 'f')
		hex = ch - 'a' + 0x0A;
	else
		hex = 0x00;
	return hex;
}
/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
void StringToHex(char *pch,uint8_t* phex,int charLen)
{
	uint8_t hexh;
	uint8_t hexl;
	while(charLen > 0)
	{
		hexh = 0x0F & CharToHex(*pch++);
		hexl = 0x0F & CharToHex(*pch++);
		*phex++ = hexh << 4 | hexl;
		charLen -= 2;
	}
}

/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpHand(void)
{
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'H';
	UVTxBuff[j++] = 'A';
	UVTxBuff[j++] = 'N';
	UVTxBuff[j++] = 'D';
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,200);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,8);
	if(osSemaphoreWait(UVSEMHandle , 500) == osOK)
	{
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
		return false;
	}
}

/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpSetIntegralTime(uint16_t integralTime)
{
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'I';
	UVTxBuff[j++] = 'N';
	UVTxBuff[j++] = 'T';
	UVTxBuff[j++] = 'T';
	UVTxBuff[j++] = 'M';
	UVTxBuff[j++] = '=';
	j += sprintf(UVTxBuff + j,"%d",integralTime);
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';
	
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,200);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,8);
	if(osSemaphoreWait(UVSEMHandle , 500) == osOK)
	{
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
		return false;
	}
}

/*******************************************************************************
* Function Name  :
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpSetRefWavelength(uint16_t refwave)
{
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'R';
	UVTxBuff[j++] = 'W';
	UVTxBuff[j++] = 'L';
	UVTxBuff[j++] = '=';
	j += sprintf(UVTxBuff + j,"%d",refwave);
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';	
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,200);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,8);
	if(osSemaphoreWait(UVSEMHandle , 500) == osOK)
	{
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
		return false;
	}
}

/*******************************************************************************
* Function Name  :
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpSetAvrgTimes(uint16_t avrgTimes)
{
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'S';
	UVTxBuff[j++] = 'P';
	UVTxBuff[j++] = 'E';
	UVTxBuff[j++] = 'E';
	UVTxBuff[j++] = 'D';
	UVTxBuff[j++] = '=';
	j += sprintf(UVTxBuff + j,"%d",avrgTimes);
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';	
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,200);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,8);
	if(osSemaphoreWait(UVSEMHandle , 500) == osOK)
	{
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
		return false;
	}
}
/*******************************************************************************
* Function Name  :
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpSetLightType(void)
{
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'L';
	UVTxBuff[j++] = 'T';
	UVTxBuff[j++] = 'T';
	UVTxBuff[j++] = 'Y';
	UVTxBuff[j++] = 'P';
	UVTxBuff[j++] = 'E';
	UVTxBuff[j++] = '=';
	UVTxBuff[j++] = 'P';
	UVTxBuff[j++] = 'U';
	UVTxBuff[j++] = 'L';
	UVTxBuff[j++] = 'S';
	UVTxBuff[j++] = 'E';
//	j += sprintf(UVTxBuff + j,"%d",avrgTimes);
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';	
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,200);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,j);
	if(osSemaphoreWait(UVSEMHandle , 500) == osOK)
	{
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
		return false;
	}
}
/*******************************************************************************
* Function Name  :
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpSetTriggerMode(void)
{
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'X';
	UVTxBuff[j++] = 'T';
	UVTxBuff[j++] = '=';
	UVTxBuff[j++] = 'P';
	UVTxBuff[j++] = 'W';
	UVTxBuff[j++] = 'M';
//	j += sprintf(UVTxBuff + j,"%d",avrgTimes);
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';	
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,200);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,j);
	if(osSemaphoreWait(UVSEMHandle , 500) == osOK)
	{
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
		return false;
	}
}
/*******************************************************************************
* Function Name  :
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpSetLightFrq(uint16_t frq)
{
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'L';
	UVTxBuff[j++] = 'T';
	UVTxBuff[j++] = 'S';
	UVTxBuff[j++] = 'T';
	UVTxBuff[j++] = 'E';
	UVTxBuff[j++] = 'P';
	UVTxBuff[j++] = '=';
	j += sprintf(UVTxBuff + j,"%d",frq);
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';	
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,200);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,8);
	if(osSemaphoreWait(UVSEMHandle , 500) == osOK)
	{
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
		return false;
	}
}
///*******************************************************************************
//* Function Name  :
//* Description    :
//* Input          : 	
//* Output         :
//* Return         : 
//*******************************************************************************/
//bool OOpSetWaveZero(uint16_t wave)
//{
//	int j = 0;
//	UVTxBuff[j++] = ':';
//	UVTxBuff[j++] = '0';
//	UVTxBuff[j++] = '4';
//	UVTxBuff[j++] = ' ';
//	UVTxBuff[j++] = 'A';
//	UVTxBuff[j++] = 'B';
//	UVTxBuff[j++] = 'S';
//	UVTxBuff[j++] = 'Z';
//	UVTxBuff[j++] = '=';
//	j += sprintf(UVTxBuff + j,"%d",wave);
//	UVTxBuff[j++] = '\r';
//	UVTxBuff[j++] = '\n';	
//	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,500);
//	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,17);
//	if(osSemaphoreWait(UVSEMHandle , 10000) == osOK)
//	{
//		
//		return true;
//	}
//	else
//		return false;
//}

/*******************************************************************************
* Function Name  :
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpInit(uint16_t integralTime,uint16_t refwave,uint16_t avrgTimes)
{
	OOpHand();
	OOpSetIntegralTime(integralTime);
	OOpSetLightType();
	OOpSetTriggerMode();
//	OOpSetLightFrq(100);
//	OOpSetWaveZero(220);
//	OOpSetWaveZero(275);
//	OOpSetRefWavelength(refwave);
	OOpSetAvrgTimes(avrgTimes);
	
	return true;
}
/*******************************************************************************
* Function Name  :
* Description    :
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpGetZeroData(uint16_t wave,uint16_t* pZeroData)
{
	char* pbuf;
//	uint8_t*phex;
	uint8_t len;
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'A';
	UVTxBuff[j++] = 'B';
	UVTxBuff[j++] = 'S';
	UVTxBuff[j++] = 'Z';
	UVTxBuff[j++] = '=';
	j += sprintf(UVTxBuff + j,"%d",wave);
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';	
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,500);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,17);
	if(osSemaphoreWait(UVSEMHandle , 30000) == osOK)
	{
		pbuf = UVRxBuff;
		len = 12;
		while((*pbuf != '=') && len )
		{
			pbuf ++;
			len --;
		}
		if(len == 0)
			return false;
//		phex = (uint8_t*)pWaveData;
//		phex++;
		pbuf[0] = '0';
		StringToHex(pbuf,(uint8_t*)pbuf,4);
		pbuf[2] = pbuf[0];
		pbuf[0] = pbuf[1];
		pbuf[1] = pbuf[2];
		*pZeroData = *(uint16_t*)pbuf;
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
		return false;
	}

}
/*******************************************************************************
* Function Name  :
* Description    :
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpGetWaveData(uint16_t wave,uint16_t* pWaveData)
{
	char* pbuf;
//	uint8_t*phex;
	uint8_t len;
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'A';
	UVTxBuff[j++] = 'B';
	UVTxBuff[j++] = 'S';
	UVTxBuff[j++] = 'D';
	UVTxBuff[j++] = '=';
	j += sprintf(UVTxBuff + j,"%d",wave);
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';	
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,500);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,17);
	if(osSemaphoreWait(UVSEMHandle , 20000) == osOK)
	{
		pbuf = UVRxBuff;
		len = 12;
		while((*pbuf != '=') && len )
		{
			pbuf ++;
			len --;
		}
		if(len == 0)
			return false;
//		phex = (uint8_t*)pWaveData;
//		phex++;
		pbuf[0] = '0';
		StringToHex(pbuf,(uint8_t*)pbuf,4);
		pbuf[2] = pbuf[0];
		pbuf[0] = pbuf[1];
		pbuf[1] = pbuf[2];
		*pWaveData = *(uint32_t*)pbuf;
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
		return false;
	}

}
/*******************************************************************************
* Function Name  :
* Description    :
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
bool OOpGetDarkCur(uint16_t wave,uint16_t* pDarkCurData)
{
	char* pbuf;
//	uint8_t*phex;
	uint8_t len;
	int j = 0;
	UVTxBuff[j++] = ':';
	UVTxBuff[j++] = '0';
	UVTxBuff[j++] = '4';
	UVTxBuff[j++] = ' ';
	UVTxBuff[j++] = 'A';
	UVTxBuff[j++] = 'B';
	UVTxBuff[j++] = 'S';
	UVTxBuff[j++] = 'D';
	UVTxBuff[j++] = '=';
	UVTxBuff[j++] = '-';
	j += sprintf(UVTxBuff + j,"%d",wave);
	UVTxBuff[j++] = '\r';
	UVTxBuff[j++] = '\n';	
	HAL_UART_Transmit(&huart2,(uint8_t*)UVTxBuff,j,500);
	HAL_UART_Receive_IT(&huart2,(uint8_t*)UVRxBuff,17);
	if(osSemaphoreWait(UVSEMHandle , 20000) == osOK)
	{
		pbuf = UVRxBuff;
		len = 12;
		while((*pbuf != '=') && len )
		{
			pbuf ++;
			len --;
		}
		if(len == 0)
			return false;
//		phex = (uint8_t*)pWaveData;
//		phex++;
		pbuf[0] = '0';
		StringToHex(pbuf,(uint8_t*)pbuf,4);
		pbuf[2] = pbuf[0];
		pbuf[0] = pbuf[1];
		pbuf[1] = pbuf[2];
		*pDarkCurData = *(uint32_t*)pbuf;
		return true;
	}
	else
	{
		UART_EndReceive_IT(&huart2);
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
bool ReadOOpticsDarkData(uint16_t* pDark220, uint16_t* pDark275)
{
	uint16_t data220,data275;
	uint32_t sum220 = 0;
	uint32_t sum275 = 0;
	for(int i = 0;i < 10;i++)
	{
		osDelay(100);
		OOpGetDarkCur(220,&data220);		
		OOpGetDarkCur(275,&data275);
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
* Function Name  :
* Description    :
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/

bool ReadOOpticsData(uint16_t* pData220, uint16_t* pData275)
{	
	uint16_t dark220,dark275;
	uint16_t data220,data275;
	uint32_t sum220 = 0;
	uint32_t sum275 = 0;
//	
//	if(false == OOpHand())
//		return false;
//	if(false == OOpSetIntegralTime(5))
//		return false;
////	if(false == OOpSetLightType())
////		return false;
////	if(false == OOpSetTriggerMode())
////		return false;
//	if(false == OOpSetAvrgTimes(1))
//		return false;
	
	if(false == ReadOOpticsDarkData(&dark220,&dark275))
		return false;
	
	for(int i = 0;i < 10;i++)
	{
		osDelay(100);
		if(false == OOpGetWaveData(220,&data220))
			return false;			
		if(false == OOpGetWaveData(275,&data275))
			return false;
		sum220 += data220;
		sum275 += data275;
	}
	sum220 += 5;
	sum275 += 5;
	*pData220 = sum220 / 10 - dark220;
	*pData275 = sum275 / 10 - dark275;
	return true;
	
	
}

