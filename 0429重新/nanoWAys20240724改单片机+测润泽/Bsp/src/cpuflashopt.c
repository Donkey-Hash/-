
#include "cpuflashopt.h"


//#if STM32_FLASH_SIZE < 256
//  #define STM_SECTOR_SIZE  1024 //字节
//#else 
//  #define STM_SECTOR_SIZE	 2048
//#endif




  

/*******************************************************************************
* 函数名: STMFlashWriteInOnePage
* 功  能: 向一个页内写数据,数据量大于1页,则报错.
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/
HAL_StatusTypeDef STMFlashWriteInOnePage ( uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite )	
{
	FLASH_EraseInitTypeDef erase;
	HAL_StatusTypeDef status;
	uint32_t eraseErrorAddr = 0;
//	if(NumToWrite > FLASH_PAGE_SIZE)   //超过一页报错
//		return HAL_ERROR;
	HAL_FLASH_Unlock();
	
//	erase.TypeErase = FLASH_TYPEERASE_PAGES;
//	erase.PageAddress = WriteAddr;
//	erase.NbPages = 1;

	status = HAL_FLASHEx_Erase(&erase, &eraseErrorAddr);
	if(HAL_OK == status)
	{
		uint16_t i=0;
		while(i<NumToWrite)
		{
			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,WriteAddr,pBuffer[i]);
			if(HAL_OK == status)
			{
				if(*(uint16_t *)WriteAddr == pBuffer[i++])
					WriteAddr+=2; 
				else
				{
					HAL_FLASH_Lock();
					return status;
				}
			}
			else
			{
				HAL_FLASH_Lock();
				return status;
			}			                                   
		}  
	}
	else
	{
		HAL_FLASH_Lock();
		return status;
	}
	HAL_FLASH_Lock();	
	return status;
}


