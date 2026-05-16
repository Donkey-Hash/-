
#ifndef	__PROCESS_CONTROL_TASK_H__
#define	__PROCESS_CONTROL_TASK_H__

#include "cmsis_os.h"
#include "instrumentinfo.h"

//#define  PROCESS_TEST							//测流程宏定义

//#define COMM_CMD_FLOW_CTRL				0x0001  	//流体控制命令,用于串口进程通知流程处理进程.
//#define COMM_CMD_PROC_HANDLE			0x0002		//流程处理命令,用于串口进程通知流程处理进程.
#define COMM_HOST_CMD							0x0001		//上位机指令
#define COMM_VALVE_OPT						0x0002		//上位机指令



//typedef enum{
//	RROC_NORMAL								= 1,
//	RROC_AUTO_RROC_CALIBRAT_LOW					= 2,	
//	RROC_AUTO_RROC_CALIBRAT_HIGH				= 3,
//	RROC_CALIBRAT_LOW							= 4,
//	RROC_CALIBRAT_HIGH							= 5,
//	PROC_INIT               	= 6,
//	RROC_VERIFY_LOW								= 7,
//	RROC_VERIFY_HIGH								= 8,
//	
//}PROC_CODE;


/* 错误代码枚举类型 */
typedef enum{
	
	STEP_NONE = 0,			// 步骤0
	STEP_EMPTING,			// 排空管路
	STEP_ZEROTEST,			// 零标流程
	STEP_ZEROCALIBRATION,	// 零标校准
	STEP_CIRCULATE,			// 外接泵循环
	STEP_INSAMPLE,			// 进样
	STEP_INREAGENT1,		// 进试剂1
	STEP_HOTING,			// 加热消解
	STEP_INWATER,			// 进蒸馏水
	STEP_INREAGENT2,		// 进试剂2
	STEP_COMPENSATE,		// 浊度补偿
	STEP_INREAGENT3,		// 进试剂3
	STEP_KEEPTEMP,			// 温控50°，300秒
	STEP_CAULCU_V2,			// 计算V2
	STEP_PUMPOUT,			// 排液清洗
	STEP_TEMPCONTAL_V1,		// 出V1值的第一步，温控到50°
	STEP_CAULCU_V1,			// 出V1值第二步
	STEP_CAULCU_ABSOR,		// 计算吸光度
	STEP_CLOSE_ALL,			// 关闭所有外设
	
}Step;

/* 受控命令枚举类型 */
typedef enum{
	
	SS_NOME = 0,			// 空命令
	SS_VAVECTOL,			// 控制阀门
	
}SINGEL_STEP;



#define QUANTITY_TEST_LL1			1							//设备维护操作中,抽取试剂量.
#define QUANTITY_TEST_LL2			3							//设备维护操作中,抽取试剂量.






void ProcessControlTask(void const * argument);
void AnalysisProc(DEVICE_CODE dev,uint8_t range,PROC_CODE procCode,
									uint8_t hotTemp,uint8_t hotTime,uint8_t pumpTime);
//void LiquidFlow(DEVICE_CODE dev,LIQUID_FLOW_MODE mode);
void ProcADtest(DEVICE_CODE dev);
extern uint8_t Check_Reset(void);				// 复位检测（返回1为需要复位）

#endif
