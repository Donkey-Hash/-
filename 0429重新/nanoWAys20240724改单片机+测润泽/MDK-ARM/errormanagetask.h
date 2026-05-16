#ifndef _ERRORMANAGETASK_H_
#define _ERRORMANAGETASK_H_
#include "stm32f4xx_hal.h"
#include "instrumentinfo.h"
#include "cmsis_os.h"

/* 错误代码联合体 */
typedef enum{
	
	SYS_OK,					// 运行正常
	MEASURE_ERROR,			// 量程选择错误(模块5，进样)
	WORKEMODE_ERROR,		// 工作模式错误(模块5，进样)
	AD_CALIBRATION_ERROR,	// 液位信号异常(模块3：AD液位标定,模块5：进样)
	AD_UPLEVEL_ERROR,		// 上行液位错误(模块5：进样)
	AD_DOWNLEVEL_ERROR,		// 下行液位错误(模块5：进样)
	Reagent1OUT_ERROR,		// 试剂1空错误码(模块：进试剂1)
	Reagent2OUT_ERROR,		// 试剂2空错误码(模块：进试剂2)
	Reagent3OUT_ERROR,		// 试剂3空错误码(模块：进试剂3)
	HOT_TIMEOUT_ERROR,		// 加热时间超时，达不到目标值(模式：加热消解)
	FITLINE_ERROR,			// 线性拟合出错误(模块19：流程结束后计算
	REAGENT1_EMPTY ,		// 试剂1空
	REAGENT2_EMPTY ,		// 试剂2空
	REAGENT3_EMPTY ,		// 试剂3空
	
}nErrorCode;

extern osSemaphoreId_t ErrorCodeSemHandle;

void ErrorCodeManageTask(void *argument);
void SendFunErrorCode(nErrorCode errorcode);	// 发送错误代码
void HandleError(nErrorCode errorcode);			// 处理错误代码

#endif
