
#ifndef _VALVE_DEF__
#define _VALVE_DEF__
#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "relaysdef.h"
#include "port.h"

#define 	MAX_MB_VALVES								64
#define 	MAX_Y_VALVES								32

#define RELAY_NONE              ((uint32_t)0x00000000) 
#define RELAY_1                 ((uint32_t)0x00000001) 
#define RELAY_2                 ((uint32_t)0x00000002)  
#define RELAY_3                 ((uint32_t)0x00000004) 
#define RELAY_4                 ((uint32_t)0x00000008)  
#define RELAY_5                 ((uint32_t)0x00000010)  
#define RELAY_6                 ((uint32_t)0x00000020)  
#define RELAY_7                 ((uint32_t)0x00000040)  
#define RELAY_8                 ((uint32_t)0x00000080)  
#define RELAY_9                 ((uint32_t)0x00000100)  
#define RELAY_10                ((uint32_t)0x00000200)  
#define RELAY_11                ((uint32_t)0x00000400) 
#define RELAY_12                ((uint32_t)0x00000800)  
#define RELAY_13                ((uint32_t)0x00001000)  
#define RELAY_14                ((uint32_t)0x00002000) 
#define RELAY_15                ((uint32_t)0x00004000)  
#define RELAY_16                ((uint32_t)0x00008000)  
#define RELAY_17                ((uint32_t)0x00010000)  
#define RELAY_18                ((uint32_t)0x00020000) 
#define RELAY_19                ((uint32_t)0x00040000)  
#define RELAY_20                ((uint32_t)0x00080000)  
#define RELAY_21         				((uint32_t)0x00100000) 
#define RELAY_22         				((uint32_t)0x00200000)  
#define RELAY_23         				((uint32_t)0x00400000) 
#define RELAY_24         				((uint32_t)0x00800000) 
#define SWITCH1_VALVE_1         ((uint32_t)0x01000000)
#define SWITCH1_VALVE_2         ((uint32_t)0x02000000)
#define SWITCH1_VALVE_3         ((uint32_t)0x04000000)
#define SWITCH1_VALVE_4         ((uint32_t)0x08000000)
#define SWITCH1_VALVE_5         ((uint32_t)0x10000000)
#define SWITCH1_VALVE_6        	((uint32_t)0x20000000)
#define SWITCH1_VALVE_7         ((uint32_t)0x40000000)
#define SWITCH1_VALVE_8         ((uint32_t)0x80000000)

#define RELAY_All             	((uint32_t)0xFFFFFFFF)  

typedef enum{
	VALVE_ERROR = 0,
	VALVE_1 = 1,
	VALVE_2,
	VALVE_3,
	VALVE_4,
	VALVE_5,
	VALVE_6,
	VALVE_7,
	VALVE_8,
	VALVE_9,
	VALVE_10,
	VALVE_11,
	VALVE_12,
	VALVE_13,	
	VALVE_14,
	VALVE_15,
	VALVE_16,
	VALVE_17,
	VALVE_18,
	VALVE_19,
	VALVE_20,	
	VALVE_21,
	VALVE_22,
	VALVE_23,
	VALVE_24,
	VIRTUAL_RELAY_1 = 25,
	VIRTUAL_RELAY_2 = 26,
	VIRTUAL_RELAY_3 = 27,
	VIRTUAL_RELAY_4 = 28,
	VIRTUAL_RELAY_5 = 29,
	VIRTUAL_RELAY_6 = 30,
	VIRTUAL_RELAY_7 = 31,
	VIRTUAL_RELAY_8 = 32,
	POOL_LIGHT								= 45,
//	PERISTALTIC_PUMP_PUSH,
//	PERISTALTIC_PUMP_PULL,
}VALVE_NAME;


extern bool VirtualRelay1;
extern bool VirtualRelay2;
extern bool VirtualRelay3;
extern bool VirtualRelay4;
extern bool VirtualRelay5;
extern bool VirtualRelay6;
extern bool VirtualRelay7;
extern bool VirtualRelay8;
extern bool VirtualRelay9;
extern bool VirtualRelay10;
extern bool *pvrCurrenttest;
extern bool *pvrAutoRang;
extern bool *pvrAmaxOut;
extern bool *pvrAminOut;

void UpdataValveState(UCHAR *pCoilBuf);
void OpenValveX(VALVE_NAME valve);
void CloseValveX(VALVE_NAME valve);
void OpenAllValves(void);
void CloseAllValves(void);

void OpenValves(uint32_t valveReg);
void CloseValves(uint32_t valveReg);

#endif
