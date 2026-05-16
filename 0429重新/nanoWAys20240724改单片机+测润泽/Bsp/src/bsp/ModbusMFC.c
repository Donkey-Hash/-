
#include "port.h"
#include "mbcrc.h"
#include "ModbusMFC.h"
#include "ModbusMaster.h"

//#include "softtimer.h"

extern bool SetMFCFlowFlag;
//extern SET_FLOWS_t tSetFlows;
/******************************************************/
bool ReadMFCFlowFlag = false;

uint8_t MbFlowSetStep = 0;
bool ModbusBusy = false;

MB_MFC_DEV_INFO MbMFCDevInfo[8];


//SET_FLOWS_t tMFCCurrentSet = {0};
bool MFCUpdateFlag[5];
uint8_t MFCRevError;
//MODBUS_MFC_UPDATE_FLAG MFCUpdateFlag;
/******************************************************/

void GetMbMFCFlow(MB_MFC_DEV dev)
{
	CMDReadDevRegs(dev,0x800E,1);
}


void SetMbMFCFlow(MB_MFC_DEV dev,uint16_t flow)
{
	CMDWriteDevRegs(dev,0x800F,1,&flow);
}


void ReadDevResponse(uint8_t* pData)
{
	uint16_t dlen;
	uint16_t data;
//	StopTimer(TX_MFC_TIMEOUT);
	if(*(pData+1) == 0x03)   //读Modbus MFC的返回数据
	{
		dlen = *(pData+2);
//		dlen <<= 8;
//		dlen += *(pData+3);
		if(dlen == 0x02)
		{		
			data = *(pData+3);
			data <<= 8;
			data += *(pData+4);		
			
			switch(*pData)
			{
				case MB_MFC_CH4:
//					pReplyADs->nCH4Flow = data;
					break;
				case MB_MFC_HeO2:
//					pReplyADs->nHeO2Flow = data;
					break;
				case MB_MFC_He:
//					pReplyADs->nHeFlow = data;
					break;
				case MB_MFC_HePurge:
//					pReplyADs->nHePurgeFlow = data;
					break;
				case MB_MFC_Sample:
//					pReplyADs->nSampleFlow = data/10;
					break;
				default:
					break;
			}
			
			MbMFCDevInfo[*pData].nFlowPV = data;
			MbMFCDevInfo[*pData].bUpdataFlag = true;
		}
		else
		{
			MFCRevError = 1;
		}
//		StopTimer(TX_MFC_TIMEOUT);	
		MbFlowSetStep++;
		ModbusBusy = false;	
	}
	else if(*(pData+1) == 0x10) //设置Modbus MFC数据返回
	{
		data = *(pData+2);
		data <<= 8;
		data += *(pData+3);
		if(data == 0x800F)
		{
		switch(*pData)
			{
				case MB_MFC_CH4:
//					 tMFCCurrentSet.nFlow[0] = tSetFlows.nFlow[0];
					break;
				case MB_MFC_HeO2:
//					tMFCCurrentSet.nFlow[1] = tSetFlows.nFlow[1];
					break;
				case MB_MFC_He:
//					tMFCCurrentSet.nFlow[2] = tSetFlows.nFlow[2];
					break;
				case MB_MFC_HePurge:
//					tMFCCurrentSet.nFlow[3] = tSetFlows.nFlow[3];
					break;
				case MB_MFC_Sample:
//					tMFCCurrentSet.nFlow[4] = tSetFlows.nFlow[4];
					break;
				default:
					break;
			}
		}
		else
		{
			MFCRevError = 2;
		}
//		StopTimer(TX_MFC_TIMEOUT);	
		MbFlowSetStep++;
		ModbusBusy = false;
	}
	
}




/*******************************************************************************
* 函数名: 
* 功  能: 
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/

//void fSetMbMFCFlows(SET_FLOWS_t* pad)
//{
//	float flow;
//	switch(MbFlowSetStep)
//	{
//		case 0:
//			if(ReadMFCFlowFlag)
//				MbFlowSetStep = 1;
//			break;		
//		case 1:
//			if(MFCUpdateFlag[0])
//			{
//				if(!ModbusBusy)
//				{
//					ModbusBusy = true;
//					ENTER_CRITICAL_SECTION();							//EXIT_CRITICAL_SECTION()
//					flow =  pad->nFlow[0];
//					EXIT_CRITICAL_SECTION();	
//					SetMbMFCFlow(MB_MFC_CH4,flow*10);
////					EnableTimer(TX_MFC_TIMEOUT);
//					MFCUpdateFlag[0] = false;
//	//				MbFlowSetStep++;
//				}
//			}
//			else
//			{
//				MbFlowSetStep++;
//			}
//			break;
//		case 2:
//			if(!ModbusBusy)
//			{
//				ModbusBusy = true;
//				GetMbMFCFlow(MB_MFC_CH4);
//				EnableTimer(TX_MFC_TIMEOUT);
////				MbFlowSetStep++;
//			}
//		  break;
//		case 3:
//			if(MFCUpdateFlag[1])
//			{
//				if(!ModbusBusy)
//				{
//					ModbusBusy = true;
//					ENTER_CRITICAL_SECTION();							//EXIT_CRITICAL_SECTION()
//					flow =  pad->nFlow[1];
//					EXIT_CRITICAL_SECTION();	
//					SetMbMFCFlow(MB_MFC_HeO2,flow*10);
//					EnableTimer(TX_MFC_TIMEOUT);
//					MFCUpdateFlag[1] = false;
//	//				
//				}
//			}
//			else
//			{
//				MbFlowSetStep++;
//			}
//			break;
//		case 4:
//			if(!ModbusBusy)
//			{
//				ModbusBusy = true;
//				GetMbMFCFlow(MB_MFC_HeO2);
//				EnableTimer(TX_MFC_TIMEOUT);
////				MbFlowSetStep++;
//			}
//			break;
//		case 5:
//			if(MFCUpdateFlag[2])
//			{
//				if(!ModbusBusy)
//				{
//					ModbusBusy = true;
//					ENTER_CRITICAL_SECTION();							//EXIT_CRITICAL_SECTION()
//					flow =  pad->nFlow[2];
//					EXIT_CRITICAL_SECTION();	
//					SetMbMFCFlow(MB_MFC_He,flow*10);
//					EnableTimer(TX_MFC_TIMEOUT);
//					MFCUpdateFlag[2] = false;
//				}
//			}
//			else
//			{
//				MbFlowSetStep++;
//			}
//			break;
//		case 6:
//			if(!ModbusBusy)
//			{
//				ModbusBusy = true;
//				GetMbMFCFlow(MB_MFC_He);
//				EnableTimer(TX_MFC_TIMEOUT);
////				MbFlowSetStep++;
//			}
//			break;
//			case 7:
//			if(MFCUpdateFlag[3])
//			{
//				if(!ModbusBusy)
//				{
//					ModbusBusy = true;
//					ENTER_CRITICAL_SECTION();							//EXIT_CRITICAL_SECTION()
//					flow =  pad->nFlow[3];
//					EXIT_CRITICAL_SECTION();	
//					SetMbMFCFlow(MB_MFC_HePurge,flow*10);
//					EnableTimer(TX_MFC_TIMEOUT);
//					MFCUpdateFlag[3] = false;
//				}
//			}
//			else
//			{
//				MbFlowSetStep++;
//			}
//			break;
//		case 8:
//			if(!ModbusBusy)
//			{
//				ModbusBusy = true;
//				GetMbMFCFlow(MB_MFC_HePurge);
//				EnableTimer(TX_MFC_TIMEOUT);
////				MbFlowSetStep++;
//			}
//			break;
//			case 9:
//			if(MFCUpdateFlag[4])
//			{
//				if(!ModbusBusy)
//				{
//					ModbusBusy = true;
//					ENTER_CRITICAL_SECTION();							//EXIT_CRITICAL_SECTION()
//					flow =  pad->nFlow[4];
//					EXIT_CRITICAL_SECTION();	
//					SetMbMFCFlow(MB_MFC_Sample,flow*10);
//					EnableTimer(TX_MFC_TIMEOUT);
//					MFCUpdateFlag[4] = false;
//				}
//			}
//			else
//			{
//				MbFlowSetStep++;
//			}
//			break;
//		case 10:
//			if(!ModbusBusy)
//			{
//				ModbusBusy = true;
//				GetMbMFCFlow(MB_MFC_Sample);
//				EnableTimer(TX_MFC_TIMEOUT);
////				MbFlowSetStep++;
//			}
//			break;
//		default:
//			MbFlowSetStep = 0;
//			ReadMFCFlowFlag = false;
//			LED0_OFF
//			break;
//	}
//	
//}



/*******************************************************************************
* 函数名: 
* 功  能: 
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/
void ReadMFCCycleTimeoutISR(void)
{
	ReadMFCFlowFlag = true;
//	LED0_TOG
}

/*******************************************************************************
* 函数名: 
* 功  能: 
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/
void SetReadMFCCycleTimer(void)
{
//	SetSTimer(READ_MFC_CYCLE_TIMER,READ_MFC_CYCLE_TIMER_TICKS,APP_TIMER_MODE_REPEATED,ReadMFCCycleTimeoutISR);
//	EnableTimer(READ_MFC_CYCLE_TIMER);
}

/*******************************************************************************
* 函数名: 
* 功  能: 
* 输  入: None
* 输  出: None
* 返回值: None
*******************************************************************************/
void	EnableReadMFCCycleTimer(void)
{
//		EnableTimer(READ_MFC_CYCLE_TIMER);
}

