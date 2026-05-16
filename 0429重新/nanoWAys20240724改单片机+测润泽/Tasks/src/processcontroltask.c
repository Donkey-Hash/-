
#include <math.h>
#include <string.h>
#include "processcontroltask.h"
#include "cmsis_os2.h"
// #include "ProcAPI.h"
// #include "replyhost.h"
#include "ProcCommon.h"
#include "leddef.h"
#include "TestLEDCtr.h"
#include "peristalticpump.h"
#include "tempcontroltask.h"
// #include "ltc1867.h"
#include "iwdg.h"
#include "FlashUserData.h"
#include "UVSpectrogragh.h"
#include "progressbar.h"
#include "syringepump.h"
#include "usart.h"
#include "OPT2000.h"
#include "ModbusMSMD.h"
#include "FlashUserData.h"
// #include "hj212crc.h"
#include "valvedef.h"
#include "ProcAPI.h"
#include "ModbusMaster.h"
#include "ysspectro.h"
#include "encryptmcu.h"
#include "instrumentdef.h"
#include "yssyringepump.h"
#include "errormanagetask.h"
#include "Clock.h"

extern UCHAR *usCoilBuf;				// 线圈状态缓冲区
extern READ_UV_DATA_FUN ReadUVDataFun;	// 读取UV（紫外线）数据
extern ALL_DEV_DATA AllDevsData;		// 所有储存的设备数据
extern bool TimeoutFlag;				// 超时标志位
extern osTimerId_t SampleTimeOutHandle;

extern bool FlagStir;					// 搅拌机开关状态标志位
extern bool TimeoutFlag;				// 电机运行完成标志位(2025.6.18)
uint8_t AutoStep = 0;					// 检测步骤标志位(2025.6.18)
uint8_t AutoStep1_isWater = 1;			// 检测步骤1中比色皿的溶液是否为蒸馏水(2025.6.18)
float ZeroAbsorbanc	= 0.0;				// 初始化零标吸光度
float MeasureAbsorbanc	= 0.0;			// 初始化量程吸光度
nErrorCode error = SYS_OK;				// 初始化错误码
extern TEMP_CTRL_t TempCtrlData[MAX_TEMP_CTRL_CHANNEL];	// 导入温控通道结构体

void HostCmdProcess(void);				// 用于处理上位机指令

void HostCmdProcess_test(void);			// 测试，单个步骤

extern uint8_t Check_Reset(void);		// 复位检测（返回1为需要复位）

/* 外部可调用函数 */
extern void ProcessControlTask(void const *argument);

/* 流程程序2025.6.20改 */
void StartMode_Judgment(void);							// 启动模式判断
nErrorCode AutoRun(void);								// 开机自动运行流程(2025.6.20)

/* 流程子程序 */
void PumpOut_Cuvettes(int16_t speed,uint16_t lapNum);	// 排比色皿
void PumpIn_Cuvettes(int16_t speed,uint16_t lapNum);	// 进比色皿
void Valve_Operation(uint8_t regBit, uint8_t state);	// 阀操作
void PumpOut_WasteWater(int16_t speed, uint16_t lapNum, uint8_t waterState);	// 排废液或清洗废液
void PumpOut_Tube(int16_t speed,uint16_t lapNum, uint8_t waterState);			// 增、泄压排空计量管
void PumpInWater_Tube(int16_t speed,uint16_t lapNum);							// 采蒸馏水进计量管
nErrorCode PumpIn_Sample(int16_t speed, uint16_t lapNum, uint8_t regBit, uint8_t AD_Level);	// 采某个样
void Continue_Mode(void);	// 连续模式
void Periodic_Mode(void);	// 周期模式					
void FixedPoint_Mode(void);	// 定点模式
void Controlled_Mode(void);	// 受控模式
void Manual_Mode(void);		// 手动模式初始化
void Valve_Set(void);		// 单步调试-泵阀调试（受控模式和手动模式使用）
void Singel_Step(void);		// 多步调试-单一动作（受控模式和手动模式使用）

float TheAbs;							// 
bool AnalyzerBeWorking = false;			// 用于指示分析仪是否在工作
bool UVtestFlag = false;				// 用于控制UV测试的开关

bool ProcPreFill(DEVICE_CODE dev);
FAULT_CODE ProcInitLLAD(void);

uint16_t *pDevRangs;

// unsigned char crctest[256] = "QN=20160801085857223;ST=21;CN=1062;PW=123456;MN=A110000_0001;Flag=9;CP=&&RtdInterval=10&&";
// unsigned int crcret;
// unsigned int testlen;

uint16_t SpectroData[4];

void ProcessControlTask(void const *argument)
{
	//	osEvent pevent;
	uint32_t pevent = 0;	// 用于储存事件标志
	//	uint16_t data = 1;

	InitProcHandles();		// 初始化进程句柄(每个成员都是一个任务函数)
	//	pMbInputData->nProcCode = PROC_NULL;
	//	pDevRangs = &pMbHoldData->nRange1Set;
	DISABLE_PUMP_DRIVER		// 失能蠕动泵
#if !APP_USART3_MODBUS_SLAVE_MB3
	USART_DMA_CONFIG();		// 配置USART3主站口(DMA+IDLE)；当USART3作为MB3从站口时必须关闭
#endif
#if !APP_USART6_MODBUS_SLAVE_MB6
	ExDevDMAConfig();		// USART6 外设通信(DMA+IDLE)配置；MB6 模式下必须禁用
#endif
	
	osDelay(1000);
#ifdef MODBUS_MOTOR
	StopMbMotor(HOST_MB_MOTOR);	// 关闭Modbus搅拌机
	osDelay(500);
#endif
	// int state = GetMcuEncryptState();//判断硬件合法性
	// if(0 != state)
	// {
	// 	while(1)
	// 	{
	// 		Buzzer_ON
	// 		osDelay(50);
	// 		Buzzer_OFF
	// 		osDelay(100);
	// 		Buzzer_ON
	// 		osDelay(50);
	// 		Buzzer_OFF
	// 		osDelay(800);
	// 	}
	// }
	// else
	// 	;
	/* 这里进行启动模式类型的判断，来初始化参量 */
	StartMode_Judgment();					// 判断工作类型
	
	/* 测试代码：手动初始化参量 */
//	pMbHoldData->nAutoRun = 0;				// 初始化自动运行标志为0
//	pMbHoldData->RW50 = 0;					// 工作模式为0
//	pMbHoldData->nUpdateData = 0;			// 先清空日志更新
//	pMbHoldData->nSampleComplet = 0;		// 情况样本检测完标志位
//	pMbHoldData->fConcentrationVale = 0.0;	// 初始化检测值	
//	pMbHoldData->fAbsorbanc = 0.0;			// 初始化吸光度

	for (;;)
	{
		//		StartMbMotor(1,1200);
		//		osDelay(2000);
		//		StopMbMotor(HOST_MB_MOTOR);
		//		osDelay(2000);
		if (!AnalyzerBeWorking && UVtestFlag) // UVtestFlag在接收到上位机指令后取反.
		{
			osDelay(1000);
			ReadUVDataFun(&pMbInputData->nPoolTranLtBackAD, &pMbInputData->nPoolRefLtBackAD);	// UPT2000Data数据读取（ReadOPT2000Data(...)函数的引用）
		}

		//		StartPump(DRAW_LIQUID,600,2);
		//		ProcInitLLAD();
		//		SetAbsorbLEDCurrent(CurrentLEDAbs);     //设置比色光源强度
		osDelay(50);
		
		/*********************************正在使用的代码****************************************/
		/* 手动进入自动流程 */
		if(pMbHoldData->nAutoRun>0 || (pMbHoldData->nStartMode == 4 && pMbHoldData->nControlledStep) || pMbHoldData->nStartMode == 5) // 如果仪器还没有运行
		{
			/* 初始化参数 */
			pMbHoldData->RW50 = pMbHoldData->nAutoRun;		// 把工作模式状态赋值到对应位置
			if(pMbHoldData->RW50 == 24 || pMbHoldData->RW50 == 21 || pMbHoldData->RW50 == 19)	// 两点标定转水样或者C1标定
			{
				pMbHoldData->nZeroComplet = 0;				// 重新开始零点标定
				pMbHoldData->nMeasureComplet = 0;			// 重新开始量程标定
			}
			pMbHoldData->nAutoRun = 0;						// 清除运行标志
			pMbInputData->nDevBeRunning = 1;				// 仪器正在运行
			SaveUserData();									// 保存一下设备运行参数
			
			/* 启动模式判断 */
			
			if(pMbHoldData->nStartMode != 5)		// 判断不是受控模式,也不是手动模式
			{
				AutoStep = 1;		// 不是受控或手动模式就从第一步开始运行 
				error = AutoRun();	// 开始流程
			}
			else					// 是受控模式
			{	
				//HostCmdProcess();
				Valve_Set();	// 单步调试泵阀操作
				Singel_Step();	// 多步调试
			}
			
			
//			if((pMbHoldData->nStartMode != 4)&&(pMbHoldData->nStartMode != 5))		// 判断不是受控模式,也不是手动模式
//			{
//				AutoStep = 1;		// 不是受控或手动模式就从第一步开始运行 
//				error = AutoRun();	// 开始流程
//			}
//			else					// 是受控模式
//			{
//				//HostCmdProcess();
//				Valve_Set();	// 单步调试泵阀操作
//				Singel_Step();	// 多步调试
//			}
			SendFunErrorCode(error);							// 处理错误代码
		}
		/*************************************************************************/
		
		
		/* 以下两行代码功能相同，重复为增加稳定性（等待信号标志位） */
		pevent = osThreadFlagsGet();	// 获取当前线程标志
		pevent = osThreadFlagsWait(COMM_HOST_CMD | COMM_VALVE_OPT, osFlagsNoClear, 500); // 等待上位机发出"液体流动"或"流程"命令
		if (((pevent & osFlagsError) != osFlagsError) && (pMbHoldData->nAutoRun == 0))
		{
			if ((pevent & COMM_HOST_CMD) != 0) // 上位机指令
			{
				if(pMbHoldData->nStartMode == 4)
				{
					pMbInputData->nDevBeRunning = 1;	// 设备正在运行标志位置位
					HostCmdProcess();					// 上位机命令处理
				}
			}
			if ((pevent & COMM_VALVE_OPT) != 0) // 上位机指令
			{
				UpdataValveState(AllDevsData.tModbusData.mbCoilBuf);	// 更新阀的状态
			}
			osThreadFlagsClear(COMM_HOST_CMD | COMM_VALVE_OPT);	// 清除标志位
			AnalyzerBeWorking = false;
		}
		
		osDelay(100);
	}
}

/*******************************************************************************
* Function Name  	: Valve_Set
* Description    	: 单步调试-泵阀调试（受控模式和手动模式使用）
* Input          	: 

* Output         	:
* Return         	:
*******************************************************************************/
void Valve_Set(void)
{
	// usCoilBuf
	static uint8_t reg1 = 0;		// 取出0x线圈寄存器第1个byte(0-7)
	uint8_t reg2 = (uint8_t)(*(usCoilBuf+1));	// 取出0x线圈寄存器第2个byte(8-15)
	uint8_t reg3 = (uint8_t)(*(usCoilBuf+2));	// 取出0x线圈寄存器第3个byte(16-23)

	if(reg1 != (uint8_t)(*usCoilBuf))	// 说明有数据更新
	{
		for(int j = 0; j < 8; j++)
		{
			if(reg1 & (0x01<<j))
			{
				pMbHoldData->nCombiValve |= (0x00000001<<(j-1));
				pMbHoldData->nValveCtl = 0x01<<j;
				OpenValves(pMbHoldData->nCombiValve);
				osDelay(100);
			}
			else
			{
				CloseValves((0x00000001<<(j-1)));
			}
		}
		reg1 = (uint8_t)(*usCoilBuf);	// 更新reg1
	}
	
	
	
//	uint16_t offset = pMbHoldData->nValveCtl;	
//	if(offset)
//	{
//		pMbHoldData->nCombiValve = (pMbHoldData->nCombiValve | (0x00000001 << (offset-1)));
//		pMbHoldData->nValveCtl = 0;
//		osDelay(1000);
//		OpenValves(pMbHoldData->nCombiValve);
//	}	
}

/*******************************************************************************
* Function Name  	: Singel_Step
* Description    	: 单一动作模式
* Input          	: 

* Output         	:
* Return         	:
*******************************************************************************/
void Singel_Step(void)
{
	// 受控模式
	SINGEL_STEP CMD = (SINGEL_STEP)pMbHoldData->nControlledStep;
	
	switch(CMD)	// 指令字节
	{
		case SS_NOME:	// 空命令：0 = 0x00
			break;
		case SS_VAVECTOL:	// 阀控制:
			Valve_Set();
			break;		
		
		default:
			break;
	}
	
	pMbHoldData->nControlledStep = SS_NOME;
}


/*******************************************************************************
* Function Name  	: RS232CmdProcess
* Description    	: RS232命令数据包解析
* Input          	: 接收数据缓存地址

* Output         	:
* Return         	:
*******************************************************************************/
void HostCmdProcess(void)
{
	HOST_CMD_t CMD = (HOST_CMD_t)pMbHoldData->nHostCMD;
	pMbHoldData->nCMDTranMoni = 11;
	//	pMbHoldData->nHostCMD = 0;
	
	switch (CMD) // 指令字节
	{
	case CMD_START_COMBI_VALVE: // 0x0A, //10  开组合阀。组合阀以位或的方式组合，需要开启的位置一，不影响其他阀位。全开使用0xFFFFFFFF，写零不会有任何操作。
		OpenValves(pMbHoldData->nCombiValve);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_STOP_COMBI_VALVE: // 0x0B, //11  关组合阀。组合阀以位或的方式组合，需要关闭的位置一，不影响其他阀位。全关使用0xFFFFFFFF，写零不会有任何操作。
		CloseValves(pMbHoldData->nCombiValve);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_PERISTALTIC_PUMP_OPT: // 0x0C, //12 操作蠕动泵，参数1是执行圈数；参数2是速度，速度值为正则正转，为负则反转；
		StopPump();
		osDelay(300);
		StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);
		while (!TimeoutFlag)
			osDelay(100);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_DRAW_REAGENT: // 0x0D, //13 抽取试剂。参数1：最大抽取圈数，超过此圈数报警，报警值01；参数2：抽取速度；参数3：抽取液位。
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_DETERMIN_LEVEL_DOWN: // 0x0E, //14 下行判定液位。参数1：最多执行圈数，超过此圈数报警，报警值02；参数2：下行速度；参数3：液位
		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_DETERMIN_LEVEL_UP: // 0x0F, //15 上行判定液位。参数1：最多执行圈数，超过此圈数报警，报警值02；参数2：下行速度；参数3：液位
		pMbInputData->nSysErrorCode = DeterminLevelUp(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_START_TEMPER_CTRL: // 0x10, //16 开启温控。模式1：按目标值和保温时间温控，温控函数为阻塞函数，直到完成温控之后才能解除阻塞，运行下一步骤。
								// 开启加热控制
		ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_START_TEMPER_CTRL_CONST: // 0x11, //17 开启温控。模式2：按目标温度执行温控操作，一旦达到目标温度，程序即刻解除阻塞，但温控持续进行。
		ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_STOP_TEMPER_CTRL: // 0x12, //18 停止温控。对模式1和模式2均有效。
		StopTempCtrl(TEMP_CTRL_CH_POOL);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_READ_DARK_LED: // 0x13, //19 读取暗电流
	case CMD_READ_LED:		// 0x14, //20 读取光强值（AD值）
		ProcReadYSADs(10);
		pMbHoldData->nHostCMD = CMD_NONE;
		//pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_SET_SPECTRO_SCAN_TIMES:	//0x03 //3 
		SetAverageTimes();
		break;
	case CMD_SET_SPECTRO_INTEGRAL_TIME:
		SetIntegralTime();
		break;
	case CMD_SPECTRO_LIGHT_ON: // 21 光谱仪开光源
		OpenSpectroLight();
		break;
	case CMD_SPECTRO_LIGHT_OFF: // 22 光谱仪关光源
		CloseSpectroLight();
		break;
	case CMD_SPECTRO_SCAN: // 23 光谱仪启动扫描
		StartSpectroScan();
		break;
	case CMD_SPECTRO_READ_DATA: // 24 光谱仪读数据//数据存入保持寄存器29、30
	{
		int16_t temp = ReadSpectroData(SpectroData);
		if (temp >= 0)
		{
			pMbHoldData->nSpectro220AD = SpectroData[0];
			pMbHoldData->nSpectro275AD = SpectroData[1];
		}
		else
		{
			pMbHoldData->nSpectro220AD = temp;
			pMbHoldData->nSpectro275AD = temp;
		}
	}
	break;
	case CMD_SYRINGE_RESET_ORIGIN: // 25   //注射泵复位
	ResetYsSyringeOrigin();
		break;
	case CMD_SYRINGE_MOVE: // 26   //注射泵相对运动。相对于当前位置运动，正数推，负数抽。
		SetYsSyringeSpeed();
		MoveYsSyringe(pMbHoldData->nSyringePumpMove);
		//			osDelay(3000);
		//			pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		break;
	case CMD_SYRINGE_MOVE_TO: // 27   //注射泵移动到绝对位置。绝对位置是相对于0点位置的数值。
		SetYsSyringeSpeed();
		MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
		//			osDelay(3000);
		//			pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		break;
	case CMD_SYRINGE_STOP: // 28   //注射泵停止。
		StopYsSyringe();
		break;
	case CMD_SYRINGE_READ_POS: // 35   //注射泵位置
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		break;
	case CMD_STIR_ON: // 29	//开启搅拌
		FlagStir = true;
		break;
	case CMD_STIR_OFF: // 30   //停止搅拌
		FlagStir = false;
		break;
	case CMD_4_20_MA_CALIB_TEST: // 31 ////4-20ma输出测试。
		*pvrAmaxOut = 0;
		*pvrAminOut = 0;
		*pvrCurrenttest = 1;
		break;
	case CMD_4_20_MA_CALIB_MAX: // 32   //4-20ma输出标定，标定上限电流。执行此命令后，将电流真实值写入寄存器
		*pvrAmaxOut = 1;
		*pvrAminOut = 0;
		*pvrCurrenttest = 0;
		break;
	case CMD_4_20_MA_CALIB_MIN: // 33 //4-20ma输出标定，标定下限电流。执行此命令后，将电流真实值写入寄存器
		*pvrAmaxOut = 0;
		*pvrAminOut = 1;
		*pvrCurrenttest = 0;
		break;
	case CMD_4_20_MA_NORMAL: // 34
		*pvrAmaxOut = 0;
		*pvrAminOut = 0;
		*pvrCurrenttest = 0;
		break;
	case CMD_LL_AD_CALIB: // 108
		pMbInputData->nSysErrorCode = CheckLevelsAD();	// 液位定位AD校准
		break;
	case CMD_CLEAR_POOL:
		//				SendAir(pMbHoldData->nDevCode,F_VALVE_CLEAN_WASTE,SPEED_DEFAULT,6);
		//				PoolEmptying(pMbHoldData->nDevCode,F_VALVE_CLEAN_WASTE);
		break;
	case CMD_CLEAR_SAMPLE_PIPE:
		//				SendAir(pMbHoldData->nDevCode,F_VALVE_SAMPLE,SPEED_SAMPLE,11);
		break;

	case CMD_RZ_HOLE:	// CMD_RZ_HOLE = 0x29,  		 // 41 //润泽执行切换阀
		RZdelayflag = 1;
		RUNZE_SETHOLE(pMbHoldData->RZHOLENUM);	//1~10
		RZdelayflag = 0;
		break;
	case CMD_RZ_SPEED:	// = 0x2A,  		 // 42 //润泽设置速度
		RZdelayflag = 1;
		RUNZE_SET_SPEED(pMbHoldData->RZSPEED);
		RZdelayflag = 0;
		break;
	case CMD_RZ_ML:		// = 0x2B,  			 // 43 //润泽运行ml
		RZdelayflag = 1;
		RUNZE_SET_ML(pMbHoldData->RZML);	//0~6,0.5
		RZdelayflag = 0;
		break;
	case CMD_RZ_OPEN:	// = 0x2C,  			 // 44 //润泽打开阀
		RUNZE_SET_NC();	//开阀
		break;
	case CMD_RZ_CLOSE:	// = 0x2D,  			 // 45 //润泽关闭阀
		RUNZE_SET_NO();	//关阀。默认是关
		break;
	case CMD_HC_HOLE:	// CMD_RZ_HOLE = 0x29,  		 // 46 //HC执行切换阀
		RZdelayflag = 1;
		HC_SETHOLE(pMbHoldData->RZHOLENUM);	//1~10
		RZdelayflag = 0;
		break;
	case CMD_HC_SPEED:	// = 0x2A,  		 // 47 //HC设置速度
		RZdelayflag = 1;
		HC_SET_SPEED(pMbHoldData->RZSPEED);
		RZdelayflag = 0;
		break;
	case CMD_HC_ML:		// = 0x2B,  			 // 48 //HC运行ml
		RZdelayflag = 1;
		HC_SET_ML(pMbHoldData->RZML);	//0~6,0.5
		RZdelayflag = 0;
		break;
	case CMD_HC_OPEN:	// = 0x2C,  			 // 49 //HC打开阀
		HC_SET_NC();	//开阀
		break;
	case CMD_HC_CLOSE:	// = 0x2D,  			 // 50 //HC关闭阀
		HC_SET_NO();	//关阀。默认是关
		break;
	default:
		break;
	}
	pMbHoldData->nHostCMD = CMD_NONE;
	//		pMbHoldData->nRunningFlag = 0;
}

/*******************************************************************************
* Function Name  	: RS232CmdProcess_test
* Description    	: RS232命令数据包解析
* Input          	: 接收数据缓存地址

* Output         	:
* Return         	:
*******************************************************************************/
void HostCmdProcess_test(void)
{
	HOST_CMD_t CMD = (HOST_CMD_t)pMbHoldData->nHostCMD;
	pMbHoldData->nCMDTranMoni = 11;
	//	pMbHoldData->nHostCMD = 0;
	
	/* 切换到受控模式执行的步骤 */
	CMD = (HOST_CMD_t)pMbHoldData->nControlledStep;
	osDelay(200);
	
	switch (CMD) // 指令字节
	{
	case CMD_START_COMBI_VALVE: // 0x0A, //10  开组合阀。组合阀以位或的方式组合，需要开启的位置一，不影响其他阀位。全开使用0xFFFFFFFF，写零不会有任何操作。
		OpenValves(pMbHoldData->nCombiValve);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_STOP_COMBI_VALVE: // 0x0B, //11  关组合阀。组合阀以位或的方式组合，需要关闭的位置一，不影响其他阀位。全关使用0xFFFFFFFF，写零不会有任何操作。
		CloseValves(pMbHoldData->nCombiValve);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_PERISTALTIC_PUMP_OPT: // 0x0C, //12 操作蠕动泵，参数1是执行圈数；参数2是速度，速度值为正则正转，为负则反转；
		StopPump();
		osDelay(300);
		StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);
		while (!TimeoutFlag)
			osDelay(100);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_DRAW_REAGENT: // 0x0D, //13 抽取试剂。参数1：最大抽取圈数，超过此圈数报警，报警值01；参数2：抽取速度；参数3：抽取液位。
		pMbInputData->nSysErrorCode = mDrawLiquid(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_DETERMIN_LEVEL_DOWN: // 0x0E, //14 下行判定液位。参数1：最多执行圈数，超过此圈数报警，报警值02；参数2：下行速度；参数3：液位
		pMbInputData->nSysErrorCode = DeterminLevelDown(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_DETERMIN_LEVEL_UP: // 0x0F, //15 上行判定液位。参数1：最多执行圈数，超过此圈数报警，报警值02；参数2：下行速度；参数3：液位
		pMbInputData->nSysErrorCode = DeterminLevelUp(pMbHoldData->nPeristalticPumpParm, pMbHoldData->nStepperMotorSpeed, pMbHoldData->nLevelNum);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_START_TEMPER_CTRL: // 0x10, //16 开启温控。模式1：按目标值和保温时间温控，温控函数为阻塞函数，直到完成温控之后才能解除阻塞，运行下一步骤。
								// 开启加热控制
		ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_START_TEMPER_CTRL_CONST: // 0x11, //17 开启温控。模式2：按目标温度执行温控操作，一旦达到目标温度，程序即刻解除阻塞，但温控持续进行。
		ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_STOP_TEMPER_CTRL: // 0x12, //18 停止温控。对模式1和模式2均有效。
		StopTempCtrl(TEMP_CTRL_CH_POOL);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_READ_DARK_LED: // 0x13, //19 读取暗电流
	case CMD_READ_LED:		// 0x14, //20 读取光强值（AD值）
		ProcReadYSADs(10);
		pMbHoldData->nHostCMD = CMD_NONE;
		pMbHoldData->nControlledStep = CMD_NONE;
		break;
	case CMD_SET_SPECTRO_SCAN_TIMES:	//0x03 //3 
		SetAverageTimes();
		break;
	case CMD_SET_SPECTRO_INTEGRAL_TIME:
		SetIntegralTime();
		break;
	case CMD_SPECTRO_LIGHT_ON: // 21 光谱仪开光源
		OpenSpectroLight();
		break;
	case CMD_SPECTRO_LIGHT_OFF: // 22 光谱仪关光源
		CloseSpectroLight();
		break;
	case CMD_SPECTRO_SCAN: // 23 光谱仪启动扫描
		StartSpectroScan();
		break;
	case CMD_SPECTRO_READ_DATA: // 24 光谱仪读数据//数据存入保持寄存器29、30
	{
		int16_t temp = ReadSpectroData(SpectroData);
		if (temp >= 0)
		{
			pMbHoldData->nSpectro220AD = SpectroData[0];
			pMbHoldData->nSpectro275AD = SpectroData[1];
		}
		else
		{
			pMbHoldData->nSpectro220AD = temp;
			pMbHoldData->nSpectro275AD = temp;
		}
	}
	break;
	case CMD_SYRINGE_RESET_ORIGIN: // 25   //注射泵复位
	ResetYsSyringeOrigin();
		break;
	case CMD_SYRINGE_MOVE: // 26   //注射泵相对运动。相对于当前位置运动，正数推，负数抽。
		SetYsSyringeSpeed();
		MoveYsSyringe(pMbHoldData->nSyringePumpMove);
		//			osDelay(3000);
		//			pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		break;
	case CMD_SYRINGE_MOVE_TO: // 27   //注射泵移动到绝对位置。绝对位置是相对于0点位置的数值。
		SetYsSyringeSpeed();
		MoveYsSyringeTo(pMbHoldData->nSyringePumpMoveTo);
		//			osDelay(3000);
		//			pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		break;
	case CMD_SYRINGE_STOP: // 28   //注射泵停止。
		StopYsSyringe();
		break;
	case CMD_SYRINGE_READ_POS: // 35   //注射泵位置
		pMbInputData->nSyringePumpPos = (int32_t)ReadPos();
		break;
	case CMD_STIR_ON: // 29	//开启搅拌
		FlagStir = true;
		break;
	case CMD_STIR_OFF: // 30   //停止搅拌
		FlagStir = false;
		break;
	case CMD_4_20_MA_CALIB_TEST: // 31 ////4-20ma输出测试。
		*pvrAmaxOut = 0;
		*pvrAminOut = 0;
		*pvrCurrenttest = 1;
		break;
	case CMD_4_20_MA_CALIB_MAX: // 32   //4-20ma输出标定，标定上限电流。执行此命令后，将电流真实值写入寄存器
		*pvrAmaxOut = 1;
		*pvrAminOut = 0;
		*pvrCurrenttest = 0;
		break;
	case CMD_4_20_MA_CALIB_MIN: // 33 //4-20ma输出标定，标定下限电流。执行此命令后，将电流真实值写入寄存器
		*pvrAmaxOut = 0;
		*pvrAminOut = 1;
		*pvrCurrenttest = 0;
		break;
	case CMD_4_20_MA_NORMAL: // 34
		*pvrAmaxOut = 0;
		*pvrAminOut = 0;
		*pvrCurrenttest = 0;
		break;
	case CMD_LL_AD_CALIB: // 108
		pMbInputData->nSysErrorCode = CheckLevelsAD();	// 液位定位AD校准
		break;
	case CMD_CLEAR_POOL:
		//				SendAir(pMbHoldData->nDevCode,F_VALVE_CLEAN_WASTE,SPEED_DEFAULT,6);
		//				PoolEmptying(pMbHoldData->nDevCode,F_VALVE_CLEAN_WASTE);
		break;
	case CMD_CLEAR_SAMPLE_PIPE:
		//				SendAir(pMbHoldData->nDevCode,F_VALVE_SAMPLE,SPEED_SAMPLE,11);
		break;

	case CMD_RZ_HOLE:	// CMD_RZ_HOLE = 0x29,  		 // 41 //润泽执行切换阀
		RZdelayflag = 1;
		RUNZE_SETHOLE(pMbHoldData->RZHOLENUM);	//1~10
		RZdelayflag = 0;
		break;
	case CMD_RZ_SPEED:	// = 0x2A,  		 // 42 //润泽设置速度
		RZdelayflag = 1;
		RUNZE_SET_SPEED(pMbHoldData->RZSPEED);
		RZdelayflag = 0;
		break;
	case CMD_RZ_ML:		// = 0x2B,  			 // 43 //润泽运行ml
		RZdelayflag = 1;
		RUNZE_SET_ML(pMbHoldData->RZML);	//0~6,0.5
		RZdelayflag = 0;
		break;
	case CMD_RZ_OPEN:	// = 0x2C,  			 // 44 //润泽打开阀
		RUNZE_SET_NC();	//开阀
		break;
	case CMD_RZ_CLOSE:	// = 0x2D,  			 // 45 //润泽关闭阀
		RUNZE_SET_NO();	//关阀。默认是关
		break;
	case CMD_HC_HOLE:	// CMD_RZ_HOLE = 0x29,  		 // 46 //HC执行切换阀
		RZdelayflag = 1;
		HC_SETHOLE(pMbHoldData->RZHOLENUM);	//1~10
		RZdelayflag = 0;
		break;
	case CMD_HC_SPEED:	// = 0x2A,  		 // 47 //HC设置速度
		RZdelayflag = 1;
		HC_SET_SPEED(pMbHoldData->RZSPEED);
		RZdelayflag = 0;
		break;
	case CMD_HC_ML:		// = 0x2B,  			 // 48 //HC运行ml
		RZdelayflag = 1;
		HC_SET_ML(pMbHoldData->RZML);	//0~6,0.5
		RZdelayflag = 0;
		break;
	case CMD_HC_OPEN:	// = 0x2C,  			 // 49 //HC打开阀
		HC_SET_NC();	//开阀
		break;
	case CMD_HC_CLOSE:	// = 0x2D,  			 // 50 //HC关闭阀
		HC_SET_NO();	//关阀。默认是关
		break;
	default:
		break;
	}
	pMbHoldData->nHostCMD = CMD_NONE;
	
	pMbHoldData->nControlledStep = CMD_NONE;
	
	//		pMbHoldData->nRunningFlag = 0;
}




/*******************************************************************************
* Function Name  	: 开机自启动运行流程(重新封装) 2025.6.20
* Description    	: 
* Input          	: 

* Output         	:
* Return         	:
*******************************************************************************/
nErrorCode AutoRun(void)
{
	/* 以下两行代码用于测试 */
	//OpenValves(pMbHoldData->nCombiValve);
	//StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);
	
	/* 函数内整体参数定义 */
	uint8_t loopNum = 0;	// 定义一个循环变量用来计次
	uint8_t resetState = 0;	// 定义一个复位标志
	
	/* 模块3的参数定义 */
	uint16_t AD_Low;		// 定义3X15液位1AD值
	uint16_t AD_High;		// 定义3X16液位2AD值
	uint8_t AD_LowFlag; 	// 低液位合理标志位
	uint8_t AD_HighFlag;	// 高液位合理标志位
				
	/* 模块5的参数定义 */
	uint8_t SampleLevel;	// 定义进样的高低液位标志（0为低液位，1为高液位）
	
	/* 模块19的参数定义 */
	
	while(pMbInputData->nDevBeRunning == 1)// 仪器已经打开，下面开始运行主流程
	{
		/* 运行初始化 */
		CloseAllValves();	// 先关闭所有阀
		StopPump();			// 关闭蠕动泵
		
		/* 周期模式：定时一个小时做水样 */
		if(pMbHoldData->nStartMode == 2)
		{
			osTimerStart(SampleTimeOutHandle, (pMbHoldData->nClockTime) * 60 * 1000);	// 定时1小时后再来来采水样
		}
		
		/* 模块1：排空管路、比色皿 */
		while(AutoStep == 1)
		{
			//pMbHoldData->RW580 = 0;	// 下位机结束标志
			
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			CloseAllValves();				//	先关闭所有阀
			pMbInputData->nStepCode = 1;	// 排空管路
			
			// step1: 排空管路
			// 开阀
			pMbHoldData->nCombiValve = (0x00000001 << 12);			// 设置废液阀的寄存器操作位
			OpenValves(pMbHoldData->nCombiValve);					// 打开对应设置操作为的阀门
			osDelay(500);
			
			// 排水
			pMbHoldData->nStepperMotorSpeed = -1200;			// 设置蠕动泵的速度寄存器位（负数为排液）
			pMbHoldData->nPeristalticPumpParm = 250;		// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; // 清除电机运行完标志位
			osDelay(500);
			
			// 关阀
			pMbHoldData->nCombiValve = (0x00000001 << 12);			// 设置废液阀的寄存器操作位
			CloseValves(pMbHoldData->nCombiValve);					// 关阀	
			osDelay(500);
			
			//pMbHoldData->RW580 = 1;									// 工作流程状态标志(未标志)
			
			// step2: A插入：闷抽比色皿
			pMbHoldData->nStepperMotorSpeed = 1000;			// 设置蠕动泵的速度寄存器位（正数为闷抽比色皿）
			pMbHoldData->nPeristalticPumpParm = 150;		// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; // 清除电机运行完标志位
			//osDelay(500);
			
			// 开阀
			pMbHoldData->nCombiValve = (0x00000001 << 3);			// 设置比色阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 8);			// 设置消解下阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 9);			// 设置消解上阀的寄存器操作位
			OpenValves(pMbHoldData->nCombiValve);		// 打开对应设置操作为的阀门
			osDelay(500);
			
			// 正速度排控比色皿
			pMbHoldData->nStepperMotorSpeed = 1000;			// 设置蠕动泵的速度寄存器位（正数为闷抽比色皿）
			pMbHoldData->nPeristalticPumpParm = 150;		// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);		// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; 	// 清除电机运行完标志位
			//osDelay(500);
			
			// 关阀
			pMbHoldData->nCombiValve = (0x00000001 << 3);			// 设置比色阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 8);			// 设置消解下阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 9);			// 设置消解上阀的寄存器操作位
			CloseValves(pMbHoldData->nCombiValve);					// 关闭对应设置操作为的阀门
			osDelay(500);
			
			// 开废水阀:检测开哪个阀
			if(AutoStep1_isWater == 1)// 1:蒸馏水标志(首次启动为1，连续工作为0)
			{
				pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
				pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
				OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
			}
			else
			{
				pMbHoldData->nCombiValve = (0x00000001 << 12);	// 设置废液阀的寄存器操作位
				OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
			}
			osDelay(300);	// 给0.3s时间开阀
			
			// 排空计量管废液
			pMbHoldData->nStepperMotorSpeed = -1200;			// 设置蠕动泵的速度寄存器位（负数为排废液）
			pMbHoldData->nPeristalticPumpParm = 150;			// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; // 清除电机运行完标志位
			//osDelay(500);
			
			// 关废液阀
			pMbHoldData->nCombiValve = (0x00000001 << 0);		// 设置清洗废液阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 12);		// 设置废液阀的寄存器操作位
			CloseValves(pMbHoldData->nCombiValve);				// 关闭对应设置操作为的阀门
			osDelay(500);
			
			// step3:排空比色皿(循环3次)
			while(loopNum < 3)
			{
				// 开阀
				pMbHoldData->nCombiValve = (0x00000001 << 3);			// 设置比色阀的寄存器操作位
				pMbHoldData->nCombiValve |= (0x00000001 << 8);			// 设置消解下阀的寄存器操作位
				pMbHoldData->nCombiValve |= (0x00000001 << 9);			// 设置消解上阀的寄存器操作位
				OpenValves(pMbHoldData->nCombiValve);		// 打开对应设置操作为的阀门
				osDelay(500);
				
				// 抽水
				pMbHoldData->nStepperMotorSpeed = 1000;		// 设置蠕动泵的速度寄存器位（负数为排液）
				pMbHoldData->nPeristalticPumpParm = 150;	// 设置蠕动泵的转圈数寄存器位
				StopPump();
				osDelay(300);
				StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
				osDelay(500);
				while(!TimeoutFlag)
				{
					osDelay(100);		// 等待蠕动泵操作完成
				}
				TimeoutFlag = false; 	// 清除电机运行完标志位
				//osDelay(500);
				
				// 关阀
				pMbHoldData->nCombiValve = (0x00000001 << 3);			// 设置比色阀的寄存器操作位
				pMbHoldData->nCombiValve |= (0x00000001 << 8);			// 设置消解下阀的寄存器操作位
				pMbHoldData->nCombiValve |= (0x00000001 << 9);			// 设置消解上阀的寄存器操作位
				CloseValves(pMbHoldData->nCombiValve);					// 关闭对应设置操作为的阀门
				osDelay(500);
				
				// 开阀:检测开哪个阀
				if(AutoStep1_isWater == 1)// 1:蒸馏水标志(首次启动为1，连续工作为0)
				{
					pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
					pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
					OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
				}
				else
				{
					pMbHoldData->nCombiValve = (0x00000001 << 12);	// 设置废液阀的寄存器操作位
					OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
				}
				osDelay(300);	// 给0.3s时间开阀
				
				// 排水
				pMbHoldData->nStepperMotorSpeed = -1200;		// 设置蠕动泵的速度寄存器位（负数为排液）
				pMbHoldData->nPeristalticPumpParm = 250;		// 设置蠕动泵的转圈数寄存器位
				StopPump();
				osDelay(300);
				StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
				osDelay(500);
				while(!TimeoutFlag)
				{
					osDelay(100);// 等待蠕动泵操作完成
				}
				TimeoutFlag = false; // 清除电机运行完标志位
				//osDelay(500);
				
				// 关阀
				pMbHoldData->nCombiValve = (0x00000001 << 0);		// 设置清洗废液阀的寄存器操作位
				pMbHoldData->nCombiValve |= (0x00000001 << 12);		// 设置废液阀的寄存器操作位
				CloseValves(pMbHoldData->nCombiValve);				// 关闭对应设置操作为的阀门
				osDelay(500);
				
				loopNum ++;	//循环+1
			}
			loopNum = 0;	// 循环结束后清空循环次数
			
//			/* 如果是受控模式直接退出大循环 */
//			if(pMbHoldData->nStartMode == 4)
//			{
//				AutoStep = 0;
//				break;
//			}
			
			AutoStep = 2;	// 切换到步骤模块2
			break;			// 跳出模块1
		}
		
		/* 模块2：零标流程 */
		while(AutoStep == 2)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1:计量管增压再泄压
			// 排水增压
			pMbHoldData->nStepperMotorSpeed = -800;		// 设置蠕动泵的速度寄存器位（负数为排液）
			pMbHoldData->nPeristalticPumpParm = 150;		// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);	// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; // 清除电机运行完标志位
			//osDelay(500);
			
			// 开阀
			pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
			OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
			osDelay(500);
			
			// 开阀排水泄压
			pMbHoldData->nStepperMotorSpeed = -800;			// 设置蠕动泵的速度寄存器位（负数为排液）
			pMbHoldData->nPeristalticPumpParm = 150;		// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);	// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; // 清除电机运行完标志位
			//osDelay(500);
			
			// 泄压后关阀
			pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
			CloseValves(pMbHoldData->nCombiValve);			// 关闭对应设置操作为的阀门
			osDelay(500);
			
			/* 中间有个RW50寄存器工作类型判断流程：用于步骤跳转 */
			// 判断工作流程
			if((pMbHoldData->RW50 != 8)&&(pMbHoldData->RW50 != 19)&&
				(pMbHoldData->RW50 != 21)&&(pMbHoldData->RW50 != 24)&&
				(pMbHoldData->RW50 != 29)&&(pMbHoldData->RW50 != 31)&&
				(pMbHoldData->RW50 != 33)&&(pMbHoldData->RW50 != 42))
			{
				AutoStep = 4;	// 直接跳转到外界泵循环
				break;
			}
			
			// step2:抽蒸馏水清洗管
			while(loopNum < 3)
			{
				// 开阀
				pMbHoldData->nCombiValve = (0x00000001 << 4);	// 设置蒸馏水阀的寄存器操作位
				OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
				osDelay(500);
				
				// 抽清水（蒸馏水）
				pMbHoldData->nStepperMotorSpeed = 1000;			// 设置蠕动泵的速度寄存器位（负数为排液）
				pMbHoldData->nPeristalticPumpParm = 300;		// 设置蠕动泵的转圈数寄存器位
				StopPump();
				osDelay(300);
				StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
				osDelay(500);
				while(!TimeoutFlag)
				{
					osDelay(100);	// 等待蠕动泵操作完成
				}
				TimeoutFlag = false; // 清除电机运行完标志位
				osDelay(500);
				
				// 关阀
				pMbHoldData->nCombiValve = (0x00000001 << 4);	// 设置蒸馏水阀的寄存器操作位
				CloseValves(pMbHoldData->nCombiValve);			// 关闭对应设置操作为的阀门
				osDelay(500);
				
				// 开阀
				pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
				pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
				OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
				osDelay(500);
				
				// 排清水（蒸馏水）
				pMbHoldData->nStepperMotorSpeed = -1200;			// 设置蠕动泵的速度寄存器位（负数为排液）
				pMbHoldData->nPeristalticPumpParm = 400;			// 设置蠕动泵的转圈数寄存器位
				StopPump();
				osDelay(300);
				StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
				osDelay(500);
				while(!TimeoutFlag)
				{
					osDelay(100);	// 等待蠕动泵操作完成
				}
				TimeoutFlag = false; // 清除电机运行完标志位
				osDelay(500);
				
				// 上位机计算蒸馏水和清洗废液的某些量值
				//SetFloat(@蒸馏水实际体积@,0,GetFloat(@蒸馏水实际体积@,0)-4);
				//SetFloat(@蒸馏水余量@,0,GetFloat(@蒸馏水实际体积@,0)/GetFloat(@蒸馏水满瓶体积@,0)*100);
				//SetFloat(@清洗废液实际体积@,0,GetFloat(@清洗废液实际体积@,0)+4);
				//SetFloat(@清洗废液积量@,0,GetFloat(@清洗废液实际体积@,0)/GetFloat(@清洗废液满瓶体积@,0)*100);
				
				// 关阀
				pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
				pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
				CloseValves(pMbHoldData->nCombiValve);			// 关闭对应设置操作为的阀门
				osDelay(500);
				
				loopNum ++;
			}
			
			loopNum = 0;	// 清空循环标志位
			AutoStep = 3;	// 步骤跳转到模块3
			break;			// 跳出模块2
		}
		
		/* 模块三: 零点标定液位校准*/
		while(AutoStep == 3)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1:计量管增压再泄压
			// 排水增压
			pMbHoldData->nStepperMotorSpeed = -1200;		// 设置蠕动泵的速度寄存器位（负数为排液）
			pMbHoldData->nPeristalticPumpParm = 150;		// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);	// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; // 清除电机运行完标志位
			osDelay(500);
			
			// 开阀
			pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
			OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
			osDelay(500);
			
			// 开阀排水泄压
			pMbHoldData->nStepperMotorSpeed = -1200;			// 设置蠕动泵的速度寄存器位（负数为排液）
			pMbHoldData->nPeristalticPumpParm = 150;		// 设置蠕动泵的转圈数寄存器位
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);	// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; // 清除电机运行完标志位
			osDelay(500);
			
			// 关阀
			pMbHoldData->nCombiValve = (0x00000001 << 0);	// 设置清洗废液阀的寄存器操作位
			pMbHoldData->nCombiValve |= (0x00000001 << 12);	// 设置废液阀的寄存器操作位
			CloseValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
			osDelay(500);
			
			// step2:AD校准
			while(1)											// 这里可以用死循环
			{
				pMbInputData->nSysErrorCode = CheckLevelsAD();	// 液位定位AD校准
				
				while(pMbInputData->nSysErrorCode != PUMP_LIQUID_OK)
				{
					osDelay(100);	// 等待液位定位AD校准完成
				}
				
				AD_Low  = pMbInputData->nLL1AD;	// 获取3X15低位信号
				AD_High = pMbInputData->nLL2AD;	// 获取3X16高位信号
				
				AD_LowFlag  = (AD_Low >= 1200) && (AD_Low <= 1400); // 低液位合理标志位
				AD_HighFlag = (AD_High >= 1200) && (AD_High <= 1400);	// 高液位合理标志位
				
				if(AD_LowFlag && AD_HighFlag)	// 判断高低位信号是否在规定范围
				{
					break;	// 标定准确，跳出循环执行下一步
				}
				
				loopNum ++;
				
				/* 这里可以判断一下loopNum次数，或者在死循环里到一定次数后就跳出并处理错误代码 */
				// step3:返回错误码
				if(loopNum >= 5)
				{
					return AD_CALIBRATION_ERROR;
				}
			}
			
			loopNum = 0;	// 清空循环标志位
			
			// step3:赋值空杯电压
			pMbHoldData->RW30 = AD_Low;		// RW30低位空杯电压=3x15地位信号
			pMbHoldData->RW31 = AD_High;	// RW31低位空杯电压=3x16地位信号
			
			pMbHoldData->RW32 = AD_Low * 1.2;	// RW32低水位阀值=3x15地位信号*1.2
			pMbHoldData->RW33 = AD_High * 1.2;	// RW33低水位阀值=3x16地位信号*1.2
			
			pMbHoldData->RW34 = AD_Low * 0.5;	// RW34低水位阀值=3x15地位信号*0.5
			pMbHoldData->RW35 = AD_High * 0.5;	// RW34低水位阀值=3x15地位信号*0.5
			
			SaveUserData();						// 保存用户数据
			
			AutoStep = 4;				// 步骤跳转到模块4
			osDelay(500);
			break;						// 跳出模块3
		}
		
		/* 模块4：外接泵循环 */
		while(AutoStep == 4)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1：开启保温
			// 开水泵阀
			pMbHoldData->nCombiValve = (0x00000001 << 14);	// 设置水泵的寄存器操作位
			OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
			osDelay(500);
			
			// 设置温度和PID参数
			pMbHoldData->nTestTemper = 50;	// 设置测试温度
			pMbHoldData->fTemp1Kp = 500;	// 设置温控PID
			pMbHoldData->fTemp1Ki = 0.3;
			pMbHoldData->fTemp1Kd = 2000;
			
			// 指令17
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
			
			/* 判断RW50工作方式 */
			if(pMbHoldData->RW50 == 1)
			{
				pMbHoldData->RW36 = 9;
				osDelay(pMbHoldData->RW36 * 1000);	// 水泵循环时间（9秒）
			}
			
			// 关水泵阀
			pMbHoldData->nCombiValve = (0x00000001 << 14);	// 设置水泵的寄存器操作位
			CloseValves(pMbHoldData->nCombiValve);			// 关闭对应设置操作为的阀门
			osDelay(500);
			
			AutoStep = 5;				// 步骤跳转到模块5
			osDelay(500);
			break;						// 跳出模块4
		}
		
		/* 模块5：进样  */
		while(AutoStep == 5)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1：判断选择哪个量程
			switch(pMbHoldData->nMeasureChoose)
			{
				case 1: 
					SampleLevel = 1;break;
				case 2: 
					SampleLevel = 0;break;
				case 3: 
					SampleLevel = 0;break;
				default: 
					//pMbHoldData->nMeasureChoose = 55;
					//Check_Reset();
					return MEASURE_ERROR;		// 错误直接返回量程错误
					break;
			}
			osDelay(200);
			
			// step2：
			switch(pMbHoldData->RW50)						// 判断工作状态
			{
				case 1:
					error = PumpIn_Sample(600, 400, 2, SampleLevel);				// 进水样
					break;	
				case 2:	
					error = PumpIn_Sample(600, 400, 14, SampleLevel);				// 进核查液体
					break;	
				case 3:	
					error = PumpIn_Sample(600, 400, 5, SampleLevel);				// 进零点核查
					break;	
				case 4:																// 跨度核查
					if(pMbHoldData->nValueChoose == 14)
						error = PumpIn_Sample(600, 400, 14, SampleLevel);			// 开核查阀
					else if(pMbHoldData->nValueChoose == 8)
						error = PumpIn_Sample(600, 400, 8, SampleLevel);			// 开量程阀
					break;
				case 5:
					error = PumpIn_Sample(600, 400, 2, SampleLevel);				// 空白测试
					break;	
				case 6:	
					error = PumpIn_Sample(600, 400, 2, SampleLevel);				// 平行样测试
					break;
				case 21:	
					error = PumpIn_Sample(600, 400, 5, SampleLevel);				// 两点标定转水样，先进蒸馏水
					break;
				case 23:															// 维护测试
					if(pMbHoldData->nValueChoose == 14)
						error = PumpIn_Sample(600, 400, 14, SampleLevel);			// 开核查阀
					else if(pMbHoldData->nValueChoose == 8)
						error = PumpIn_Sample(600, 400, 8, SampleLevel);			// 开量程阀
					else if(pMbHoldData->nValueChoose == 5)
						error = PumpIn_Sample(600, 400, 5, SampleLevel);			// 开蒸馏水阀
					else if(pMbHoldData->nValueChoose == 2)
						error = PumpIn_Sample(600, 400, 2, SampleLevel);			// 开水样阀
					break;
				case 24:
					error = PumpIn_Sample(600, 400, 5, SampleLevel);				// 进蒸馏水
					break;	
				case 25:	
					error = PumpIn_Sample(600, 400, 8, SampleLevel);				// 进量程液
					break;	
				default:	
					return WORKEMODE_ERROR;											// 错误返回工作模式错误		
					break;
			}
			
			//if(error != SYS_OK) return error;				// 如果有错误返回错误码
			
			AutoStep = 6;	// 步骤跳转到模块6
			osDelay(500);
			break;			// 跳出模块5
		}
		
		/* 模块6: 进试剂1 */
		while(AutoStep == 6)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			error = PumpIn_Sample(600, 400, 3, 0);			// 进试剂1
			if(error != SYS_OK) return error;				// 如果有错误就返回错误码
			
			pMbHoldData->nReagent1Volume -= 1;				// 试剂1体积减去1ml
			if(pMbHoldData->nReagent1Volume <= 50) return Reagent1OUT_ERROR;	// 试剂1需要补充
			
			AutoStep = 7;	// 步骤跳转到模块7
			osDelay(500);
			break;			// 跳出模块6
		}
		
		/* 模块7: 加热消解-控温120°300秒 */
		while(AutoStep == 7)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1：光谱仪开光源
			OpenSpectroLight();
			
			// step2：温控
			pMbHoldData->nTestTemper = 120;				// 120摄氏度
			pMbHoldData->nTestTemperHoldtime = 5;		// 300秒(5分钟)
			
			pMbHoldData->fTemp1Kp = 3600;				// 温控PID
			pMbHoldData->fTemp1Ki = 8;
			pMbHoldData->fTemp1Kd = 800;
			
			ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
			osDelay(1000);
			//ProcTempCtrl(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);			
			
			while(1)
			{
				// 这里应该判断比色皿温度是否达到设定值120，达到则跳出循环
				if(pMbHoldData->fPoolTemp >= (pMbHoldData->nTestTemper - 1))
				{
					break;
				}
				else if(TempCtrlData[0].nConstTempTimerFlag == 1)	// 温控时间超时
				{
					// 如果温控时间超出了设定值还没退出说明温控到不了120°
					TempCtrlData[0].nConstTempTimerFlag = 0;	// 先清空标志位
					return HOT_TIMEOUT_ERROR;					// 加热到不到120°
				}
				/* 这里还应该加一个判断超时错误函数 */
				osDelay(200);
			}
			
			// step3：停止温控关闭光谱仪光源
			StopTempCtrl(TEMP_CTRL_CH_POOL);	// 停止温控
			CloseSpectroLight();				// 关闭光谱仪光源
			
			// step4：控制温度回到50摄氏度
			Valve_Operation(11, 1);				// 打开冷却风扇：阀位置为11
			
			while(1)
			{
				// 这里应该判断比色皿温度是否降到50，达到则跳出循环
				if(pMbHoldData->fPoolTemp <= 50)
				{
					break;
				}
				StopTempCtrl(TEMP_CTRL_CH_POOL);	// 停止温控
				osDelay(100);
				
				// 这里需要用寄存器的方式开电风扇
				pMbHoldData->nCombiValve |= (0x00000001 << 10);	// 设置冷却风扇阀的寄存器操作位
				OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
				
				osDelay(100);
			}
			Valve_Operation(11, 0);								// 关闭冷却风扇：阀位置为11
			
			AutoStep = 8;	// 步骤跳转到模块8
			osDelay(500);
			break;			// 跳出模块7
		}
		
		/* 模块8: 进蒸馏水 */
		while(AutoStep == 8)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			error = PumpIn_Sample(600, 400, 5, (!SampleLevel));		// 进蒸馏水,和采样液位相反
			if(error != SYS_OK) return error;						// 如果有错误返回错误码
			
			AutoStep = 9;	// 步骤跳转到模块9
			osDelay(500);
			break;			// 跳出模块8
		}
		
		/* 模块9: 进试剂2 */
		while(AutoStep == 9)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			error = PumpIn_Sample(600, 400, 6, 0);			// 进试剂2
			if(error != SYS_OK) return error;				// 如果有错误就返回错误码
			
			pMbHoldData->nReagent2Volume -= 1;	// 试剂2体积减1ml
			if(pMbHoldData->nReagent2Volume <= 50) return Reagent2OUT_ERROR;	// 试剂2需要补充错误码
			
			AutoStep = 10;	// 步骤跳转到模块10
			osDelay(500);
			break;			// 跳出模块9
		}
		
		/* 模块10: 浊度补偿V3 */
		while(AutoStep == 10)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			/* 考虑加一个浊度补偿 */
			
			AutoStep = 11;	// 步骤跳转到模块11
			osDelay(500);
			break;			// 跳出模块10
		}
		
		/* 模块11: 进试剂3 */
		while(AutoStep == 11)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			error = PumpIn_Sample(600, 400, 7, 0);		// 进试剂3
			if(error != SYS_OK) return error;				// 如果有错误就返回错误码
			
			pMbHoldData->nReagent3Volume -= 1;	// 试剂2体积减1ml
			if(pMbHoldData->nReagent3Volume <= 50) return Reagent3OUT_ERROR;	// 试剂2需要补充错误码
			
			// 鼓泡(往比色皿进空气10秒)
			PumpIn_Cuvettes(1500,250);			// 鼓泡（算出时间为 10秒）
			
			AutoStep = 12;	// 步骤跳转到模块12
			osDelay(500);
			break;			// 跳出模块11
		}
		
		
		/* 模块12: 静置反应-定时300秒，控温50° */
		while(AutoStep == 12)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1：停止温控
			StopTempCtrl(TEMP_CTRL_CH_POOL);
			osDelay(1000);
			
			// step2：温控50摄氏度恒温
			pMbHoldData->nTestTemper = 50;				// 50摄氏度
			
			pMbHoldData->fTemp1Kp = 3800;				// 温控PID
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
			osDelay(500);
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
			
			while(1)
			{
				// 这里应该判断比色皿温度是否达到设定值50，达到则跳出循环
				if(pMbHoldData->fPoolTemp >= 49.5)
				{
					break;
				}
				/* 这里还应该加一个判断温度错误的几种情况 */
				osDelay(500);
			}
			
			// step3: 等待比色皿温度小于50摄氏度读取V2值
			StopTempCtrl(TEMP_CTRL_CH_POOL);			// 停止温控
			osDelay(500);
			Valve_Operation(11, 1);						// 打开冷却风扇：阀位置为11
			osDelay(500);
			while(1)
			{
				// 这里应该判断比色皿温度是否达到设定值50，达到则跳出循环
				if(pMbHoldData->fPoolTemp <= 49.5)
				{
					break;
				}
				/* 这里还应该加一个判断温度错误的几种情况 */
				osDelay(500);
			}
			Valve_Operation(11, 0);						// 打开冷却风扇：阀位置为11
			
			AutoStep = 13;	// 步骤跳转到模块13
			osDelay(500);
			break;			// 跳出模块12
		}
		
		/* 模块13: 出V2值 */
		while(AutoStep == 13)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1：十次AD取平均
			float sum2 = 0;	// 十次取AD值
			float Vdq2 = 0;	// Vdq2采样平均
			for(uint8_t i=0; i < 10; i++)
			{
				sum2 += pMbInputData->nPoolTranLtAD;
				osDelay(200);
			}
			Vdq2 = sum2 / 10;				// 取10次算平均值
			pMbInputData->RW81 = Vdq2;		// 取比色皿电压V2
			
			AutoStep = 14;	// 步骤跳转到模块14
			osDelay(500);
			break;			// 跳出模块13
		}
		
		/* 模块14: 排液清洗 */
		while(AutoStep == 14)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1：排液
			while(loopNum < 3)
			{
				PumpOut_Cuvettes(800,250);		// 排空比色皿
				PumpOut_Tube(1200, 250, 1);		// 排空计量管
				loopNum ++;
			}
			loopNum = 0;						// 清除循环标志位
			
			// step2：清洗
			while(loopNum < 2)
			{
				// 进2次水
				PumpInWater_Tube(800, 200);		// 进蒸馏水到计量管
				PumpIn_Cuvettes(800,250);			// 进比色皿
				PumpInWater_Tube(800, 200);		// 进蒸馏水到计量管
				PumpIn_Cuvettes(800,250);			// 进比色皿
				
				// 鼓泡(往比色皿进空气10秒)
				PumpIn_Cuvettes(1500,250);			// 鼓泡（算出时间为 10秒）
				
				// 排3次水
				for(int i=0; i < 3; i++)
				{
					PumpOut_Cuvettes(800,250);		// 排空比色皿
					PumpOut_Tube(1000, 250, 1);		// 排空计量管
					loopNum ++;
				}
				
				loopNum ++;
			}
			loopNum = 0;
			
			// step3：进2计量管蒸馏水
			// 进2次水
			PumpInWater_Tube(800, 200);		// 进蒸馏水到计量管
			PumpIn_Cuvettes(800,250);			// 进比色皿
			PumpInWater_Tube(800, 200);		// 进蒸馏水到计量管
			PumpIn_Cuvettes(800,250);			// 进比色皿
			AutoStep = 15;	// 步骤跳转到模块15
			osDelay(500);
			break;			// 跳出模块14
		}
		
		/* 模块15: 控温50°，不限时间，出V1值 */
		while(AutoStep == 15)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1：停止温控
			StopTempCtrl(TEMP_CTRL_CH_POOL);
			osDelay(1000);
			
			// step2：温控50摄氏度恒温
			pMbHoldData->nTestTemper = 50;				// 50摄氏度
			
			pMbHoldData->fTemp1Kp = 3800;				// 温控PID
			pMbHoldData->fTemp1Ki = 6;
			pMbHoldData->fTemp1Kd = 800;
			
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
			osDelay(500);
			ProcTempCtrlWithoutHoldTime(false, TEMP_CTRL_CH_POOL, (uint8_t)pMbHoldData->nTestTemper, (int16_t)pMbHoldData->nTestTemperHoldtime);
			
			while(1)
			{
				// 这里应该判断比色皿温度是否达到设定值50，达到则跳出循环
				if(pMbHoldData->fPoolTemp >= 49.8)
				{
					break;
				}
				/* 这里还应该加一个判断温度错误的几种情况 */
				osDelay(500);
			}
			
			// step3: 等待比色皿温度小于50摄氏度读取V2值
			StopTempCtrl(TEMP_CTRL_CH_POOL);			// 停止温控
			osDelay(100);
			StopTempCtrl(TEMP_CTRL_CH_POOL);			// 再发一次停止温控
			Valve_Operation(11, 1);						// 打开冷却风扇：阀位置为11
			osDelay(500);
			while(1)
			{
				// 这里应该判断比色皿温度是否达到设定值50，达到则跳出循环
				if(pMbHoldData->fPoolTemp <= 49.8)
				{
					break;
				}
				/* 这里还应该加一个判断温度错误的几种情况 */
				
				StopTempCtrl(TEMP_CTRL_CH_POOL);	// 停止温控
				osDelay(100);
				
				// 这里需要用寄存器的方式开电风扇
				pMbHoldData->nCombiValve |= (0x00000001 << 10);	// 设置冷却风扇阀的寄存器操作位
				OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
				
				osDelay(100);
			}
			Valve_Operation(11, 0);						// 关闭冷却风扇：阀位置为11
			
			AutoStep = 16;	// 步骤跳转到模块16
			osDelay(500);
			break;			// 跳出模块15
		}
		
		/* 模块16: 控温50°，不限时间，出V1值 */
		while(AutoStep == 16)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1: 确保温度到达可执行范围 
			while(1)
			{
				// 这里应该判断比色皿温度是否达到设定值50，达到则跳出循环
				if(pMbHoldData->fPoolTemp <= 49.5)
				{
					break;
				}
				/* 这里还应该加一个判断温度错误的几种情况 */
				osDelay(100);
				StopTempCtrl(TEMP_CTRL_CH_POOL);	// 停止温控
			}
			
			// step2: 命令20
			ProcReadYSADs(10);
			osDelay(500);
			
			// step2：出值V1
			float sum1 = 0;	// 十次取AD值
			float Vdq1 = 0;	// Vdq1采样平均
			for(uint8_t i=0; i < 10; i++)
			{
				sum1 += pMbInputData->nPoolTranLtAD;	// 比色电压3X14寄存器
				osDelay(200);
			}
			Vdq1 = sum1 / 10;				// 取10次算平均值
			pMbInputData->RW82 = Vdq1;		// 取比色皿电压V1
			
			AutoStep = 17;	// 步骤跳转到模块17
			osDelay(500);
			break;			// 跳出模块16
		}
		
		/* 模块17: 计算获取吸光度 */
		while(AutoStep == 17)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1: 计算吸光度
			float absorbanc = 0;	
			uint16_t V1 = pMbInputData->RW82;	// 获取V1电压
			uint16_t V2 = pMbInputData->RW81;	// 获取V2电压
			
			absorbanc = log10((float)V1/(float)V2);	// 算吸光度
			osDelay(200);
			pMbHoldData->fAbsorbanc = absorbanc;	// 写入对应吸光度寄存器
			
			AutoStep = 18;	// 步骤跳转到模块18
			osDelay(300);
			break;			// 跳出模块17
		}
		
		/* 模块18: 流程结束关闭外设 */
		while(AutoStep == 18)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1: 关闭所有
			CloseAllValves();					// 关闭所有阀
			StopPump();							// 停止蠕动泵
			StopTempCtrl(TEMP_CTRL_CH_POOL);	// 停止温控
			
			AutoStep = 19;	// 步骤跳转到模块19
			osDelay(500);
			break;			// 跳出模块18
		}
		
		/* 模块19：流程结束后计算并判断下一工作 */
		while(AutoStep == 19)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1：判断在哪种工作情况
			if(pMbHoldData->RW50 != 21 && pMbHoldData->RW50 != 24 && pMbHoldData->RW50 != 25 && pMbHoldData->RW50 != 19)	// 如果不是零标或量标或核查
			{
				// 计算水样浓度检测值
				if(pMbHoldData->nZeroComplet == 1 && pMbHoldData->nMeasureComplet == 1)	// 判断曲线事先拟合与否
				{
					// 初始化计算
					float SampleValue = 0;								// 初始化检测值
					float K = pMbHoldData->fK;							// 初始化K
					float b = pMbHoldData->fB;							// 初始化b
					float SampleAbsorbanc = pMbHoldData->fAbsorbanc;	// 初始化水样吸光度
					
					SampleValue = K * SampleAbsorbanc + b;
					pMbHoldData->fConcentrationVale = SampleValue;		// 把计算值写入检测值寄存器
					
					osDelay(100);
					// 更新屏幕历史数据
					pMbHoldData->nSampleComplet = 1;					// 样本检测完标志位置1
					
				}
				// 更新屏幕历史数据,并开启定时器
				if(pMbHoldData->nSampleComplet == 1)					// 水样检测完成并成功
				{
					//AutoStep = 1;	
					pMbHoldData->nSampleComplet = 0;					// 从第一步开始循环采水样
					pMbHoldData->nUpdateData = 1;						// 更新日志标志位置1
					osDelay(100);
					
					// step2: 退出采水样等待4小时定时
					AutoStep = 20;						// 跳到20步
					osDelay(500);
					break;			// 跳出模块19
				}
			}
			else
			{
				if(pMbHoldData->RW50 == 24 || pMbHoldData->RW50 == 21 || pMbHoldData->RW50 == 19)	// 如果在零标模式下
				{
					ZeroAbsorbanc = pMbHoldData->fAbsorbanc;			// 全局变量存储零标吸光度
					pMbHoldData->nZeroComplet = 1;						// 零标完成，置标志位
					if(pMbHoldData->RW50 == 21)
					{
						pMbHoldData->nToMode = 1;						// 两点标定转水样
						pMbHoldData->RW50 = 25;							// 开始测量程(两点标定转水样)
						AutoStep = 1;									// 回到第一步开始测量程
					}else if(pMbHoldData->RW50 == 19)	
					{
						pMbHoldData->nToMode = 2;						// 两点标定转维护
						pMbHoldData->RW50 = 25;							// 开始测量程(两点标定转维护)
						AutoStep = 1;									// 回到第一步开始测量程
					}else if(pMbHoldData->RW50 == 24)
					{													// C1标定
						pMbHoldData->RW50 = 0;							// 工作模式转为0
						AutoStep = 0;									// 跳出大循环,等待下一次命令
					}
					
					osDelay(100);		
					SaveUserData();										// 零标结束，保存用户数据
					break;												// 跳出小循环回到第一步
				}		
				else if(pMbHoldData->RW50 == 25)						// 如果在量程模式下C2标定
				{		
					MeasureAbsorbanc = pMbHoldData->fAbsorbanc;			// 全局变量存储量程吸光度
					pMbHoldData->nMeasureComplet = 1;					// 量程完成，置标志位
					
					osDelay(100);
				}
				
				if(pMbHoldData->nZeroComplet == 1 && pMbHoldData->nMeasureComplet == 1)	// 两个条件都满足则可以计算拟合曲线
				{
					// 计算K和b值
					float K = 0;
					float b = 0;
					float y = 0;
					switch(pMbHoldData->nMeasureChoose)
					{
						case 1: y = 2;break;	// 量程1
						case 2: y = 8;break;	// 量程2
						case 3: y = 10;break;	// 量程3
						default: return FITLINE_ERROR;		// 错误直接退出函数
					}
					
					K = (float)y / (MeasureAbsorbanc - ZeroAbsorbanc);
					b = -(ZeroAbsorbanc * K);
					osDelay(100);
					
					// 更新对应寄存器
					pMbHoldData->fK = K;
					pMbHoldData->fB = b;
					osDelay(100);
					
					// 根据对应量程记录k和b值
					switch(pMbHoldData->nMeasureChoose)
					{
						case 1: 										// 如果是量程1
							pMbInputData->fMea1Cali1 = pMbHoldData->fK;	// 量程1标定一次项
							pMbInputData->fMea1Cali0 = pMbHoldData->fB;	// 量程1标定常数项
							break;
						case 2: 										// 如果是量程2
							pMbInputData->fMea2Cali1 = pMbHoldData->fK;	// 量程2标定一次项
							pMbInputData->fMea2Cali0 = pMbHoldData->fB;	// 量程2标定常数项
							break;
						case 3:  										// 如果是量程3
							pMbInputData->fMea3Cali1 = pMbHoldData->fK;	// 量程3标定一次项
							pMbInputData->fMea3Cali0 = pMbHoldData->fB;	// 量程3标定常数项
							break;
						default: 
							pMbHoldData->nMeasureChoose = 55;
							Check_Reset();
							return MEASURE_ERROR;		// 错误直接返回量程错误
					}
					
					// 进行步骤跳转
					if(pMbHoldData->nToMode == 1)
					{
						pMbHoldData->RW50 = 1;						// 标定转水样
						AutoStep = 1;								// 回到第一步
					}else if(pMbHoldData->nToMode == 2)
					{
						pMbHoldData->RW50 = 23;						// 标定转维护
						AutoStep = 1;								// 回到第一步
					}else
					{
						pMbHoldData->RW50 = 0;						// 退出量程标定
						AutoStep = 0;								// 退出量程标定
					}
					
					osDelay(100);
					SaveUserData();									// 量程结束，保存用户数据()
					break;											// 跳出小循环回到第一步
				}
			}
			
		}
		
		/* 模块20: 模式判断 */
		while(AutoStep == 20)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			
			// step1: 关闭所有
			CloseAllValves();					// 关闭所有阀
			StopPump();							// 停止蠕动泵
			StopTempCtrl(TEMP_CTRL_CH_POOL);	// 停止温控
			
			switch(pMbHoldData->nStartMode)
			{
				case 1:						// 连续模式	
					osDelay(100);
					//pMbHoldData->RW50 = 1;	// 回到开始采水样状态
					AutoStep = 1;				
					SaveUserData();			// 保存用户数据
					break;			    	
				case 2:                 	// 周期模式
					osDelay(100);
					pMbHoldData->RW50 = 0;
					AutoStep = 0;			// 跳出循环
			
					/* 开始定时下一次采集水样:改到了大流程的最开头进行定时 */
					//osTimerStart(SampleTimeOutHandle, (pMbHoldData->nClockTime) * 60 * 1000);	// 定时1小时后再来来采水样
					
					break;              	
				case 3:                 			// 定点模式
					Set_MultiAlarm(usCoilBuf);		// 定时为下一个闹钟
					pMbHoldData->RW50 = 0;
					AutoStep = 0;					// 跳出循环(由闹钟中断触发采样)
					break;              	
				case 4:                 	// 受控模式
					pMbHoldData->RW50 = 0;
					AutoStep = 0;			// 跳出循环(等待下一次命令)						
					break;              	
				case 5:                 	// 手动模式	
					pMbHoldData->RW50 = 0;
					AutoStep = 0;			// 跳出循环(等待下一次命令)								
					break;              	
			}
			osDelay(500);
			break;			// 跳出模块20
		}
		
		// 检测大循环是否需要退出（采水样的时候是循环的:在模块20中体现）
		if(AutoStep == 0)
		{
			// 实时步骤
			pMbHoldData->RW4229 = AutoStep;
			osDelay(20);
			break;
		}
	}
	SaveUserData();						// 保存用户数据
	return SYS_OK;						// 运行正常
}

/*******************************************************************************
* Function Name  	: 程序急停检查
* Description    	: 
* Input          	: 

* Output         	:
* Return         	: 返回1为检测到复位标志
*******************************************************************************/
uint8_t Check_Reset(void)
{
	if(pMbHoldData->nResetRun == 1)	// 如果检测到复位了
	{
		pMbInputData->nDevBeRunning = 0;		// 机器停止，表示正常复位
		CloseAllValves();
		StopPump();
		StopTempCtrl(TEMP_CTRL_CH_POOL);		// 停止温控
		osTimerStop(SampleTimeOutHandle);		// 关闭间歇性采样定时器
		pMbHoldData->nResetRun = 0;				// 清空复位标志位
		pMbHoldData->RW50 = 0;					// 清空工作模式
		pMbHoldData->RW4229 = 0;				// 步骤置为0
		
		error = SYS_OK;							// 系统正常运行
		pMbInputData->nFunErrorCode = error;	
		return 1;
	}
	else
	{
		return 0;
	}
}

/*******************************************************************************
* Function Name  	: 阀操作 2025.6.20
* Description    	: 
* Input          	: regBit:寄存器位（对应excel）	;direction:方向(1:开阀，0:关阀)

* Output         	:
* Return         	:
*******************************************************************************/
void Valve_Operation(uint8_t regBit, uint8_t state)
{
	pMbHoldData->nCombiValve = (0x00000001 << (regBit - 1));	// 设置清洗废液阀的寄存器操作位
	if(state == 1)
	{
		OpenValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
	}
	else if(state == 0)
	{
		CloseValves(pMbHoldData->nCombiValve);			// 打开对应设置操作为的阀门
	}
}

/*******************************************************************************
* Function Name  	: 排废液或清洗废液 2025.6.20
* Description    	: 
* Input          	: speed:速度（自动设为负）; lapNum：圈数 ;waterState: 有RW577决定(为1则开清洗废液阀)

* Output         	:
* Return         	:
*******************************************************************************/
void PumpOut_WasteWater(int16_t speed, uint16_t lapNum, uint8_t waterState)
{
	// step1:开阀
	Valve_Operation(13, 1);		// 开废液阀
	if(waterState == 1)
		Valve_Operation(1, 1);	// 开清洗废液阀
	osDelay(500);				// 开阀时间裕度
	
	// step2:排水
	if(speed >= 0)
		pMbHoldData->nStepperMotorSpeed = -speed;		
	else
		pMbHoldData->nStepperMotorSpeed = speed;	// 设置蠕动泵的速度寄存器位（负数为排液）
	pMbHoldData->nPeristalticPumpParm = lapNum;		// 设置蠕动泵的转圈数寄存器位
	StopPump();
	osDelay(300);
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
	osDelay(500);
	while(!TimeoutFlag)
	{
		osDelay(100);		 // 等待蠕动泵操作完成
	}
	TimeoutFlag = false;	 // 清除电机运行完标志位
	osDelay(500);
	
	// step3:关阀
	Valve_Operation(1, 0);	// 关清洗废液阀
	Valve_Operation(13, 0);	// 关废液阀
	osDelay(500);			// 关阀时间裕度
}

/*******************************************************************************
* Function Name  	: 抽比色皿 2025.6.20
* Description    	: 
* Input          	: speed:速度（正数排液）; lapNum：圈数

* Output         	:
* Return         	:
*******************************************************************************/
void PumpOut_Cuvettes(int16_t speed,uint16_t lapNum)
{
	// step1: 开阀
	Valve_Operation(4, 1);	// 开比色阀
	Valve_Operation(9, 1);	// 开消解下阀
	Valve_Operation(10, 1);	// 开消解上阀
	osDelay(500);			// 开阀时间裕度
	
	// step2: 抽液
	if(speed >= 0)
		pMbHoldData->nStepperMotorSpeed = speed;		
	else
		pMbHoldData->nStepperMotorSpeed = -speed;	// 设置蠕动泵的速度寄存器位（正数为抽）
	pMbHoldData->nPeristalticPumpParm = lapNum;		// 设置蠕动泵的转圈数寄存器位
	StopPump();
	osDelay(300);
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
	osDelay(500);
	while(!TimeoutFlag)
	{
		osDelay(100);	// 等待蠕动泵操作完成
	}
	TimeoutFlag = false; // 清除电机运行完标志位
	osDelay(500);
	
	// step3: 关阀
	Valve_Operation(4, 0);	// 关比色阀
	Valve_Operation(9, 0);	// 关消解下阀
	Valve_Operation(10, 0);	// 关消解上阀
	osDelay(500);			// 关阀时间裕度
}

/*******************************************************************************
* Function Name  	: 排空计量管(增、泄压排空) 2025.6.20
* Description    	: 
* Input          	: speed:速度（负数排液）; lapNum：圈数; waterState: 有RW577决定(为1则开清洗废液阀)

* Output         	:
* Return         	:
*******************************************************************************/
void PumpOut_Tube(int16_t speed,uint16_t lapNum, uint8_t waterState)
{
	// step1: 增压
	if(speed >= 0)
		pMbHoldData->nStepperMotorSpeed = -speed;		
	else
		pMbHoldData->nStepperMotorSpeed = speed;	// 设置蠕动泵的速度寄存器位（负数为排）
	pMbHoldData->nPeristalticPumpParm = lapNum;		// 设置蠕动泵的转圈数寄存器位
	StopPump();
	osDelay(300);
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
	osDelay(500);
	while(!TimeoutFlag)
	{
		osDelay(100);	// 等待蠕动泵操作完成
	}
	TimeoutFlag = false; // 清除电机运行完标志位
	osDelay(500);
	
	// step2: 开阀
	Valve_Operation(13, 1);		// 开废液阀
	if(waterState == 1)
		Valve_Operation(1, 1);	// 开清洗废液阀
	osDelay(500);				// 开阀时间裕度
	
	// step3: 开阀后泄压
	if(speed >= 0)
		pMbHoldData->nStepperMotorSpeed = -speed;		
	else
		pMbHoldData->nStepperMotorSpeed = speed;	// 设置蠕动泵的速度寄存器位（负数为排）
	pMbHoldData->nPeristalticPumpParm = lapNum;		// 设置蠕动泵的转圈数寄存器位
	StopPump();
	osDelay(300);
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
	osDelay(500);
	while(!TimeoutFlag)
	{
		osDelay(100);	// 等待蠕动泵操作完成
	}
	TimeoutFlag = false; // 清除电机运行完标志位
	osDelay(500);
	
	// step4: 关阀
	Valve_Operation(1, 0);	// 关废液阀
	Valve_Operation(13, 0);	// 关清洗废液阀
	osDelay(500);			// 关阀时间裕度
}

/*******************************************************************************
* Function Name  	: 采蒸馏水到计量管 2025.6.20
* Description    	: 
* Input          	: speed:速度（负数排液）; lapNum：圈数; waterState: 有RW577决定(为1则开清洗废液阀)

* Output         	:
* Return         	:
*******************************************************************************/
void PumpInWater_Tube(int16_t speed,uint16_t lapNum)
{
	CloseAllValves();					// 关闭所有阀
	Valve_Operation(5, 1);				// regBit对应阀打开
	osDelay(300);						// 开阀时间裕度
	
	pMbHoldData->nStepperMotorSpeed = speed;			// 设置蠕动泵的速度寄存器位（正数为抽）:固定值
	pMbHoldData->nPeristalticPumpParm = lapNum;			// 设置蠕动泵的转圈数寄存器位:固定值
	StopPump();
	osDelay(300);
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
	osDelay(500);
	while(!TimeoutFlag)
	{
		osDelay(100);	// 等待蠕动泵操作完成
	}
	TimeoutFlag = false; // 清除电机运行完标志位
	osDelay(500);
	
	Valve_Operation(5, 0);				// regBit对应阀关闭
	osDelay(300);						// 开阀时间裕度
}


/*******************************************************************************
* Function Name  	: 进液（进入比色皿）（采样） 2025.6.20
* Description    	: 
* Input          	: speed:速度（负数为抽液）; lapNum：圈数

* Output         	:
* Return         	:
*******************************************************************************/
void PumpIn_Cuvettes(int16_t speed,uint16_t lapNum)
{
	Valve_Operation(4, 1);	// 开比色阀
	Valve_Operation(9, 1);	// 开消解下阀
	Valve_Operation(10, 1);	// 开消解上阀
	osDelay(500);			// 开阀时间裕度
	
	if(speed >= 0)
		pMbHoldData->nStepperMotorSpeed = -speed;		
	else
		pMbHoldData->nStepperMotorSpeed = speed;	// 设置蠕动泵的速度寄存器位（正数为抽）
	pMbHoldData->nPeristalticPumpParm = lapNum;		// 设置蠕动泵的转圈数寄存器位
	StopPump();
	osDelay(300);
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
	osDelay(500);
	while(!TimeoutFlag)
	{
		osDelay(100);	// 等待蠕动泵操作完成
	}
	TimeoutFlag = false; // 清除电机运行完标志位
	osDelay(500);
	
	Valve_Operation(4, 0);	// 关比色阀
	Valve_Operation(9, 0);	// 关消解下阀
	Valve_Operation(10, 0);	// 关消解上阀
	osDelay(500);			// 关阀时间裕度
}

/*******************************************************************************
* Function Name  	: 采样 2025.6.20
* Description    	: 
* Input          	: speed:速度; lapNum：圈数 ;regBit: 阀寄存器位; 
					  AD_Level: 判断计量管高低液位(0为判断低液位，1为判断高液位)

* Output         	:
* Return         	:
*******************************************************************************/
nErrorCode PumpIn_Sample(int16_t speed, uint16_t lapNum, uint8_t regBit, uint8_t AD_Level)
{
	uint8_t loopNum = 0;	// 定义一个循环变量用来计次
	
	// 参数定义
	uint16_t AD_Low;		// 定义3X15液位1AD值
	uint16_t AD_High;		// 定义3X16液位2AD值
	uint8_t AD_LowFlag; 	// 低液位合理标志位
	uint8_t AD_HighFlag;	// 高液位合理标志位
	
	uint16_t RW43_Num;		// 获取保持寄存器RW43清洗次数
	
	// step1:增、泄压排空管路(开清水阀)
	PumpOut_Tube(1200, 250, 1);
	
	// step2:采样前信号判断（有误则进入重新进行AD校准）
	if(pMbHoldData->RW34+50<=pMbInputData->nLL1AD && pMbInputData->nLL1AD<=pMbHoldData->RW32-50 &&
		pMbHoldData->RW35+50<=pMbInputData->nLL2AD && pMbInputData->nLL2AD<=pMbHoldData->RW33-50)
	{
		// step2.1：液位信号异常清洗管道
		while(loopNum < 5)
		{
			PumpInWater_Tube(800, 200);						// 进蒸馏水
			osDelay(300);									// 关阀时间裕度
			PumpOut_WasteWater(-1200, 250, 1);				// 排废液
			
			if(pMbHoldData->RW34+50<=pMbInputData->nLL1AD && pMbInputData->nLL1AD<=pMbHoldData->RW32-50 &&
				pMbHoldData->RW35+50<=pMbInputData->nLL2AD && pMbInputData->nLL2AD<=pMbHoldData->RW33-50)
			{
				break;
			}
			
			loopNum ++;
		}
		
		// 判断液位洗涤的次数是否大于5次。如果大于5次，进行AD液位的重新校准
		if(loopNum >= 5)
		{
			loopNum = 0;		// 清0循环标志位
			// step2.2:AD校准
			while(1)	// 这里可以用死循环
			{
				pMbInputData->nSysErrorCode = CheckLevelsAD();	// 液位定位AD校准
				
				while(pMbInputData->nSysErrorCode != PUMP_LIQUID_OK)
				{
					osDelay(100);	// 等待液位定位AD校准完成
				}
				
				AD_Low  = pMbInputData->nLL1AD;	// 获取3X15低位信号
				AD_High = pMbInputData->nLL2AD;	// 获取3X16高位信号
				
				AD_LowFlag  = (AD_Low >= 1200) && (AD_Low <= 1400); 	// 低液位合理标志位
				AD_HighFlag = (AD_High >= 1200) && (AD_High <= 1400);	// 高液位合理标志位
				
				if(AD_LowFlag && AD_HighFlag)	// 判断高低位信号是否在规定范围
				{
					break;						// 标定准确，跳出循环执行下一步
				}
				
				loopNum ++;
				
				/* 这里可以判断一下loopNum次数，或者在死循环里到一定次数后就跳出并处理错误代码 */
				if(loopNum >= 3)				// 三次液位标定后还是没有标定准确，就返回错误码
				{
					return AD_CALIBRATION_ERROR;	// 标定错误
				}
			}
			loopNum = 0;	// 清空循环标志位
			
			// step2.3:赋值空杯电压
			pMbHoldData->RW30 = AD_Low;		// RW30低位空杯电压=3x15地位信号
			pMbHoldData->RW31 = AD_High;	// RW31低位空杯电压=3x16地位信号
			
			pMbHoldData->RW32 = AD_Low * 1.2;	// RW32低水位阀值=3x15地位信号*1.2
			pMbHoldData->RW33 = AD_High * 1.2;	// RW33低水位阀值=3x16地位信号*1.2
			
			pMbHoldData->RW34 = AD_Low * 0.5;	// RW34低水位阀值=3x15地位信号*0.5
			pMbHoldData->RW35 = AD_High * 0.5;	// RW34低水位阀值=3x15地位信号*0.5
			
			SaveUserData();						// 保存用户数据
			osDelay(500);						// 数据保存时间裕度
		}
		loopNum = 0;	// 清0循环标志位
	}
	
//	// step2:AD校准
//	while(loopNum < 4)	// 这里可以用死循环
//	{
//		pMbInputData->nSysErrorCode = CheckLevelsAD();	// 液位定位AD校准
//		
//		while(pMbInputData->nSysErrorCode != PUMP_LIQUID_OK)
//		{
//			osDelay(100);	// 等待液位定位AD校准完成
//		}
//		
//		AD_Low  = pMbInputData->nLL1AD;	// 获取3X15低位信号
//		AD_High = pMbInputData->nLL2AD;	// 获取3X16高位信号
//		
//		AD_LowFlag  = (AD_Low >= 1200) && (AD_Low <= 1400); 	// 低液位合理标志位
//		AD_HighFlag = (AD_High >= 1200) && (AD_High <= 1400);	// 高液位合理标志位
//		
//		if(AD_LowFlag && AD_HighFlag)	// 判断高低位信号是否在规定范围
//		{
//			break;	// 标定准确，跳出循环执行下一步
//		}
//		
//		loopNum ++;
//	}
//	/* 这里可以判断一下loopNum次数，或者在死循环里到一定次数后就跳出并处理错误代码 */
//	loopNum = 0;	// 清空循环标志位
//	
//	// step3:赋值空杯电压
//	pMbHoldData->RW30 = AD_Low;		// RW30低位空杯电压=3x15地位信号
//	pMbHoldData->RW31 = AD_High;	// RW31低位空杯电压=3x16地位信号
//	
//	pMbHoldData->RW32 = AD_Low * 1.2;	// RW32低水位阀值=3x15地位信号*1.2
//	pMbHoldData->RW33 = AD_High * 1.2;	// RW33低水位阀值=3x16地位信号*1.2
//	
//	pMbHoldData->RW34 = AD_Low * 0.5;	// RW34低水位阀值=3x15地位信号*0.5
//	pMbHoldData->RW35 = AD_High * 0.5;	// RW34低水位阀值=3x15地位信号*0.5
//	
//	SaveUserData();						// 保存用户数据
//	osDelay(500);						// 数据保存时间裕度
	
	// step4：润洗计量管(水样、蒸馏水、量程、核查需要润洗)
	if((regBit == 2)||(regBit == 5)||(regBit == 8)||(regBit == 14))
	{
//		RW43_Num = pMbHoldData->RW43;
//		RW43_Num = (RW43_Num > 2 ? RW43_Num : 2);			// 保证润洗次数大于两次(固定次数)
//		pMbHoldData->RW43 = RW43_Num;						// 反向赋值给寄存器
		
		RW43_Num = 2;
		while(loopNum < 2)
		{
			CloseAllValves();								// 关闭所有阀
			
			if(regBit == 8) Valve_Operation(14, 1);			// 如果要开量程，那么核查阀也要打开
			Valve_Operation(regBit, 1);						// regBit对应阀打开
			osDelay(300);									// 开阀时间裕度
		
			pMbHoldData->nStepperMotorSpeed = 1000;			// 设置蠕动泵的速度寄存器位（正数为抽）:固定值
			pMbHoldData->nPeristalticPumpParm = 250;		// 设置蠕动泵的转圈数寄存器位:固定值
			StopPump();
			osDelay(300);
			StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
			osDelay(500);
			while(!TimeoutFlag)
			{
				osDelay(100);	// 等待蠕动泵操作完成
			}
			TimeoutFlag = false; // 清除电机运行完标志位
			osDelay(500);
			
			if(regBit == 8) Valve_Operation(14, 0);			// 如果要开量程，那么核查阀也要关闭
			Valve_Operation(regBit, 0);						// regBit对应阀关闭
			osDelay(300);									// 关阀时间裕度
			
			PumpOut_WasteWater(-1200, 250, 1);				// 排废液
			
			loopNum ++;
		}
		loopNum = 0;	// 清0循环标志位
	}
	
	// step5：进液
	CloseAllValves();								// 关闭所有阀
	if(regBit == 8) Valve_Operation(14, 1);			// 如果要开量程，那么核查阀也要打开
	Valve_Operation(regBit, 1);						// regBit对应阀打开
	osDelay(300);									// 开阀时间裕度
	
	pMbHoldData->nStepperMotorSpeed = 600;			// 设置蠕动泵的速度寄存器位（正数为抽）:固定值
	pMbHoldData->nPeristalticPumpParm = 4000;		// 设置蠕动泵的转圈数寄存器位:固定值
	StopPump();
	osDelay(300);
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
	osDelay(500);
	
	// step6：上行判断
	while(1)	// 不断判断AD液位
	{
		AD_Low  = pMbInputData->nLL1AD;	// 获取3X15低位信号
		AD_High = pMbInputData->nLL2AD;	// 获取3X16高位信号
		
		if(AD_Level == 0)		// AD_Level=0,判断低液位
		{
			if(AD_Low > pMbHoldData->RW32)	// RW32：空杯的1.2倍
			{
				osDelay(1000);	// 延时1秒
				StopPump();		
				osDelay(3000);	// 延时3秒
				break;			// 检测低液位达到标准退出
			}
		}
		else if(AD_Level == 1)	// AD_Level=1,判断高液位
		{
			if(AD_High > pMbHoldData->RW33)
			{
				osDelay(1000);	// 延时1秒
				StopPump();
				osDelay(3000);	// 延时3秒
				break;			// 检测高液位达到标准退出
			}	
		}
		else if(TimeoutFlag == true)
		{
			return AD_UPLEVEL_ERROR;			// 蠕动泵软件定时器超时退出（上行液位采集错误）
		}
		osDelay(100);		// 保证任务实时性
	}
	
	// step7:下行定位判断
	pMbHoldData->nStepperMotorSpeed = -400;			// 设置蠕动泵的速度寄存器位（正数为抽）:固定值
	pMbHoldData->nPeristalticPumpParm =30;			// 设置蠕动泵的转圈数寄存器位:固定值
	StopPump();
	osDelay(300);
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
	osDelay(500);
	while(!TimeoutFlag)
	{
		osDelay(100);	// 等待蠕动泵操作完成
	}
	TimeoutFlag = false; // 清除电机运行完标志位
	osDelay(500);
	
	// step8:上抽定位判断
	pMbHoldData->nStepperMotorSpeed = 60;			// 设置蠕动泵的速度寄存器位（正数为抽）:固定值
	pMbHoldData->nPeristalticPumpParm = 400;		// 设置蠕动泵的转圈数寄存器位:固定值
	StopPump();
	osDelay(300);
	StartPeristalticPump(pMbHoldData->nStepperMotorSpeed, (float)pMbHoldData->nPeristalticPumpParm / 10);	// 操作蠕动泵
	osDelay(500);
	while(1)	// 不断判断AD液位
	{
		AD_Low  = pMbInputData->nLL1AD;	// 获取3X15低位信号
		AD_High = pMbInputData->nLL2AD;	// 获取3X16高位信号
			
		if(AD_Level == 0)							// AD_Level=0,判断低液位
		{
			if(AD_Low < pMbHoldData->RW34)
			{
				StopPump();
				if(regBit == 8) Valve_Operation(14, 0);			// 如果要开量程阀位，那么核查阀也要关闭
				Valve_Operation(regBit, 0);						// regBit对应阀关闭
				break;											// 检测低液位达到标准退出
			}
				
		}
		else if(AD_Level == 1)						// AD_Level=1,判断高液位
		{
			if(AD_High < pMbHoldData->RW35)			// RW35:空杯的0.5倍
			{
				StopPump();
				if(regBit == 8) Valve_Operation(14, 0);			// 如果要开量程阀位，那么核查阀也要关闭
				Valve_Operation(regBit, 0);						// regBit对应阀关闭
				break;											// 检测高液位达到标准退出
			}	
		}
		else if(TimeoutFlag == true)
		{
			return AD_DOWNLEVEL_ERROR;							// 蠕动泵软件定时器超时退出（下行液位采集错误）
		}
		osDelay(100);								// 保证任务实时性
	}
	osDelay(500);									// 保证任务实时性
	
	// step8：进比色皿
	CloseAllValves();					// 关闭所有阀
	PumpIn_Cuvettes(-1200, 250); 		// 进比色皿
	
	// step9: 排空和计量管(只开废液阀)
	PumpOut_Tube(-1200, 250, 0);		// 增、泄压排空计量管
	
	// step10：一切正常返回正常值
	return SYS_OK;
}

/*******************************************************************************
* Function Name  	: 采样完定时4小时
* Description    	: FreeRtos软件定时器回调函数
* Input          	: 

* Output         	:
* Return         	:
*******************************************************************************/
void SampleTimeOutCallback(void *argument) {
    // 在此添加4小时超时后的处理逻辑
	
	static uint8_t times = 0;		// 定义一个全局变量来记录定时了几个小时
	
	if(pMbHoldData->nStartMode == 2)// 如果工作在周期模式下,每小时times自增
		times++;		
	
	if(times == 5)						// 如果定时已经到了4个小时，就开始重新进行零标量标
	{
		AutoStep = 1;					// 回到第一步
		pMbHoldData->nUpdateData = 0;	// 清空水样计算完成日志
		pMbHoldData->RW50 = 21;			// 两点标定转水样，工作状态
		pMbHoldData->nAutoRun = 21;		// 打开采水样样流程
		pMbHoldData->nZeroComplet = 0;		// 清空零点已完成标志位
		pMbHoldData->nMeasureComplet = 0;	// 清空量程已完成标志位
		times = 0;							// 开始下一轮四小时定时，采水样
	}
	else								// 如果没定时到4小时，接着测水样
	{
		AutoStep = 1;					// 回到第一步
		pMbHoldData->nUpdateData = 0;	// 清空水样计算完成日志
		pMbHoldData->RW50 = 1;			// 开始采水样
		pMbHoldData->nAutoRun = 1;		// 打开采水样样流程
	}
	
}

/*******************************************************************************
 * Function Name  : 上电启动模式的判断初始化
 * Description    :
 * Input          :
 * Output         : 无
 * Return         :
 *******************************************************************************/
void StartMode_Judgment(void)
{
	switch(pMbHoldData->nStartMode)
	{
		case 1:
			Continue_Mode();	// 连续模式		
			break;				// 连续模式
		case 2: 	
			Periodic_Mode();	// 周期模式					
			break;				// 周期模式
		case 3: 	
			FixedPoint_Mode();	// 定点模式
			break;				// 定点模式
		case 4: 	
			Controlled_Mode();	// 受控模式
			break;				// 受控模式
		case 5: 	
			Manual_Mode();		// 手动模式	
			break;				// 手动模式
		default:
			pMbHoldData->nStartMode = 1;	// 默认为连续模式
			Continue_Mode();				// 连续模式	
			break;
	}
}

/*******************************************************************************
 * Function Name  : 连续模式
 * Description    :	1、如果仪器之前是未运行的则，一直停在该处，等待启动再进入标定检测宏
					2、如果仪器之前是运行的，则直接进入标定检测，继续上次的工作状态
 * Input          :
 * Output         : 无
 * Return         :
 *******************************************************************************/
void Continue_Mode(void)
{
	if(pMbInputData->nDevBeRunning == 1)	// 如果仪器之前是运行的但被断电打断，直接进入标定检测，继续上次工作状态
	{
		pMbHoldData->nAutoRun = pMbHoldData->RW50;	// 让运行步骤回到原来的步骤
		pMbHoldData->nUpdateData = 0;				// 清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 样本检测完标志位置0
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度
	}
	else	// 仪器之前未运行，等待手动启动
	{
		pMbHoldData->nAutoRun = 0;					// 正常停止，一直停在该处，等待启动再进入标定检测宏
		pMbHoldData->RW50 = 0;						// 工作模式为0
		pMbHoldData->nUpdateData = 0;				// 先清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 情况样本检测完
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度	
	}
}

/*******************************************************************************
 * Function Name  : 周期模式
 * Description    :
 * Input          :
 * Output         : 无
 * Return         :
 *******************************************************************************/
void Periodic_Mode(void)
{
	if(pMbInputData->nDevBeRunning == 1)	// 如果仪器之前是运行的但被断电打断，直接进入标定检测，继续上次工作状态
	{
		pMbHoldData->nAutoRun = pMbHoldData->RW50;	// 让运行步骤回到原来的步骤
		pMbHoldData->nUpdateData = 0;				// 清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 样本检测完标志位置0
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度
	}
	else	// 仪器之前未运行，等待手动启动
	{
		pMbHoldData->nAutoRun = 0;					// 正常停止，一直停在该处，等待启动再进入标定检测宏
		pMbHoldData->RW50 = 0;						// 工作模式为0
		pMbHoldData->nUpdateData = 0;				// 先清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 情况样本检测完
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度	
	}
}

/*******************************************************************************
 * Function Name  : 定点模式
 * Description    :	整点采样
 * Input          :
 * Output         : 无
 * Return         :
 *******************************************************************************/
void FixedPoint_Mode(void)
{
	/* 四个小时一次测零标量标 */
	
	
	
	
	/* *********************************************** */
	
	if(pMbInputData->nDevBeRunning == 1)	// 如果仪器之前是运行的但被断电打断，直接进入标定检测，继续上次工作状态
	{
		pMbHoldData->nAutoRun = pMbHoldData->RW50;	// 让运行步骤回到原来的步骤
		pMbHoldData->nUpdateData = 0;				// 清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 样本检测完标志位置0
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度
	}
	else	// 仪器之前未运行，等待手动启动
	{
		pMbHoldData->nAutoRun = 0;					// 正常停止，一直停在该处，等待启动再进入标定检测宏
		pMbHoldData->RW50 = 0;						// 工作模式为0
		pMbHoldData->nUpdateData = 0;				// 先清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 情况样本检测完
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度	
	}
}

/*******************************************************************************
 * Function Name  : 受控模式
 * Description    :
 * Input          :
 * Output         : 无
 * Return         :
 *******************************************************************************/
void Controlled_Mode(void)
{
	if(pMbInputData->nDevBeRunning == 1)	// 如果仪器之前是运行的但被断电打断，直接进入标定检测，继续上次工作状态
	{
		pMbHoldData->nAutoRun = pMbHoldData->RW50;	// 让运行步骤回到原来的步骤
		pMbHoldData->nUpdateData = 0;				// 清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 样本检测完标志位置0
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度
	}
	else	// 仪器之前未运行，等待手动启动
	{
		pMbHoldData->nAutoRun = 0;					// 正常停止，一直停在该处，等待启动再进入标定检测宏
		pMbHoldData->RW50 = 0;						// 工作模式为0
		pMbHoldData->nUpdateData = 0;				// 先清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 情况样本检测完
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度	
	}
}

/*******************************************************************************
 * Function Name  : 手动模式
 * Description    :
 * Input          :
 * Output         : 无
 * Return         :
 *******************************************************************************/
void Manual_Mode(void)
{
	if(pMbInputData->nDevBeRunning == 1)	// 如果仪器之前是运行的但被断电打断，直接进入标定检测，继续上次工作状态
	{
		pMbHoldData->nAutoRun = pMbHoldData->RW50;	// 让运行步骤回到原来的步骤
		pMbHoldData->nUpdateData = 0;				// 清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 样本检测完标志位置0
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度
	}
	else	// 仪器之前未运行，等待手动启动
	{
		pMbHoldData->nAutoRun = 0;					// 正常停止，一直停在该处，等待启动再进入标定检测宏
		pMbHoldData->RW50 = 0;						// 工作模式为0
		pMbHoldData->nUpdateData = 0;				// 先清空日志更新
		pMbHoldData->nSampleComplet = 0;			// 情况样本检测完
		pMbHoldData->fConcentrationVale = 0.0;		// 初始化检测值	
		pMbHoldData->fAbsorbanc = 0.0;				// 初始化吸光度	
	}
}


