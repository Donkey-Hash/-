
#ifndef __SYRINGE_PUMP_H__
#define __SYRINGE_PUMP_H__

#include "valvedef.h"
#include "instrumentinfo.h"

#define 	SYRINGE_VOLUME						2500
#define   STEP_PUMP_RESOLUTION   		12000
#define 	STEPS_PER_MICROLITER			STEP_PUMP_RESOLUTION / SYRINGE_VOLUME



typedef enum {
	SYRINGE_VALVE_3_NON_DISTRI = 1,
	SYRINGE_VALVE_3_DISTRI = 2,
	SYRINGE_VALVE_4_NON_DISTRI = 3,
	SYRINGE_VALVE_4_DISTRI = 4,
	SYRINGE_VALVE_5_NON_DISTRI = 5,
	SYRINGE_VALVE_5_DISTRI = 6,
	SYRINGE_VALVE_6_NON_DISTRI = 7,
	SYRINGE_VALVE_8_NON_DISTRI = 9,
	SYRINGE_VALVE_8_DISTRI = 10,
	SYRINGE_VALVE_12_DISTRI = 11,
	SYRINGE_VALVE_2_DISTRI = 13,
	SYRINGE_VALVE_6_DISTRI = 14,
	SYRINGE_VALVE_2_FAS_SOLENOID = 17,
	
	
	
}SYRINGE_VALVE_TYPE;

typedef enum {
	SYRINGE_VALVE_A = 1,
	SYRINGE_VALVE_B = 2,
	SYRINGE_VALVE_C = 3,
	SYRINGE_VALVE_D = 4,
	SYRINGE_VALVE_E = 5,
	SYRINGE_VALVE_F = 6,
	SYRINGE_VALVE_G = 7,
	
}SYRINGE_VALVE_NAME;

void SetSyringeVolume(uint16_t volume);
void  SetSyringeValveNum(uint8_t valve_NUM);
void SetSyringeSpeed(uint16_t speed);
void InitSyringeValve(void);
void SetSyringeValveSwitch(DEVICE_CODE dev,FUN_VALVE_CODE funCode);
void InitPumpSyringe(void);
void SyringePumpAspirateFull(void);
void SyringePumpAspirate(uint16_t quanty);
void SyringePumpDispense(uint16_t quanty);
void InitSyringeTitrationPumpParam(void);
DrawLiquidERROR TrsLiquidBySyringe( DEVICE_CODE dev,FUN_VALVE_CODE src,FUN_VALVE_CODE des,
																uint16_t speed,uint16_t quanty,bool fine);
bool QuerySyringePumpState(void);
#endif


