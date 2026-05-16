
 #ifndef  __UV_OPT2000_H__
 #define  __UV_OPT2000_H__
 #include <stdbool.h>
#include "stm32f4xx_hal.h"
//#include <stdlib.h>


  bool ReadOPT2000Data(uint16_t* pData220, uint16_t* pData275);
  bool ReadOPTDarkData(uint16_t* pDark220, uint16_t* pDark275);
 #endif



