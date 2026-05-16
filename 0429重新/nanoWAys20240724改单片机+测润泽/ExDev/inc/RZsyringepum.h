
 #ifndef  __UV_SPECTROGRAGH_H__
 #define  __UV_SPECTROGRAGH_H__
 #include <stdbool.h>
#include "stm32f1xx_hal.h"
//#include <stdlib.h>

typedef enum {
	UV_OK = 0x00,
	UV_NO_RESPONSE,
	UV_NO_DATA,
	UV_CHECK_ERROR,
	UV_NO_AA,
	
	
}UV_StatusTypeDef;
bool OOpGetZeroData(uint16_t wave,uint16_t* pZeroData);
bool OOpGetWaveData(uint16_t wave,uint16_t* pWaveData);
bool OOpGetDarkCur(uint16_t wave,uint16_t* pDarkCurData);
bool OOpInit(uint16_t integralTime,uint16_t refwave,uint16_t avrgTimes);
  UV_StatusTypeDef ReadUVData(uint16_t* pData220, uint16_t* pData275);
  
 #endif



