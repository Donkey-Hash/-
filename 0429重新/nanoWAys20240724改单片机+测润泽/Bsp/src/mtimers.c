#include <stdbool.h>
#include "mtimers.h"
#include "stm32f4xx_ll_tim.h"
#include "peristalticpump.h"
extern osSemaphoreId Comm3RxHandle;
/*******************************************************************************
* Function Name  : 
* Description    :
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//void SetPulseWidth(TIM_HandleTypeDef *htim,uint32_t channel,uint16_t value,bool polarity)
//{
//	TIM_OC_InitTypeDef sConfigOC;

//	sConfigOC.OCMode = TIM_OCMODE_PWM1;
//	sConfigOC.Pulse = value;
//	if(polarity)
//		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
//	else
//		sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
//	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//	HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, channel);
//	if(value == 0)
//		HAL_TIM_PWM_Stop(htim, channel);
//	else
//	HAL_TIM_PWM_Start(htim, channel);   
//}
/*******************************************************************************
* Function Name  : 
* Description    :
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void StopStepMotor(void)
{
	DISABLE_PUMP_DRIVER;
	LL_TIM_CC_DisableChannel(MOTOE_TIMER,MOTOR_CHANNEL);
	LL_TIM_DisableCounter(MOTOE_TIMER);
}
/*******************************************************************************
* Function Name  : 
* Description    :
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void StartStepMotorByFrequency(uint16_t frequency)
{
	LL_TIM_InitTypeDef TIM_InitStruct = {0};
//  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
	
	if(frequency < 30) frequency = 30;
	if(frequency >= 6000)frequency = 6000;
	
  TIM_InitStruct.Prescaler = 69;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;   //Autoreload = (y * 600)/(3200 * speed) //600 0.1rpm - 1rps
  TIM_InitStruct.Autoreload = (225000/frequency);         //600  0.1rpm ---> 3200 (16Ï¸·Ö),1rps  6400£¨32Ï¸·Ö£©
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(MOTOE_TIMER, &TIM_InitStruct);
	SET_MT_CHNL_FUN(MOTOE_TIMER,TIM_InitStruct.Autoreload/2);
	LL_TIM_CC_EnableChannel(MOTOE_TIMER,MOTOR_CHANNEL);
	LL_TIM_EnableCounter(MOTOE_TIMER);
}


void EnableRS232Timers(void)
{		

//	__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
//	__HAL_TIM_SET_COUNTER(&htim7, 0);
//	__HAL_TIM_ENABLE(&htim7);
	
	
	
}

void DisableRS232Timers(void)
{}



