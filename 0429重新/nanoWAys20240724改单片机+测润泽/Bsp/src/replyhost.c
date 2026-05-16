#include <stdlib.h>
#include "replyhost.h"
//#include "bspuart3.h"
#include "leddef.h"
#include "FlashUserData.h"


//extern osSemaphoreId HostRepeatSEMHandle;
extern bool HostResponseFlag;
//extern DEVICE_CODE ThisDev;
//extern bool HaveNewCMD;

//extern osMailQId ReplyHostMailId;



//extern HOST_DATA HostData;
//extern osMutexId osCMDdataMutex;

//HOST_DATA HostData;

//uint16_t ADref0,ADabs0,ADref,ADabs,ADll1,ADll2;

uint8_t XorFF(uint8_t* pdata,uint8_t nlen)
{
	uint8_t temp = 0xFF;
	while(nlen--)
		temp ^= (*pdata++);
	return temp;
}


/*******************************************************************************
      ReplyHostSyn,ReplyHost4Bytes,ReplyHost7Bytes是三个最底层串口发送函数。
      所有向上位机发送数据的函数，都是对后两个函数的封装。
     
*******************************************************************************/



/*******************************************************************************
* Function Name  	: ReplyHostSyn
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHost2Data;
//	uint8_t ReplyHost2Data[2] = {0xAA,0x66};
//void ReplyHostSyn(void)
//{
//	ReplyHost2Data.nDataLenth = 2;
//	ReplyHost2Data.nData[0] = 0xAA;
//	ReplyHost2Data.nData[1] = 0x66;

//	if(osMailPut(ReplyHostMailId, &ReplyHost2Data) != osOK) /* Send Mail */  
//		{      
//      LED_WARN_ON
//    }
//		else
//		{
//			LED_TEST_ON	
//		}
//	osRS232Tx(ReplyHost2Data,2);

//	osRS232Tx(ReplyHost2Data,2);		//再发送一次，减少组态屏数据同步失败概率

//}
/*******************************************************************************
* Function Name  	: ReplyHost4Bytes
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:0:成功发送(连续两次发送均收到上位机回复),1:成功两次，失败一次,2:成功两次，失败两次,3:成功一次，失败三次,4:发送失败
*******************************************************************************/
//REPLY_HOST_DATA ReplyHost4Data;
//uint8_t ReplyHost4Bytes(REPLY_HOST_DATA* pReplyHostData,uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4)
//{
//	pReplyHostData->nSendStat = REPLY_HOST_START;
//	pReplyHostData->nDataLenth = 6;
//	pReplyHostData->nData[0] = 0xAA;
//	pReplyHostData->nData[1] = data1;
//	pReplyHostData->nData[2] = data2;
//	pReplyHostData->nData[3] = data3;
//	pReplyHostData->nData[4] = data4;
//	pReplyHostData->nData[5] = 0;
//	pReplyHostData->nData[6] = 0;
//	pReplyHostData->nData[7] = 0;
//	pReplyHostData->nData[5] = XorFF(pReplyHostData->nData+1,4);
//	if(osMailPut(ReplyHostMailId, pReplyHostData) != osOK) /* Send Mail */  
//		{      
//      LED_WARN_ON
//			return 0;
//    }
//		else
//		{
//			LED_TEST_ON	
//			return 1;
//		}
//	
//	
//}

/*************************以下是回复四字节数据的函数，都是对函数*ReplyHost4Bytes的再封装************************************/

/*******************************************************************************
* Function Name  	: ReadFirmVersion
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostVersionData;
uint8_t ReadFirmVersion(void)
{
	
//	ReplyHostVersionData.nReSendCount = 2;
//	ReplyHostVersionData.nType = REPLY_PROC_DATA;
//	ReplyHostVersionData.nTryTimes = 3;
//	return ReplyHost4Bytes(&ReplyHostVersionData,(FIRM_VERSION>>8&0xff),(FIRM_VERSION&0xFF),0,0);
		//如果发送失败(三次发送均没收到上位机回复)，不再发送。
	return 1;
}

/*******************************************************************************
* Function Name  	: ReportFaultInFlowCtr
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostFaultinFlowData;
//uint8_t ReplyFaultInFlowCtr(FAULT_CODE fault)
//{
//	ReplyHostFaultinFlowData.nReSendCount = 2;
//	ReplyHostFaultinFlowData.nType = REPLY_PROC_DATA;
//	ReplyHostFaultinFlowData.nTryTimes = 3;
//	return ReplyHost4Bytes(&ReplyHostFaultinFlowData,2,fault,0,0);
//}
/*******************************************************************************
* Function Name  	: ReportSuccessInFlowCtr
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostSuccInflowData;
//uint8_t ReplySuccessInFlowCtr(void)
//{
//	ReplyHostSuccInflowData.nReSendCount = 2;
//	ReplyHostSuccInflowData.nType = REPLY_PROC_DATA;
//	ReplyHostSuccInflowData.nTryTimes = 3;
//	return ReplyHost4Bytes(&ReplyHostSuccInflowData,1,0,0,0);
//}

/*******************************************************************************
* Function Name  	: ReportFaultInFlowCtr
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostFaultInTempData;
//uint8_t ReplyFaultInTempCtr(FAULT_CODE fault)
//{
//	ReplyHostFaultInTempData.nReSendCount = 2;
//	ReplyHostFaultInTempData.nType = REPLY_PROC_DATA;
//	return ReplyHost4Bytes(&ReplyHostFaultInTempData,0xFE,fault,0,0);
//}
/*******************************************************************************
* Function Name  	: ReportFaultInFlowCtr
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostCtrDoneData;
//uint8_t ReplyCtrDoneInTempCtr(void)
//{
//	ReplyHostCtrDoneData.nReSendCount = 2;
//	ReplyHostCtrDoneData.nType = REPLY_PROC_DATA;
//	ReplyHostCtrDoneData.nTryTimes = 3;
//	return ReplyHost4Bytes(&ReplyHostCtrDoneData,0xFF,0xFF,0,0);
//}
/*******************************************************************************
* Function Name  	: ReportFaultInFlowCtr
* Description    	: 在加热测试控制中，上报加热信息
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostHotDataInCtrData;
//uint8_t ReplyHotDataInTempCtr(uint16_t temp,uint16_t time)
//{
//	ReplyHostHotDataInCtrData.nReSendCount = 1;
//	ReplyHostHotDataInCtrData.nType = REPLY_PROC_DATA;
//	ReplyHostHotDataInCtrData.nTryTimes = 1;
//	return ReplyHost4Bytes(&ReplyHostHotDataInCtrData,(uint8_t)(temp>>8),(uint8_t)(temp),(uint8_t)(time>>8),(uint8_t)(time));
//}

/*************************以下是回复7字节数据的函数，都是对函数*ReplyHost7Bytes的再封装************************************/
/*******************************************************************************
* Function Name  	: ReplyHost7Bytes
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:0:成功发送(连续两次发送均收到上位机回复),1:成功两次，失败一次,2:成功两次，失败两次,3:成功一次，失败三次,4:发送失败
*******************************************************************************/
//REPLY_HOST_DATA ReplyHost7Data;
//uint8_t ReplyHost7Bytes(REPLY_HOST_DATA* pReplyHostData,uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4,uint8_t data5,uint8_t data6,uint8_t data7)
//{
//	pReplyHostData->nSendStat = REPLY_HOST_START;
//	pReplyHostData->nDataLenth = 9;
//	pReplyHostData->nData[0] = 0xAA;
//	pReplyHostData->nData[1] = data1;
//	pReplyHostData->nData[2] = data2;
//	pReplyHostData->nData[3] = data3;
//	pReplyHostData->nData[4] = data4;
//	pReplyHostData->nData[5] = data5;
//	pReplyHostData->nData[6] = data6;
//	pReplyHostData->nData[7] = data7;
//	pReplyHostData->nData[8] = XorFF(pReplyHostData->nData+1,7);
//	if(osMailPut(ReplyHostMailId, pReplyHostData) != osOK) /* Send Mail */  
//		{      
//      LED_WARN_ON
//			AddErrRecord(SEND_MAIL_FALSE,data1);
//			return 0;
//    }
//		else
//		{
//			LED_TEST_ON	

//			return 1;
//		}

//}


/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/

//uint8_t ReplyProcDone[9] = {0xAA,0xFF,0,0,0,0,0,0,0};
//void ProcDone(void)
//{
//	int i;
//	ReplyProcDone[7] = ThisDev;
//	ReplyProcDone[8] = XorFF(ReplyProcDone+1,7);
//	for(i=0;i<20;i++)
//	{
//  osRS232Tx(ReplyProcDone,9);
//	osDelay(750);
//	}
//	osDelay(10000);
//}

//REPLY_HOST_DATA ReplyProcDone;
//uint8_t ReplyProcDone1[9] = {0xAA,0xFF,0,0,0,0,0,0,0};

void ProcDone(void)
{
	// pMbInputData->nStepCode = STEP_PROC_DONE;
//  uint8_t times = 0;	
//	ReplyProcDone.nReSendCount = 1;
//	ReplyProcDone.nType = REPLY_PROC_DONE;
//	ReplyProcDone.nTryTimes = 20;
//	ReplyHost7Bytes(&ReplyProcDone,0xFF,0,0,0,0,0,ThisDev);	
//	while((ReplyProcDone.nSendStat != REPLY_HOST_ACK_OK)  && (times<20))
//	{
//		times++;
//		osDelay(500);
//	}

}
/*******************************************************************************
* Function Name  	: ReplyHotDataInProcess
* Description    	: 在流程中上报加热信息
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostHotData;
//uint8_t ReplyHotDataInProcess(uint16_t temp,uint16_t time)
//{
//	ReplyHostHotData.nReSendCount = 1;
//	ReplyHostHotData.nType = REPLY_PROC_DATA;
//	ReplyHostHotData.nTryTimes = 1;
//	return ReplyHost7Bytes(&ReplyHostHotData,2,(uint8_t)(temp>>8),(uint8_t)(temp),(uint8_t)(time>>8),(uint8_t)(time),0,ThisDev);

//}

/*******************************************************************************
* Function Name  	: ReplyHostProc
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostReactAbsData;
//uint8_t ReplyHostProcReactAbs(uint16_t ref,uint16_t abs)
//{
//	ReplyHostReactAbsData.nReSendCount = 2;
//	ReplyHostReactAbsData.nType = REPLY_PROC_DATA;
//	ReplyHostReactAbsData.nTryTimes = 3;
//	return ReplyHost7Bytes(&ReplyHostReactAbsData,5,(uint8_t)(ref >> 8),(uint8_t)ref,(uint8_t)(abs >> 8),(uint8_t)abs,0,ThisDev);
//	
//}
/*******************************************************************************
* Function Name  	: ReplyHostProc
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostBlankAbsData;
//uint8_t ReplyHostProcBlankAbs(uint16_t ref,uint16_t abs)  							//空白吸光度
//{
//	ReplyHostBlankAbsData.nReSendCount = 2;
//	ReplyHostBlankAbsData.nType = REPLY_PROC_DATA;
//	ReplyHostBlankAbsData.nTryTimes = 3;
//	return ReplyHost7Bytes(&ReplyHostBlankAbsData,4,(uint8_t)(ref >> 8),(uint8_t)ref,(uint8_t)(abs >> 8),(uint8_t)abs,0,ThisDev);
//}
/*******************************************************************************
* Function Name  	: ReplyFaultInProcess
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostFaultData;
uint8_t ReplyFaultInProcess(FAULT_CODE fault)
{
	AddErrRecord(PROC_FAULT,fault);
	pMbInputData->nSysErrorCode = fault;
	return 1;
}

/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostCalculateData;
void Calculation(void)
{
//	float re;
//	if(pMbHoldData->nDevCode == DEV_COD_CR || pMbHoldData->nDevCode == DEV_COD_CL)
//		pMbHoldData->fMeasAbs = pMbHoldData->fReactAbs;	
//	else
//		pMbHoldData->fMeasAbs = pMbHoldData->fReactAbs - pMbHoldData->fBlankAbs;
//	
//	re = (pMbHoldData->fMeasAbs) * pMbHoldData->fCalibKValue + pMbHoldData->fCalibBValue;
//	if(re < 0)
//	{
//		float ran = (float)(rand() & 0x00FF);
//		ran /= 256;
//		re = ran * 0.1;
//	}
//	
//	pMbHoldData->fMeasureResult = re;
////	pMbHoldData->nStepCode = STEP_NOTIFY_CALCULAT_ABS;
}
/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostFitCurveData;
void FittingCurve(void)
{
	pMbHoldData->fCalibKValue = (pMbHoldData->fHighStdConcent - pMbHoldData->fLowStdConcent)
															 / (pMbHoldData->fCalibHighAbs - pMbHoldData->fCalibLowAbs);
	pMbHoldData->fCalibBValue = pMbHoldData->fLowStdConcent - pMbHoldData->fCalibKValue * pMbHoldData->fCalibLowAbs;
	
//	SaveDevOperatPara();
//	SaveUserData();
//	pMbHoldData->nStepCode = STEP_NOTIFY_FIT_CURVE;
}
/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostCalculateLowData;
void CalculatLowAbs(void)
{
//	if(pMbHoldData->nDevCode == DEV_COD_CR || pMbHoldData->nDevCode == DEV_COD_CL)
//		pMbHoldData->fMeasAbs = pMbHoldData->fReactAbs;	
//	else
//		pMbHoldData->fMeasAbs = pMbHoldData->fReactAbs - pMbHoldData->fBlankAbs;
//	
//	pMbHoldData->fCalibLowAbs = pMbHoldData->fMeasAbs;
//	
}
/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostCalculateHightData;
void CalculatHighAbs(void)
{
//	if(pMbHoldData->nDevCode == DEV_COD_CR || pMbHoldData->nDevCode == DEV_COD_CL)
//		pMbHoldData->fMeasAbs = pMbHoldData->fReactAbs;	
//	else
//		pMbHoldData->fMeasAbs = pMbHoldData->fReactAbs - pMbHoldData->fBlankAbs;
//	
//	pMbHoldData->fCalibHighAbs = pMbHoldData->fMeasAbs;
}
/*******************************************************************************
* Function Name  	: ReplyAbsADsInADTest
* Description    	: 在AD测试时使用此函数上报abs的AD值和温度值
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostAbsAdsData;
//uint8_t ReplyAbsADsInADTest(uint16_t ADref,uint16_t ADabs,uint8_t temp)
//{
//	ReplyHostAbsAdsData.nReSendCount = 2;
//	ReplyHostAbsAdsData.nType = REPLY_PROC_DATA;
//	ReplyHostAbsAdsData.nTryTimes = 3;
//	return ReplyHost7Bytes(&ReplyHostAbsAdsData,2,ADref>>8,ADref,ADabs>>8,ADabs,temp,ThisDev);
//}
/*******************************************************************************
* Function Name  	: ReplyAbsADsInADTest
* Description    	: 在AD测试时使用此函数上报abs的AD值和温度值
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
//REPLY_HOST_DATA ReplyHostLLADsData;
//uint8_t ReplyLLADsInADTest(uint16_t ADll1,uint16_t ADll2)
//{
//	ReplyHostLLADsData.nReSendCount = 2;
//	ReplyHostLLADsData.nType = REPLY_PROC_DATA;
//	ReplyHostLLADsData.nTryTimes = 3;
//	return ReplyHost7Bytes(&ReplyHostLLADsData,3,ADll1>>8,ADll1,ADll2>>8,ADll2,0,ThisDev);
//}





