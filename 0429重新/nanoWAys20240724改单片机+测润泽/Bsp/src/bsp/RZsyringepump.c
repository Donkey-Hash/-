
#include <stdbool.h>
#include "usart.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "replyhost.h"
#include "RZsyringepum.h"
extern osSemaphoreId HostMbSEMHandle;   						//主控modbus接收信息用

char RZRxBuff[64]={0};
char RZTxBuff[64]={0};

/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/


bool QueryRZMiniSyringeState(void)
{
	uint16_t sum = 0;
	RZTxBuff[0] = 0xCC;
	sum += RZTxBuff[0];
	RZTxBuff[1] = 0x00;
	sum += RZTxBuff[1];
	RZTxBuff[2] = 0x4A;
	sum += RZTxBuff[2];
	RZTxBuff[3] = 0x00;
	sum += RZTxBuff[3];
	RZTxBuff[4] = 0x00;
	sum += RZTxBuff[4];
	RZTxBuff[5] = 0xDD;
	sum += RZTxBuff[5];
	RZTxBuff[6] = sum;
	RZTxBuff[7] = sum >> 8;

	HAL_UART_Transmit(&huart1,(uint8_t*)RZTxBuff,8,200);
	HAL_UART_Receive_IT(&huart1,(uint8_t*)RZRxBuff,8);
	if(osSemaphoreWait(HostMbSEMHandle , 500) == osOK)
	{
		if(RZRxBuff[2] == 0)
			return false;
		else
			return true;
	}
	else
	{
		UART_EndReceive_IT(&huart1);
		return true;
	}
}
/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
void RZMiniSyringeCW(uint16_t steps)
{
	uint16_t sum = 0;
	RZTxBuff[0] = 0xCC;
	sum += RZTxBuff[0];
	RZTxBuff[1] = 0x00;
	sum += RZTxBuff[1];
	RZTxBuff[2] = 0x42;
	sum += RZTxBuff[2];
	RZTxBuff[3] = steps;
	sum += RZTxBuff[3];
	RZTxBuff[4] = steps >> 8;
	sum += RZTxBuff[4];
	RZTxBuff[5] = 0xDD;
	sum += RZTxBuff[5];
	RZTxBuff[6] = sum;
	RZTxBuff[7] = sum >> 8;

	HAL_UART_Transmit(&huart1,(uint8_t*)RZTxBuff,8,200);
	HAL_UART_Receive_IT(&huart1,(uint8_t*)RZRxBuff,8);
	if(osSemaphoreWait(HostMbSEMHandle , 500) != osOK)
	{
		UART_EndReceive_IT(&huart1);
	}
	bool state;
	do{		
		osDelay(500);
		state = QueryRZMiniSyringeState();
	}while(state);
}
/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
void RZMiniSyringeCCW(uint16_t steps)
{
	uint16_t sum = 0;
	RZTxBuff[0] = 0xCC;
	sum += RZTxBuff[0];
	RZTxBuff[1] = 0x00;
	sum += RZTxBuff[1];
	RZTxBuff[2] = 0x4D;
	sum += RZTxBuff[2];
	RZTxBuff[3] = steps;
	sum += RZTxBuff[3];
	RZTxBuff[4] = steps >> 8;
	sum += RZTxBuff[4];
	RZTxBuff[5] = 0xDD;
	sum += RZTxBuff[5];
	RZTxBuff[6] = sum;
	RZTxBuff[7] = sum >> 8;

	HAL_UART_Transmit(&huart1,(uint8_t*)RZTxBuff,8,200);
	HAL_UART_Receive_IT(&huart1,(uint8_t*)RZRxBuff,8);
	if(osSemaphoreWait(HostMbSEMHandle , 500) != osOK)
	{
		UART_EndReceive_IT(&huart1);
	}
	bool state;
	do{		
		osDelay(500);
		state = QueryRZMiniSyringeState();
	}while(state);
}
/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
void RZMiniSyringeReset(void)
{
	uint16_t sum = 0;
	RZTxBuff[0] = 0xCC;
	sum += RZTxBuff[0];
	RZTxBuff[1] = 0x00;
	sum += RZTxBuff[1];
	RZTxBuff[2] = 0x45;
	sum += RZTxBuff[2];
	RZTxBuff[3] = 0x00;
	sum += RZTxBuff[3];
	RZTxBuff[4] = 0x00;
	sum += RZTxBuff[4];
	RZTxBuff[5] = 0xDD;
	sum += RZTxBuff[5];
	RZTxBuff[6] = sum;
	RZTxBuff[7] = sum >> 8;

	HAL_UART_Transmit(&huart1,(uint8_t*)RZTxBuff,8,200);
	HAL_UART_Receive_IT(&huart1,(uint8_t*)RZRxBuff,8);
	if(osSemaphoreWait(HostMbSEMHandle , 500) != osOK)
	{
		UART_EndReceive_IT(&huart1);
	}
}
/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
void RZMiniSyringeStop(void)
{
	uint16_t sum = 0;
	RZTxBuff[0] = 0xCC;
	sum += RZTxBuff[0];
	RZTxBuff[1] = 0x00;
	sum += RZTxBuff[1];
	RZTxBuff[2] = 0x49;
	sum += RZTxBuff[2];
	RZTxBuff[3] = 0x00;
	sum += RZTxBuff[3];
	RZTxBuff[4] = 0x00;
	sum += RZTxBuff[4];
	RZTxBuff[5] = 0xDD;
	sum += RZTxBuff[5];
	RZTxBuff[6] = sum;
	RZTxBuff[7] = sum >> 8;

	HAL_UART_Transmit(&huart1,(uint8_t*)RZTxBuff,8,200);
	HAL_UART_Receive_IT(&huart1,(uint8_t*)RZRxBuff,8);
	if(osSemaphoreWait(HostMbSEMHandle , 500) != osOK)
	{
		UART_EndReceive_IT(&huart1);
	}
}
/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
void RZMiniSyringeSetSpeed(uint16_t speed)
{
	uint16_t sum = 0;
	RZTxBuff[0] = 0xCC;
	sum += RZTxBuff[0];
	RZTxBuff[1] = 0x00;
	sum += RZTxBuff[1];
	RZTxBuff[2] = 0x4D;
	sum += RZTxBuff[2];
	RZTxBuff[3] = speed;
	sum += RZTxBuff[3];
	RZTxBuff[4] = speed >> 8;
	sum += RZTxBuff[4];
	RZTxBuff[5] = 0xDD;
	sum += RZTxBuff[5];
	RZTxBuff[6] = sum;
	RZTxBuff[7] = sum >> 8;

	HAL_UART_Transmit(&huart1,(uint8_t*)RZTxBuff,8,200);
	HAL_UART_Receive_IT(&huart1,(uint8_t*)RZRxBuff,8);
	if(osSemaphoreWait(HostMbSEMHandle , 500) != osOK)
	{
		UART_EndReceive_IT(&huart1);
	}
}
/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
void RZMiniSyringeAspirate(uint16_t quanty)
{
	float temp = quanty;
	temp *= RZ_MINI_STEP_PUMP_RESOLUTION;
	temp /= RZ_MINI_SYRINGE_VOLUME;
	temp += 0.5;
	RZMiniSyringeCCW((uint16_t)temp);
}
	/*******************************************************************************
* Function Name  : 
* Description    : 
* Input          : 	
* Output         :
* Return         : 
*******************************************************************************/
void RZMiniSyringeDispense(uint16_t quanty)
{
	float temp = quanty;
	temp *= RZ_MINI_STEP_PUMP_RESOLUTION;
	temp /= RZ_MINI_SYRINGE_VOLUME;
	temp += 0.5;
	RZMiniSyringeCW((uint16_t)temp);
}

/*******************************************************************************
* Function Name  : DrawLiquid
* Description    : 抽取液体
* Input          : 	valve:液体对应的电磁阀; quanty:抽取量(1:定量抽1个单位; 3:定量抽3各单位)//单位：uL
                    speed:0.1rpm; fine:是否精确定量 true 精确定量，false 非精确定量
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
DrawLiquidERROR TrsLiquidByRZMiniSyringe( DEVICE_CODE dev,FUN_VALVE_CODE src,FUN_VALVE_CODE des,
																uint16_t speed,uint16_t quanty,bool fine)
{

	uint16_t lq = quanty / RZ_MINI_SYRINGE_VOLUME;						//满管
	uint16_t sq = quanty % RZ_MINI_SYRINGE_VOLUME;						//非满管
	while(lq--)   											
	{
		CloseFunValve(dev,src);
		RZMiniSyringeAspirate(RZ_MINI_SYRINGE_VOLUME);
		OpenFunValve(dev,src);
		RZMiniSyringeDispense(RZ_MINI_SYRINGE_VOLUME);	
	}
	if(sq > 0)
	{
		CloseFunValve(dev,src);
		RZMiniSyringeAspirate(sq);
		OpenFunValve(dev,src);
		RZMiniSyringeDispense(sq);
	}

	return DRAW_OK;
}

