
#ifndef __DATA_COLLECT_TASK__
#define __DATA_COLLECT_TASK__
#include "main.h"


#define TEMP_CH2					4		//温度
#define TEMP_CH1					5		//温度
#define LED_REF_CH				0		//比色池参比
#define LED_ABS_CH				1		//比色池吸收
#define LL1_CH						2		//液位1
#define LL2_CH						3		//液位2


#define  NUM_ADC_CHANNELS  6
#define 	MUTI_ADC_SAMPLE		32
#define  AD_BUFF_SIZE		NUM_ADC_CHANNELS * MUTI_ADC_SAMPLE

extern uint16_t AD1Buffer[AD_BUFF_SIZE];
extern uint16_t AD1BufferM1[AD_BUFF_SIZE];
//extern uint16_t AD2Buffer[AD_BUFF_SIZE];
//extern uint16_t AD3Buffer[AD_BUFF_SIZE];
//extern uint16_t AD4Buffer[AD_BUFF_SIZE];

typedef struct {
	uint16_t nNumOfCollect;
	uint16_t ntemp;
	uint32_t nSumOfRefAD;
	uint32_t nSumOfTranAD;
	
} POOL_LT_ADS_OPT;


typedef struct {
	uint16_t nLLAD1;
	uint16_t nLLAD2;
}LL_ADs;

typedef struct {
	uint16_t nAD_IN0;
	uint16_t nAD_IN1;
	uint16_t nAD_IN7;
	uint16_t nAD_IN9;	
	uint16_t nAD_VTEMP;
	uint16_t nAD_VBat;
}IN_ADCS_VALULE;

typedef struct {
	uint32_t nAD_IN0;
	uint32_t nAD_IN1;
	uint32_t nAD_IN7;
	uint32_t nAD_IN9;		
	uint32_t nAD_VTEMP;
	uint32_t nAD_VBat;
}IN_ADCS_VALULE_SUM;

extern POOL_LT_ADS_OPT PoolLtADsOpt;
extern uint32_t CounterADSMutexError ;

#endif
