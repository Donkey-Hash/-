
#include "replyhost.h"
#include "peristalticpump.h"
#include "tim.h"
#include "relaysdef.h"
#include "valvedef.h"
#include "ltc1867.h"
#include "usart.h"
#include "mtimers.h"
#include "tlc5620.h"
#include "bspuart3.h"
#include "tempcontroltask.h"
#include "ProcCommon.h"
#include "leddef.h"

extern osSemaphoreId HostRepeatSEMHandle;
extern bool HostResponseFlag;

extern bool HaveNewCMD;

HOST_DATA HostData;

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
void ReplyHostSyn(void)
{
	uint8_t da[2] = {0xAA,0x66};
	LED_TEST_ON
	osRS232Tx(da,2);
	
	osRS232Tx(da,2);		//再发送一次，减少组态屏数据同步失败概率
	LED_TEST_OFF
}
/*******************************************************************************
* Function Name  	: ReplyHost4Bytes
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:0:成功发送(连续两次发送均收到上位机回复),1:成功两次，失败一次,2:成功两次，失败两次,3:成功一次，失败三次,4:发送失败
*******************************************************************************/
uint8_t ReplyHost4Bytes(uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4)
{
	uint8_t re = COMM_3_TX_MAX_TIMES;		//每条数据尝试重发次数。
	uint8_t da[6] = {0xAA,data1,data2,data3,data4,0};
	da[5] = XorFF(da+1,4);
	LED_TEST_ON
	do{
	osRS232Tx(da,6);
	}while(osSemaphoreWait(HostRepeatSEMHandle , 800) != osOK && --re);//收到上位机回应才算成功,一旦成功则不再重发.
	
	if(re > 0)  //如果成功发送，则再发送一次，以此来减少上位机同步出错概率
	{
		osDelay(200);
		do{
		osRS232Tx(da,6);
		}while(osSemaphoreWait(HostRepeatSEMHandle , 800) != osOK && --re);//收到上位机回应才算成功,一旦成功则不再重发.
		LED_TEST_OFF
		return (COMM_3_TX_MAX_TIMES-re);
	}
	else
		return (COMM_3_TX_MAX_TIMES + 1);
	
}
/*******************************************************************************
* Function Name  	: ReplyHost7Bytes
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:0:成功发送(连续两次发送均收到上位机回复),1:成功两次，失败一次,2:成功两次，失败两次,3:成功一次，失败三次,4:发送失败
*******************************************************************************/
uint8_t ReplyHost7Bytes(uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4,uint8_t data5,uint8_t data6)
{
	uint8_t re = COMM_3_TX_MAX_TIMES;
	uint8_t da[9] = {0xAA,data1,data2,data3,data4,data5,data6,0,0};
	da[8] = XorFF(da+1,7);
	LED_TEST_ON
	do
	{		
		osRS232Tx(da,9);
	}while((osSemaphoreWait(HostRepeatSEMHandle , 800) != osOK) && --re);
	if(re > 0)  //如果成功发送，则再发送一次，以此来减少上位机同步出错概率
	{
		osDelay(200);
		do
		{		
			osRS232Tx(da,9);
		}while((osSemaphoreWait(HostRepeatSEMHandle , 800) != osOK) && --re);
		LED_TEST_OFF
		return (COMM_3_TX_MAX_TIMES-re);
	}
	else
		return (COMM_3_TX_MAX_TIMES + 1);
}

/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
void ProcDone(void)
{
	//ReplyHost7Bytes(0xFF,0,0,0,0,0);
//	uint8_t re = COMM_3_TX_MAX_TIMES;
	uint8_t da[9] = {0xAA,0xFF,0,0,0,0,0,0,0};
//	da[8] = XorFF(da+1,7);
//	LED_TEST_ON
	do
	{		
		osRS232Tx(da,9);
	}while((osSemaphoreWait(HostRepeatSEMHandle , 800) != osOK));// && --re);
}
/*************************以下是回复四字节数据的函数，都是对函数*ReplyHost4Bytes的再封装************************************/

/*******************************************************************************
* Function Name  	: ReadFirmVersion
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReadFirmVersion(void)
{
	return ReplyHost4Bytes((FIRM_VERSION>>8&0xff),(FIRM_VERSION&0xFF),0,0);
		//如果发送失败(三次发送均没收到上位机回复)，不再发送。
}

/*******************************************************************************
* Function Name  	: ReportFaultInFlowCtr
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplyFaultInFlowCtr(FAULT_CODE fault)
{
	return ReplyHost4Bytes(2,fault,0,0);
}
/*******************************************************************************
* Function Name  	: ReportSuccessInFlowCtr
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplySuccessInFlowCtr(void)
{
	return ReplyHost4Bytes(1,0,0,0);
}

/*******************************************************************************
* Function Name  	: ReportFaultInFlowCtr
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplyFaultInTempCtr(FAULT_CODE fault)
{
	return ReplyHost4Bytes(0xFE,fault,0,0);
}
/*******************************************************************************
* Function Name  	: ReportFaultInFlowCtr
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplyCtrDoneInTempCtr(void)
{
	return ReplyHost4Bytes(0xFF,0xFF,0,0);
}
/*******************************************************************************
* Function Name  	: ReportFaultInFlowCtr
* Description    	: 在加热测试控制中，上报加热信息
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplyHotDataInTempCtr(uint16_t temp,uint16_t time)
{
	return ReplyHost4Bytes((uint8_t)(temp>>8),(uint8_t)(temp),(uint8_t)(time>>8),(uint8_t)(time));
}

/*************************以下是回复7字节数据的函数，都是对函数*ReplyHost7Bytes的再封装************************************/

/*******************************************************************************
* Function Name  	: ReplyHotDataInProcess
* Description    	: 在流程中上报加热信息
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplyHotDataInProcess(uint16_t temp,uint16_t time)
{
	return ReplyHost7Bytes(2,(uint8_t)(temp>>8),(uint8_t)(temp),(uint8_t)(time>>8),(uint8_t)(time),0);
}

/*******************************************************************************
* Function Name  	: ReplyHostProc
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplyHostProcCalculatAbs(uint16_t ref,uint16_t abs)
{
	return ReplyHost7Bytes(5,(uint8_t)(ref >> 8),(uint8_t)ref,(uint8_t)(abs >> 8),(uint8_t)abs,0);
	
}
/*******************************************************************************
* Function Name  	: ReplyFaultInProcess
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplyFaultInProcess(FAULT_CODE fault)
{
	return ReplyHost7Bytes(0xFE,fault,0,0,0,0);
}

/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t Calculation(void)
{
	return ReplyHost7Bytes(0x06,0,0,0,0,0);
}
/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t FittingCurve(void)
{
	return ReplyHost7Bytes(0x07,0,0,0,0,0);
}
/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t CalculatLowAbs(void)
{
	return ReplyHost7Bytes(0x08,0,0,0,0,0);
	
}
/*******************************************************************************
* Function Name  	: ProcDone
* Description    	: 
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t CalculatHighAbs(void)
{
	return ReplyHost7Bytes(0x09,0,0,0,0,0);
}
/*******************************************************************************
* Function Name  	: ReplyAbsADsInADTest
* Description    	: 在AD测试时使用此函数上报abs的AD值和温度值
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplyAbsADsInADTest(uint16_t ADref,uint16_t ADabs,uint8_t temp)
{
	return ReplyHost7Bytes(2,ADref>>8,ADref,ADabs>>8,ADabs,temp);
}
/*******************************************************************************
* Function Name  	: ReplyAbsADsInADTest
* Description    	: 在AD测试时使用此函数上报abs的AD值和温度值
* Input          	:
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t ReplyLLADsInADTest(uint16_t ADll1,uint16_t ADll2)
{
	return ReplyHost7Bytes(3,ADll1>>8,ADll1,ADll2>>8,ADll2,0);
}

/*******************************************************************************
* Function Name  	: SendStepInfo
* Description    	: 向上位机发送当前要执行的步骤
* Input          	:preStep：当前步骤数，allSteps：本流程的总步骤数，stepName：当前步骤的名称，test：当前流程的工作模式
									
* Output         	:
* Return         	:
*******************************************************************************/
uint8_t SendStepInfo(uint8_t preStep,uint8_t allSteps,STEP_NAME_CODE stepName,CALIB_MODE test)
{
	if(preStep > allSteps)
		preStep = allSteps;
	return ReplyHost7Bytes(0x01,preStep,allSteps,stepName,0,test);
}



