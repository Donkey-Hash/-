
#ifndef	__MODBUS_DATA_H__
#define	__MODBUS_DATA_H__
#include "Modbus.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "instrumentdef.h"


//typedef enum{
//	START_APP = 0x1,
//	START_BOOT = 0x02,
//}MPU_START_MODE;
/**************************定义保持寄存器位移量*************************************/
typedef enum{							//

	HOST_CMD = 0,
	COMM_BAUD = 19,
	LOCAL_ADDR = 20,	
	
}HOLD_BUFF_CFG;

/***************************保持寄存器定义*****************************************/
typedef struct {	
	//////////////////////1-10.上位机指令与设备参数///////////////////////////////////
	HOST_CMD_t 					nHostCMD;      					//上位机指令		
	uint16_t 						nBand;									////波特率
	uint16_t						nLocalAddr;							//Modbus本地地址
	uint16_t						nSpectroAverageTimes;								//*ys*1*光谱仪扫描次数	
	uint16_t 						nSpectroIntegralTime;							//*ys*2*光谱仪积分时间
	uint16_t 						nHotTemper;							//加热温度
	uint16_t 						nHotHoldTime;						//温控时长
	uint16_t 						nWaterPumpHoldTime;     //水泵时间	
	uint16_t						nRange1Set;
	uint16_t						nRange2Set;	
	//////////////////////11-20.上位机指令与设备参数///////////////////////////////////
	uint16_t						nRange3Set;
	uint16_t						nRange4Set;		
	uint16_t						nYearSet;
	uint16_t						nMonthSet;
	uint16_t						nDaySet;
	uint16_t						nHourSet;
	uint16_t						nMinuteSet;
	uint16_t						nSecondSet;
	uint16_t						ntemp19;
	uint16_t						nTempCtrlMaxUk;
	
	
	////////////////////////21-30.下位机数据采集/////////////////////////////
	uint32_t						nSyringePumpScale;    //*ys*3*注射泵量程,单位：步进电机脉冲数
	int32_t						nSyringePumpMove;     	 //*ys*4*注射泵位置相对运动量，相对于当前位置，单位：步进电机脉冲数
	int32_t						nSyringePumpMoveTo;				//*ys*5*注射泵位置绝对运动，相对0点位置，单位：步进电机脉冲数
	uint16_t						nSyringePumpSpeed;				//*ys*6*注射泵运动速度，单位：0.1rpm//最大速度6000rpm
	uint16_t 						nTemp28;						//*ys*7* 上位机设定的工作量程值，整数。用于4-20ma计算
	uint16_t 						nSpectro275AD;				//*ys*8* //光谱仪275nmAD值
	uint16_t 						nSpectro220AD;			  //*ys*9* //光谱仪220nmAD值
	////////////////////////////31-40.内部运行参数/////////////////////////////////	
	uint16_t						nStirWorkTime;						//*ys*10*搅拌工作时长，单位：ms
	uint16_t						nStirSleepTime;					//*ys*11*搅拌休止时长，单位：ms
	uint16_t						nCoolFanValve;					//*ys*12*冷却风扇继电器序号
	uint16_t 						nStirValve;							//*ys*13*搅拌电机继电器序号
	uint32_t 						nCombiValve;						//*in*1*组合阀。位0-18分别代表端口Y1-Y19. 1:需要操作（开启或关闭）；0：不需要操作
	uint16_t						nInitTemp;							//温控初始温度
	uint16_t						nTimeToSV;							//加热至设定温度花费时长
	uint16_t						nMaintainTimePast;			//保温已花费时长
	uint16_t						nMaintainDuty;					//保持占空比
	///////////////////////////41-50.内部参数设置//////////////////////////////////
	uint16_t 						nPoolLdCur;							//池体光源电流
	uint16_t 						nLL1LdCur;							//液位1光源电流
	uint16_t 						nLL2LdCur;							//液位2光源电流	
	uint16_t						nBlankPipeLowTHR;				//1000*空管下限/空管  
	uint16_t						nBlankPipeUpTHR;				//1000*空管上限/空管 
	uint16_t						nTotalRefTHR;						//1000*全反射点上限/空管
	int16_t							nStepperMotorSpeed;						//*in*3*参数3，步进电机速度，单位0.1rpm
	uint16_t						nAutoRang;				//
	uint16_t						nLastWorkRange;							//工作量程;
	uint16_t						nLevelNum; //*in*4*需要判定的液位 1：低液位，4：高液位
	
	////////////////////////51-54.运行结果相关/////////////////////////////////////
	uint16_t						nBlankRefLtAD;            //*out*2*暗电流  V0
	uint16_t						nBlankTranLtAD;						//*out*3*比色电压  V3
	uint16_t						nReactionRefLtAD;					//反应参比  V2
	uint16_t						nReactionTranLtAD;				//反应透射  V1
			/////////////55-60.上位机指令参数///////////
	uint16_t						nTestTemper;						//*in*5*温度调试指令参数
	uint16_t						nTestTemperHoldtime;		//*in*6*温度调试指令参数							
	FUN_VALVE_CODE			nFunVavleCode;					//功能阀代码//指令104,105参数
	uint16_t						nPeristalticPumpParm;		//*in*2*蠕动泵执行圈数 //指令101,102参数	
	uint16_t						nReagentCode;						//向反应池添加试剂指令参数
	uint16_t						nReagentQuantity;				//向反应池添加试剂指令参数

	////////////////////////61-64.外部设置/////////////////////////////////////						
	uint16_t						nCMDTranMoni;
	uint16_t						RW44;             //RW44
	uint16_t						ntemp63;					//
	uint16_t						ntemp64;
	
	/////////////////////////////////////////////////////////////	
	float								fAbsorbance;						//65
	float								fPoolTemp;							//池体温度67
	float 								fCaveTemp;							//69
	float								ftemp;								//保留71
	float								fTemp1Kp;							//*in*7*73
	float								fTemp1Ki;							//*in*8*75
	float								fTemp1Kd;							//*in*9*77
	float								fBlankAbs;							//79
	float								fReactAbs;							//81
	float								fMeasAbs;							//83
	float 							fHighStdConcent;   						//高标试剂浓度85
	float 							fLowStdConcent;							//低标试剂浓度87
	float								fCalibHighAbs;						//高点标定吸光度89
	float								fCalibLowAbs;          				//低点标定吸光度91	
	float 							fCalibKValue;							//标定K值93
	float								fCalibBValue;						//标定B值95
	float								fMeasureResult;						//97 //*ys*14* 测量结果用于计算4-20ma输出值
	float								fCurrentTestSet;					//99 //*ys*15* 设置4-20ma测试电流。
	float 							fAmaxOut;								//101//*ys*16* 4-20ma上限标定时输出电流值，由标定人员从标准表读出，从上位机写入。
	float            		fAminOut;										//103//*ys*17* 4-20ma下限标定时输出电流值，由标定人员从标准表读出，从上位机写入。
	float								fWorkRangeSet;    					//105//*ys*18*
	//107
	uint16_t						RW576;									//107,量程阀联动
	uint16_t						RW577;									//108,清洗废液阀联动
	uint16_t						RW50;									//109,工作类型
	uint16_t						RW30;									//110,低位空杯电压
	uint16_t						RW31;									//111,高位空杯电压
	uint16_t						RW32;									//112,低位满水阈值
	uint16_t						RW33;									//113,高位满水阈值
	uint16_t						RW34;									//114,低位定位阈值
	uint16_t						RW35;									//115,高位定位阈值
	uint16_t						RW579;									//116,更新液位电压
	uint16_t						RW36;									//117,水泵循环时间
	uint16_t						RW4229;									//118,
	uint16_t						RW4230;									//119,
	uint16_t						RW43;									//120,
	uint16_t						RW45;									//121,
	uint16_t						RW47;									//122,
	uint16_t						RW48;									//123,
	uint16_t						RW84;									//124,
	float							RW125;									//125,126
	uint16_t						RW580;									//127,
	uint16_t						RW516;									//128,
	uint16_t						RZHOLENUM;								//129,润泽阀孔序号
	uint16_t						RZSPEED;								//130,润泽速度
	uint16_t						RZML;									//131,润泽泵的位置
	///////////////////////////////132- 自动流程中新曾标志//////////////////////////////////////////////
	uint16_t 						nAutoRun;								//132,自动流程
	uint16_t						nResetRun;								//133,复位标志
	uint16_t						nMeasureChoose;							//134,量程选择
	float							fAbsorbanc;								//135,136吸光度
	float							fK;										//137,138 K值
	float							fB;										//139,140 b值
	uint16_t						nUpdateData;							//141,更新日志标志位
	uint16_t						nZeroComplet;							//142,零标已测完标志位
	uint16_t						nMeasureComplet;						//143,量程已测完标志位
	uint16_t						nSampleComplet;							//144,水样已测完标志位
	float							fConcentrationVale;						//145,146 水样浓度检测值
	uint16_t						nReagent1Volume;						//147 试剂1的体积
	uint16_t						nReagent2Volume;						//148 试剂2的体积
	uint16_t						nReagent3Volume;						//149 试剂3的体积
	uint16_t						nStartMode;								//150 启动模式选择
	uint16_t						nClockTime;								//151 采样定时时间(周期模式：分钟)
	uint16_t						nControlledStep;						//152 受控模式选择步骤
	float							RW86;									//153,154 消解温度
	uint16_t						RW83;									//155 消解时间（秒）
	uint16_t						nWaterVolume;							//156 蒸馏水体积
	uint16_t						nFullReagent1;							//157 试剂1满体积
	uint16_t						nFullReagent2;							//158 试剂2满体积
	uint16_t						nFullReagent3;							//159 试剂3满体积
	uint16_t						nFullWater;								//160 蒸馏水满体积
	float							fUPWarning;								//161,162 报警上限值
	uint16_t						nSurpassTime;							//163 超标输出时间(秒)
	float							fAnalogRange;							//164,165 报警上限值
	
	uint16_t						nBlankCalibDays;						//166 空白校准天数
	uint16_t						nBlankCalibHours;						//167 空白校准点数
	uint16_t						nBlankCaliMea;							//168 空白校准量程

	uint16_t						nStandardCalibDays;						//169 标样校准天数
	uint16_t						nStandardCalibHours;					//170 标样校准点数
	uint16_t						nStandardCalibMea;						//171 标样校准量程

	uint16_t						nBlankDemaDays;							//172 空白标定天数
	uint16_t						nBlankDemaHours;						//173 空白标定点数
	uint16_t						nBlankDemaMea;							//174 空白标定量程

	uint16_t						nStandardDemaDays;						//175 标样标定天数
	uint16_t						nStandardDemaHours;						//176 标样标定点数
	uint16_t						nStandardDemaMea;						//177 标样标定量程

	uint16_t						nStandardCheckDays;						//178 标样核查天数
	uint16_t						nStandardCheckHours;					//179 标样核查点数
	float							fStandardCheckDens;						//180,181 标样核查浓度
	float							fAllowError;							//182,183 允许绝对误差
	float							fStandardUpWarn;						//184,185 标样核查报警上限值
	float							fStandardDownWarn;						//186,187 标样核查报警上限值
	
	uint16_t						nZeroCheckDays;							//188 零点核查点数
	uint16_t						nZeroCheckHours;						//189 零点核查量程
	float							fZeroCheckDens;							//190,191 零点核查浓度
	float							fZeroUpWarn;							//192,193 零点核查报警上限值
	float							fZeroDownWarn;							//194,195 零点核查报警上限值
	
	uint16_t						nSpanCheckDays;							//196 跨度核查点数
	uint16_t						nSpanCheckHours;						//197 跨度核查量程
	float							fSpanCheckDens;							//198,199 跨度核查浓度
	float							fSpanUpWarn;							//200,201 跨度核查报警上限值
	float							fSpanDownWarn;							//202,203 跨度核查报警上限值
	
	uint16_t						nMea1CalibYear;							//204 量程1标定时间年	
	uint16_t						nMea1CalibMonth;						//205 量程1标定时间月
	uint16_t						nMea1CalibDay;							//206 量程1标定时间日
	uint16_t						nMea1CalibHour;							//207 量程1标定时间时
	uint16_t						nMea1CalibMinu;							//208 量程1标定时间分
	
	uint16_t						nMea1AligYear;							//209 量程1校准时间年	
	uint16_t						nMea1AligMonth;							//210 量程1校准时间月
	uint16_t						nMea1AligDay;							//211 量程1校准时间日
	uint16_t						nMea1AligHour;							//212 量程1校准时间时
	uint16_t						nMea1AligMinu;							//213 量程1校准时间分	
		
	uint16_t						nMea2CalibYear;							//214 量程2标定时间年	
	uint16_t						nMea2CalibMonth;						//215 量程2标定时间月
	uint16_t						nMea2CalibDay;							//216 量程2标定时间日
	uint16_t						nMea2CalibHour;							//217 量程2标定时间时
	uint16_t						nMea2CalibMinu;							//218 量程2标定时间分
	
	uint16_t						nMea2AligYear;							//219 量程2校准时间年	
	uint16_t						nMea2AligMonth;							//220 量程2校准时间月
	uint16_t						nMea2AligDay;							//221 量程2校准时间日
	uint16_t						nMea2AligHour;							//222 量程2校准时间时
	uint16_t						nMea2AligMinu;							//223 量程2校准时间分	
	
	uint16_t						nMea3CalibYear;							//224 量程3标定时间年	
	uint16_t						nMea3CalibMonth;						//225 量程3标定时间月
	uint16_t						nMea3CalibDay;							//226 量程3标定时间日
	uint16_t						nMea3CalibHour;							//227 量程3标定时间时
	uint16_t						nMea3CalibMinu;							//228 量程3标定时间分
										                                          
	uint16_t						nMea3AligYear;							//229 量程3校准时间年	
	uint16_t						nMea3AligMonth;							//230 量程3校准时间月
	uint16_t						nMea3AligDay;							//231 量程3校准时间日
	uint16_t						nMea3AligHour;							//232 量程3校准时间时
	uint16_t						nMea3AligMinu;							//233 量程3校准时间分	
	
	uint16_t						nMea4CalibYear;							//234 量程4标定时间年	
	uint16_t						nMea4CalibMonth;						//235 量程4标定时间月
	uint16_t						nMea4CalibDay;							//236 量程4标定时间日
	uint16_t						nMea4CalibHour;							//237 量程4标定时间时
	uint16_t						nMea4CalibMinu;							//238 量程4标定时间分
																					  
	uint16_t						nMea4AligYear;							//239 量程4校准时间年	
	uint16_t						nMea4AligMonth;							//240 量程4校准时间月
	uint16_t						nMea4AligDay;							//241 量程4校准时间日
	uint16_t						nMea4AligHour;							//242 量程4校准时间时
	uint16_t						nMea4AligMinu;							//243 量程4校准时间分	
	
	uint16_t						nMea5CalibYear;							//244 量程5标定时间年	
	uint16_t						nMea5CalibMonth;						//245 量程5标定时间月
	uint16_t						nMea5CalibDay;							//246 量程5标定时间日
	uint16_t						nMea5CalibHour;							//247 量程5标定时间时
	uint16_t						nMea5CalibMinu;							//248 量程5标定时间分
																					  
	uint16_t						nMea5AligYear;							//249 量程5校准时间年	
	uint16_t						nMea5AligMonth;							//250 量程5校准时间月
	uint16_t						nMea5AligDay;							//251 量程5校准时间日
	uint16_t						nMea5AligHour;							//252 量程5校准时间时
	uint16_t						nMea5AligMinu;							//253 量程5校准时间分
	
	uint16_t						nValueChoose;							//254 跨度核查选择阀
	uint16_t						nTestTime;								//255 设置检测次数
	uint16_t						nToMode;								//256 两点标定转对应模式
	uint16_t						nValveCtl;								//257 控制单个阀门
}MODBUS_HOLDING_REG_t;
/*************************输入寄存器定义（待扩展）****************************************/
typedef struct {
	uint16_t						nCtrState;							//控制状态
	uint16_t						nFirmVersion;           //固件版本
	uint16_t 						nSysErrorCode;							//*out*1*错误代码
	uint16_t						nDevBeRunning;
	int32_t							nSyringePumpPos; 						//*ys*18*注射泵当前位置,单位：步进电机脉冲数
	uint16_t						nDebBeOnline;
//	uint16_t						nBootVersion;
	uint16_t						nHotDuty;         //加热丝占空比，60000 -> 100% 0 -> 0%
	int16_t							nMotorSpeedPV;    //蠕动泵当前速度单位：0.1rpm；正值为抽取，负值为推出
	uint16_t						nTempCtrState;
	//////////////////////11-20.//////////////////
	uint16_t						nCaveTempAD;
	uint16_t						nPoolTempAD;
	uint16_t 						nPoolRefLtAD;    				//池体入射光AD，比色池电压
	uint16_t 						nPoolTranLtAD;					//池体透射光AD
	uint16_t						nLL1AD;									//液位1AD值
	uint16_t						nLL2AD;									//液位2AD值	
	uint16_t 						nLL1BlankAD;						//液位1空白值
	uint16_t 						nLL2BlankAD;						//液位2空白值
	uint16_t 						nPoolRefLtBackAD;				//比色池参比背景光AD值/
	uint16_t 						nPoolTranLtBackAD;			//比色池透射背景光AD值/
	//////////////////////21-30.//////////////////
	uint16_t						nStepPercent;						//当前进度百分数
	uint16_t						nStepCode;					//当前步骤名称
	uint16_t						nPrecTotalTime;					//流程总时间	
	uint16_t						RW80;
	uint16_t						RW81;
	uint16_t						RW82;
	uint16_t						RW426;
	uint16_t						RW427;
	uint16_t						RW428;
	uint16_t						RW429;
	//////////////////////31-40.//////////////////
	uint16_t						RW430;
	uint16_t						RW431;	
	int32_t							RW514;	
	int32_t							RW512;	
	uint16_t						RW509;
	uint16_t						RW554;
	uint16_t						nFunValve16;
	uint16_t						nFunValve17;
		//////////////////////41-50.//////////////////
	uint16_t						nFunValve18;
	uint16_t						nFunValve19;
	uint16_t						nFunValve20;
	
	uint16_t 						nFunErrorCode;							
	uint16_t						nProcTimeElapsed;
	uint16_t						nInTemp33;
	uint16_t						nYearBuild;
	uint16_t						nMonthBuild;
	uint16_t						nDayBuild;
	uint16_t						nHourBuild;
		//////////////////////41-50.//////////////////
	uint16_t						nMinuteBuild;
	uint16_t						nSecondBuild;
	
	float							fVbat;
	float							fCPUTemp;							//CPU温度71
	float							fReagent1Percent;					//57 试剂1余量（百分比）
	float							fReagent2Percent;					//59 试剂2余量（百分比）
	float							fReagent3Percent;					//61 试剂3余量（百分比）
	float							fWarterPercent;						//63 蒸馏水余量（百分比）
	
	float							fMea1Cali2;							//65 量程1标定二次项
	float							fMea1Cali1;							//67 量程1标定一次项
	float							fMea1Cali0;							//69 量程1标定常数项
	float							fMea1Coe;							//71 量程1校准系数
	float							fMea1C1Concent;						//73 量程1C1标定浓度
	float							fMea1C2Concent;						//75 量程1C2标定浓度
	float							fMea1C3Concent;						//77 量程1C3标定浓度
	float							fMea1C4Concent;						//79 量程1C4标定浓度
	float							fMea1C5Concent;						//81 量程1C5标定浓度
	float							fMea1C1Aborban;						//83 量程1C1标定吸光度
	float							fMea1C2Aborban;						//85 量程1C2标定吸光度
	float							fMea1C3Aborban;						//87 量程1C3标定吸光度
	float							fMea1C4Aborban;						//89 量程1C4标定吸光度
	float							fMea1C5Aborban;						//91 量程1C5标定吸光度
	
	float							fMea1Alig2;							//93 量程1校准二次项
	float							fMea1Alig1;							//95 量程1校准一次项
	float							fMea1Alig0;							//97 量程1校准常数项
	float							fMea1ZeroAligAbor;					//99 量程1零点校准校准吸光度
	float							fMea1AligCons;						//101 量程1标样校准浓度
	float							fMea1AligAbor;						//103 量程1标样校准吸光度
	
	float							fMea2Cali2;							//105 量程2标定二次项
	float							fMea2Cali1;							//107 量程2标定一次项
	float							fMea2Cali0;							//109 量程2标定常数项
	float							fMea2Coe;							//111 量程2校准系数
	float							fMea2C1Concent;						//113 量程2C1标定浓度
	float							fMea2C2Concent;						//115 量程2C2标定浓度
	float							fMea2C3Concent;						//117 量程2C3标定浓度
	float							fMea2C4Concent;						//119 量程2C4标定浓度
	float							fMea2C5Concent;						//121 量程2C5标定浓度
	float							fMea2C1Aborban;						//123 量程2C1标定吸光度
	float							fMea2C2Aborban;						//125 量程2C2标定吸光度
	float							fMea2C3Aborban;						//127 量程2C3标定吸光度
	float							fMea2C4Aborban;						//129 量程2C4标定吸光度
	float							fMea2C5Aborban;						//131 量程2C5标定吸光度	
	
	float							fMea2Alig2;							//133 量程2校准二次项
	float							fMea2Alig1;							//135 量程2校准一次项
	float							fMea2Alig0;							//137 量程2校准常数项
	float							fMea2ZeroAligAbor;					//139 量程2零点校准校准吸光度
	float							fMea2AligCons;						//141 量程2标样校准浓度
	float							fMea2AligAbor;						//143 量程2标样校准吸光度
	
	float							fMea3Cali2;							//145 量程3标定二次项
	float							fMea3Cali1;							//147 量程3标定一次项
	float							fMea3Cali0;							//149 量程3标定常数项
	float							fMea3Coe;							//151 量程3校准系数
	float							fMea3C1Concent;						//153 量程3C1标定浓度
	float							fMea3C2Concent;						//155 量程3C2标定浓度
	float							fMea3C3Concent;						//157 量程3C3标定浓度
	float							fMea3C4Concent;						//159 量程3C4标定浓度
	float							fMea3C5Concent;						//161 量程3C5标定浓度
	float							fMea3C1Aborban;						//163 量程3C1标定吸光度
	float							fMea3C2Aborban;						//165 量程3C2标定吸光度
	float							fMea3C3Aborban;						//167 量程3C3标定吸光度
	float							fMea3C4Aborban;						//169 量程3C4标定吸光度
	float							fMea3C5Aborban;						//171 量程3C5标定吸光度	
	
	float							fMea3Alig2;							//173 量程3校准二次项
	float							fMea3Alig1;							//175 量程3校准一次项
	float							fMea3Alig0;							//177 量程3校准常数项
	float							fMea3ZeroAligAbor;					//179 量程3零点校准校准吸光度
	float							fMea3AligCons;						//181 量程3标样校准浓度
	float							fMea3AligAbor;						//183 量程3标样校准吸光度
	
	float							fMea4Cali2;							//185 量程4标定二次项
	float							fMea4Cali1;							//187 量程4标定一次项
	float							fMea4Cali0;							//189 量程4标定常数项
	float							fMea4Coe;							//191 量程4校准系数
	float							fMea4C1Concent;						//193 量程4C1标定浓度
	float							fMea4C2Concent;						//195 量程4C2标定浓度
	float							fMea4C3Concent;						//197 量程4C3标定浓度
	float							fMea4C4Concent;						//199 量程4C4标定浓度
	float							fMea4C5Concent;						//201 量程4C5标定浓度
	float							fMea4C1Aborban;						//203 量程4C1标定吸光度
	float							fMea4C2Aborban;						//205 量程4C2标定吸光度
	float							fMea4C3Aborban;						//207 量程4C3标定吸光度
	float							fMea4C4Aborban;						//209 量程4C4标定吸光度
	float							fMea4C5Aborban;						//211 量程4C5标定吸光度	
																			  
	float							fMea4Alig2;							//213 量程4校准二次项
	float							fMea4Alig1;							//215 量程4校准一次项
	float							fMea4Alig0;							//217 量程4校准常数项
	float							fMea4ZeroAligAbor;					//219 量程4零点校准校准吸光度
	float							fMea4AligCons;						//221 量程4标样校准浓度
	float							fMea4AligAbor;						//223 量程4标样校准吸光度
	
	float							fMea5Cali2;							//225 量程5标定二次项
	float							fMea5Cali1;							//227 量程5标定一次项
	float							fMea5Cali0;							//229 量程5标定常数项
	float							fMea5Coe;							//231 量程5校准系数
	float							fMea5C1Concent;						//233 量程5C1标定浓度
	float							fMea5C2Concent;						//235 量程5C2标定浓度
	float							fMea5C3Concent;						//237 量程5C3标定浓度
	float							fMea5C4Concent;						//239 量程5C4标定浓度
	float							fMea5C5Concent;						//241 量程5C5标定浓度
	float							fMea5C1Aborban;						//243 量程5C1标定吸光度
	float							fMea5C2Aborban;						//245 量程5C2标定吸光度
	float							fMea5C3Aborban;						//247 量程5C3标定吸光度
	float							fMea5C4Aborban;						//249 量程5C4标定吸光度
	float							fMea5C5Aborban;						//251 量程5C5标定吸光度	
																				  
	float							fMea5Alig2;							//253 量程5校准二次项
	float							fMea5Alig1;							//255 量程5校准一次项
	float							fMea5Alig0;							//257 量程5校准常数项
	float							fMea5ZeroAligAbor;					//259 量程5零点校准校准吸光度
	float							fMea5AligCons;						//261 量程5标样校准浓度
	float							fMea5AligAbor;						//263 量程5标样校准吸光度
	
	uint16_t						nCurMea;							//265 当前量程
	uint16_t						nCurState;							//266 当前状态
	uint16_t						nRunStep;							//267 运行步骤
	
	float							fTestAccu;							//268,269 测量精度
}MODBUS_INPUT_REG_t;



#endif
