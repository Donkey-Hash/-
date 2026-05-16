#include <stdio.h>
#include "syringepump.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

extern FUN_VALVE_RELARS_INFO *pDevsValveRelayInfo[MAX_DEV_NUM];


char SPRxBuff[64]={0};
char SPTxBuff[64]={0};
uint16_t SyringePumpSpeed = 3000;
/*******************************************************************************
* Function Name  : SetSyringeVolume
* Description    : 设置注射泵体积
* Input          : 	
* Output         : None
* Return         : 
*******************************************************************************/
void SetSyringeVolume(uint16_t volume)
{
	int j = 0;
	SPTxBuff[j++] = 0x2F;
	SPTxBuff[j++] = 0x31;
	SPTxBuff[j++] = 0x7A;
	SPTxBuff[j++] = 0x23;
	SPTxBuff[j++] = 0x40;
	SPTxBuff[j++] = 0x32;
	SPTxBuff[j++] = 0x39;
	SPTxBuff[j++] = 0x3D;
	j += sprintf(SPTxBuff + j,"%d",volume);
	SPTxBuff[j++] = 0x0D;
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,7,500);
	do{osDelay(500);
	}while(QuerySyringePumpState());
//	osDelay(1000);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,1000);
}

/*******************************************************************************
* Function Name  : SetSyringeValveNum
* Description    : 设置注射泵阀门类型
* Input          : 	
* Output         : None
* Return         : 
*******************************************************************************/
void  SetSyringeValveType(SYRINGE_VALVE_TYPE valveType)
{
	int j = 0;
	SPTxBuff[j++] = 0x2F;
	SPTxBuff[j++] = 0x31;
	SPTxBuff[j++] = 0x7E;
	SPTxBuff[j++] = 0x56;
	j += sprintf(SPTxBuff + j,"%d",valveType);
	SPTxBuff[j++] = 0x0D;
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,7,500);
	do{osDelay(500);
	}while(QuerySyringePumpState());
}
/*******************************************************************************
* Function Name  : SetSyringeSpeed
* Description    : 设置注射泵抽推速度 
* Input          :	设置速度。单位：uL/min
* Output         : None
* Return         : 
*******************************************************************************/

void SetSyringeSpeed(uint16_t speed)
{
	int j = 0;
	SPTxBuff[j++] = '/';
	SPTxBuff[j++] = '1';
	SPTxBuff[j++] = 'V';
	j += sprintf(SPTxBuff + j,"%d",speed/5);
	SPTxBuff[j++] = '\r';
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,7,500);
	do{osDelay(500);
	}while(QuerySyringePumpState());
//	osDelay(1000);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,1000);
}
/*******************************************************************************
* Function Name  : SetSyringeSpeed
* Description    : //	UINT8 valve_init;//阀门初始化位置 1为A 2为B 3为C
* Input          : 	
* Output         : None
* Return         : 
*******************************************************************************/
void InitSyringeValve(void)
{		
	int j = 0;
	SPTxBuff[j++] = 0x2F;
	SPTxBuff[j++] = 0x31;
	SPTxBuff[j++] = 0x57;
	SPTxBuff[j++] = 0x37;
	SPTxBuff[j++] = 0x52;
	SPTxBuff[j++] = 0x0D;
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
	osDelay(1000);
}
 /*******************************************************************************
* Function Name  : GetSyringeValveNum
* Description    : 通过功能阀代码查找继电器组合
* Input          : dev：设备代码，funCode：功能阀代码
* Output         : 无
* Return         : 继电器组合
*******************************************************************************/
static uint16_t GetSyringeValveNum(DEVICE_CODE dev,FUN_VALVE_CODE funCode)
{
	uint8_t i;
	uint16_t valvenum;
	FUN_VALVE_RELARS_INFO *pa = pDevsValveRelayInfo[dev];
	for(i=0;i<MAX_FUN_VALVE_NUM;i++)
	if(pa[i].nValveFunCode == funCode)
	{
		valvenum = pa[i].nRelays;
		return valvenum;
	}
	return RELAY_NONE;
}
/*******************************************************************************
* Function Name  : SetSyringeSpeed
* Description    : 设置注射泵抽推速度 6000步每秒   一次走60步 5uL  10mS//  extern UINT8 valveNo;//阀门初始化位置 1为A 2为B 3为C
* Input          : 	
* Output         : None
* Return         : 
*******************************************************************************/
void SetSyringeValveSwitch(DEVICE_CODE dev,FUN_VALVE_CODE funCode)
{
	uint8_t valveNum = GetSyringeValveNum(dev,funCode);

	int j = 0;
	SPTxBuff[j++] = ('/');
	SPTxBuff[j++] = ('1');
	SPTxBuff[j++] = ('o');
	SPTxBuff[j++] = (valveNum + 0x30);
	SPTxBuff[j++] = ('R');
	SPTxBuff[j++] = ('\r');
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,7,500);
	do{osDelay(500);
	}while(QuerySyringePumpState());
//	osDelay(1000);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,1000);
}
/*******************************************************************************
* Function Name  : SetSyringeVolume
* Description    : 设置注射泵体积
* Input          : 	
* Output         : None
* Return         : 
*******************************************************************************/
void InitPumpSyringe(void)
{
	int j = 0;
	SPTxBuff[j++] = ('/');
	SPTxBuff[j++] = ('1');  	//1
	SPTxBuff[j++] = ('W');		//W
	SPTxBuff[j++] = ('4');		//4
	SPTxBuff[j++] = ('R');		//R
	SPTxBuff[j++] = ('\r');
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,7,500);
	do{osDelay(500);
	}while(QuerySyringePumpState());
//	osDelay(12000);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,12000);
}
/*******************************************************************************
* Function Name  : SyringePumpAspirateFull
* Description    : 设置注射泵体积//抽满为抽12000步//泵相对位置命令   抽满
* Input          : 	
* Output         : None
* Return         : 
*******************************************************************************/

void SyringePumpAspirateFull(void)   
{	
//	uint32_t delay;
	int j = 0;
	SPTxBuff[j++] = ('/');
	SPTxBuff[j++] = ('1');
	SPTxBuff[j++] = ('A');
	j += sprintf(SPTxBuff + j,"%d",12000);
	SPTxBuff[j++] = ('R');
	SPTxBuff[j++] = ('\r');
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,7,500);
	do{osDelay(500);
	}while(QuerySyringePumpState());
//	delay = SyringePumpSpeed / 60;
//	delay = (12000 * 1000) / delay;
//	osDelay(delay);
//	osDelay(2000);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,);
	
}
/*******************************************************************************
* Function Name  : SyringePumpAspirate
* Description    : 设置注射泵体积//泵抽吸   单次抽5uL  对应60步
* Input          : 	
* Output         : None
* Return         : 
*******************************************************************************/

void SyringePumpAspirate(uint16_t quanty)
{
//	uint32_t delay;
	int j = 0;
	SPTxBuff[j++] = ('/');    ///
	SPTxBuff[j++] = ('1');   	//1
	SPTxBuff[j++] = ('A');
	j += sprintf(SPTxBuff + j,"%d",quanty * STEPS_PER_MICROLITER);
	SPTxBuff[j++] = ('R');
	SPTxBuff[j++] = ('\r');
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,7,500);
	do{osDelay(500);
	}while(QuerySyringePumpState());
}
/*******************************************************************************
* Function Name  : SyringePumpDispense
* Description    : //泵排液   单次排液1uL   对应12步
* Input          : 	
* Output         : None
* Return         : 
*******************************************************************************/

void SyringePumpDispense(uint16_t quanty)
{
	int j = 0;
//	uint32_t delay;
	SPTxBuff[j++] = ('/');
	SPTxBuff[j++] = ('1');
	SPTxBuff[j++] = ('D');
	j += sprintf(SPTxBuff + j,"%d",quanty * STEPS_PER_MICROLITER);
	SPTxBuff[j++] = ('R');
	SPTxBuff[j++] = ('\r');
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,7,500);
	do{osDelay(500);
	}while(QuerySyringePumpState());
}
/*******************************************************************************
* Function Name  : QuerySyringePumpState
* Description    : 查询泵状态
* Input          : 	
* Output         : 
* Return         : buzy：true,idle：false.
*******************************************************************************/

bool QuerySyringePumpState(void)
{
//	uint32_t delay;
	int j = 0;
	SPTxBuff[j++] = ('/');    ///
	SPTxBuff[j++] = ('1');   	//1
	SPTxBuff[j++] = ('?');
	SPTxBuff[j++] = ('\r');
//	HAL_UART_Transmit(&huart6,(uint8_t*)SPTxBuff,j,100);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,15,500);
	if(SPRxBuff[2] == '`')
		return false;
	else if(SPRxBuff[2] == '@')
		return true;
	else
		return true;
	
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,);
}
/*******************************************************************************
* Function Name  : InitSyringeTitrationPumpParam
* Description    : 初始化滴定用注射泵参数
* Input          : 	
* Output         : None
* Return         : 
*******************************************************************************/
void InitSyringeTitrationPumpParam(void)
{
	SetSyringeVolume(SYRINGE_VOLUME);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,1000);
	SetSyringeValveType(SYRINGE_VALVE_6_DISTRI);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,1000);
	SyringePumpSpeed = 6000;
	SetSyringeSpeed(SyringePumpSpeed);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,1000);
	InitPumpSyringe();
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,12000);	
	SyringePumpSpeed = 3000;
	SetSyringeSpeed(SyringePumpSpeed);
//	HAL_UART_Receive(&huart6,(uint8_t*)SPRxBuff,10,1000);
}

/*******************************************************************************
* Function Name  : DrawLiquid
* Description    : 抽取液体
* Input          : 	valve:液体对应的电磁阀; quanty:抽取量(1:定量抽1个单位; 3:定量抽3各单位)//单位：uL
                    speed:0.1rpm; fine:是否精确定量 true 精确定量，false 非精确定量
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
DrawLiquidERROR TrsLiquidBySyringe( DEVICE_CODE dev,FUN_VALVE_CODE src,FUN_VALVE_CODE des,
																uint16_t speed,uint16_t quanty,bool fine)
{

	uint16_t lq = quanty / SYRINGE_VOLUME;						//满管
	uint16_t sq = quanty % SYRINGE_VOLUME;						//非满管
	while(lq--)   											
	{
		SetSyringeValveSwitch(dev,src);
		SyringePumpAspirate(SYRINGE_VOLUME);
		SetSyringeValveSwitch(dev,F_VALVE_POOL_SYRINGE);
		SyringePumpDispense(SYRINGE_VOLUME);	
	}
	SetSyringeValveSwitch(dev,src);
	SyringePumpAspirate(sq);
	SetSyringeValveSwitch(dev,F_VALVE_POOL_SYRINGE);
	SyringePumpDispense(sq);

	return DRAW_OK;
}


