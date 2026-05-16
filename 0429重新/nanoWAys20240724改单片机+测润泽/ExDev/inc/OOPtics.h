
 #ifndef  __O_OPTICS_H__
 #define  __O_OPTICS_H__
 #include <stdbool.h>
#include "stm32f4xx_hal.h"
//#include <stdlib.h>

bool OOpGetZeroData(uint16_t wave,uint16_t* pZeroData);
bool OOpGetWaveData(uint16_t wave,uint16_t* pWaveData);
bool OOpGetDarkCur(uint16_t wave,uint16_t* pDarkCurData);
bool OOpInit(uint16_t integralTime,uint16_t refwave,uint16_t avrgTimes);
bool ReadOOpticsData(uint16_t* pData220, uint16_t* pData275);
  
 #endif



