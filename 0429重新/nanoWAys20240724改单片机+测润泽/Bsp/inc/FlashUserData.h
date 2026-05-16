
#ifndef __FLASH_USER_DATA__
#define __FLASH_USER_DATA__
#include <stdbool.h>
#include "pid.h"
#include "Modbus.h"
#include "modbusdata.h"
#include "instrumentinfo.h"
//#include "gasflow.h"


//#define FLASH_USER_DATA_START_ADDRESS    (uint32_t)0x0800FC00
#define MAX_NUM_DEVS   32	// 定义最多使用的设备数量

//#define THE_FIRMSOFT_VERSION 					12  //1.2
typedef enum{
	SYSTEM_START = 0x00,
	UART_3_TX_FALSE = 1,
	HOST_NO_REPAY  = 2,
	HOST_SYN_FALSE = 3,
	HOST_FRAME_ERR = 4,
	PROC_START = 5,
	PROC_END = 6,	
	PROC_FAULT = 7,
	SEND_MAIL_FALSE = 8,
	PROC_UPDATE_STEP_NAME = 9,
	PROC_UPDATE_STEP = 10,   		//0x0A
	PROC_RUN_FALSE = 11,    		//0x0B
	PROC_CMD_FINISHED = 12,			//0x0C
	PROC_WORK_MODE = 13,				//0x0D
	PROC_CALIB_FALSE = 14,			//0x0E	
	PROC_START_INIT_PROC = 15,	//0x0F
	
	ERROR_SPECTRO_TROUBLE = 16,
	ERROR_SPECTRO_TROUBLE_BLANK = 17,
	PROC_LL_INIT_FALSE = 18,
	
	SYSTEM_RESET = 0xFE,
	
	
}RECORD_ERROR_CODE;

typedef enum{
	FLAG_BOOT_NORMAL,  //正常模式下监测uart1手动输入C，等待5秒
	FLAG_BOOT_APP_CMD, //APP中接收到boot指令，通过uart1进行代码升级，跳过监测等待。
	FLAG_BOOT_U_DICK,  
	FLAG_BOOT_HTTP,
	FLAG_BOOT_GPRS,
	FLAG_BOOT_4G,
	FLAG_BOOT_BLUE_TOOTH,
	FLAG_BOOT_Ex_FLASH,
	FLAG_BOOT_JUMP_APP,
	
}FLAG_BOOT_MODE;
//typedef struct{
//	uint8_t nErrCount;
//	RECORD_ERROR_CODE nErrCodes[512];
//	uint8_t nRemarks[512];
//}ERROR_TYPE;

typedef struct{
	uint8_t nProcCount;
	RECORD_ERROR_CODE nErrCodes;
	uint8_t nRemarks;
}ERROR_RECORD_t;

typedef struct{
	uint16_t nErrCount;
	ERROR_RECORD_t nErrRecord[32];
}ERROR_RECORDS;

typedef struct{
	USHORT mbHoldingBuf[REG_HOLDING_NREGS];		// 04保持寄存器
	USHORT mbInputBuf[REG_INPUT_NREGS];			// 03输入寄存器
	UCHAR  mbCoilBuf[COIL_NCOILS/8];			// 01线圈
	UCHAR  mbDiscreteInputBuf[DISCRETE_INPUT_NDISCRETES/8];	// 02离散输入
	uint32_t nFlag;	
}MODBUS_BUF_DATA_IN_FLASH;

typedef struct{
	DEV_OPERAT_PARA tAllDevsOperatPara[MAX_NUM_DEVS];
	MODBUS_BUF_DATA_IN_FLASH tModbusData;
	FLAG_BOOT_MODE nFlagBoot;
}ALL_DEV_DATA;


//extern MODBUS_BUF_DATA_IN_FLASH UserData; 
extern DEV_OPERAT_PARA ThisDevOperatPara ; 
//extern USHORT* pErroCode;
//extern USHORT* pDataMulti;
//extern USHORT DataMulti;
//extern FlashData_t FlashData[10];
//void InitUserData(void);
void SaveUserData(void);
void SetBootJumpAppNoWait(void);
extern MODBUS_HOLDING_REG_t *pMbHoldData;
extern MODBUS_INPUT_REG_t *pMbInputData;
extern UCHAR*    usDiscreteInputBuf;
//extern DEV_OPERAT_PARA pDevOperatPara[MAX_DEV_NUM];	;


void InitUserData(void);

void AddErrRecord(RECORD_ERROR_CODE err,uint16_t remark);

void GetDevParaDataFromFlash(DEVICE_CODE dev);
HAL_StatusTypeDef SaveDevParaDataToFlash(DEVICE_CODE dev);

#endif
