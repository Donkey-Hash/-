
#include "peristalticpump.h"
#include "tim.h"
#include "relaysdef.h"
#include "valvedef.h"
// #include "ltc1867.h"
#include "usart.h"
#include "mtimers.h"
#include "TestLEDCtr.h"
#include "replyhost.h"
#include "leddef.h"
#include "iwdg.h"
#include "modbusdata.h"
// #include "bspuart2.h"
#include "ModbusMSMD.h"
#include "FlashUserData.h"

#define COEF_OVERFLOW 0.4 // 过冲系数 单位：圈
#define SAMPLE_CYCLE 10	  // 采样周期 单位：ms
#define MAX_FULL_AD_LL1 390
#define MAX_FULL_AD_LL2 390
extern osTimerId MotorTimerHandle;
extern float LL1InitCurrent;
extern float LL2InitCurrent;
extern uint16_t LL1ADBlank, LL2ADBlank;
// extern TimerCallbackFunction_t pOSTimerCallback;
bool TimeoutFlag = false; // 定量超时标志
uint16_t Bubbles;		  // 泡泡个数
uint16_t LiqUpDetTimes;	  // 有液体读数的次数
uint16_t LiqLowDetTimes;  // 有液体读数的次数
uint16_t EmptyLLAD;		  // 空管AD值
uint16_t PracticalLLAD;	  // 当前AD值
uint16_t LastLLAD;		  // 上次测量AD值
uint16_t BlankPipeUpTHR;  // 空管阈值上限  空管值百分比*1000
uint16_t BlankPipeLowTHR; // 空管阈值下限  空管值百分比*1000
uint16_t TotalRefTHR;	  // 全反射阈值    空管值百分比*1000
uint8_t LLCheckState;	  // 判断状态机
uint32_t SampleCount = 0; // 数据采样次数
// uint16_t DevADs[500];

/*******************************************************************************
 * Function Name  : PumpTimerout
 * Description    : 泵驱动定时超时处理函数
 * Input          : dir:方向; speed:速度(0.1rpm); maxcircle:蠕动泵转动最大圈数。
 * Output         : None
 * Return         : 对应通道AD值
 *******************************************************************************/
FAULT_CODE ErrorCodeConvert(DrawLiquidERROR dError, AGENT_CODE agent)
{
	switch (dError)
	{
	case DRAW_OK:
		return NO_FAULT;
	case DRAW_TIMEOUT:
		return (FAULT_CODE)agent;
	case DETECT_LEVEL1_FALSE:
		return LL_1_FAULT;
	case DETECT_LEVEL2_FALSE:
		return LL_2_FAULT;
	case FULL_BUBBLES:
		return (FAULT_CODE)agent;
	case ERROR_IMI_SYRINGE:
		return IMI_SYRINGE_PUMP_FAULT;
	case ERROR_RZ_SYRINGE:
		return RZ_SYRINGE_PUMP_FAULT;
	default:
		return NO_FAULT;
	}
}
/*******************************************************************************
 * Function Name  : PumpTimerout
 * Description    : 泵驱动定时超时处理函数
 * Input          : dir:方向; speed:速度(0.1rpm); maxcircle:蠕动泵转动最大圈数。
 * Output         : None
 * Return         : 对应通道AD值
 *******************************************************************************/
void PumpTimerout(void const *argument)
{
	StopStepMotor();
#ifdef MODBUS_MOTOR
	StopMbMotor(HOST_MB_MOTOR);
#endif
	osTimerStop(MotorTimerHandle);
	TimeoutFlag = true;
}
///*******************************************************************************
//* Function Name  : StartPumpWithXms
//* Description    : 启动蠕动泵
//* Input          : dir:方向; speed:速度(0.1rpm); ms:蠕动泵转动时长
//* Output         : None
//* Return         : 对应通道AD值
//*******************************************************************************/
// void StartPumpWithXms(bool dir,uint16_t speed,uint32_t ms)
//{

//	if(dir == CW)
//		{PUMP_DIE_CW;}
//	else if(dir == CCW)
//		{PUMP_DIE_CCW;}
//	TimeoutFlag = false;
////	pOSTimerCallback = (TimerCallbackFunction_t)PumpTimerout;
//	osTimerStart(MotorTimerHandle, ms);
//	StartStepMotorByFrequency(speed);
//
//}
/*******************************************************************************
 * Function Name  : StartPump
 * Description    : 启动蠕动泵
 * Input          : dir:方向; speed:速度(0.1rpm); maxcircle:蠕动泵转动最大圈数。
 * Output         : None
 * Return         : 对应通道AD值
 *******************************************************************************/
void StartPump(bool dir, uint16_t speed, uint8_t maxcircle)
{
	if (maxcircle <= 0)
		return;
	if (dir)
		pMbInputData->nMotorSpeedPV = speed;
	else
		pMbInputData->nMotorSpeedPV = -speed;
	uint32_t timeout = (maxcircle * 600 * 1000) / speed;
	ENABLE_PUMP_DRIVER
	//	HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port,MOTOR_EN_Pin,GPIO_PIN_RESET);
	//	HAL_GPIO_WritePin(MOTOR_STDBY_GPIO_Port,MOTOR_STDBY_Pin,GPIO_PIN_RESET);
	TimeoutFlag = false;
#ifdef MODBUS_MOTOR
	uint16_t sp = speed;
	if (dir == CCW)
		sp = 0 - speed;
	StartMbMotor(HOST_MB_MOTOR, sp);
#else
	if (dir == CW)
	{
		PUMP_DIE_CW;
	}
	else if (dir == CCW)
	{
		PUMP_DIE_CCW;
	}
	StartStepMotorByFrequency(speed);
#endif

	osTimerStart(MotorTimerHandle, timeout);
}
/*******************************************************************************
 * Function Name  : StartPump
 * Description    : 启动蠕动泵
 * Input          : dir:方向; speed:速度(0.1rpm); maxcircle:蠕动泵转动最大圈数。
 * Output         : None
 * Return         : 对应通道AD值
 *******************************************************************************/
void StartPeristalticPump(int16_t speed, float maxcircle)
{
	bool dir;
	if (maxcircle <= 0)
		return;
	pMbInputData->nMotorSpeedPV = speed;
	if (speed > 0)
		dir = true;
	else if (speed < 0)
	{
		dir = false;
		speed = -speed;
	}
	uint32_t timeout = (maxcircle * 600 * 1000) / speed;
	ENABLE_PUMP_DRIVER	// 使能蠕动泵
	//	HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port,MOTOR_EN_Pin,GPIO_PIN_RESET);
	//	HAL_GPIO_WritePin(MOTOR_STDBY_GPIO_Port,MOTOR_STDBY_Pin,GPIO_PIN_RESET);
	TimeoutFlag = false;
#ifdef MODBUS_MOTOR	// 还没有定义
	uint16_t sp = speed;
	if (dir == CCW)
		sp = 0 - speed;
	StartMbMotor(HOST_MB_MOTOR, sp);
#else
	if (dir == CW)	// 设置蠕动泵转动方向(出液或者进液)
	{
		PUMP_DIE_CW;
	}
	else if (dir == CCW)
	{
		PUMP_DIE_CCW;
	}
	StartStepMotorByFrequency(speed);	// 启动蠕动泵对应的定时器输出比较（启动电机频率并开启电机）
#endif

	osTimerStart(MotorTimerHandle, timeout);
}
/*******************************************************************************
 * Function Name  : ChangePumpSpeed
 * Description    : 改变泵速度
 * Input          :  speed:速度(0.1rpm); maxcircle:蠕动泵转动最大圈数。
 * Output         : None
 * Return         : 对应通道AD值
 *******************************************************************************/
void ChangePumpSpeed(bool dir, uint16_t speed)
{
#ifdef MODBUS_MOTOR
	//	uint16_t sp = speed;
	if (dir == CCW)
		speed = 0 - speed;
	ChangeSpeedMbMotor(HOST_MB_MOTOR, speed);
#else
	StartStepMotorByFrequency(speed);
#endif
}
///*******************************************************************************
//* Function Name  : StartPump
//* Description    : 启动蠕动泵
//* Input          : dir:方向; speed:速度(0.1rpm); maxcircle:蠕动泵转动最大圈数。
//* Output         : None
//* Return         : 对应通道AD值
//*******************************************************************************/
// void SetPump(bool dir,uint16_t speed)
//{
////	uint32_t timeout = (maxcircle * 600 * 1000)/speed;
//	if(dir == CW)
//		{PUMP_DIE_CW;}
//	else if(dir == CCW)
//		{PUMP_DIE_CCW;}
//	TimeoutFlag = false;
////	pOSTimerCallback = (TimerCallbackFunction_t)PumpTimerout;
////	osTimerStart(MotorTimerHandle, timeout);
//	StartStepMotorByFrequency(speed);
//}
/*******************************************************************************
 * Function Name  : StartPump
 * Description    : 启动蠕动泵
 * Input          : dir:方向; speed:速度(0.1rpm); Timeout:定时(s)。
 * Output         : None
 * Return         : 对应通道AD值
 *******************************************************************************/
void StopPump(void)
{
	StopStepMotor();
#ifdef MODBUS_MOTOR
	StopMbMotor(HOST_MB_MOTOR);
#endif
	osTimerStop(MotorTimerHandle);
}
/*******************************************************************************
* Function Name  : DrawLiquid
* Description    : 抽取液体
* Input          : 	valve:液体对应的电磁阀; quanty:抽取量(1:定量抽1个单位; 3:定量抽3各单位)
					speed:0.1rpm; fine:是否精确定量 true 精确定量，false 非精确定量
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
bool DrawTestFlag = false;
bool beDetected = false;
long SumOfDetect = 0;
uint16_t pumpSpeed;
PUMP_LIQUID_ERROR mDrawLiquid(uint16_t maxDelayCicle, int16_t speed, uint16_t ch)
{
	//	uint8_t ch;
	uint16_t timeth;
	//	uint16_t maxDelayCicle =  MAX_PULL_LIQUID_CIRCLE;
	pumpSpeed = speed;
	beDetected = false;
	Bubbles = 0;
	LiqUpDetTimes = 0; // 用于某一类型值的计数
	SampleCount = 0;   // 用于采样值的计数
	LLCheckState = 0;  // 状态机
	SumOfDetect = 0;
	//	if(fine)					//如果是精确定量，判断次数为100
	//		timeth = (((float )COEF_OVERFLOW * 1000 * 600) / speed)/SAMPLE_CYCLE;
	//	else
	timeth = 30;
	ch -= 1;
	if (ch == 0)
	{
		//		ch = 0;//LL1_CH;
		EmptyLLAD = LL1ADBlank;
	}
	else
	{
		ch = 1; // LL2_CH;
		EmptyLLAD = LL2ADBlank;
	}
	BlankPipeLowTHR = (EmptyLLAD * pMbHoldData->nBlankPipeLowTHR / 1000); // 空管下限值
	BlankPipeUpTHR = (EmptyLLAD * pMbHoldData->nBlankPipeUpTHR / 1000);	  // 空管上限值
	TotalRefTHR = EmptyLLAD * pMbHoldData->nTotalRefTHR / 1000;			  /// TotalRefTHR

	//	OpenFunValve(dev,valve);													//打开阀门
	osDelay(1000);

	//	TotalRefTHR = (EmptyLLAD/2);											//设置全反射点AD阈值
	do
	{
		osDelay(SAMPLE_CYCLE);							 // 5ms判断一次    //如果AD值的变化超过阈值，接着判断，如果连续两次的值比较平稳则视为检测到液位。
		PracticalLLAD = *(&(pMbInputData->nLL1AD) + ch); // 读取当前值
		SampleCount++;
		switch (LLCheckState)
		{
		case 0:															// 初始化步骤
			StartPeristalticPump(pumpSpeed, (float)maxDelayCicle / 10); // DRAW_LIQUID //50
			LLCheckState = 1;											// 液位判断状态机.进入下一状态
			LiqUpDetTimes = 0;											// 大于空管上限阈值次数计数,用于吸光度大于空管值的判断
			LiqLowDetTimes = 0;											// 小于空管下限阈值次数计数,用于吸光度小于空管值的判断
			SumOfDetect = 0;											// 检测值累加
			break;
		case 1:									 // 等待抽液体上来
			if (PracticalLLAD >= BlankPipeUpTHR) // 透光率大于空管值
			{
				if (LiqLowDetTimes > 0) // 如果测到过小于空管下限阈值,对其计数清零
				{
					LiqLowDetTimes = 0;
					SumOfDetect = 0;
				}
				LiqUpDetTimes++;
				SumOfDetect += PracticalLLAD;
				if (LiqUpDetTimes >= timeth) // 连续timeth次检出满液,判定为有效检出,排除气泡干扰.
				{
					osDelay(50);
					StopPump();
					//							if(fine)                      //如果需要精确定量
					//								LLCheckState = 6;	         //去计算采样均值
					//							else																					//粗定量,定量完成
					{
						beDetected = true;
					}
				}
			}
			else if (PracticalLLAD <= BlankPipeLowTHR)
			{
				LiqLowDetTimes++;
				if (LiqUpDetTimes > 0)
				{
					LiqUpDetTimes = 0;
					SumOfDetect = 0;
				}
				SumOfDetect += PracticalLLAD;
				if (LiqLowDetTimes >= timeth) // 连续timeth次检出满液,判定为有效检出,排除气泡干扰.
				{
					osDelay(50);
					StopPump();
					//							if(fine)
					//								LLCheckState = 6;	         //去计算采样均值
					//							else																					//粗定量,定量完成
					{
						beDetected = true;
					}
				}
			}
			else
			{
				LiqUpDetTimes = 0;
				LiqLowDetTimes = 0;
				SumOfDetect = 0;
			}
			break;

		default:
			break;
		}
		LastLLAD = PracticalLLAD;

	} while (!TimeoutFlag && !beDetected);
	if (TimeoutFlag)
	{
		StopStepMotor(); // 当在空管状态下,超时事件中关闭的步进电机会重新启动后再跳出while循环,此处再次关闭.
		osTimerStop(MotorTimerHandle);
		osDelay(500);
		return PUMP_LIQUID_NO_REAGENT; // 定量失败
	}
	else if (beDetected)
	{
		//		CloseFunValve(dev,valve);
		osDelay(500);
		return PUMP_LIQUID_OK; // 抽取完成
	}
	else
		return PUMP_LIQUID_OK;
}

/*******************************************************************************
* Function Name  : DrawLiquid
* Description    : 抽取液体
* Input          : 	valve:液体对应的电磁阀; quanty:抽取量(1:定量抽1个单位; 3:定量抽3各单位)
					speed:0.1rpm; fine:是否精确定量 true 精确定量，false 非精确定量
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
PUMP_LIQUID_ERROR DeterminLevelDown(uint16_t maxDelayCicle, int16_t speed, uint16_t ch)
{
	uint16_t timeth;
	pumpSpeed = speed;
	beDetected = false;
	Bubbles = 0;
	LiqUpDetTimes = 0; // 用于某一类型值的计数
	SampleCount = 0;   // 用于采样值的计数
	LLCheckState = 0;  // 状态机
	SumOfDetect = 0;
	//	if(fine)					//如果是精确定量，判断次数为100
	//		timeth = (((float )COEF_OVERFLOW * 1000 * 600) / speed)/SAMPLE_CYCLE;
	//	else
	timeth = 30;
	ch -= 1;
	if (ch == 0)
	{
		//		ch = 0;//LL1_CH;
		EmptyLLAD = LL1ADBlank;	//低液位
	}
	else
	{
		ch = 1; // LL2_CH;
		EmptyLLAD = LL2ADBlank;
	}
	BlankPipeLowTHR = (EmptyLLAD * pMbHoldData->nBlankPipeLowTHR / 1000); // 空管下限值
	BlankPipeUpTHR = (EmptyLLAD * pMbHoldData->nBlankPipeUpTHR / 1000);	  // 空管上限值
	TotalRefTHR = EmptyLLAD * pMbHoldData->nTotalRefTHR / 1000;			  /// TotalRefTHR

	//	OpenFunValve(dev,valve);													//打开阀门
	osDelay(1000);
	do
	{
		osDelay(SAMPLE_CYCLE);							 // 5ms判断一次    //如果AD值的变化超过阈值，接着判断，如果连续两次的值比较平稳则视为检测到液位。
		PracticalLLAD = *(&(pMbInputData->nLL1AD) + ch); // 读取当前值
		SampleCount++;
		switch (LLCheckState)
		{
		case 0: // 初始化步骤

			LiqUpDetTimes = 0;
			StartPeristalticPump(pumpSpeed, (float)maxDelayCicle / 10); // 往回推,找液面底
			LLCheckState = 1;
			break;
		case 1:								  // 等待抽液体上来
			if (PracticalLLAD <= TotalRefTHR) // 到达全反射点
			{
				beDetected = true;
				StopPump();
			}
			else if (PracticalLLAD >= BlankPipeLowTHR && PracticalLLAD <= BlankPipeUpTHR) // 检测到空管
			{
				//					LiqUpDetTimes ++;
				//					if(LiqUpDetTimes >= timeth)                           //检测到空管，跳回第一步重新开始
				//					{
				//						StopPump();
				//						maxDelayCicle >>= 1;
				//						Bubbles = 0;
				//						pumpSpeed = speed;
				//						LLCheckState = 0;
				//					}
			}
			else
			{
				LiqUpDetTimes = 0; // 保证空管值是连续检测。
			}
			break;
		case 2: // 切换转速和方向

			break;
		case 3: // 下推找全反射点

			break;
		case 4:

			break;
		case 5: // 下推找界面

			break;
		case 6:

			break;
		default:
			break;
		}
		LastLLAD = PracticalLLAD;

	} while (!TimeoutFlag && !beDetected);
	if (TimeoutFlag)
	{
		StopStepMotor(); // 当在空管状态下,超时事件中关闭的步进电机会重新启动后再跳出while循环,此处再次关闭.
		osTimerStop(MotorTimerHandle);
		osDelay(500);
		return PUMP_LIQUID_DETECT_DOWN_ERROR; // 定量失败
	}
	else if (beDetected)
	{
		//		CloseFunValve(dev,valve);
		osDelay(500);
		return PUMP_LIQUID_OK; // 抽取完成
	}
	else
		return PUMP_LIQUID_OK;
}
/*******************************************************************************
* Function Name  : DrawLiquid
* Description    : 抽取液体
* Input          : 	valve:液体对应的电磁阀; quanty:抽取量(1:定量抽1个单位; 3:定量抽3各单位)
					speed:0.1rpm; fine:是否精确定量 true 精确定量，false 非精确定量
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
PUMP_LIQUID_ERROR DeterminLevelUp(uint16_t maxDelayCicle, int16_t speed, uint16_t ch)
{
	uint16_t timeth;
	pumpSpeed = speed;
	beDetected = false;
	Bubbles = 0;
	LiqUpDetTimes = 0; // 用于某一类型值的计数
	SampleCount = 0;   // 用于采样值的计数
	LLCheckState = 0;  // 状态机
	SumOfDetect = 0;
	//	if(fine)					//如果是精确定量，判断次数为100
	//		timeth = (((float )COEF_OVERFLOW * 1000 * 600) / speed)/SAMPLE_CYCLE;
	//	else
	timeth = 30;
	ch -= 1;
	if (ch == 0)
	{
		//		ch = 0;//LL1_CH;
		EmptyLLAD = LL1ADBlank;
	}
	else
	{
		ch = 1; // LL2_CH;
		EmptyLLAD = LL2ADBlank;
	}
	BlankPipeLowTHR = (EmptyLLAD * pMbHoldData->nBlankPipeLowTHR / 1000); // 空管下限值
	BlankPipeUpTHR = (EmptyLLAD * pMbHoldData->nBlankPipeUpTHR / 1000);	  // 空管上限值
	TotalRefTHR = EmptyLLAD * pMbHoldData->nTotalRefTHR / 1000;			  /// TotalRefTHR

	//	OpenFunValve(dev,valve);													//打开阀门
	osDelay(1000);
	do
	{
		osDelay(SAMPLE_CYCLE);							 // 5ms判断一次    //如果AD值的变化超过阈值，接着判断，如果连续两次的值比较平稳则视为检测到液位。
		PracticalLLAD = *(&(pMbInputData->nLL1AD) + ch); // 读取当前值
		SampleCount++;
		switch (LLCheckState)
		{
		case 0: // 初始化步骤

			LiqUpDetTimes = 0;
			StartPeristalticPump(pumpSpeed, (float)maxDelayCicle / 10); // 往回推,找液面底
			LLCheckState = 1;
			break;
		case 1:																	 // 等待抽液体上来
			if (PracticalLLAD <= TotalRefTHR || PracticalLLAD >= BlankPipeUpTHR) // 到达全反射点
			{
				beDetected = true;
				StopPump();
			}
			else if (PracticalLLAD >= BlankPipeLowTHR && PracticalLLAD <= BlankPipeUpTHR) // 检测到空管
			{
				//					LiqUpDetTimes ++;
				//					if(LiqUpDetTimes >= timeth)                           //检测到空管，跳回第一步重新开始
				//					{
				//						StopPump();
				//						maxDelayCicle >>= 1;
				//						Bubbles = 0;
				//						pumpSpeed = speed;
				//						LLCheckState = 0;
				//					}
			}
			else
			{
				LiqUpDetTimes = 0; // 保证空管值是连续检测。
			}
			break;
		case 2: // 切换转速和方向

			break;
		case 3: // 下推找全反射点

			break;
		case 4:

			break;
		case 5: // 下推找界面

			break;
		case 6:

			break;
		default:
			break;
		}
		LastLLAD = PracticalLLAD;

	} while (!TimeoutFlag && !beDetected);
	if (TimeoutFlag)
	{
		StopStepMotor(); // 当在空管状态下,超时事件中关闭的步进电机会重新启动后再跳出while循环,此处再次关闭.
		osTimerStop(MotorTimerHandle);
		osDelay(500);
		return PUMP_LIQUID_DETECT_UP_ERROR; // 定量失败
	}
	else if (beDetected)
	{
		//		CloseFunValve(dev,valve);
		osDelay(500);
		return PUMP_LIQUID_OK; // 抽取完成
	}
	else
		return PUMP_LIQUID_OK;
}

uint16_t count1, count2, count3, count4;
uint16_t MaxRiseAD, MinRiseAD;
uint16_t ButtonCheckedCount;
float AvgRiseAD;
/*******************************************************************************
* Function Name  : DrawLiquid
* Description    : 抽取液体
* Input          : 	valve:液体对应的电磁阀; quanty:抽取量(1:定量抽1个单位; 3:定量抽3各单位)
					speed:0.1rpm; fine:是否精确定量 true 精确定量，false 非精确定量
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
// bool DrawTestFlag = false;
// bool beDetected = false;
// long SumOfDetect = 0;
// uint16_t pumpSpeed;
DrawLiquidERROR DrawLiquid(DEVICE_CODE dev, FUN_VALVE_CODE valve, uint16_t speed, uint16_t quanty, bool fine)
{
	uint8_t ch;
	uint16_t timeth;
	uint16_t maxDelayCicle = MAX_PULL_LIQUID_CIRCLE;
	pumpSpeed = speed;
	beDetected = false;
	Bubbles = 0;
	LiqUpDetTimes = 0; // 用于某一类型值的计数
	SampleCount = 0;   // 用于采样值的计数
	LLCheckState = 0;  // 状态机
	SumOfDetect = 0;
	if (fine) // 如果是精确定量，判断次数为100
		timeth = (((float)COEF_OVERFLOW * 1000 * 600) / speed) / SAMPLE_CYCLE;
	else
		timeth = 30;
	if (quanty == 1)
	{
		ch = 0; // LL1_CH;
		EmptyLLAD = LL1ADBlank;
	}
	else
	{
		ch = 1; // LL2_CH;
		EmptyLLAD = LL2ADBlank;
	}
	BlankPipeLowTHR = (EmptyLLAD * pMbHoldData->nBlankPipeLowTHR / 1000); // 空管下限值
	BlankPipeUpTHR = (EmptyLLAD * pMbHoldData->nBlankPipeUpTHR / 1000);	  // 空管上限值
	TotalRefTHR = EmptyLLAD * pMbHoldData->nTotalRefTHR / 1000;			  /// TotalRefTHR

	OpenFunValve(dev, valve); // 打开阀门
	osDelay(1000);

	//	TotalRefTHR = (EmptyLLAD/2);											//设置全反射点AD阈值
	do
	{
		osDelay(SAMPLE_CYCLE);							 // 5ms判断一次    //如果AD值的变化超过阈值，接着判断，如果连续两次的值比较平稳则视为检测到液位。
		PracticalLLAD = *(&(pMbInputData->nLL1AD) + ch); // 读取当前值
		SampleCount++;
		switch (LLCheckState)
		{
		case 0:												  // 初始化步骤
			StartPump(DRAW_LIQUID, pumpSpeed, maxDelayCicle); // DRAW_LIQUID //50
			LLCheckState = 1;								  // 液位判断状态机.进入下一状态
			LiqUpDetTimes = 0;								  // 大于空管上限阈值次数计数,用于吸光度大于空管值的判断
			LiqLowDetTimes = 0;								  // 小于空管下限阈值次数计数,用于吸光度小于空管值的判断
			SumOfDetect = 0;								  // 检测值累加
			break;
		case 1:									 // 等待抽液体上来
			if (PracticalLLAD >= BlankPipeUpTHR) // 透光率大于空管值
			{
				if (LiqLowDetTimes > 0) // 如果测到过小于空管下限阈值,对其计数清零
				{
					LiqLowDetTimes = 0;
					SumOfDetect = 0;
				}
				LiqUpDetTimes++;
				SumOfDetect += PracticalLLAD;
				if (LiqUpDetTimes >= timeth) // 连续timeth次检出满液,判定为有效检出,排除气泡干扰.
				{
					osDelay(50);
					StopPump();
					if (fine)			  // 如果需要精确定量
						LLCheckState = 6; // 去计算采样均值
					else				  // 粗定量,定量完成
					{
						beDetected = true;
					}
				}
			}
			else if (PracticalLLAD <= BlankPipeLowTHR)
			{
				LiqLowDetTimes++;
				if (LiqUpDetTimes > 0)
				{
					LiqUpDetTimes = 0;
					SumOfDetect = 0;
				}
				SumOfDetect += PracticalLLAD;
				if (LiqLowDetTimes >= timeth) // 连续timeth次检出满液,判定为有效检出,排除气泡干扰.
				{
					osDelay(50);
					StopPump();
					if (fine)
						LLCheckState = 6; // 去计算采样均值
					else				  // 粗定量,定量完成
					{
						beDetected = true;
					}
				}
			}
			else
			{
				LiqUpDetTimes = 0;
				LiqLowDetTimes = 0;
				SumOfDetect = 0;
			}
			break;
		case 2: // 切换转速和方向
			osDelay(1500);

			if (pumpSpeed < 100)
				pumpSpeed = 10;
			else
				pumpSpeed /= 10;
			LiqUpDetTimes = 0;
			StartPump(PUSH_LIQUID, pumpSpeed, 5); // 往回推,找液面底
			LLCheckState = 3;
			break;
		case 3:								  // 下推找全反射点
			if (PracticalLLAD <= TotalRefTHR) // 到达全反射点
			{
				beDetected = true;
				StopPump();
			}
			else if (PracticalLLAD >= BlankPipeLowTHR && PracticalLLAD <= BlankPipeUpTHR) // 检测到空管
			{
				LiqUpDetTimes++;
				if (LiqUpDetTimes >= timeth) // 检测到空管，跳回第一步重新开始
				{
					StopPump();
					maxDelayCicle >>= 1;
					Bubbles = 0;
					pumpSpeed = speed;
					LLCheckState = 0;
				}
			}
			else
			{
				LiqUpDetTimes = 0; // 保证空管值是连续检测。
			}
			break;
		case 4:
			osDelay(1500);

			if (pumpSpeed < 100)
				pumpSpeed = 10;
			else
				pumpSpeed /= 10;
			LiqUpDetTimes = 0;
			StartPump(PUSH_LIQUID, pumpSpeed, 5); // 往回推,找液面底
			LLCheckState = 5;
			break;
		case 5: // 下推找界面
			if (PracticalLLAD > TotalRefTHR)
			{
				beDetected = true;
				StopPump();
			}
			break;
		case 6:
			PracticalLLAD = SumOfDetect / (LiqUpDetTimes + LiqLowDetTimes);
			if (PracticalLLAD > TotalRefTHR)
				LLCheckState = 2;
			else
				LLCheckState = 4;
			break;
		default:
			break;
		}
		LastLLAD = PracticalLLAD;

	} while (!TimeoutFlag && !beDetected);
	if (TimeoutFlag)
	{
		StopStepMotor(); // 当在空管状态下,超时事件中关闭的步进电机会重新启动后再跳出while循环,此处再次关闭.
		osTimerStop(MotorTimerHandle);
		osDelay(1000);
		CloseFunValve(dev, valve);
		osDelay(500);
		if (LLCheckState == 3)
		{
			if (quanty == 3)
				return DETECT_LEVEL2_FALSE; // 定量失败
			else
				return DETECT_LEVEL1_FALSE; // 定量失败
		}
		return DRAW_TIMEOUT; // 抽取液体超时,返回
	}
	else if (beDetected)
	{
		CloseFunValve(dev, valve);
		osDelay(500);
		return DRAW_OK; // 抽取完成
	}
	else
		return NEVER_HAPPEN;
}
// bool DrawTestFlag = false;
// extern uint16_t ExADCsValue[6];
// bool beDetected = false;
// long SumOfDetect = 0;
// uint16_t pumpSpeed;
//	uint16_t timeth;
// DrawLiquidERROR DrawLiquid(DEVICE_CODE dev,FUN_VALVE_CODE valve,uint16_t speed,uint16_t quanty,bool fine)
//{
//	uint8_t ch;

//	uint16_t maxDelayCicle =  MAX_PULL_LIQUID_CIRCLE;
//	pMbHoldData->nTotalRefTHR = 20;
//	pumpSpeed = speed;
//	beDetected = false;
//	Bubbles = 0;
//	LiqUpDetTimes = 0;   //用于某一类型值的计数
//	SampleCount = 0;     //用于采样值的计数
//	LLCheckState = 0;     //状态机
//	SumOfDetect = 0;
//	if(fine)					//如果是精确定量，判断次数为100
//	{
//		timeth = (((float )COEF_OVERFLOW * 1000 * 600) / speed)/SAMPLE_CYCLE;
//		if(timeth >= 10)
//			Bubbles = timeth - 10;
//		else
//			Bubbles = 0;
//	}
//	else
//		timeth = 30;
//	if(quanty == 1)
//	{
//		ch = 0;//LL1_CH;
//		EmptyLLAD = LL1ADBlank;
//		BlankPipeLowTHR = MAX_FULL_AD_LL1;
//	}
//	else
//	{
//		ch = 1;//LL2_CH;
//		EmptyLLAD = LL2ADBlank;
//		BlankPipeLowTHR = MAX_FULL_AD_LL1;
//	}
//	BlankPipeLowTHR = (EmptyLLAD * pMbHoldData->nBlankPipeLowTHR / 1000);	//65535;//										//空管下限值
//	BlankPipeUpTHR = (EmptyLLAD * pMbHoldData->nBlankPipeUpTHR / 1000);	//65535;//										//空管上限值
//	TotalRefTHR = EmptyLLAD * pMbHoldData->nTotalRefTHR / 1000; ///TotalRefTHR
//
//	OpenFunValve(dev,valve);													//打开阀门
//	osDelay(1000);

////	TotalRefTHR = (EmptyLLAD/2);											//设置全反射点AD阈值
//	do{
//		osDelay(SAMPLE_CYCLE);															//5ms判断一次    //如果AD值的变化超过阈值，接着判断，如果连续两次的值比较平稳则视为检测到液位。
//		PracticalLLAD = *(&(pMbInputData->nLL1AD) + ch);							//读取当前值
//		SampleCount++;
//		switch(LLCheckState)
//		{
//			case 0:     //初始化步骤
//					StartPump(DRAW_LIQUID,pumpSpeed,maxDelayCicle);								//DRAW_LIQUID //50
//					LLCheckState = 1;	    //液位判断状态机.进入下一状态
//					LiqUpDetTimes = 0;		//大于空管上限阈值次数计数,用于吸光度大于空管值的判断
//					LiqLowDetTimes = 0;   //小于空管下限阈值次数计数,用于吸光度小于空管值的判断
//					SumOfDetect = 0;      //检测值累加
//				break;
//			case 1://等待抽液体上来
//				if(PracticalLLAD >= BlankPipeUpTHR )   //透光率大于空管值
//					{
//						if(LiqLowDetTimes > 0)       //如果测到过小于空管下限阈值,对其计数清零
//						{
//							LiqLowDetTimes = 0;
//							SumOfDetect = 0;
//						}
//						LiqUpDetTimes ++;
//						SumOfDetect += PracticalLLAD;
//						if(LiqUpDetTimes >= timeth)             				//连续timeth次检出满液,判定为有效检出,排除气泡干扰.
//						{
//							osDelay(50);
//							StopPump();
//							if(fine)                      //如果需要精确定量
//								LLCheckState = 4;	         //去计算采样均值
//							else																					//粗定量,定量完成
//							{
//								beDetected = true;
//							}
//						}
//					}
//					else if(PracticalLLAD <= BlankPipeLowTHR)
//					{
//						LiqLowDetTimes ++;
//						if(LiqUpDetTimes > 0)
//						{
//							LiqUpDetTimes = 0;
//							SumOfDetect = 0;
//						}
//						if(LiqLowDetTimes >= Bubbles)
//							SumOfDetect += PracticalLLAD;
//						else
//							SumOfDetect = 0;
//						if(LiqLowDetTimes >= timeth)             				//连续timeth次检出满液,判定为有效检出,排除气泡干扰.
//						{
//							osDelay(50);
//							StopPump();
//							if(fine)
//								LLCheckState = 6;	         //去计算采样均值
//							else																					//粗定量,定量完成
//							{
//								beDetected = true;
//							}
//						}
//					}
//					else
//					{
//						LiqUpDetTimes = 0;
//						LiqLowDetTimes = 0;
//						SumOfDetect = 0;
//					}
//				break;
//			case 2:	//切换转速和方向
//				osDelay(1500);
//
//				if(pumpSpeed<100)
//					pumpSpeed = 10;
//				else
//					pumpSpeed /= 10;
//				LiqUpDetTimes = 0;
//				StartPump(PUSH_LIQUID,pumpSpeed,5);								//往回推,找液面底
//				LLCheckState = 3;
//				break;
//			case 3:	//下推找全反射点
//				if(PracticalLLAD <= TotalRefTHR)   //到达全反射点
//				{
//					beDetected = true;
//					StopPump();
//				}
//				else if(PracticalLLAD >= BlankPipeLowTHR && PracticalLLAD <= BlankPipeUpTHR)    //检测到空管
//				{
//					LiqUpDetTimes ++;
//					if(LiqUpDetTimes >= timeth)                           //检测到空管，跳回第一步重新开始
//					{
//						StopPump();
//						maxDelayCicle >>= 1;
//						Bubbles = 0;
//						pumpSpeed = speed;
//						LLCheckState = 0;
//					}
//				}
//				else
//				{
//					LiqUpDetTimes = 0;   //保证空管值是连续检测。
//				}
//				break;
//			case 4:
//				osDelay(500);
//
//				if(pumpSpeed<100)
//					pumpSpeed = 10;
//				else
//					pumpSpeed /= 10;
//				LiqUpDetTimes = 0;
//				StartPump(PUSH_LIQUID,pumpSpeed,5);								//往回推,找液面底
//				LLCheckState = 5;
//				break;
//			case 5://下推找界面
//				if(PracticalLLAD > TotalRefTHR)
//				{
//					beDetected = true;
//					StopPump();
//				}
//				break;
//			case 6:
//				osDelay(500);
//				if(pumpSpeed<400)
//					pumpSpeed = 20;
//				else
//					pumpSpeed /= 20;
//				TotalRefTHR  = SumOfDetect / 10;  		//全反射点阈值以满管试剂值为基础
//				TotalRefTHR /= 3;            											//全反射点阈值为满管试剂值的1/2
//				StartPump(PUSH_LIQUID,pumpSpeed,5);								//往回推,找液面底
//				LLCheckState = 7;
//				break;
//			case 7:
//				if(PracticalLLAD > BlankPipeLowTHR) //找到空管
//				{
//					StopPump();
//					beDetected = true;
//				}
//				else if(PracticalLLAD < TotalRefTHR)//找到全反射点
//				{
////					StopPump();
////					beDetected = true;
//				}
//				break;
//			default:
//				break;
//		}
//		LastLLAD = PracticalLLAD;

//	}while(!TimeoutFlag && !beDetected);
//	if(TimeoutFlag)
//	{
//		StopStepMotor();                   //当在空管状态下,超时事件中关闭的步进电机会重新启动后再跳出while循环,此处再次关闭.
//		osTimerStop(MotorTimerHandle);
////		osDelay(50);
//		CloseFunValve(dev,valve);
//		osDelay(500);
//		if(LLCheckState == 3)
//		{
//			if(quanty == 3)
//				return DETECT_LEVEL2_FALSE;											//定量失败
//			else
//				return DETECT_LEVEL1_FALSE;											//定量失败
//		}
//		return DRAW_TIMEOUT;											//抽取液体超时,返回
//	}
//	else if(beDetected)
//	{
//		CloseFunValve(dev,valve);
//		osDelay(500);
//		return DRAW_OK;														//抽取完成
//	}
//	else
//		return NEVER_HAPPEN;
//
//
//
//
//}
uint32_t sumAD = 0;
bool WashPipe(DEVICE_CODE dev, uint16_t speed, uint16_t quanty)
{
	//	uint8_t ch;
	//	uint16_t timeth;

	//	sumAD = 0;
	//	pumpSpeed = speed;
	//	SampleCount = 0;     //用于采样值的计数
	//	LLCheckState = 0;     //状态机
	//	timeth = 0;	          //冲洗次数
	//	if(quanty == 1)
	//	{
	//		ch = 0;//LL1_CH;
	//		EmptyLLAD = LL1ADBlank;
	//	}
	//	else
	//	{
	//		ch = 1;//LL2_CH;
	//		EmptyLLAD = LL2ADBlank;
	//	}
	//	BlankPipeLowTHR = (EmptyLLAD * pMbHoldData->nBlankPipeLowTHR / 1000);											//设置全反射点AD阈值
	//	BlankPipeUpTHR = (EmptyLLAD * pMbHoldData->nBlankPipeUpTHR / 1000);											//设置全反射点AD阈值
	//	TotalRefTHR = EmptyLLAD * pMbHoldData->nTotalRefTHR / 1000; ///TotalRefTHR
	//	do{
	//		osDelay(5);															//5ms判断一次    //如果AD值的变化超过阈值，接着判断，如果连续两次的值比较平稳则视为检测到液位。
	//		PracticalLLAD = *(&(pMbInputData->nLL1AD) + ch);							//读取当前值
	//		SampleCount++;
	//		switch(LLCheckState)
	//		{
	//			case 0:
	//					sumAD += PracticalLLAD;							//读取100次空管值
	//					if(SampleCount >= 100)
	//					{
	//						sumAD += 50;
	//						sumAD /= 100;
	//						SampleCount = 0;
	//						if(sumAD > BlankPipeLowTHR && sumAD < BlankPipeUpTHR)
	//						{
	//							return true;                                             //空管值在阈值范围内,冲洗完成.
	//						}
	//						else
	//						{
	//							if(timeth >= 3)
	//								return false;                                           //冲洗三次,仍不能满足标准,冲洗失败.
	//							OpenFunValve(dev,F_VALVE_DISTILLED_WATER);													//打开纯水阀
	//							osDelay(500);
	//							StartPump(DRAW_LIQUID,pumpSpeed,10);          						//抽
	//							LLCheckState = 1;
	//							LiqUpDetTimes = 0;
	//							LiqLowDetTimes = 0;
	//						}
	//					}
	//					break;
	//			case 1:
	//				if(TimeoutFlag)																										//等待抽纯水结束
	//					{
	//						CloseFunValve(dev,F_VALVE_DISTILLED_WATER);
	//						OpenFunValve(dev,F_VALVE_CLEAN_WASTE);													//开清洗废液阀
	//						osDelay(500);
	//						StartPump(PUSH_LIQUID,speed,15);
	//						LLCheckState = 2;
	//					}
	//					break;
	//			case 2:
	//				if(TimeoutFlag)
	//					{
	//						CloseFunValve(dev,F_VALVE_CLEAN_WASTE);
	//						osDelay(500);
	//						timeth++;																											//完成一次冲洗
	//						sumAD = 0;
	//						SampleCount = 0;
	//						LLCheckState = 0;
	//					}
	//				break;
	//			case 3:
	//				break;
	//			default:
	//				break;
	//		}
	//	}while(1);
	return true;
}

/*******************************************************************************
 * Function Name  : PushLiquid
 * Description    : 推送液体
 * Input          : valve:液体目的地对应的电磁阀;
 * Output         : None
 * Return         : 对应通道AD值
 *******************************************************************************/
void PushLiquid(DEVICE_CODE dev, FUN_VALVE_CODE valve, uint16_t speed, uint8_t maxcircle)
{
	OpenFunValve(dev, valve);
	StartPump(PUSH_LIQUID, speed, maxcircle);
	while (!TimeoutFlag)
		osDelay(50);
	osDelay(500);
	CloseFunValve(dev, valve);
}
/*******************************************************************************
* Function Name  : FillTubeWithAgent
* Description    : 向比色池加试剂
* Input          : 	src:试剂对应的电磁阀; speed:加试剂速度；
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
FAULT_CODE FillTubeWithAgent(DEVICE_CODE dev, AGENT_CODE agent, uint16_t quanty, bool fine)
{

	DrawLiquidERROR ret = DRAW_OK;
	uint16_t speed = PumpAgentSpeed[agent];

	ret = DrawLiquid(dev, (FUN_VALVE_CODE)agent, speed, quanty, fine);
	if (DRAW_OK != ret)
	{
		SendAir(dev, VALVE_WASTE_LIQUID, SPEED_AIR_TUBE, MAX_PULL_LIQUID_CIRCLE / 2); // 清空定量管
		ret = DrawLiquid(dev, (FUN_VALVE_CODE)agent, speed, quanty, fine);
		if ((ret == DETECT_LEVEL1_FALSE) || (ret == DETECT_LEVEL2_FALSE))
			ret = DRAW_TIMEOUT;
		if (DRAW_OK != ret)
		{
			//			SendString("再次抽取失败！\r\n\0");
			osDelay(300);
		}
	}
	return ErrorCodeConvert(ret, agent);
}
/*******************************************************************************
* Function Name  : DrawLiquid
* Description    : 抽取液体
* Input          : 	valve:液体对应的电磁阀; quanty:抽取量(1:定量抽1个单位; 3:定量抽3各单位)//单位：uL
					speed:0.1rpm; fine:是否精确定量 true 精确定量，false 非精确定量
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
DrawLiquidERROR TrsLiquidByTube(DEVICE_CODE dev, FUN_VALVE_CODE src, FUN_VALVE_CODE des,
								uint16_t speed, uint16_t quanty, bool fine)
{
	DrawLiquidERROR ret = DRAW_OK;
	//	quanty /= 1000;
	uint8_t lq = quanty / 3; // 高液位需要定量的次数
	uint8_t sq = quanty % 3; // 低液位需要定量的次数
	while (sq--)			 // 先抽取小液位定量
	{
		if (false == WashPipe(dev, SPEED_REACTION_POOL_SOLUTION, 1))
			ReplyFaultInProcess(LL1_WASH_FAULT);
		ret = DrawLiquid(dev, src, speed, 1, fine);
		if (DRAW_OK == ret)
		{
			osDelay(1000);
			PushLiquid(dev, des, speed, MAX_PUSH_LIQUID_CIRCLE);
		}
		else // if(DRAW_OK != ret)
		{
			SendAir(dev, VALVE_WASTE_LIQUID, SPEED_AIR_TUBE, MAX_PULL_LIQUID_CIRCLE / 2); // 清空定量管
			ret = DrawLiquid(dev, src, speed, 1, fine);
			if (DRAW_OK == ret)
			{
				osDelay(1000);
				PushLiquid(dev, des, speed, MAX_PUSH_LIQUID_CIRCLE);
			}
			else // 第二次抽取,无论什么错误,直接返回
			{
				return ret;
			}
		}
	}
	while (lq--)
	{
		if (false == WashPipe(dev, SPEED_REACTION_POOL_SOLUTION, 3))
			ReplyFaultInProcess(LL2_WASH_FAULT);
		ret = DrawLiquid(dev, src, speed, 3, fine);
		if (DRAW_OK == ret)
		{
			osDelay(1000);
			PushLiquid(dev, des, speed, MAX_PUSH_LIQUID_CIRCLE);
		}
		else // if(DRAW_OK != ret)
		{
			SendAir(dev, VALVE_WASTE_LIQUID, SPEED_AIR_TUBE, MAX_PULL_LIQUID_CIRCLE / 2); // 清空定量管
			ret = DrawLiquid(dev, src, speed, 3, fine);
			if (DRAW_OK == ret)
			{
				osDelay(1000);
				PushLiquid(dev, des, speed, MAX_PUSH_LIQUID_CIRCLE);
			}
			else
			{
				return ret;
			}
		}
	}
	return ret;
}
/*******************************************************************************
* Function Name  	: SendAir
* Description    	:
* Input          	:

* Output         	:
* Return         	:
*******************************************************************************/
void SendAir(DEVICE_CODE dev, FUN_VALVE_CODE valve, uint16_t speed, uint8_t maxcircle)
{
	PushLiquid(dev, valve, speed, maxcircle);
}

/*******************************************************************************
* Function Name  : PoolEmptying
* Description    : 向比色池加试剂
* Input          : 	dev:设备代码; des:排废液功能阀代码
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
DrawLiquidERROR PoolEmptying(DEVICE_CODE dev, FUN_VALVE_CODE des)
{
	DrawLiquidERROR ret = DRAW_OK;
	uint8_t times = 0;

	while (ret == DRAW_OK)
	{
		ret = TrsLiquidByTube(dev, F_VALVE_POOL_ONLY, des, SPEED_REACTION_POOL_SOLUTION, 3, false);
		if (++times >= 15)
		{
			ReplyFaultInProcess(DRAIN_WASTE_FAULT);
			return CANNOT_FINISHED;
		}
	}
	SendAir(dev, des, SPEED_REACTION_POOL_SOLUTION, MAX_PUSH_LIQUID_CIRCLE);
	return ret;
}
/*******************************************************************************
* Function Name  : FillLoop
* Description    : 注满定量环。从反应池中抽试剂，用液位1定量，粗定量，定量完成后
										将定量管中的试剂排到清洗废液桶，然后排空反应池。调用此函数前
										确保反应池有足够的试剂。
* Input          : 	dev:设备代码; des:排废液功能阀代码
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
FAULT_CODE FillLoop(DEVICE_CODE dev)
{
	DrawLiquidERROR ret = DRAW_OK;
	FAULT_CODE fault;
	ret = TrsLiquidByTube(dev, F_VALVE_LOOP_ONLINE_POOL, F_VALVE_CLEAN_WASTE, SPEED_LOOP_POOL, 1, false);
	ret = TrsLiquidByTube(dev, F_VALVE_LOOP_ONLINE_POOL, F_VALVE_CLEAN_WASTE, SPEED_LOOP_POOL, 1, false);
	if (ret == DRAW_OK)
	{

		PoolEmptying(dev, F_VALVE_CLEAN_WASTE);
	}
	fault = ErrorCodeConvert(ret, AGENT_REACTION_POOL_SOLUTION);
	return fault;
}
/*******************************************************************************
* Function Name  : AddAgentToPoolThrough
* Description    : 向比色池加试剂
* Input          : 	src:试剂对应的电磁阀; speed:加试剂速度；
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
FAULT_CODE AddAgentToPoolThrough(DEVICE_CODE dev, AGENT_CODE agent, FUN_VALVE_CODE throughvalve, uint16_t quanty, bool fine)
{
	DrawLiquidERROR ret = DRAW_OK;
	FAULT_CODE fault;
	uint16_t speed = PumpAgentSpeed[agent];
	TRANS_LIQUID_FUN fun;
	fun = GetLiquidFun(dev, (FUN_VALVE_CODE)agent);
	if (fun != NULL)
		ret = fun(dev, (FUN_VALVE_CODE)agent, throughvalve, speed, quanty, fine);
	else
		ret = NO_DRIV_FUN;
	fault = ErrorCodeConvert(ret, agent);
	return fault;
}
/*******************************************************************************
* Function Name  : AddSolutionToPool
* Description    : 向比色池加试剂
* Input          : 	src:试剂对应的电磁阀; speed:加试剂速度；
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
FAULT_CODE AddAgentToPool(DEVICE_CODE dev, AGENT_CODE agent, uint16_t quanty, bool fine)
{
	DrawLiquidERROR ret = DRAW_OK;
	FAULT_CODE fault;
	uint16_t speed = PumpAgentSpeed[agent];
	if (speed > SPEED_MAX || speed < SPEED_MIN)
		speed = SPEED_MIN;
	TRANS_LIQUID_FUN fun;
	fun = GetLiquidFun(dev, (FUN_VALVE_CODE)agent);
	if (fun != NULL)
		ret = fun(dev, (FUN_VALVE_CODE)agent, F_VALVE_POOL_ONLY, speed, quanty, fine);
	else
		ret = NO_DRIV_FUN;
	fault = ErrorCodeConvert(ret, agent);
	return fault;
}
/*******************************************************************************
* Function Name  : AddAgentLoopPool
* Description    : 经定量环向比色池加试剂
* Input          : 	src:试剂对应的电磁阀; speed:加试剂速度；
										quanty：添加量；fine:定量方式（true:精确定量，false：粗定量）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
FAULT_CODE AddAgentLoopPool(DEVICE_CODE dev, AGENT_CODE agent, uint16_t quanty, bool fine)
{
	FAULT_CODE fault;
	DrawLiquidERROR ret = DRAW_OK;
	uint16_t speed = PumpAgentSpeed[agent];

	ret = TrsLiquidByTube(dev, (FUN_VALVE_CODE)agent, F_VALVE_LOOP_ONLINE_POOL, speed, quanty, fine);
	fault = ErrorCodeConvert(ret, agent);
	return fault;
}

/*******************************************************************************
* Function Name  : WashPoolWithAgent
* Description    : 向比色池加试剂
* Input          : 	valve:试剂对应的电磁阀; speed:加试剂速度；
										maxcircle：吹气圈数（蠕动泵）
* Output         : qdone:已经加液量
* Return         : 完成状态
*******************************************************************************/
FAULT_CODE WashPoolWithAgent(DEVICE_CODE dev, AGENT_CODE agent, uint16_t quanty)
{
	FAULT_CODE faultcode = NO_FAULT;
	//  printf("向比色池加水\r\n");
	faultcode = AddAgentToPool(dev, agent, quanty, false);
	if (faultcode == NO_FAULT)
	{
		//		printf("向比色池吹气\r\n");
		SendAir(dev, VALVE_GLASS_TANK, SPEED_REACTION_POOL_SOLUTION, 6);
		//		printf("排空比色池\r\n");
		PoolEmptying(dev, F_VALVE_CLEAN_WASTE);
	}
	else
	{
		//		printf("\r\n比色池加水失败，错误代码 %d。\r\n",faultcode);
		ReplyFaultInProcess(faultcode);
		SendAir(dev, VALVE_WASTE_LIQUID, SPEED_DEFAULT, 3);
		PoolEmptying(dev, F_VALVE_CLEAN_WASTE);
	}

	return faultcode;
}

/*******************************************************************************
* Function Name  : TrsLiquidByTitration
* Description    : 滴定泵进液
* Input          : 	valve:液体对应的电磁阀; quanty:抽取量(1:定量抽1个单位; 3:定量抽3各单位)
					speed:0.1rpm; fine:是否精确定量 true 精确定量，false 非精确定量
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
DrawLiquidERROR TrsLiquidByTitration(DEVICE_CODE dev, FUN_VALVE_CODE src, FUN_VALVE_CODE des,
									 uint16_t speed, uint16_t quanty, bool fine)
{
	uint16_t times = quanty;
	//	times = quanty / 2.5;
	while (times--)
	{
		OpenFunValve(dev, src);
		osDelay(150);
		CloseFunValve(dev, src);
		osDelay(100);
	}
	return DRAW_OK;
	//	ProcTitraPump(dev,TITRATION_PUMP,1000);      //滴定泵加氧化剂
}
/*******************************************************************************
* Function Name  : 溢流进样
* Description    : 抽取液体
* Input          : 	valve:液体对应的电磁阀; quanty:抽取量(1:定量抽1个单位; 3:定量抽3各单位)
					speed:0.1rpm; fine:是否精确定量 true 精确定量，false 非精确定量
* Output         : None
* Return         : 对应通道AD值
*******************************************************************************/
DrawLiquidERROR TrsLiquidByOverflow(DEVICE_CODE dev, FUN_VALVE_CODE src, FUN_VALVE_CODE des,
									uint16_t speed, uint16_t quanty, bool fine)
{
	uint16_t times = quanty;
	//	times = quanty / 2.5;
	while (times--)
	{
		OpenFunValve(dev, src);
		osDelay(150);
		CloseFunValve(dev, src);
		osDelay(100);
	}
	return DRAW_OK;
	//	ProcTitraPump(dev,TITRATION_PUMP,1000);      //滴定泵加氧化剂
}
