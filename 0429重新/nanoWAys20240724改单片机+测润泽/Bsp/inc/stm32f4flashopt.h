#ifndef __STM32F4_FLASH_H__
#define __STM32F4_FLASH_H__
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "stm32flashmap.h"

#define FLASH_WAITETIME  50000          //FLASH等待超时时间

 
 uint32_t Stm32ReadWord(uint32_t faddr);
 HAL_StatusTypeDef Stm32f4WriteWords(uint32_t WriteAddr,uint32_t *pBuffer,uint32_t NumToWrite);
 void Stm32f4ReadWords(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t NumToRead);
 void Test_Write(uint32_t WriteAddr,uint32_t WriteData);

void TestWrite(uint32_t WriteAddr,uint32_t WriteData);	
#endif
