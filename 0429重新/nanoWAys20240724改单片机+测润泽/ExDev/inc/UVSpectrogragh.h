
 #ifndef  __UV_SPECTROGRAGH_H__
 #define  __UV_SPECTROGRAGH_H__
 #include <stdbool.h>
#include "stm32f4xx_hal.h"
//#include <stdlib.h>

typedef enum {
	UV_OK = 0x00,
	UV_NO_RESPONSE,
	UV_NO_DATA,
	UV_CHECK_ERROR,
	UV_NO_AA,	
}UV_StatusTypeDef;

typedef bool (*READ_UV_DATA_FUN)(uint16_t* pData220, uint16_t* pData275);
 bool ReadUVData(uint16_t* pData220, uint16_t* pData275);
  
 #endif



