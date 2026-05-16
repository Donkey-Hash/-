

#ifndef	__PERISTALTIC_H__
#define	__PERISTALTIC_H__
#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "cmsis_os.h"
#include "valvedef.h"
#include "instrumentinfo.h"

#include <stdio.h>

//#define	RAW_DATA_DEBUG

#define LEVEL_THRESHOLD			9000
#define NOISE_THRESHOLD			256

#define CW 		false				//Clockwise
#define CCW		true				//Counterclockwise
	
#define DRAW_LIQUID				true					//抽取
#define	PUSH_LIQUID				false					//推出
	
#define MAX_PULL_LIQUID_CIRCLE			18      //抽取液体，蠕动泵转动最大圈数
#define MAX_PUSH_LIQUID_CIRCLE			18			//推送液体，蠕动泵转动最大圈数



	

typedef enum{
	LL_CHECK_INIT = 0,
	LL_CHECK_FIND_LEVEL,
	
}LL_CHECK_STATE;

	
#define PUMP_DIE_CW					HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port,MOTOR_DIR_Pin,GPIO_PIN_SET);
#define PUMP_DIE_CCW 				HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port,MOTOR_DIR_Pin,GPIO_PIN_RESET);

#define ENABLE_PUMP_DRIVER  	{	HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port,MOTOR_EN_Pin,GPIO_PIN_RESET);}//HAL_GPIO_WritePin(MOTOR_STDBY_GPIO_Port,MOTOR_STDBY_Pin,GPIO_PIN_RESET);}

#define DISABLE_PUMP_DRIVER	  {	HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port,MOTOR_EN_Pin,GPIO_PIN_SET);}//HAL_GPIO_WritePin(MOTOR_STDBY_GPIO_Port,MOTOR_STDBY_Pin,GPIO_PIN_SET);}


PUMP_LIQUID_ERROR mDrawLiquid(uint16_t maxDelayCicle,int16_t speed,uint16_t ch);
PUMP_LIQUID_ERROR DeterminLevelDown(uint16_t maxDelayCicle,int16_t speed,uint16_t ch);
PUMP_LIQUID_ERROR DeterminLevelUp(uint16_t maxDelayCicle,int16_t speed,uint16_t ch);
DrawLiquidERROR DrawLiquid(DEVICE_CODE dev,FUN_VALVE_CODE valve,uint16_t speed,uint16_t quanty,bool fine);
void PushLiquid(DEVICE_CODE dev,FUN_VALVE_CODE valve,uint16_t speed,uint8_t maxcircle);

DrawLiquidERROR TrsLiquidByTube(	DEVICE_CODE dev,FUN_VALVE_CODE src,FUN_VALVE_CODE des,
																uint16_t speed,uint16_t quanty,bool fine);

void SendAir(DEVICE_CODE dev,FUN_VALVE_CODE valve,uint16_t speed,uint8_t maxcircle);

DrawLiquidERROR PoolEmptying(DEVICE_CODE dev,FUN_VALVE_CODE valve);

FAULT_CODE FillLoop(DEVICE_CODE dev);
FAULT_CODE AddAgentToPool(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine);
FAULT_CODE AddAgentLoopPool(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine);
FAULT_CODE WashPoolWithAgent(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty);


DrawLiquidERROR TrsLiquidByTitration( DEVICE_CODE dev,FUN_VALVE_CODE src,FUN_VALVE_CODE des,
																uint16_t speed,uint16_t quanty,bool fine);
void StartPeristalticPump(int16_t speed,float maxcircle);
void StartPump(bool dir,uint16_t speed,uint8_t maxcircle);
void StopPump(void);
void SetPump(bool dir,uint16_t speed);
FAULT_CODE FillTubeWithAgent(DEVICE_CODE dev,AGENT_CODE agent,uint16_t quanty,bool fine);


bool WashPipe(DEVICE_CODE dev,uint16_t speed,uint16_t quanty);       //20181228
#endif
