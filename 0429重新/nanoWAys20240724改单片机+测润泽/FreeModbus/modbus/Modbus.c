
#include <stdbool.h>
#include "Modbus.h"
#include "modbusdata.h"
#include "modbusdata_mb3.h"
#include "processcontroltask.h"
#include "cmsis_os.h"
extern osThreadId ProcTaskHandle;
extern osSemaphoreId RS485Comm3RxHandle;
extern bool HaveNewCMDFlag ;
extern bool AnalyzerBeWorking;
//#include "FlashUserData.h"
//#include "HoldingBuffCfg.h"

//extern osSemaphoreId UpdataSEMHandle;
/* ----------------------- Variables ---------------------------------*/
USHORT   usDiscreteInputStart                             = DISCRETE_INPUT_START;
//UCHAR    usDiscreteInputBuf[DISCRETE_INPUT_NDISCRETES/8]  ;
USHORT   usCoilStart                                      = COIL_START;
//UCHAR    usCoilBuf[COIL_NCOILS/8]                         ;
USHORT   usRegInputStart                                  = REG_INPUT_START;
//USHORT   usRegInputBuf[REG_INPUT_NREGS]                   ;
USHORT   usRegHoldingStart                                = REG_HOLDING_START;

USHORT   usRegBlockStart                                	= REG_BLOCK_START;
//USHORT   usRegHoldingBuf[REG_HOLDING_NREGS]               ={0x55};
extern UCHAR    *usDiscreteInputBuf;
extern UCHAR *usCoilBuf;
extern USHORT*   	usRegHoldingBuf;
extern USHORT* 		usRegInputBuf;

UCHAR usBlockBuf[REG_BLOCK_NREGS][128];
//******************************输入寄存器回调函数**********************************
//函数定义: eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
//描    述：输入寄存器相关的功能（读、连续读）
//入口参数：pucRegBuffer : 回调函数将Modbus寄存器的当前值写入的缓冲区
//			usAddress    : 寄存器的起始地址，输入寄存器的地址范围是1-65535。
//			usNRegs      : 寄存器数量
//出口参数：eMBErrorCode : 这个函数将返回的错误码
//备    注：Editor：Armink 2010-10-31    Company: BXXJS
//**********************************************************************************
eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
		uint16_t		iRegIndex;
		USHORT *pInpBuf;
		const USHORT BASE_ADDR = 1; 
    const USHORT MAX_REGS = 1024; // 你的数组大小
		    // 【关键】边界检查
//    if (usAddress < BASE_ADDR || (usAddress - BASE_ADDR) >= MAX_REGS) {
//         // 非法访问！直接返回错误，不要写内存
//         return MB_ENOERR; 
//    }
    
    USHORT index = usAddress - BASE_ADDR;
    
    // 如果 NRegs 超过剩余空间，也会越界
    if (index + usNRegs > MAX_REGS) {
         return MB_ENOERR;
    }
	
	
		if (g_mb_inst == 3) {
				pInpBuf = usRegInputBuf3;
//		} else if (g_mb_inst == 6) {
//				pInpBuf = usRegInputBuf6;
		} else {
				pInpBuf = usRegInputBuf;
		}
		
    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( unsigned char )( pInpBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ = ( unsigned char )( pInpBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}
//******************************保持寄存器回调函数**********************************
//函数定义: eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
//描    述：保持寄存器相关的功能（读、连续读、写、连续写）
//入口参数：pucRegBuffer : 如果需要更新用户寄存器数值，这个缓冲区必须指向新的寄存器数值。
//                         如果协议栈想知道当前的数值，回调函数必须将当前值写入这个缓冲区
//			usAddress    : 寄存器的起始地址。
//			usNRegs      : 寄存器数量
//          eMode        : 如果该参数为eMBRegisterMode::MB_REG_WRITE，用户的应用数值将从pucRegBuffer中得到更新。
//                         如果该参数为eMBRegisterMode::MB_REG_READ，用户需要将当前的应用数据存储在pucRegBuffer中
//出口参数：eMBErrorCode : 这个函数将返回的错误码
//备    注：Editor：Armink 2010-10-31    Company: BXXJS
//**********************************************************************************
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
		uint16_t		iRegIndex;
    USHORT          *pHoldBuf;
		const USHORT BASE_ADDR = 1; 
    const USHORT MAX_REGS = 1024; // 你的数组大小
 
    USHORT index = usAddress - BASE_ADDR;
    // 如果 NRegs 超过剩余空间，也会越界
    if (index + usNRegs > MAX_REGS) {
         return MB_ENOERR;
    }		
	
    if (g_mb_inst == 3) {
        pHoldBuf = usRegHoldingBuf3;
    } 
//    else if (g_mb_inst == 6) {
//        pHoldBuf = usRegHoldingBuf6; // 指向串口6的表
//    } 
    else {
        pHoldBuf = usRegHoldingBuf;  // 默认串口1
    }
		
		if( ( usAddress >= REG_HOLDING_START ) &&
        ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( unsigned char )( pHoldBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( unsigned char )( pHoldBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                pHoldBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pHoldBuf[iRegIndex] |= *pucRegBuffer++;
							if(iRegIndex == 0)
								HaveNewCMDFlag = true;
							else if(iRegIndex > 0 && iRegIndex < 10)
								{
									usRegHoldingBuf[0] = iRegIndex;
									HaveNewCMDFlag = true;
								}
							
                iRegIndex++;
                usNRegs--;
            }
//							osSemaphoreRelease(UpdataSEMHandle);
        }

    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
//****************************线圈状态寄存器回调函数********************************
//函数定义: eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
//描    述：线圈状态寄存器相关的功能（读、连续读、写、连续写）
//入口参数：pucRegBuffer : 位组成一个字节，起始寄存器对应的位处于该字节pucRegBuffer的最低位LSB。
//                         如果回调函数要写这个缓冲区，没有用到的线圈（例如不是8个一组的线圈状态）对应的位的数值必须设置位0。
//			usAddress    : 第一个线圈地址。
//			usNCoils     : 请求的线圈个数
//          eMode        ；如果该参数为eMBRegisterMode::MB_REG_WRITE，用户的应用数值将从pucRegBuffer中得到更新。
//                         如果该参数为eMBRegisterMode::MB_REG_READ，用户需要将当前的应用数据存储在pucRegBuffer中
//出口参数：eMBErrorCode : 这个函数将返回的错误码
//备    注：Editor：Armink 2010-10-31    Company: BXXJS
//**********************************************************************************
eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex , iRegBitIndex , iNReg;
    iNReg =  usNCoils / 8 + 1;        //占用寄存器数量
    if( ( usAddress >= COIL_START ) &&
        ( usAddress + usNCoils <= COIL_START + COIL_NCOILS ) )
    {
        iRegIndex    = ( int )( usAddress - usCoilStart ) / 8 ;    //每个寄存器存8个
		iRegBitIndex = ( int )( usAddress - usCoilStart ) % 8 ;	   //相对于寄存器内部的位地址
        switch ( eMode )
        {
            /* Pass current coil values to the protocol stack. */
        case MB_REG_READ:
            while( iNReg > 0 )
            {
				*pucRegBuffer++ = xMBUtilGetBits(&usCoilBuf[iRegIndex++] , iRegBitIndex , 8);
                iNReg --;
            }
			pucRegBuffer --;
			usNCoils = usNCoils % 8;                        //余下的线圈数	
			*pucRegBuffer = *pucRegBuffer <<(8 - usNCoils); //高位补零
			*pucRegBuffer = *pucRegBuffer >>(8 - usNCoils);
            break;

            /* Update current coil values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while(iNReg > 1)									 //最后面余下来的数单独算
            {
							xMBUtilSetBits(&usCoilBuf[iRegIndex++] , iRegBitIndex  , 8 , *pucRegBuffer++);
                iNReg--;
            }
						usNCoils = usNCoils % 8;                            //余下的线圈数
						xMBUtilSetBits(&usCoilBuf[iRegIndex++] , iRegBitIndex  , usNCoils , *pucRegBuffer++);
						if(false == AnalyzerBeWorking)
						{
							AnalyzerBeWorking = true;
							osThreadFlagsSet(ProcTaskHandle,COMM_VALVE_OPT);//osSignalSet(ProcTaskHandle,COMM_VALVE_OPT);	
						}
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
//****************************离散输入寄存器回调函数********************************
//函数定义: eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
//描    述：离散输入寄存器相关的功能（读、连续读）
//入口参数：pucRegBuffer : 用当前的线圈数据更新这个寄存器，起始寄存器对应的位处于该字节pucRegBuffer的最低位LSB。
//                         如果回调函数要写这个缓冲区，没有用到的线圈（例如不是8个一组的线圈状态）对应的位的数值必须设置为0。
//			usAddress    : 离散输入的起始地址
//			usNDiscrete  : 离散输入点数量
//出口参数：eMBErrorCode : 这个函数将返回的错误码
//备    注：Editor：Armink 2010-10-31    Company: BXXJS
//**********************************************************************************
eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode    eStatus = MB_ENOERR;
	int             iRegIndex , iRegBitIndex , iNReg;
	iNReg =  usNDiscrete / 8 + 1;        //占用寄存器数量
    if( ( usAddress >= DISCRETE_INPUT_START )
        && ( usAddress + usNDiscrete <= DISCRETE_INPUT_START + DISCRETE_INPUT_NDISCRETES ) )
    {
        iRegIndex    = ( int )( usAddress - usDiscreteInputStart ) / 8 ;    //每个寄存器存8个
		iRegBitIndex = ( int )( usAddress - usDiscreteInputStart ) % 8 ;	   //相对于寄存器内部的位地址
	    while( iNReg > 0 )
        {
			*pucRegBuffer++ = xMBUtilGetBits(&usDiscreteInputBuf[iRegIndex++] , iRegBitIndex , 8);
            iNReg --;
        }
		pucRegBuffer --;
		usNDiscrete = usNDiscrete % 8;                     //余下的线圈数	
		*pucRegBuffer = *pucRegBuffer <<(8 - usNDiscrete); //高位补零
		*pucRegBuffer = *pucRegBuffer >>(8 - usNDiscrete);
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}
//******************************Modbus寄存器初始化函数**************************************
//函数定义: void ModbusRegInit(uint8_t Fun)
//描    述：把RAM中Modbus的保持寄存器的值存到Flash里，或把Flash中的数取出放到Modbus RAM寄存器
//入口参数：Fun 0 : 把RAM中Modbus的保持寄存器的值存到Flash里
//              1 : 把Flash中的数取出,放到Modbus的RAM寄存器中
//出口参数：无
//备    注：Editor：Armink 2010-11-3    Company: BXXJS
//**********************************************************************************
void ModbusRegInit(uint8_t Fun)
{
	uint16_t i ;
	uint32_t Addr ;
	if (0 == Fun)
	{
//		SaveUserData();
	}
	else if (1 == Fun)
	{
		for (i = 0 ,Addr = FALSH_USER_START_ADDRESS; i < REG_HOLDING_NREGS ; i ++ , Addr += 2)
		{
			 usRegHoldingBuf[i] = (uint16_t)(*(__IO uint16_t*) (Addr));    //16bit = 2byte = 1/2 word	
		}		
	}
	
}	 
//******************************保持寄存器回调函数**********************************
//函数定义: eMBErrorCode eMBRegBlockCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
//描    述：保持寄存器相关的功能（读、连续读、写、连续写）
//入口参数：pucRegBuffer : 如果需要更新用户寄存器数值，这个缓冲区必须指向新的寄存器数值。
//                         如果协议栈想知道当前的数值，回调函数必须将当前值写入这个缓冲区
//			usAddress    : 寄存器的起始地址。
//			usNRegs      : 寄存器数量
//          eMode        : 如果该参数为eMBRegisterMode::MB_REG_WRITE，用户的应用数值将从pucRegBuffer中得到更新。
//                         如果该参数为eMBRegisterMode::MB_REG_READ，用户需要将当前的应用数据存储在pucRegBuffer中
//出口参数：eMBErrorCode : 这个函数将返回的错误码
//备    注：Editor：Armink 2010-10-31    Company: BXXJS
//**********************************************************************************
eMBErrorCode
eMBRegBlockCB( UCHAR * pucRegBuffer, USHORT usAddress, UCHAR usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
		int 						iDataIndex = 0;
    if( ( usAddress >= REG_BLOCK_START ) &&
        ( usAddress <= REG_BLOCK_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegBlockStart );
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( usBlockBuf[iRegIndex][iDataIndex] );
                iDataIndex++;
                usNRegs--;
            }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usBlockBuf[iRegIndex][iDataIndex] = *pucRegBuffer++;
                iDataIndex++;
                usNRegs--;
            }
//							osSemaphoreRelease(UpdataSEMHandle);
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
