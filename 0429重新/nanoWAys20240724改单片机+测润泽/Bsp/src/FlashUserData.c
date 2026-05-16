

#include "FlashUserData.h"
#include "modbusdata.h"
#include "instrumentinfo.h"
#include "TestLEDCtr.h"
#include "replyhost.h"
#include "stm32f4flashopt.h"
#include "STMcalendar.h"

extern float LL1InitCurrent;	// 光路1的初始电流
extern float LL2InitCurrent;	// 光路2的初始电流
extern float CurrentLEDAbs;		// LED灯的初始电流
extern uint16_t LL1ADBlank;		// 光路1的空白AD值
extern uint16_t LL2ADBlank;		// 光路2的空白AD值

char ch[4] = {0x01, 0x45, 0x9e, 0xb3};	// 用于测试或调试，将一个字符数组映射为浮点数指针
float *fl = (float *)ch;				// 用于测试或调试，将一个字符数组映射为浮点数指针

// MODBUS_BUF_DATA_IN_FLASH AllDevsData.tModbusData;     // 需要保存的数据
ALL_DEV_DATA AllDevsData;				// 存储所有设备的数据，包括Modbus数据和设备操作参数
int SizeOfData = sizeof(ALL_DEV_DATA);	// ALL_DEV_DATA所占用字节数
// DEV_OPERAT_PARA AllDevsOperatPara[30];

DEV_OPERAT_PARA ThisDevOperatPara = {		// 存储设备的操作参数，如工作范围、反应温度、反应时间
	DEV_COD_CR,
	0,				  //	uint16_t nLastWorkRange;
	170,			  //	uint16_t nReactionTemp;
	15,				  //	uint16_t nReactionTime;
	1,				  //	uint16_t nWaterPumpTime;
	2000,			  //	uint16_t nWorkRange1Set;
	5000,			  //	uint16_t nWorkRange2Set;
	0,				  //	uint16_t nWorkRange3Set;
	0,				  //	uint16_t nWorkRange4Set;
	45000,			  // uint16_t nTempCtrlMaxUk;
	0, 0, 0, 0, 1, 0, //	CALIBRATION_PARA RangesCurvePara[4];    //
	0, 0, 0, 0, 1, 0, //	CALIBRATION_PARA RangesCurvePara[4];    //
	0, 0, 0, 0, 1, 0, //	CALIBRATION_PARA RangesCurvePara[4];    //
	0, 0, 0, 0, 1, 0, //	CALIBRATION_PARA RangesCurvePara[4];    //
	1000,			  // float								fTemp1Kp;
	2.5,			  // float								fTemp1Ki;
	1000			  // float								fTemp1Kd;
};


/****************************************************************************
 *  1: 指向 Modbus 数据结构中的离散输入缓冲区:用于读取和操作离散输入寄存器的数据。离散输入通常用于表示数字输入信号的状态（如开关、按钮等）
 *  2: 指向 Modbus 数据结构中的线圈缓冲区:用于读取和操作线圈寄存器的数据。线圈通常用于控制数字输出信号（如继电器、指示灯等）
 *  3: 指向 Modbus 数据结构中的保持寄存器缓冲区:用于读取和操作保持寄存器的数据。保持寄存器用于存储需要保持的数据，即使设备断电后数据也不会丢失
 *  4: 指向 Modbus 数据结构中的输入寄存器缓冲区:用于读取输入寄存器的数据。输入寄存器通常用于存储从外部设备读取的数据，这些数据通常是只读的
 *  5: 将保持寄存器缓冲区的地址强制转换为 MODBUS_HOLDING_REG_t 类型的指针
 *  6: 将输入寄存器缓冲区的地址强制转换为 MODBUS_INPUT_REG_t 类型的指针
********************************************************************************/
// DEV_OPERAT_PARA pDevOperatPara[MAX_DEV_NUM];
UCHAR *usDiscreteInputBuf = AllDevsData.tModbusData.mbDiscreteInputBuf;		// 1
UCHAR *usCoilBuf = AllDevsData.tModbusData.mbCoilBuf;						// 2
USHORT *usRegHoldingBuf = AllDevsData.tModbusData.mbHoldingBuf;				// 3
USHORT *usRegInputBuf = AllDevsData.tModbusData.mbInputBuf;					// 4
MODBUS_HOLDING_REG_t *pMbHoldData = (MODBUS_HOLDING_REG_t *)AllDevsData.tModbusData.mbHoldingBuf;	// 5
MODBUS_INPUT_REG_t *pMbInputData = (MODBUS_INPUT_REG_t *)AllDevsData.tModbusData.mbInputBuf;		// 6

/*******************************************************************************
 * 函数名: 保存指定设备的操作参数到Flash存储器
 * 功  能: 保存指定设备的操作参数到Flash存储器
 * 输  入: dev为一个DEVICE_CODE枚举变量，表示设备设备编号
 * 输  出: None
 * 返回值: None
 *******************************************************************************/

HAL_StatusTypeDef SaveDevParaDataToFlash(DEVICE_CODE dev)
{
	HAL_StatusTypeDef status = HAL_OK;
	AllDevsData.tAllDevsOperatPara[dev - 1] = ThisDevOperatPara;
	// 如果要保存到flash中取消注释掉下一行代码
	//	status = Stm32f4WriteWords(ADDR_FLASH_ALL_DEVS_DATA,(uint32_t*)&AllDevsData,sizeof(ALL_DEV_DATA)/4);
	return status;
}
/*******************************************************************************
 * 函数名: 设置一个标志，指示系统在启动时跳转到应用程序
 * 功  能: 
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/

void SetBootJumpAppNoWait(void)
{
	AllDevsData.nFlagBoot = FLAG_BOOT_JUMP_APP;
}
/*******************************************************************************
 * 函数名: 从Flash存储器中读取指定设备的操作参数
 * 功  能:
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/

void GetDevParaDataFromFlash(DEVICE_CODE dev)
{
	//	if(dev == DEV_NONE || dev > DEV_MAX) //如果设备不存在就返回
	//		return;
	////	AllDevsData.tAllDevsOperatPara
	//	ThisDevOperatPara = AllDevsData.tAllDevsOperatPara[dev-1]; 	// 从所有操作数据中找到对应要读取的编号数据
	
		/* 以下是把读到的数据写入到 Modbus 协议保持寄存器中 */
	//	pMbHoldData->nDevCode = ThisDevOperatPara.nThisDevCode;		
	//	pMbHoldData->nWorkRange = ThisDevOperatPara.nLastWorkRange;
	//	pMbHoldData->nHotHoldTime = ThisDevOperatPara.nReactionTime;
	//	pMbHoldData->nHotTemper = ThisDevOperatPara.nReactionTemp;
	//	pMbHoldData->nRange1Set = ThisDevOperatPara.nWorkRange1Set;
	//	pMbHoldData->nRange2Set = ThisDevOperatPara.nWorkRange2Set;
	//	pMbHoldData->nRange3Set = ThisDevOperatPara.nWorkRange3Set;
	//	pMbHoldData->nRange4Set = ThisDevOperatPara.nWorkRange4Set;
	//	pMbHoldData->nWaterPumpHoldTime = ThisDevOperatPara.nWaterPumpTime;
	//	pMbHoldData->fTemp1Kp = ThisDevOperatPara.fTemp1Kp;
	//	pMbHoldData->fTemp1Ki = ThisDevOperatPara.fTemp1Ki;
	//	pMbHoldData->fTemp1Kd = ThisDevOperatPara.fTemp1Kd;
	//	pMbHoldData->nTempCtrlMaxUk = ThisDevOperatPara.nTempCtrlMaxUk;
	
		/* 将校准参数写入到Modbus保持器中 */
	//	*(CALIBRATION_PARA*)&(pMbHoldData->fHighStdConcent) = ThisDevOperatPara.RangesCurvePara[ThisDevOperatPara.nLastWorkRange];
}

/*******************************************************************************
 * 函数名: SaveUserData
 * 功  能: 保存数据
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void SaveUserData(void)
{
	//  LL_IWDG_ReloadCounter(IWDG);
	Stm32f4WriteWords(ADDR_FLASH_ALL_DEVS_DATA, (uint32_t *)&AllDevsData, sizeof(ALL_DEV_DATA) / 4); // 保存用户数据到Flash中
	//	LL_IWDG_ReloadCounter(IWDG);
	//	HAL_StatusTypeDef status;
	AllDevsData.tModbusData.nFlag = 0x55AA;	// 表示数据保存成功
	//	status = STMFlashWriteInOnePage(FLASH_USER_DATA_START_ADDRESS,(uint16_t*)&AllDevsData.tModbusData,sizeof(MODBUS_BUF_DATA_IN_FLASH)/2);
	//	if(status != HAL_OK)
	//	{
	//		AllDevsData.tModbusData.nFlag = 0;
	//		pMbInputData->nFunErrorCode = status;
	//		ReplyFaultInProcess(FAULT_SAVE_DEV_PARA);
	//	}
	//	else
	//		AllDevsData.tModbusData.nFlag = 0x55AA;
}
/*******************************************************************************
 * 函数名: AddErrRecord
 * 功  能: 增加错误记录
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void AddErrRecord(RECORD_ERROR_CODE err, uint16_t remark)
{
	//	uint16_t count;
	////	taskENTER_CRITICAL();
	//	count = *pErrorCount;
	//	pErrCode[count].nErrCodes = err;
	//	pErrCode[count].nRemarks = remark;
	//	pErrCode[count].nProcCount = AllDevsData.tModbusData.nProcCount;
	//	count++;
	//	if(count >= 32)
	//		count = 0x00;
	//	*pErrorCount = count;
	//	taskEXIT_CRITICAL();
	//	SaveUserData();
}
/*******************************************************************************
 * 函数名: InitUserData
 * 功  能: 初始化数据
 * 输  入: None
 * 输  出: None
 * 返回值: None
 *******************************************************************************/
void InitUserData(void)
{
	/* 初始化数据大小 */ 
	SizeOfData = sizeof(DEV_OPERAT_PARA);				// 获取 DEV_OPERAT_PARA 参数结构体的大小
	SizeOfData = offsetof(ALL_DEV_DATA, tModbusData);	// 获取 tModbusData 在 ALL_DEV_DATA 结构体中的偏移量
	
	/* 从 Flash 读取数据 */
	AllDevsData = *(ALL_DEV_DATA *)ADDR_FLASH_ALL_DEVS_DATA; // ADDR_FLASH_ALL_DEVS_DATA 是 Flash 中存储设备数据的地址
	
	/* 初始化 Modbus 数据缓冲区 */
	usDiscreteInputBuf = AllDevsData.tModbusData.mbDiscreteInputBuf;	// 指向离散输入缓冲区
	usCoilBuf = AllDevsData.tModbusData.mbCoilBuf;			// 指向线圈缓冲区
	usRegHoldingBuf = AllDevsData.tModbusData.mbHoldingBuf; // 指向保存寄存器缓冲区
	usRegInputBuf = AllDevsData.tModbusData.mbInputBuf;		// 指向输入寄存器缓冲区
	pMbHoldData = (MODBUS_HOLDING_REG_t *)AllDevsData.tModbusData.mbHoldingBuf;	// 指向保持寄存器的结构体
	pMbInputData = (MODBUS_INPUT_REG_t *)AllDevsData.tModbusData.mbInputBuf;	// 指向输入寄存器的结构体

	/* 初始化 Modbus 命令 */
	pMbHoldData->nHostCMD = CMD_NONE;	// 将 Modbus 命令设置为 CMD_NONE，表示没有命令
	
	/* 检查数据是否已保存 */
	if (AllDevsData.tModbusData.nFlag != 0x55AA) // 如果没有保存过数据,需要初始化默认参数
	{
		/* 初始化默认参数 */
		//		if(0xFFFF == (uint16_t)pMbHoldData->nDevCode)//如果是初始电路板
		//			*pMbHoldData = (MODBUS_HOLDING_REG_t){0};
		//		GetDevParaDataFromFlash(pMbHoldData->nDevCode); //读取设备信息
		LL1InitCurrent = 3;
		LL2InitCurrent = 3;
		CurrentLEDAbs = 1.2; // 吸光度LED灯电流=CurrentLEDAbs*10,原为1.9
		LL1ADBlank = 1200;
		LL2ADBlank = 1200;

		pMbInputData->nLL1BlankAD = LL1ADBlank;
		pMbInputData->nLL2BlankAD = LL2ADBlank;
		pMbHoldData->nLL1LdCur = LL1InitCurrent * 10;
		pMbHoldData->nLL2LdCur = LL2InitCurrent * 10;
		pMbHoldData->nPoolLdCur = CurrentLEDAbs * 10;
		pMbHoldData->nTotalRefTHR = 300; // 原为50
		pMbHoldData->nBlankPipeLowTHR = 700;
		pMbHoldData->nBlankPipeUpTHR = 1300;
		pMbInputData->nPrecTotalTime = 3000;
		pMbHoldData->nStepperMotorSpeed = 600;
		pMbHoldData->nBand = 1152;
		pMbHoldData->nLocalAddr = 1;

		pMbHoldData->nStirWorkTime = 30;	//搅拌时长
		pMbHoldData->nStirSleepTime = 700;	//停搅拌时长

		//		pMbHoldData->nFullScale = 100;
		pMbHoldData->fTemp1Kp = 1000;
		pMbHoldData->fTemp1Ki = 600;
		pMbHoldData->fTemp1Kd = 1000;
		pMbHoldData->nTempCtrlMaxUk = 50000;
		//		pMbHoldData->fKturbidity = 0.0;
		pMbHoldData->nCoolFanValve = 16;
		pMbHoldData->nStirValve = 18;

		for (int i = 0; i < COIL_NCOILS / 8; i++) // 线圈寄存器全部置零，阀全关
			usCoilBuf[i] = 0;
	}
	else
	{
		/* 数据保存过，从 pMbHoldData 和 pMbInputData 中读取参数 */
		//		if(pMbHoldData->nDevCode == 0 || pMbHoldData->nDevCode > MAX_DEV_NUM) //排除设备号意外出错
		//		{
		//			pMbHoldData->nDevCode = DEV_COD_CR;
		//		}
		//		GetDevParaDataFromFlash(pMbHoldData->nDevCode);
		LL1InitCurrent = pMbHoldData->nLL1LdCur;
		LL1InitCurrent /= 10;
		if (LL1InitCurrent > I_MAX_LED || LL1InitCurrent < 0.0f)
			LL1InitCurrent = 3.0f;
		LL2InitCurrent = pMbHoldData->nLL2LdCur;
		LL2InitCurrent /= 10;
		if (LL2InitCurrent > I_MAX_LED || LL2InitCurrent < 0.0f)
			LL2InitCurrent = 3;
		CurrentLEDAbs = pMbHoldData->nPoolLdCur;
		CurrentLEDAbs /= 10;

		LL1ADBlank = pMbInputData->nLL1BlankAD;
		if (LL1ADBlank == 0xFFFF || LL1ADBlank == 0)
			LL1ADBlank = 12000;
		LL2ADBlank = pMbInputData->nLL2BlankAD;
		if (LL2ADBlank == 0xFFFF || LL2ADBlank == 0)
			LL2ADBlank = 12000;

		if (pMbHoldData->nBand > 1152)
			pMbHoldData->nBand = 1152;
		else if (pMbHoldData->nBand < 92)
			pMbHoldData->nBand = 1152;
		if (pMbHoldData->nLocalAddr > 128 || pMbHoldData->nLocalAddr == 0)
			pMbHoldData->nLocalAddr = 1;
		//		if(pMbHoldData->nDevCode > MAX_DEV_NUM)
		//
		//		pMbHoldData->nDevCode = DEV_NONE;
		if (pMbInputData->nPrecTotalTime > 3000)
			pMbInputData->nPrecTotalTime = 3000;
		//		if(pMbHoldData->fKturbidity > 1 || pMbHoldData->fKturbidity < 0)
		//		pMbHoldData->fKturbidity = 0.0;
		if (pMbHoldData->nCoolFanValve > 19 || pMbHoldData->nCoolFanValve < 1)
			pMbHoldData->nCoolFanValve = 16;
		if (pMbHoldData->nStirValve > 19 || pMbHoldData->nStirValve < 1)
			pMbHoldData->nStirValve = 18;
	}

	/* 初始化其他参数 */
	if (pMbHoldData->nStepperMotorSpeed == 0 || pMbHoldData->nStepperMotorSpeed > 1000) // 初始化电机速度
		pMbHoldData->nStepperMotorSpeed = 600;
	pMbInputData->nStepCode = STEP_RESET; // 初始化步骤信息
	InitValveRelayInfo();				  // 初始化设备功能阀信息
	//	GetFunValveList(pMbHoldData->nDevCode);//更新功能阀列表，以便上位机更新控制列表
//	pMbInputData->nDevBeRunning = 0; // 初始化设备运行标志
	pMbInputData->nDebBeOnline = 1;	 // 设备在线标志

	usCoilBuf[0] = 0; // 关闭所有继电器标志位
	usCoilBuf[1] = 0;
	usCoilBuf[2] = 0;
	usCoilBuf[3] = usCoilBuf[3] & 0xFE; // 把4-20mA电流测试用的标志位清零
	UpdataValveState(usCoilBuf);		// 更新继电器输出

	pMbInputData->nYearBuild = YEAR_BUILD; // 编译时间
	pMbInputData->nMonthBuild = MONTH_BUILD;
	pMbInputData->nDayBuild = DAY_BUILD;
	pMbInputData->nHourBuild = HOUR_BUILD;
	pMbInputData->nMinuteBuild = MINUTE_BUILD;
	pMbInputData->nSecondBuild = SECOND_BUILD;
	pMbInputData->nHotDuty = 0;
	pMbInputData->nMotorSpeedPV = 0;

	pMbHoldData->nSyringePumpMove = 0;		// 初始化注射泵参数
	pMbHoldData->nSyringePumpMoveTo = 0;
	pMbHoldData->nSyringePumpScale = 48000;
	pMbHoldData->nSyringePumpSpeed = 1800;

	usDiscreteInputBuf[0] = 0;	// 将离散输入通道第一个数据设置成0低电平
}


