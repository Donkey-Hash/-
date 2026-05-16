
#include "AD7682.h"

#include "cmsis_os.h"
#include "port.h"
//#include "EleChemiRelayMode.h"

extern osMutexId osADSMutex;//AD操作互斥信号量
extern uint16_t ReactionRefAD,ReactionAbsAD;
extern uint32_t CounterADSMutexError;//AD操作互斥信号量冲突计数

//SPI_HandleTypeDef* pADhspi = &hspi1;//AD芯片LTC1867对应的SPI端口.


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
	for(j=0;j<17;j++);
}

/*******************************************************************************
* Function Name  : LTCReadADx
* Description    : 对某一通道的信号进行连续读取。
* Input          : 通道号，连续读取次数
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/


void vSPI_config(void)
{
//	DIS_ADC;
//	DIS_DAC;

//	DDRB |= _BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3) | _BV(PORTB5);    //SS,mosi,clk output

//	// SPI mode 0, msb first, fosc/4
//	//set to master mode, cpol=0, cpha=0
//	// NOTE ADC uses cpol=0 but DAC uses cpol=1
//	SPCR = _BV(MSTR) /*| _BV(CPOL) | _BV(SPR0) */;
//	SPCR |= _BV(SPE);
}
/*******************************************************************************
* 函数名: GetDAVbyVol
* 功  能: 把电压vol(-2048mV  ---- 2048mV)转换为65535 -- 0 的DA输入值
* 输  入: 工作极电压设置。(-2048mV  ---- 2048mV)
* 输  出: None
* 返回值: DA输入值
*******************************************************************************/
uint16_t GetDAVbyVol(int16_t vol) 
{
	if(vol <= -2048)
	{
		vol = -2048;
		return (uint16_t)((long)32767 - ((int16_t)(vol) << 4));//防止溢出
	}
	else
	{
		if(vol > 2048)
		{
			vol = 2048;
		}
		return (uint16_t)((long)32768 - ((int16_t)(vol) << 4));
	}
}

/*******************************************************************************
* 函数名: 
* 功  能: 
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/
//void SetDAC(uint16_t u2dac)
//{
//	uint8_t TxData;
//	uint16_t retry = 0;
////    u2dac += 1;
//	DIS_DAC;
//	delay_us(1);
//		SEL_DAC
//	delay_us(1);
//	while (!LL_SPI_IsActiveFlag_TXE(SPI2)) 
//	{
//		retry++;
//		if(retry > 200) 
//			return ;
//	}	
//	

//	TxData = 0;
//	LL_SPI_TransmitData8(SPI2, TxData);
//	retry = 0;
//	while (!LL_SPI_IsActiveFlag_TXE(SPI2)) 
//	{
//		retry++;
//		if(retry > 1000) 
//			return ;
//	}
//	TxData = u2dac >> 8;;
//	LL_SPI_TransmitData8(SPI2, TxData);
//	retry = 0;
//	while (!LL_SPI_IsActiveFlag_TXE(SPI2)) 
//	{
//		retry++;
//		if(retry > 1000) 
//			return ;
//	}
//	TxData = u2dac & 0xFF ;
//	LL_SPI_TransmitData8(SPI2, TxData);
//	retry = 0;
//	while (!LL_SPI_IsActiveFlag_TXE(SPI2)) 
//	{
//		retry++;
//		if(retry > 1000) 
//			return ;
//	}
//	delay_us(1);
//}
/*******************************************************************************
* 函数名: 
* 功  能: 
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/
//void SetDACVol(int32_t i4Vol)
//{
//   SetDAC(GetDAVbyVol(i4Vol));
//}
/*******************************************************************************
* 函数名: 
* 功  能: 
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/
uint8_t SPI2_ReadWriteByte(uint8_t TxData)
{		
	uint8_t retry = 0;				 
 
	/* Check if Tx buffer is empty */
	while (!LL_SPI_IsActiveFlag_TXE(SPI2)) 
	{
		retry++;
		if(retry > 200) 
			return 0;
	}			  
 
	/* Write character in Data register.
	TXE flag is cleared by reading data in DR register */
	LL_SPI_TransmitData8(SPI2, TxData);
	retry = 0;
 
	/* Check if Rx buffer is not empty */
	while (!LL_SPI_IsActiveFlag_RXNE(SPI2)) 
	{
		retry++;
		if(retry > 200) 
			return 0;
	}	  						    
 
	/* received byte from SPI lines. */
	return LL_SPI_ReceiveData8(SPI2); 				    
}
uint16_t SPI2_ReadWrite(uint16_t TxData)
{		
	uint16_t retry = 0;				 
 
	/* Check if Tx buffer is empty */
	while (!LL_SPI_IsActiveFlag_TXE(SPI2)) 
	{
		retry++;
		if(retry > 200) 
			return 0;
	}			  
 
	/* Write character in Data register.
	TXE flag is cleared by reading data in DR register */
	LL_SPI_TransmitData16(SPI2, TxData);
	retry = 0;
 
	/* Check if Rx buffer is not empty */
	while (!LL_SPI_IsActiveFlag_RXNE(SPI2)) 
	{
		retry++;
		if(retry > 1000) 
			return 0;
	}	  						    
 
	/* received byte from SPI lines. */
	return LL_SPI_ReceiveData16(SPI2); 				    
}
/*******************************************************************************
* 函数名: 
* 功  能: 
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/
uint16_t TempAD;
uint16_t cfgback;
uint16_t ADsRead[128];
uint16_t ReadADC(uint8_t ch, uint16_t count)
{
    uint16_t cfg;
//    uint8_t tmpH, tmpL;

    uint32_t res = 0;
    uint8_t u1BadPoint = 3;
    uint16_t i = 0;  
    uint16_t u2Samples = 0;
    cfg = 0x3C09+((int16_t)ch << 7);
    cfg <<= 2;
//    tmpH = cfg >> 8;
//    tmpL = cfg & 0xFF;

    // 1st dummy cnv
    SEL_ADC;
//		delay_us(1);
    DIS_ADC;
    delay_us(4);
	
    while(u2Samples   < count)
    {
       if (i > 0)
       {
           cfg &= 0x7FFF;
       }
       SEL_ADC;
//			 delay_us(1);
			 TempAD = SPI2_ReadWrite(cfg);
//			cfgback =LL_SPI_ReceiveData16(SPI2);
//			 tmpL = SPI2_ReadWriteByte(tmpL);
       DIS_ADC;
       delay_us(4);
//       tmp = ((uint16_t)tmpH << 8) + tmpL/*+45*/;   /*correct*/
       if (i >= u1BadPoint)//前四个数丢掉
       {
           res += TempAD;
				 ADsRead[u2Samples] = TempAD;
           u2Samples++;
						
       }
       i++;
    }    
    res /= u2Samples;
    
    return (uint16_t)res;
}

//uint16_t ReadADC(uint8_t ch, uint16_t count)
//{
//    uint16_t cfg;
//    uint8_t tmpH, tmpL;
//    uint16_t tmp;
//    uint32_t res = 0;
//    uint8_t u1BadPoint = 0;
//    uint16_t i = 0;  
//    uint16_t u2Samples = 0;
//    cfg = 0x3C31+((int16_t)ch << 7);
//    cfg <<= 2;
//    tmpH = cfg >> 8;
//    tmpL = cfg & 0xFF;

//    // 1st dummy cnv
//    SEL_ADC;
//		delay_us(10);
//    DIS_ADC;
//    delay_us(10);
//    while(u2Samples   < count)
//    {
//       if (i > 0)
//       {
//           tmpH &= 0x7F;
//       }
//       SEL_ADC;
//			 delay_us(10);
//			 tmpH = SPI2_ReadWriteByte(tmpH);
//			 tmpL = SPI2_ReadWriteByte(tmpL);
//       DIS_ADC;
//       delay_us(20);
//       tmp = ((uint16_t)tmpH << 8) + tmpL/*+45*/;   /*correct*/
//       if (i >= u1BadPoint)//前四个数丢掉
//       {
//           res += tmp;
//           u2Samples++;
//       }
//       i++;
//    }    
//    res /= u2Samples;
//    
//    return (uint16_t)res;
//}

/*******************************************************************************
* 函数名: 
* 功  能: 
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/
//int32_t CalcCurrentByADC(uint16_t u2ADC)
//{
//   int32_t i4Temp = 0;
//   CURRENT_RANGE_E eMode = eGetCurrentRange();

//   //i4Temp = 20480 - (((int32_t)u2ADC * 40960) / 65536); 
//   switch(eMode)
//   {
//   case CURRENT_0_4uA:
//      //i4Temp = i4Temp * 100 / 500;
//      i4Temp = 4096 - (((int32_t)u2ADC * 5) / 40);
//      break;
//   case CURRENT_0_20uA:
//      //i4Temp = i4Temp * 100 / 100;
//      i4Temp =  20480 - (((int32_t)u2ADC * 5) / 8);
//      break;
//   case CURRENT_0_200uA:
//      //i4Temp = i4Temp * 100 / 10;
//      i4Temp =  204800 - ((int32_t)u2ADC * 50 / 8);
//      break;
//   case CURRENT_0_2000uA:
//      //i4Temp = i4Temp * 100 / 1;
//      i4Temp = 2048000 - ((int32_t)u2ADC * 500 / 8);
//      break;   
//   default:
//      break;
//   }
//   return i4Temp;   
//}
