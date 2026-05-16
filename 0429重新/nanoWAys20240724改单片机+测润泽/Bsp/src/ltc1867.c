 
#include "ltc1867.h"
#include "leddef.h"
#include "cmsis_os.h"
#include "tlc5620.h"
#include "port.h"
#include "peristalticpump.h"




extern osMutexId osADSMutex;//AD操作互斥信号量
extern uint16_t ReactionRefAD,ReactionAbsAD;
extern uint32_t CounterADSMutexError;//AD操作互斥信号量冲突计数
extern POOL_LT_ADS_OPT PoolLtADsOpt;

const char Chennel[8] = {LTC_CHNL0,LTC_CHNL1,LTC_CHNL2,LTC_CHNL3,LTC_CHNL4,LTC_CHNL5};//通道号对应的通道设置数组。数组标号对应通道号，数组内容对应通道设置。

SPI_HandleTypeDef* pADhspi = &hspi1;//AD芯片LTC1867对应的SPI端口.


HAL_StatusTypeDef SPIref;//SPI操作状态.

/*******************************************************************************
* Function Name  : delay_us
* Description    : 微秒级延时，因其阻塞性，不建议用作大延时。主要用作AD芯片操作延时。
* Input          : us:需要延时的微秒数
* Output         : None
* Return         : None
*******************************************************************************/
void delay_us(uint16_t us)
{
	uint16_t i,j;
	for(i=0;i<us;i++)
	for(j=0;j<7;j++);
}

/*******************************************************************************
* Function Name  : LTCReadADx
* Description    : 对某一通道的信号进行连续读取。
* Input          : 通道号，连续读取次数
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/

//uint16_t LTCReadADx(uint8_t ch,uint16_t nCount)
//{
//	long sum = 0;
//	uint16_t count = nCount;

//	uint8_t tdata[3] = {0};	
//	delay_us(10);
//	tdata[0] = ch;
//	SPIref = HAL_SPI_Transmit(pADhspi,tdata,2,1000);
//	if(HAL_OK == SPIref)
//		;		
//	while(count--)
//	{		
//		AD_SPI_NSS_HIGH;
//		delay_us(1);
//		AD_SPI_NSS_LOW;	
//		delay_us(2);
//		tdata[0] = ch;
//		SPIref = HAL_SPI_TransmitReceive(pADhspi,tdata,tdata+1,2,1000);	
//		tdata[0] = tdata[2];
//		sum += (*(uint16_t*)(tdata));
//	}
//	sum += (nCount/2);
//	sum /= nCount;
//	return (uint16_t)sum;
//}
/*******************************************************************************
* Function Name  : LTCReadADx
* Description    : 对透光和参照AD值进行同时连续读取。
* Input          : 通道号，连续读取次数
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
//uint16_t adabs[32],adref[32];
	
void ReadADs(uint16_t* pAD,uint16_t nCount)
{
	uint8_t spidata[4] = {0};	
	uint32_t adsum[6];
	uint8_t i;
	uint8_t ch = 0;
	uint16_t temp;
	uint16_t count = nCount;
	
	i = 0;
	while(i<6)
	{
		adsum[i++] = 0;
	}
	spidata[0] = Chennel[ch++];
	SPIref = HAL_SPI_Transmit(pADhspi,spidata,2,1000);		
	while(count--)      //一个周期：31us 
	{		
		i = 0;
		while(i<6)
		{
			AD_SPI_NSS_HIGH;
			delay_us(1);
			AD_SPI_NSS_LOW;			
			delay_us(2);
			taskENTER_CRITICAL();
			if(ch >= 6)ch = 0;
			spidata[0] = Chennel[ch++];		
			SPIref = HAL_SPI_TransmitReceive(pADhspi,spidata,spidata+1,2,1000);	
			taskEXIT_CRITICAL();
			spidata[3] = spidata[1];
			temp = (*(uint16_t*)(spidata+2));
			adsum[i++] += temp;
		}
	}
	i = 0;
	while(i<6)
	{
		adsum[i] += (nCount/2);
		pAD[i] = adsum[i] / nCount;
		i++;		
	}
	
}
/*******************************************************************************
* Function Name  : osLTCReadADx
* Description    : 此函数是对LTCReadADx函数的一次系统封装，使用互斥信号量，避免对AD芯片的操作冲突。
* Input          : 通道号，连续读取次数
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
//uint16_t osLTCReadADx(uint8_t ch,uint16_t nCount)
//{
//	uint16_t temp;
//	if(osMutexWait(osADSMutex, 500) == osOK)
//	{
//		temp= LTCReadADx(Chennel[ch],nCount);
//	if(osMutexRelease(osADSMutex) != osOK)
//		CounterADSMutexError++;
//	}
//	else
//		CounterADSMutexError++;
//	
//	return temp;
//}
/*******************************************************************************
* Function Name  : LTCReadADx
* Description    : 对透光和参照AD值进行同时连续读取。
* Input          : 通道号，连续读取次数
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/

//void ReadAbsADs(uint16_t* pADref,uint16_t* pADtrs,uint16_t nCount)
//{
//	uint8_t spidata[4] = {0};	
//	uint32_t sumref = 0;	
//	uint32_t sumabs = 0;
//	uint16_t count = nCount;
//	uint16_t temp;

//	
//	delay_us(10);
//	spidata[0] = Chennel[LED_REF_CH];
//	SPIref = HAL_SPI_Transmit(pADhspi,spidata,2,1000);		
//	while(count--)      //一个周期：31us 
//	{		
//		AD_SPI_NSS_HIGH;
//		delay_us(1);
//		AD_SPI_NSS_LOW;	
//		
//		delay_us(2);
//		taskENTER_CRITICAL();
//		spidata[0] = Chennel[LED_ABS_CH];		
//		SPIref = HAL_SPI_TransmitReceive(pADhspi,spidata,spidata+1,2,1000);	
//		taskEXIT_CRITICAL();
//		spidata[3] = spidata[1];
//		temp = (*(uint16_t*)(spidata+2));
//		sumref += temp;
////		adref[count] = temp;
//		
//		AD_SPI_NSS_HIGH;
//		delay_us(1);
//		AD_SPI_NSS_LOW;	
//		delay_us(2);
//		taskENTER_CRITICAL();
//		spidata[0] = Chennel[LED_REF_CH];		
//		SPIref = HAL_SPI_TransmitReceive(pADhspi,spidata,spidata+1,2,1000);	
//		taskEXIT_CRITICAL();
//		spidata[3] = spidata[1];
//		temp = (*(uint16_t*)(spidata+2));
//		sumabs += temp;
////		adabs[count] = temp;
//	}
//	
//	
//	
//	
//	sumref += (nCount/2);
//	sumabs += (nCount/2);
//	
//	*pADref = sumref / nCount;
//	*pADtrs = sumabs / nCount;

//}
/*******************************************************************************
* Function Name  : osLTCReadADx
* Description    : 操作系统读取吸收和参比AD值,此函数是对ReadAbsADs函数的一次系统调用封装。使用互斥信号量避免对AD芯片操作冲突。
* Input          : 通道号，连续读取次数
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
//void osReadAbsADs(uint16_t* pADref,uint16_t* pADtrs,uint16_t nCount)
//{
//	if(osMutexWait(osADSMutex, 500) == osOK)
//	{
//		ReadAbsADs(pADref,pADtrs,nCount);
//		if(osMutexRelease(osADSMutex) != osOK)
//			CounterADSMutexError++;
//	}
//	else
//		CounterADSMutexError++;
//}



void GetADabsReal(uint16_t* pADref,uint16_t* pADtrs,uint16_t nCount)
{
	DISABLE_PUMP_DRIVER
	CloseLLLED();
	osDelay(100);
	
	if(osMutexWait(osADSMutex, 500) == osOK)
		{
			PoolLtADsOpt.nSumOfRefAD = 0;
			PoolLtADsOpt.nSumOfTranAD = 0;
			PoolLtADsOpt.nNumOfCollect = nCount;
			if(osMutexRelease(osADSMutex) != osOK)
				CounterADSMutexError++;
		}
		else
			CounterADSMutexError++;
		while(PoolLtADsOpt.nNumOfCollect>0)
			osDelay(100);
		*pADref = (PoolLtADsOpt.nSumOfRefAD + nCount/2) / nCount;
		*pADtrs = (PoolLtADsOpt.nSumOfTranAD + nCount/2) / nCount;
		
		
		ENABLE_PUMP_DRIVER
		OpenLLLED();
}
//	uint32_t abs = 0;
//	uint32_t ref = 0;
//	uint16_t trsad,refad;//,trsad0,refad0;
//	int i;
//	nCount *= 20;
//	CloseAbsorbLED();	
//	i = nCount;
//	while(i>0)
//	{		
//		osDelay(20);
////		osReadAbsADs(&refad0,&trsad0,32);
////		OpenAbsorbLED();
////		delay_us(1000);
////		LED_TEST_ON	
//		osReadAbsADs(&refad,&trsad,32);	
////		LED_TEST_OFF
////		CloseAbsorbLED();		
////		trsad -= trsad0;
////		refad -= refad0;
//		abs += trsad;
//		ref += refad;		
//		i--;		
//		#ifdef LTC1867_DATA_DEBUG
//		SendLTCADsString(refad,trsad);
//		#endif
//	}
//	abs += (nCount/2);
//	ref += (nCount/2);
//	*pADtrs = abs / nCount;
//	*pADref = ref / nCount;
//}

void GetADWithBackground(uint16_t* pADref,uint16_t* pADtrs,uint16_t nCount)
{
//	uint32_t abs = 0;
//	uint32_t ref = 0;
//	uint16_t trsad,refad;//,trsad0,refad0;
//	int i;
//	nCount *= 20;
//	CloseAbsorbLED();	
//	i = nCount;
//	while(i>0)
//	{		
////		osDelay(20);
////		osReadAbsADs(&refad0,&trsad0,32);
////		OpenAbsorbLED();
//		delay_us(500);
////		LED_TEST_ON	
//		osReadAbsADs(&refad,&trsad,32);	
//////		LED_TEST_OFF
////		CloseAbsorbLED();		
////		trsad -= trsad0;
////		refad -= refad0;
//		abs += trsad;
//		ref += refad;		
//		i--;		
////		#ifdef LTC1867_DATA_DEBUG
////		SendLTCADsString(refad,trsad);
////		#endif
//	}
//	abs += (nCount/2);
//	ref += (nCount/2);
//	*pADtrs = abs / nCount;
//	*pADref = ref / nCount;
}

//void ReadADabs(uint16_t* pADref,uint16_t* pADtrs,int N)
//{
//	GetADabsReal(&ReactionRefAD,&ReactionAbsAD,N*20);
//	
//}

