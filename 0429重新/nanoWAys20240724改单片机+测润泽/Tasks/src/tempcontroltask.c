#include "tempcontroltask.h"

#include "processcontroltask.h"
#include "pid.h"
#include "mtimers.h"
#include "replyhost.h"
#include "leddef.h"
// #include "ltc1867.h"
#include "modbusdata.h"
#include "iwdg.h"
#include "FlashUserData.h"

extern osThreadId TempTaskHandle;
extern osMutexId osTempDataMutex;
// extern TIM_HandleTypeDef htim12;
// extern osTimerId HotHoldTimerHandle;
// extern osSemaphoreId HostRepeatSEMHandle;
extern osMessageQId HotMsgHandle;

// extern uint16_t ADref,ADabs;

void LedWarningHotError(void);

// void WaitEventSignal(uint32_t millisec);
static void TemperPidCtrl(TEMPER_CTRL_CHANNEL temp_ctrl_ch, float pv, float sv);
void SendTemperCtrlMsg(FAULT_CODE faultCode);
void HeatingCtrl(void);
void CoolingCtrl(void);

void TempCtrl(TEMPER_CTRL_CHANNEL temp_ctrl_ch, float pv);
void HotHoldTimerCallback(void const *argument);

float Temp1SV = 0; // 设定值
float Temp1PV = 0; // 当前值
uint16_t HotPluse; // 温度控制脉宽

float Temp1PVInit = 0; // 温度控制初始值
PID_t PidData;		   // pid控制参数

bool FlagHeatingCtrl = false;  // 加热控制标志
bool FlagCoolingCtrl = false;  // 冷却控制标志
bool FlagCtrlFromProc = false; // 命令来源,true:来着流程控制,false:来着液体流动命令
bool FlagHottingDone = false;  // 加热目标完成

uint32_t CtrlTimeInit; // 控制时间初始值
uint32_t TimePast;	   // 已消耗时间

// extern uint8_t CountRepeatTX;		//重复发送计数

TEMP_CONTROL_DATA TempControlData; // 温度控制数据安全buffer

// DEVICE_CODE ThisDev = DEV_NONE;

TEMP_CTRL_t TempCtrlData[MAX_TEMP_CTRL_CHANNEL] = {0};
// TEMP_SV* pTempCtrlSV;
// osTimerId HotHoldTimerHandle[MAX_TEMP_CTRL_CHANNEL]; 					//用于温控定时
extern osTimerId HotHoldTimerHandle; // 用于温控定时

/*******************************************************************************
* Function Name  	: TempControlTask
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void TempControlTask(void const *argument)
{
	SetPidParam(&PidData, pMbHoldData->fTemp1Kp, pMbHoldData->fTemp1Ki, pMbHoldData->fTemp1Kd, 0.0); // 初始化PID参数
	SetPidMaxUk(&PidData, pMbHoldData->nTempCtrlMaxUk);	// 设置PID控制器的最大输出值，防止输出值过大
	SetPidUkOffset(&PidData, 0);						// 设置PID控制器的输出偏移量，这里设置为0
	SetTargetValue(&PidData, 0);						// 设置PID控制器的目标值，初始目标温度设置为0
	SetPidIntegral(&PidData, 0.0);						// 将PID控制器的积分项清零
	SetPidfActualValue(&PidData, 0);					// 将PID控制器的实际值清零
	PidBumpless(&PidData);								// 执行无扰动切换，确保PID控制器在启动时不会产生突变（PID微分项的作用）
	TempPWMCtrlOut(TEMP_CTRL_CH_POOL, 0);				// 将温度控制通道TEMP_CTRL_CH_POOL的PWM输出设置为0，即初始时关闭加热或制冷设备

	for (uint8_t i = 0; i < MAX_TEMP_CTRL_CHANNEL; i++)
	{
		//		osTimerDef(HotTimer, HotHoldTimerCallback);
		//		HotHoldTimerHandle[i] = osTimerCreate(osTimer(HotTimer), osTimerOnce, TempCtrlData+i);
		TempCtrlData[i].pTimerHandle = HotHoldTimerHandle;	// 将每个温控通道都设置成同一个软件定时器
	}

	//	SetPulseWidth(&htim4,TIM_CHANNEL_4,0,true);

	for (;;)
	{
		osDelay(100);
		//		LED_RUN_TOGGLE
		Temp1PV = GetTemperature(TEMP_CH_POOL); // 更新当前温度到对应寄存器
		TempCtrl((TEMPER_CTRL_CHANNEL)TEMP_CH_POOL, Temp1PV);
		
		Temp1PV = GetTemperature(TEMP_CH_CAVE); // 更新当前温度到对应寄存器
	}
}

///*******************************************************************************
//* Function Name  	: HeatingCtrl
//* Description    	:
//* Input          	:
//
//* Output         	:
//* Return         	:
//*******************************************************************************/
void TempPWMCtrlOut(TEMPER_CTRL_CHANNEL temp_ctrl_ch, uint16_t heat_pluse)
{
	switch (temp_ctrl_ch)
	{
	case TEMP_CH_POOL:
		pMbInputData->nHotDuty = heat_pluse;
		LL_TIM_OC_SetCompareCH1(TIM2, heat_pluse);
		LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
		LL_TIM_EnableCounter(TIM2);
		break;
	case TEMP_CH_CAVE:

		break;
		//		case TEMP_CH_CPU:

		//		break;
	default:

		break;
	}
}

/*******************************************************************************
* Function Name  	: GetTemperature
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
float GetTemperature(TEMPER_CHANNEL temp_ch)
{
	double temp = 0;
	switch (temp_ch)
	{
	case TEMP_CH_POOL:

		temp = (float)pMbInputData->nPoolTempAD;
		temp = (double)pMbInputData->nPoolTempAD * (double)pMbInputData->nPoolTempAD;
		temp *= 0.0000013668f;
		temp += (double)pMbInputData->nPoolTempAD * 0.0467624383;
		temp += 0.20089;
		pMbHoldData->fPoolTemp = temp;
		//pMbHoldData->fCaveTemp = temp;
		break;
	case TEMP_CH_CAVE:
		//temp = (double)pMbHoldData->nCaveTempAD * 3300 * 1.5;
		//temp /= 4095;
		//temp /= 16;
		//temp -= 50;

		//
		//pMbHoldData->fCaveTemp = temp;
	
		temp = (double)pMbInputData->nCaveTempAD * 3300;
		temp /= 4095;
	
		temp /= 200;	//	转化成ma单位
		//temp = temp * 1.5;
		//temp /= 16;
		//temp -= 50;
	
		pMbHoldData->fCaveTemp = temp;
	
		break;
	case TEMP_CH_CPU:
		temp = (float)pMbInputData->fCPUTemp;
		break;
	default:
		temp = (float)pMbInputData->nPoolTempAD;
		break;
	}

	return temp;
}
/*******************************************************************************
* Function Name  	: HeatingCtrl
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
uint16_t TempSVCtrl(TEMPER_CTRL_CHANNEL temp_ctrl_ch, TEMP_CTRL_t *ptemp_ctrl, float pv)
{
	TEMP_SV *ptemp_sv = ptemp_ctrl->tTempCtrlSV + (ptemp_ctrl->nCurrentStage) - 1;
	switch (ptemp_sv->nSVCtrlState)					// 对当前温度进行判断并加以控制
	{
	case 0:
		break;
	case 1: // 初始化
	{
		ptemp_sv->nInitTemp = pv;					// 记录当前温度
		pMbHoldData->nInitTemp = pv;				// 更新温度寄存器
		ptemp_sv->nInitTime = xTaskGetTickCount();	// 获取计时器参考时间点
		//			pTempCtrlSV->nSVCtrlState = 2;
		if (ptemp_sv->fTempSV <= 25)
			return 1; // 如果设定温度小于室温,不进行温控操作.
		if (ptemp_sv->fTempSV < pv)					// 如果目标温度是否小于当前温度
		{
			//				OpenFunValve(pMbHoldData->nDevCode,VALVE_COOL_FAN);
			OpenCoolFan();							// 开风扇冷却
			ptemp_sv->nSVCtrlState = 3;				// 跳转到步骤3：冷却步骤
			pMbInputData->nTempCtrState = 3;		// 更新pid控制寄存器状态标志位：3-冷却
		}
		else
		{
			ptemp_sv->nSVCtrlState = 2;				// 跳到步骤2：加热步骤
			pMbInputData->nTempCtrState = 2;		// 更新pid控制寄存器状态标志位：2-加热
		}
	}
	break;
	case 2: // 开始升/降温度
		if (pv + 5 >= ptemp_sv->fTempSV)	// 如果升温快要接近目标值了（降低升温速度）
		{
			//				CloseFunValve(ThisDev,VALVE_COOL_FAN);
			CloseCoolFan();
			if (ptemp_sv->nMaintainTimeSet > 0)		// 判断温控时间是否有限制
			{
				ptemp_ctrl->nConstTempTimerFlag = 0;
				osTimerStart((ptemp_ctrl->pTimerHandle), 1000 * ptemp_sv->nMaintainTimeSet); // 启动恒温定时器
				ptemp_sv->nSVCtrlState = 6;													 // 恒温控制
				pMbInputData->nTempCtrState = 6;
				ptemp_sv->nInitTime = xTaskGetTickCount();									// 获取当前时间点作为参考
				TemperPidCtrl(temp_ctrl_ch, pv, ptemp_sv->fTempSV);							// 开始pid迭代
				return 0xFF;
			}
			else if (ptemp_sv->nMaintainTimeSet == 0)
			{
				StopHotting(temp_ctrl_ch);
				return 0x00; // 此阶段温控操作完成
			}
			else									// 
			{
				ptemp_ctrl->nConstTempTimerFlag = 0;
				//					osTimerStart((ptemp_ctrl->pTimerHandle), 1000*ptemp_sv->nMaintainTimeSet);	//启动恒温定时器
				ptemp_sv->nSVCtrlState = 6; // 恒温控制
				pMbInputData->nTempCtrState = 6;
				ptemp_sv->nInitTime = xTaskGetTickCount();
				TemperPidCtrl(temp_ctrl_ch, pv, ptemp_sv->fTempSV);
				return 0xFE;
			}
		}
		TemperPidCtrl(temp_ctrl_ch, pv, ptemp_sv->fTempSV);
		TimePast = xTaskGetTickCount(); // 获取当前时间,计算控制耗时
		TimePast -= ptemp_sv->nInitTime;
		TimePast /= 1000;
		ptemp_sv->nTimeToSV = TimePast;		// 跟新达到目标耗时时间
		pMbHoldData->nTimeToSV = TimePast;	// 更新一次PID温控耗时时间
		if ((TimePast >= TIME_CHECK_TEMPER_CTRL)							// 一段时间后 检测温度是否变化
			&& ((pv - ptemp_sv->nInitTemp) < (ptemp_sv->fTempSV - ptemp_sv->nInitTemp) * 0.2f)) // 判断阈值设定为:目标值与初始值之差的20%
		{
			StopHotting(temp_ctrl_ch);
			pMbInputData->nSysErrorCode = HOT_FAULT;
			return 1; // 加热故障
		}
		//			TemperPidCtrl();
		break;
	case 3: // cool冷却
		if (ptemp_sv->fTempSV >= pv - 0)
		{
			//				CloseFunValve(pMbHoldData->nDevCode,VALVE_COOL_FAN);
			CloseCoolFan();
			if (ptemp_sv->nMaintainTimeSet > 0)		// 如果开启了温控时长限制
			{
				ptemp_ctrl->nConstTempTimerFlag = 0;
				osTimerStart((ptemp_ctrl->pTimerHandle), 1000 * ptemp_sv->nMaintainTimeSet); // 启动恒温定时器
				ptemp_sv->nSVCtrlState = 6;													 // 恒温控制
				pMbInputData->nTempCtrState = 6;
				ptemp_sv->nInitTime = xTaskGetTickCount();						// 获取当前时间点作为参考
				TemperPidCtrl(temp_ctrl_ch, pv, ptemp_sv->fTempSV);
				return 0xFF;
			}
			else if (ptemp_sv->nMaintainTimeSet == 0)
			{
				StopHotting(temp_ctrl_ch);
				return 0x00; // 此阶段温控操作完成
			}
			else
			{
				ptemp_ctrl->nConstTempTimerFlag = 0;
				//					osTimerStart((ptemp_ctrl->pTimerHandle), 1000*ptemp_sv->nMaintainTimeSet);	//启动恒温定时器
				ptemp_sv->nSVCtrlState = 6; // 恒温控制
				pMbInputData->nTempCtrState = 6;
				ptemp_sv->nInitTime = xTaskGetTickCount();
				TemperPidCtrl(temp_ctrl_ch, pv, ptemp_sv->fTempSV);
				return 0xFE;
			}
		}

		TimePast = xTaskGetTickCount(); // 获取当前时间,计算控制耗时
		TimePast -= ptemp_sv->nInitTime;
		TimePast /= 1000;
		ptemp_sv->nTimeToSV = TimePast;
		pMbHoldData->nTimeToSV = TimePast;
		if ((TimePast >= TIME_CHECK_TEMPER_CTRL)												// 一段时间后 检测温度是否变化
			&& ((ptemp_sv->nInitTemp - pv) < (ptemp_sv->nInitTemp - ptemp_sv->fTempSV) * 0.2f)) // 判断阈值设定为:目标值与初始值之差的20%
		{
			//				CloseFunValve(pMbHoldData->nDevCode,VALVE_COOL_FAN);         //关风扇
			CloseCoolFan();
			pMbInputData->nSysErrorCode = FAN_COOL_FAULT;
			return 2; // 风扇故障
		}
		//			else
		//				return 0xFF;
		break;
	case 4:
		break;
	case 5:
		break;
	case 6: // 恒温控制
		TemperPidCtrl(temp_ctrl_ch, pv, ptemp_sv->fTempSV);
		TimePast = xTaskGetTickCount(); // 获取当前时间,计算控制耗时
		TimePast -= ptemp_sv->nInitTime;
		TimePast /= 1000;
		ptemp_sv->nMaintainTimePast = TimePast;
		pMbHoldData->nMaintainTimePast = TimePast;
		if (ptemp_ctrl->nConstTempTimerFlag == 1)	// 如果设置了定时时间的话，时间到了会停止恒温控制
		{
			ptemp_sv->nMaintainDuty = HotPluse;		// 设置维持目标温度的占空比
			pMbHoldData->nMaintainDuty = HotPluse;	// 更新PID温度参数区域的寄存器
			StopHotting(temp_ctrl_ch);
			ptemp_sv->nSVCtrlState = 0;
			pMbInputData->nTempCtrState = 0;
			return 0x00; // 完成温控
		}
		//			else
		//				return 0xFF;     //此段控温没有完成;
		break;
	default:
		break;
	}
	return 0xFF;
}
/*******************************************************************************
* Function Name  	: HeatingCtrl
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void TempCtrl(TEMPER_CTRL_CHANNEL temp_ctrl_ch, float pv)
{
	uint16_t svstate;
	TEMP_CTRL_t *ptemp_ctrl = TempCtrlData + temp_ctrl_ch;	// 指针指向对应通道
	TEMP_SV *ptemp_sv;
	switch (ptemp_ctrl->nTempCtrlState)	// 控制步骤1-8步一步一步来（nTempCtrlState在其他函数调用是会设置为1）
	{
	case 0:	// 没设置就返回
		break;
	case 1:	// 控制第一步：初始化各项参数
	{
		// 设置PID参数到PidData中(Kp, Ki, Kd, 死区)
		SetPidParam(&PidData, pMbHoldData->fTemp1Kp, pMbHoldData->fTemp1Ki, pMbHoldData->fTemp1Kd, 0.0);
		if (ptemp_ctrl->nTempStages > 8 || ptemp_ctrl->nTempStages <= 0) // 总温度段超出范围
		{
			ptemp_ctrl->nTempCtrlState = 0;
			return;
		}
		else if (ptemp_ctrl->nCurrentStage > ptemp_ctrl->nTempStages || ptemp_ctrl->nCurrentStage <= 0) // 一个温度段控制完成
		{
			ptemp_ctrl->nTempCtrlState = 0;
			return;
		}
		else
		{
			// 初始化参数
			ptemp_ctrl->nTempCtrlState = 2;	//跳转到第二步骤
			ptemp_sv = ptemp_ctrl->tTempCtrlSV + (ptemp_ctrl->nCurrentStage) - 1;	
			ptemp_sv->nSVCtrlState = 1; // Inital state
			pMbInputData->nTempCtrState = 1;
		}
		break;
	}
	case 2:
	{
		svstate = TempSVCtrl(temp_ctrl_ch, ptemp_ctrl, pv);
		if (svstate == 0x00) // 完成一个温度控制
		{
			if (++ptemp_ctrl->nCurrentStage <= ptemp_ctrl->nTempStages)
			{
				ptemp_sv = ptemp_ctrl->tTempCtrlSV + (ptemp_ctrl->nCurrentStage) - 1;
				ptemp_sv->nSVCtrlState = 1; // Inital state
				pMbInputData->nTempCtrState = 1;
			}
			else
			{
				ptemp_ctrl->nTempCtrlState = 0; // 温控操作已完成
				SendTemperCtrlMsg(NO_FAULT);
			}
		}
		else if (svstate == 0xFF) // 温段控制没有完成
		{
			return;
		}
		else if (svstate == 0xFE)
		{
			SendTemperCtrlMsg(NO_FAULT); // 已经到达恒温状态，流程函数取消阻塞，温控会继续恒温控制。
		}
		else // 出错
		{
			ptemp_ctrl->nErrorCode = svstate;
			ptemp_ctrl->nTempCtrlState = 0;
		}
		break;
	}
	default:
		break;
	}
}
/*******************************************************************************
* Function Name  	: StartOneStageTempCtrl
* Description    	: 启动温度控制,由其他任务调用
* Input          	:调用流程，温度通道，温度设定值，目标温度保持时间，温控方式（升温、冷却）

* Output         	:
* Return         	:
*******************************************************************************/
void StartOneStageTempCtrl(TEMPER_CTRL_CHANNEL temp_ctrl_ch, uint8_t sv1, int16_t sv1_time)
{
	TEMP_CTRL_t *ptemp_ctrl = TempCtrlData + temp_ctrl_ch;
	ClearPidParam(&PidData);
	PidBumpless(&PidData);
	ptemp_ctrl->tTempCtrlSV[0].fTempSV = sv1;
	ptemp_ctrl->tTempCtrlSV[0].nMaintainTimeSet = sv1_time * 60; // 分钟转换成秒
	ptemp_ctrl->nTempStages = 1;
	ptemp_ctrl->nCurrentStage = 1;
	ptemp_ctrl->nTempCtrlState = 1;
}
/*******************************************************************************
* Function Name  	: StartOneStageTempCtrl
* Description    	: 启动温度控制,由其他任务调用
* Input          	:调用流程，温度通道，温度设定值，目标温度保持时间，温控方式（升温、冷却）

* Output         	:
* Return         	:
*******************************************************************************/
void StopTempCtrl(TEMPER_CTRL_CHANNEL temp_ctrl_ch)
{
	TEMP_CTRL_t *ptemp_ctrl = TempCtrlData + temp_ctrl_ch;
	StopHotting(temp_ctrl_ch);
	ClearPidParam(&PidData);
	ptemp_ctrl->nTempStages = 0;
	ptemp_ctrl->nCurrentStage = 0;
	ptemp_ctrl->nTempCtrlState = 0;
	SendTemperCtrlMsg(NO_FAULT);
}
/*******************************************************************************
* Function Name  	: StartTempControl
* Description    	: 启动温度控制,由其他任务调用
* Input          	:调用流程，温度通道，温度设定值，目标温度保持时间，温控方式（升温、冷却）

* Output         	:
* Return         	:
*******************************************************************************/
// void StartTempControl(bool Proc,uint8_t temp_ch,uint8_t sv,uint8_t holdTime,TEMPER_CTRL_MODE mode)
//{
//	FlagCtrlFromProc = Proc;
//	if(osMutexWait(osTempDataMutex, osWaitForever) == osOK)
//	TempControlData.fTempSV = sv;
//	TempControlData.nHotTime = holdTime;
//	if(osMutexRelease(osTempDataMutex) != osOK){};
//	if(!mode)
//		osThreadFlagsSet(TempTaskHandle,USER_SIGNEL_START_HOT);
//	else
//		osThreadFlagsSet(TempTaskHandle,USER_SIGNEL_START_COOL);

//}

///*******************************************************************************
//* Function Name  	: ReadTemp
//* Description    	:
//* Input          	:
//
//* Output         	:
//* Return         	:
//*******************************************************************************/
// uint8_t ReadTemp(TEMPER_CHANNEL temp_ch)
//{
//	return GetTemperature(temp_ch) + 0.5;
//}

/*******************************************************************************
 * Function Name  	: WaitEventSignal
 * Description    	:
 * Input          	:
 * Output         	:
 * Return         	:
 *******************************************************************************/
// void WaitEventSignal(uint32_t millisec)
//{
//
//}
/*******************************************************************************
* Function Name  	: TemperPidCtrl
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
static void TemperPidCtrl(TEMPER_CTRL_CHANNEL temp_ctrl_ch, float pv, float sv)
{
	if (pv > 250 || pv < 0)					// 如果当前温度不在正常范围
		TempPWMCtrlOut(temp_ctrl_ch, 0);	// 设置占空比为0，关闭温控PWM

	if (PidData.fTargetValue != sv) 		// 如果设定值更新
	{
		PidData.fTargetValue = sv;
		PidBumpless(&PidData); // 去抖动（微分的作用）:更新上一次误差值
	}

	HotPluse = PIDRealize(&PidData, Temp1PV); 	// PID计算控制输出PWM
	if (HotPluse > 60000)						// 限幅
		HotPluse = 60000;					
	TempPWMCtrlOut(temp_ctrl_ch, HotPluse); 	// 设置PWM脉宽
}
/*******************************************************************************
* Function Name  	: HotHoldTimerCallback
* Description    	: 用于恒温定时
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void HotHoldTimerCallback(void const *argument)
{
	//	for(uint8_t i=0;i<MAX_TEMP_CTRL_CHANNEL;i++)
	//	{
	//		if(TempCtrlData[i].pTimerHandle == argument)
	//		{
	TempCtrlData[0].nConstTempTimerFlag = 1;
	//		}
	//	}
}
/*******************************************************************************
* Function Name  	: StopHotting
* Description    	: 停止加热
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void StopHotting(TEMPER_CTRL_CHANNEL temp_ctrl_ch)
{
	TEMP_CTRL_t *ptemp_ctrl = TempCtrlData + temp_ctrl_ch;
	TEMP_SV *ptemp_sv = ptemp_ctrl->tTempCtrlSV + (ptemp_ctrl->nCurrentStage) - 1;
	ptemp_sv->nSVCtrlState = 0; // Inital state
	ptemp_ctrl->nTempCtrlState = 0;
	TempPWMCtrlOut(temp_ctrl_ch, 0);
	//	CloseFunValve(pMbHoldData->nDevCode,VALVE_COOL_FAN);
	CloseCoolFan();
}
/*******************************************************************************
* Function Name  	: WaitTemperCtrlMsg
* Description    	: 用于等待温控结束
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
FAULT_CODE WaitTemperCtrlMsg(uint8_t waitMin)
{
	osStatus_t event;
	event = osSemaphoreAcquire(HotMsgHandle, waitMin * 60000); // 等温度控制信息
	if (event == osOK)
		return NO_FAULT;
	else
		return TEMPER_CTRL_TIMEOUT;
}
/*******************************************************************************
* Function Name  	: SendTemperCtrlMsg
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void SendTemperCtrlMsg(FAULT_CODE faultCode)
{
	osSemaphoreRelease(HotMsgHandle);
}
/*******************************************************************************
* Function Name  	: SendTemperCtrlMsg
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void OpenCoolFan(void)
{
	if (pMbHoldData->nCoolFanValve > 19 || pMbHoldData->nCoolFanValve < 1)
		pMbHoldData->nCoolFanValve = 16;
	OpenValveX(pMbHoldData->nCoolFanValve);
}

/*******************************************************************************
* Function Name  	: SendTemperCtrlMsg
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void CloseCoolFan(void)
{
	if (pMbHoldData->nCoolFanValve > 19 || pMbHoldData->nCoolFanValve < 1)
		pMbHoldData->nCoolFanValve = 16;
	CloseValveX(pMbHoldData->nCoolFanValve);
}
