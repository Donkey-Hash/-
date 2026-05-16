

#ifndef __LTC_1867_H__
#define __LTC_1867_H__
#include "main.h"
//#include "spi.h"

#include "gpio.h"

#define AD_SPI_NSS_LOW	 		{HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port,SPI1_NSS_Pin, GPIO_PIN_RESET);}
#define AD_SPI_NSS_HIGH	 		{HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port,SPI1_NSS_Pin, GPIO_PIN_SET);}


//#define	LTC1867_DATA_DEBUG


#define LTC_CHNL0		0x8C//0x46
#define LTC_CHNL1		0xCC//0x66
#define LTC_CHNL2		0x9C//0x4E
#define LTC_CHNL3		0xDC//0x6E
#define LTC_CHNL4		0xAC//0x56
#define LTC_CHNL5		0xEC//0x76
#define LTC_CHNL6		0xBC//0x5E

#define TEMP_CH2					0		//温度
#define TEMP_CH1					1		//温度
#define LED_REF_CH				2		//比色池参比
#define LED_ABS_CH				3		//液位1
#define LL1_CH						4		//比色池吸收
#define LL2_CH						5		//液位2





typedef struct {
	uint16_t nNumOfCollect;
	uint16_t ntemp;
	uint32_t nSumOfRefAD;
	uint32_t nSumOfTranAD;
	
} POOL_LT_ADS_OPT;


extern const char Chennel[8];








uint16_t LTC1867_ADRead(uint8_t ch);
void ReadADs(uint16_t* pAD,uint16_t nCount);
uint16_t LTCReadADx(uint8_t ch,uint16_t nCount);
uint16_t osLTCReadADx(uint8_t ch,uint16_t nCount);
void ReadAbsADs(uint16_t* pADref,uint16_t* pADtrs,uint16_t nCount);

void osReadAbsADs(uint16_t* pADref,uint16_t* pADtrs,uint16_t nCount);
void delay_us(uint16_t us);

#endif

