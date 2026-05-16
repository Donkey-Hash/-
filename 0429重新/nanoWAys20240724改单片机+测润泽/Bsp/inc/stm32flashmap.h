#ifndef __STM32F4_FLASH_MAP_H__
#define __STM32F4_FLASH_MAP_H__
#include <stdbool.h>
#include "main.h"


#define STM32_FLASH_BASE 	(uint32_t)0x08000000 	//STM32 FLASH的起始地址


//FLASH 扇区的起始地址
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) 	//扇区0起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) 	//扇区1起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) 	//扇区2起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) 	//扇区3起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) 	//扇区4起始地址, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) 	//扇区5起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) 	//扇区6起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) 	//扇区7起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) 	//扇区8起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) 	//扇区9起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) 	//扇区10起始地址,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) 	//扇区11起始地址,128 Kbytes 

#define ADDR_FLASH_SECTOR_12	((uint32_t)0x08100000) 	//扇区12起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_13	((uint32_t)0x08104000) 	//扇区13起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_14    ((uint32_t)0x08108000) 	//扇区14起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_15	((uint32_t)0x0810C000) 	//扇区15起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_16    ((uint32_t)0x08110000) 	//扇区16起始地址, 64 Kbytes  
#define ADDR_FLASH_SECTOR_17	((uint32_t)0x08120000) 	//扇区17起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_18	((uint32_t)0x08140000) 	//扇区18起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_19	((uint32_t)0x08160000) 	//扇区19起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_20    ((uint32_t)0x08180000) 	//扇区20起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_21	((uint32_t)0x081A0000) 	//扇区21起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_22    ((uint32_t)0x081C0000) 	//扇区22起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_23    ((uint32_t)0x081E0000) 	//扇区23起始地址, 128 Kbytes   



#define ADDR_FLASH_BOOT 				(uint32_t)0x08000000     //boot loader起始地址
#define LENGTH_BOOT_FLASH 				0x10000					//boot loader 空间大小 64KB
#define FLASH_SOCTER_BOOT   			FLASH_SECTOR_0 //起始扇区 //包含0/1/2/3

#define ADDR_FLASH_RESERVE 				(uint32_t)0x08010000  		//缓冲区起始地址
#define LENGTH_RESERVE_FLASH 			0x10000					//缓存区大小64KB
#define FLASH_SOCTER_RESERVE   		FLASH_SECTOR_4 	//起始扇区

#define ADDR_FLASH_DATA 					((uint32_t)0x08020000)  		//数据区起始地址
#define LENGTH_DATA_FLASH					0x20000					//数据区长度 128KB
#define FLASH_SOCTER_DATA  				FLASH_SECTOR_5 	//起始扇区 




#define ADDR_FLASH_APP 						(uint32_t)0x08040000  		//应用区起始地址
#define LENGTH_APP_FLASH					0x40000					//应用去大小 256KB
#define FLASH_SOCTER_APP   				FLASH_SECTOR_6 	//起始扇区  包含6/7
#define APP_END_ADDRESS 					0x0807FFFF

#define ADDR_OTP_BASE 						(uint32_t)0x1FFF7800  		//OTP
#define ADDR_OTP_ENCRYPT_CODE 		ADDR_OTP_BASE + 0xE8  		//OTP
#define ADDR_OTP_SN 							ADDR_OTP_BASE + 0x60  		//OTP
#define ADDR_OTP_SIGN 						ADDR_OTP_BASE + 0x80  		//OTP
 
#define ENCRYPT_CODE

#define ADDR_FLASH_ENCRYPT_CODE		(ADDR_FLASH_RESERVE + LENGTH_RESERVE_FLASH - 64)
#define ADDR_FLASH_ALL_DEVS_DATA 	ADDR_FLASH_DATA

 uint32_t Stm32ReadWord(uint32_t faddr);
 HAL_StatusTypeDef Stm32f4WriteWords(uint32_t WriteAddr,uint32_t *pBuffer,uint32_t NumToWrite);
 void Stm32f4ReadWords(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t NumToRead);
 void Test_Write(uint32_t WriteAddr,uint32_t WriteData);

void TestWrite(uint32_t WriteAddr,uint32_t WriteData);	
#endif
