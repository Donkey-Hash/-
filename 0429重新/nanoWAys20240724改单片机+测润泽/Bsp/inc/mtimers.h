
#ifndef  __M_TIMERS_H__
#define  __M_TIMERS_H__
#include <stdbool.h>
//#include "tim.h"
#include "cmsis_os.h"
#define MOTOE_TIMER TIM4
#define MOTOR_CHANNEL LL_TIM_CHANNEL_CH2
#define SET_MT_CHNL_FUN LL_TIM_OC_SetCompareCH2

//void SetPulseWidth(TIM_HandleTypeDef *htim,uint32_t channel,uint16_t value,bool polarity);
void StartStepMotorByFrequency(uint16_t frequency);
void StopStepMotor(void);
void EnableRS232Timers(void);
void DisableRS232Timers(void);
void RS232TimersOutISR(void);


#endif
