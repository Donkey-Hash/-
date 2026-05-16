
#ifndef	__TEMP_CONTROL_TASK_H__
#define	__TEMP_CONTROL_TASK_H__
#include "stm32f4xx_hal.h"
#include "instrumentinfo.h"
#include "cmsis_os.h"



#define USER_SIGNEL_START_HOT		 	0x0001
#define USER_SIGNEL_START_COOL		 	0x0002

#define TEMPER_COOL_SHRESHOLD				85
#define TIME_CHECK_TEMPER_CTRL			120		//20秒


//#define TEMP_CTRL_CH_POOL				0
#define TEMPER_CHANNEL_2				1

#define MAX_TEMP_CTRL_CHANNEL		1

typedef enum{
	TEMPER_CTRL_HOT = 0,
	TEMPER_CTRL_COOL,
}TEMPER_CTRL_MODE;

typedef enum{
	TEMP_CTRL_CH_POOL = 0,
	TEMP_CTRL_CH_CAVE,
}TEMPER_CTRL_CHANNEL;
typedef enum{
	TEMP_CH_POOL = 0,
	TEMP_CH_CAVE,
	TEMP_CH_CPU,
}TEMPER_CHANNEL;         //需要保证TEMPER_CTRL_CHANNEL是TEMPER_CHANNEL完整子集



typedef struct{
	float 		fTempSV;
	float 		fTempPV;
	uint8_t 	nHotTime;
}TEMP_CONTROL_DATA;

/* 记录温控当前温度状态 */
typedef struct{
	float 			fTempSV;     				//目标温度//单位℃        
	int16_t   	nMaintainTimeSet;				//目标温度保持时长	单位:秒	
	uint16_t		nMaintainTimePast;			//目标温度已经维持时长//单位:秒
	uint16_t		nTimeToSV;					//实现目标温度时间  //单位:秒
	uint16_t		nInitTemp;					//初始温度     //单位:℃
	uint16_t		nMaintainDuty;				//维持目标温度的占空比
	uint16_t		nSVCtrlState;				//温控状态机
	uint32_t		nInitTime;					//计时参考   //单位:毫秒
}TEMP_SV;

/* 记录温控当前控制状态 */
typedef struct{
	uint16_t 		nTempCtrlState;					//温度总控状态机
	uint16_t		nTempStages;        			//当前温控操作需要实现的总温控段
	uint16_t		nCurrentStage;					//当前温控段(当前温控通道)
	uint16_t 		nErrorCode;						//温控错误代码
	uint16_t 		nConstTempTimerFlag;    		//恒温定时器用
	osTimerId   pTimerHandle;						//定时器句柄
	TEMP_SV     tTempCtrlSV[8];						//每个温控段控制参数(一共有8个通道)
}TEMP_CTRL_t;



extern float Temp1SV;
extern float Temp1PV;
extern float Temp2SV;
extern float Temp2PV;


void TempPWMCtrlOut(TEMPER_CTRL_CHANNEL temp_ctrl_ch,uint16_t heat_pluse);
void TempControlTask(void const * argument);
float GetTemperature(TEMPER_CHANNEL temp_ch);
uint8_t ReadTemp(TEMPER_CHANNEL temp_ch);
void StopHotting(TEMPER_CTRL_CHANNEL temp_ctrl_ch);

FAULT_CODE WaitTemperCtrlMsg(uint8_t waitMin);
void StartOneStageTempCtrl(TEMPER_CTRL_CHANNEL temp_ctrl_ch,uint8_t sv1,int16_t sv1_time);
void StopTempCtrl(TEMPER_CTRL_CHANNEL temp_ctrl_ch);
void StartTempControl(bool Proc,uint8_t temp_ch,uint8_t sv,uint8_t holdTime,TEMPER_CTRL_MODE mode);

void OpenCoolFan(void);
void CloseCoolFan(void);
#endif
