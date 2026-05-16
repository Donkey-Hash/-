#ifndef __CPU_FLASH_OPT_H__
#define __CPU_FLASH_OPT_H__


#include "stm32f4xx_hal.h"


HAL_StatusTypeDef STMFlashWriteInOnePage ( uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite );	

		

#endif 
