

#ifndef __INSTRUMENT_DEF_H__
#define __INSTRUMENT_DEF_H__
#include "valvedef.h"
// #include "peristalticpump.h"

// #define		NUM_PROC_INIT_STEPS				3
// #define		NUM_COD_CR_PROC_STEPS			11

#define MAX_DEV_NUM 20

/**********设置抽取各种试剂的速度，单位(0.1rpm) e.g 500:50.0转/分钟********************/

#define SPEED_DEFAULT 600
#define SPEED_SAMPLE 600			   // 抽取样品速度   //单位:0.1rpm
#define SPEED_DISTILLED_WATER 600	   // 抽取蒸馏水
#define SPEED_STANDARD_SOLUTION 600	   // 抽取标液
#define SPEED_MERCUCY_SULFATE 600	   // 抽取硫酸汞
#define SPEED_POTASSIUM_DICHROMATE 600 // 抽取重铬酸钾
#define SPEED_SULPHURIC_ACID 200	   // 抽取硫酸
#define SPEED_LYE 600				   // 抽取碱液
#define SPEED_RELEASE_AGENT 600		   // 抽取释放剂
#define SPEED_NEUTRALIZER 600		   // 抽取中和液
#define SPEED_BUFFERS 600			   // 抽取缓冲剂
#define SPEED_CHROMOGENIC 400		   // 抽取显色剂
#define SPEED_ABSORBENT 600			   // 抽取吸收液
#define SPEED_EXTRACT 600			   // 抽取萃取液
#define SPEED_MARSKING_AGENT 600	   // 抽取掩蔽剂
#define SPEED_CLEANERS 600			   // 抽取清洗剂
#define SPEED_DIGESTION 600			   // 抽取消解液
#define SPEED_OXIDANT 400			   // 抽取氧化剂
#define SPEED_REGULATOR 600			   // 抽取调节剂
#define SPEED_REDUCER 600			   // 抽取还原剂
#define SPEED_COATING_FILM 600		   // 26,                    	//汞膜液
#define SPEED_REFERENCE 600			   // 27,													//参比液
#define SPEED_LOOP_ONLINE_POOL 600	   // 28,				//反应池串入定量环////		600 // 26,												//汞膜液阀
#define SPEED_LOOP_VALVE_OUT 600	   // 29,				//					//定量环出口阀///
// 为了不和错误代码冲突，30-49空出
#define SPEED_RECOVER 600				 // 50,														    //加标回收、质控水样//28
#define SPEED_LOW_SS 600				 // 51,															//低浓度标液，用于低点核查
#define SPEED_VERIFY_BLIND_SAMPLE 600	 // 52, 												//盲样核查
#define SPEED_VERIFY_DISTILLED_WATER 600 // 53,										//蒸馏水/零点液核查
#define SPEED_VERIFY_SS 600				 // 54,																	//标液核查
#define SPEED_VERIFY_SPAN 600			 // 55,

#define SPEED_AGENT_1 600 //
#define SPEED_AGENT_2 600 //
#define SPEED_AGENT_3 600 //
#define SPEED_AGENT_4 600 //
#define SPEED_AGENT_5 600 //

#define SPEED_REACTION_POOL_SOLUTION 800 // 抽取废液时的速度
#define SPEED_LOOP_POOL 300				 // 定量环定量速度
#define SPEED_AIR_TUBE 800				 // 吹扫定量管速度

#define SPEED_MIN 200
#define SPEED_MAX 1000

/*****************************试剂代码*****************************************/
/*      为了和故障代码兼容，试剂代码从7开始。*/
typedef enum
{
	AGENT_SAMPLE = 7,				 // 水样
	AGENT_DISTILLED_WATER = 8,		 // 蒸馏水/零点液
	AGENT_STANDARD_SOLUTION = 9,	 // 标液
	AGENT_MERCUCY_SULFATE = 10,		 // 硫酸汞
	AGENT_POTASSIUM_DICHROMATE = 11, // 重铬酸钾
	AGENT_SULPHURIC_ACID = 12,		 // 硫酸
	AGENT_LYE = 13,					 // 碱液
	AGENT_RELEASE_AGENT = 14,		 // 释放剂
	AGENT_NEUTRALIZER = 15,			 // 中和液
	AGENT_BUFFERS = 16,				 // 缓冲剂
	AGENT_CHROMOGENIC = 17,			 // 显色剂
	AGENT_ABSORBENT = 18,			 // 吸收液
	AGENT_EXTRACT = 19,				 // 萃取液
	AGENT_MARSKING_AGENT = 20,		 // 掩蔽剂
	AGENT_CLEANERS = 21,			 // 清洗剂
	AGENT_DIGESTION = 22,			 // 消解液
	AGENT_OXIDANT = 23,				 // 氧化剂
	AGENT_REGULATOR = 24,			 // 调节剂
	AGENT_REDUCER = 25,				 // 还原剂
	AGENT_LOOP_ONLINE_POOL = 26,	 // 反应池串入定量环////		= 26,												//汞膜液阀
	AGENT_LOOP_VALVE_OUT = 27,		 //					//定量环出口阀///
	AGENT_COATING_FILM = 28,		 // 汞膜液
	AGENT_REFERENCE = 29,			 // 参比液
	AGENT_VERIFY_SS = 34,			 // 标液核查//为了和以前程序兼容
	// 为了不和错误代码冲突，30-49空出
	AGENT_RECOVER = 50,				   // 加标回收、质控水样//28
	AGENT_LOW_SS = 51,				   // 低浓度标液，用于低点核查
	AGENT_VERIFY_BLIND_SAMPLE = 52,	   // 盲样核查
	AGENT_VERIFY_DISTILLED_WATER = 53, // 蒸馏水/零点液核查
	AGENT_VERIFY_SS_2 = 54,			   // 标液核查
	AGENT_VERIFY_SPAN = 55,			   // 跨度核查

	AGENT_REACTION_POOL_SOLUTION = 59, // 反应池中的溶液
	AGENT_1 = 61,
	AGENT_2 = 62,
	AGENT_3 = 63,
	AGENT_4 = 64,
	AGENT_5 = 65,
	AGENT_MAX = 70,
} AGENT_CODE;

/*****************************阀功能代码*****************************************/
/*      和试剂代码兼容*/
#define MAX_FUN_VALVE_NUM 20

typedef enum
{
	F_VALVE_NONE = 0,				   //
	F_VALVE_AIR = 1,				   // 空气阀
	F_VALVE_POOL_ONLY = 2,			   // 反应池阀
	F_VALVE_CLEAN_WASTE = 3,		   // 清洗废液排放阀
	F_VALVE_REACT_WASTE = 4,		   // 反应废液排放阀
	F_VALVE_FAN = 5,				   // 风扇阀
	F_VALVE_WATER_PUMP = 6,			   // 水泵
	F_VALVE_SAMPLE = 7,				   // 水样阀
	F_VALVE_DISTILLED_WATER = 8,	   // 蒸馏水/零点液阀
	F_VALVE_STANDARD_SOLUTION = 9,	   // 标液阀
	F_VALVE_MERCUCY_SULFATE = 10,	   // 硫酸汞阀
	F_VALVE_POTASSIUM_DICHROMATE = 11, // 重铬酸钾阀
	F_VALVE_SULPHURIC_ACID = 12,	   // 硫酸阀
	F_VALVE_LYE = 13,				   // 碱液阀
	F_VALVE_RELEASE_AGENT = 14,		   // 释放剂阀
	F_VALVE_NEUTRALIZER = 15,		   // 中和液阀
	F_VALVE_BUFFERS = 16,			   // 缓冲剂阀
	F_VALVE_CHROMOGENIC = 17,		   // 显色剂阀
	F_VALVE_ABSORBENT = 18,			   // 吸收液阀
	F_VALVE_EXTRACT = 19,			   // 萃取液阀
	F_VALVE_MARSKING_AGENT = 20,	   // 掩蔽剂阀
	F_VALVE_CLEANERS = 21,			   // 清洗剂阀
	F_VALVE_DIGESTION = 22,			   // 消解液阀
	F_VALVE_OXIDANT = 23,			   // 氧化剂阀
	F_VALVE_REGULATOR = 24,			   // 调节剂阀
	F_VALVE_REDUCER = 25,			   // 还原剂阀
	F_VALVE_LOOP_ONLINE_POOL = 26,	   // 反应池串入定量环////		= 26,												//汞膜液阀
	F_VALVE_LOOP_VALVE_OUT = 27,	   //					//定量环出口阀///
	F_VALVE_COATING_FILM = 28,
	F_VALVE_REFERENCE = 29, // 参比液	27
	/*****************************30 - 49用做功能阀定义***************************/
	F_VALVE_TITRATION_PUMP = 30,		  // 滴定泵	30开始与却试剂报警不重合												//滴定泵
	F_VALVE_POOL_SYRINGE = 31,			  // 注射泵
	F_VALVE_STIR_MOTOR = 32,			  // 搅拌电机
	F_VALVE_PUMP_WASH_WATER_TO_POOL = 33, // 泵清洗液阀
	F_VALVE_VERIFY_SS = 34,				  // 为了和以前程序兼容
	F_VALVE_WASTE_WARTER_PUMP = 35,		  // 直排废液泵
	F_VALVE_CAVE_FAN = 36,

	/*****************************50 - 59用新加液体阀定义***************************/
	F_VALVE_RECOVER = 50,				 // 加标回收、质控阀//28
	F_VALVE_LOW_SS = 51,				 // 低浓度标液阀
	F_VALVE_VERIFY_BLIND_SAMPLE = 52,	 // 盲样核查阀
	F_VALVE_VERIFY_DISTILLED_WATER = 53, // 零点液核查阀
	F_VALVE_VERIFY_SS_2 = 54,			 // 标液核查阀
	F_VALVE_VERIFY_SPAN = 55,			 ////跨度核查

	F_REACTION_POOL_SOLUTION = 59, // 反应池中的溶液
	F_VALVE_AGENT_1 = 61,
	F_VALVE_AGENT_2 = 62,
	F_VALVE_AGENT_3 = 63,
	F_VALVE_AGENT_4 = 64,
	F_VALVE_AGENT_5 = 65,

	ALL_FUN_VALVE_CODE = 100,
	MAX_FUN_VALVE_CODE = 0x100,
} FUN_VALVE_CODE;

#define VALVE_WASTE_LIQUID F_VALVE_CLEAN_WASTE
#define VALVE_GLASS_TANK F_VALVE_POOL_ONLY
#define VALVE_COOL_FAN F_VALVE_FAN
/*****************************^^阀功能代码^^*****************************************/

/***************************步骤代码*****************************************/
typedef enum
{
	STEP_RESET = 0,
	STEP_WASH_WITH_SAMPLE = 1,
	STEP_WASH_WITH_SS,		   // SS:Standard solution
	STEP_WASH_DETECTION_POOL,  // 清洗检测池
	STEP_WASH_REACTION_POOL,   // 清洗反应池
	STEP_WASH_DEGASSING_POOL,  // 清洗脱气池
	STEP_WASH_ABSORPTION_POOL, // 清洗吸收池

	STEP_ADD_SAMPLE = 7,
	STEP_ADD_DW, // DW:Distilled water				蒸馏水
	STEP_ADD_SS, // SS:Standard solution			标液
	STEP_ADD_MS, // MS：Mercury sulfate				硫酸汞
	STEP_ADD_PD, // PD:potassium dichromate 	重铬酸钾
	STEP_ADD_SA, // SA:sulphuric acid       	硫酸
	STEP_ADD_LYE,
	STEP_ADD_RELEASE_AGENT,	 // 释放剂
	STEP_ADD_NEUTRALIZER,	 // 中和液
	STEP_ADD_BUFFERS,		 // 缓冲剂
	STEP_ADD_CHROMOGENIC,	 // 显色剂
	STEP_ADD_ABSORBENT,		 // 吸收液
	STEP_ADD_EXTRACT,		 // 萃取液
	STEP_ADD_MARSKING_AGENT, // 掩蔽剂
	STEP_ADD_CLEANERS,		 // 清洗剂
	STEP_ADD_DIGESTION,		 // 消解液
	STEP_ADD_OXIDANT,		 // 氧化剂
	STEP_ADD_REGULATOR,		 // 调节剂
	STEP_ADD_REDUCER,		 // 还原剂

	STEP_HOTTING = 40,			   // 正在加热
	STEP_REACTING,				   // 正在反应
	STEP_EXTRECTING,			   // 正在萃取
	STEP_DEGASSING,				   // 正在脱气
	STEP_COLORING,				   // 正在显色
	STEP_READ_REF,				   // 读取参比值
	STEP_READ_ABS,				   // 读取吸收值
	STEP_COOLING,				   // 正在冷却
	STEP_TITRATING,				   // 正在滴定
	STEP_WASH_LIQUID_LEVEL_PIPE,   // 清洗液位管
	STEP_PUMP_WATER,			   // 正在抽水
	STEP_WASH_DIGESTION_POOL = 51, // 清洗消解池

	STEP_AGENT_1 = 61,
	STEP_AGENT_2 = 62,
	STEP_AGENT_3 = 63,
	STEP_AGENT_4 = 64,
	STEP_AGENT_5 = 65,

	//	STEP_NOTIFY_CALCULAT_ABS									= 0x80,//128
	//	STEP_NOTIFY_CALCULAT_LOW_ABS							= 0x81,//129
	//	STEP_NOTIFY_CALCULAT_HIGHT_ABS						= 0x82,//130
	//	STEP_NOTIFY_FIT_CURVE											= 0x83,//131
	STEP_READ_BLANK_AD_DONE = 0x84,	   // 132
	STEP_READ_REACTION_AD_DANE = 0x85, // 133

	STEP_SWITCH_RANG = 0xA0, // 160

	STEP_PROC_DONE = 0x1FF,

} STEP_NAME_CODE;

/****************************故障代码********************************************/

typedef enum
{
	NO_FAULT = 0,
	TEMP_FAULT = 1,
	MOTOR_FAULT,
	LL_1_FAULT, // LL: LIQUID LEVEL					液位
	LL_2_FAULT, // LL: LIQUID LEVEL
	TITRATION_TEST_TIMEOUT,
	COMM_FAULT,
	LACK_SAMPLE = 7,
	LACK_DW, // DW:Distilled water				蒸馏水
	LACK_SS, // SS:Standard solution			标液
	LACK_MS, // MS：Mercury sulfate				硫酸汞
	LACK_PD, // PD:potassium dichromate 	重铬酸钾
	LACK_SA, // SA:sulphuric acid       	硫酸
	LACK_LYE,
	LACK_RELEASE_AGENT,	 // 释放剂
	LACK_NEUTRALIZER,	 // 中和液
	LACK_BUFFERS,		 // 缓冲剂
	LACK_CHROMOGENIC,	 // 显色剂
	LACK_ABSORBENT,		 // 吸收液
	LACK_EXTRACT,		 // 萃取液
	LACK_MARSKING_AGENT, // 掩蔽剂
	LACK_CLEANERS,		 // 清洗剂
	LACK_DIGESTION,		 // 消解液
	LACK_OXIDANT,		 // 氧化剂
	LACK_REGULATOR,		 // 调节剂
	LACK_REDUCER = 25,	 // 还原剂

	//	COMM_TIME_OUT = 27,//27通讯超时
	//	PC_CALIBRATE_FAULT = 28,						//校准失败,上位机使用
	//	PC_COMM_FAULT = 29,								//上位机使用
	HOT_FAULT = 30,				// 加热丝故障
	VALVE_ASSIGNED_FAULT = 31,	// 阀分配错误
	FAN_COOL_FAULT = 32,		// 风扇故障
	FAULT_SPECTRO_TROUBLE = 33, //--->光谱仪通讯失败
	LL1_LIGHT_FAULT = 34,		// 液位1校准失败
	LL2_LIGHT_FAULT = 35,		// 液位2校准失败
	DRAIN_WASTE_FAULT = 36,		// 排液故障
	TITRATION_FAIL = 37,		// 滴定失败
	IMI_SYRINGE_PUMP_FAULT = 38,
	RZ_SYRINGE_PUMP_FAULT = 39,
	TEMPER_CTRL_TIMEOUT = 40, // 温控超时
	LL1_WASH_FAULT = 41,	  // 液位1校准失败
	LL2_WASH_FAULT = 42,	  // 液位2校准失败

	LACK_RECOVER = 50,				  // 加标回收、质控水样//28
	LACK_LOW_SS = 51,				  // 低浓度标液，用于低点核查
	LACK_VERIFY_BLIND_SAMPLE = 52,	  // 盲样核查
	LACK_VERIFY_DISTILLED_WATER = 53, // 蒸馏水/零点液核查
	LACK_VERIFY_SS = 54,			  // 标液核查
	LACK_VERIFY_SPAN = 55,			  // 跨度核查

	LACK_REACTION_POOL_SOLUTION = 59, // 反应池中的溶液
	LACK_AGENT_1 = 61,
	LACK_AGENT_2 = 62,
	LACK_AGENT_3 = 63,
	LACK_AGENT_4 = 64,
	LACK_AGENT_5 = 65,

	FAULT_DEV_NO_INIT = 100,
	FAULT_SAVE_DEV_PARA = 101,
} FAULT_CODE;

/************************通讯协议流体命令的不同工作模式**************************************/
typedef enum
{
	WASH_WITH_SAMPLE = 1,
	PUMP_SAMPLE,
	WASH_WITH_SS, // SS:Standard solution
	PUMP_SS,	  // SS:Standard solution
	WASH_WITH_DW, // DW:Distilled water
	PUMP_DW,	  // DW:Distilled water
	REAGENT_1,
	REAGENT_2,
	REAGENT_3,
	REAGENT_4,
	REAGENT_5,
	REAGENT_6,
	DRAIN_WASTE,
	DRAIN_DS, // DS:DIGESTION SOLUTION  消解液
	CLEAR_SAMPLE,
	STIR
} LIQUID_FLOW_MODE;

/***************校准模式*************************/ // 用于上位机标签显示流程类型
typedef enum
{
	CALIB_UNDO = 0,
	CALIB_LOW_POINT = 1,
	CALIB_HIGH_POINT = 2,
	CALIB_EMPTY = 3,
	CALIB_HIGH_STD = 4,
	CALIB_STD_VERIFY = 5,
	CALIB_DISTILLED_VERIFY = 6,
} CALIB_MODE;

typedef enum
{
	DRAW_OK = 0,
	DRAW_TIMEOUT,
	DETECT_LEVEL1_FALSE,
	DETECT_LEVEL2_FALSE,
	FULL_BUBBLES,
	AUTO_CALIB_ERROR,
	TUBE_NON_EMPTY,
	NEVER_HAPPEN,
	CANNOT_FINISHED,
	NO_DRIV_FUN,
	ERROR_IMI_SYRINGE,
	ERROR_RZ_SYRINGE,
} DrawLiquidERROR;

typedef enum
{
	PUMP_LIQUID_OK = 0,
	PUMP_LIQUID_NO_REAGENT,
	PUMP_LIQUID_DETECT_DOWN_ERROR,
	PUMP_LIQUID_DETECT_UP_ERROR,
	PUMP_LIQUID_CHECK_LEVEL1_ERROR,
	PUMP_LIQUID_CHECK_LEVEL2_ERROR,
} PUMP_LIQUID_ERROR;

/***************************上位机指令***************************************/
typedef enum
{
	CMD_NONE = 0x00, // 无指令，用于指令清空
	CMD_SET_BAUD = 0x01,
	CMD_SET_LOCAL_ADD = 0x02,
	CMD_SET_SPECTRO_SCAN_TIMES = 0x03,	  // 更新光谱仪平均次数
	CMD_SET_SPECTRO_INTEGRAL_TIME = 0x04, // 更新光谱仪积分时间
	CMD_Reserve5 = 0x05,
	CMD_Reserve6 = 0x06,
	CMD_Reserve7 = 0x07,
	CMD_Reserve8 = 0x08,
	CMD_Reserve9 = 0x09,

	CMD_START_COMBI_VALVE = 0x0A,		// 10  开组合阀。组合阀以位或的方式组合，需要开启的位置一，不影响其他阀位。全开使用0xFFFFFFFF，写零不会有任何操作。
	CMD_STOP_COMBI_VALVE = 0x0B,		// 11  关组合阀。组合阀以位或的方式组合，需要关闭的位置一，不影响其他阀位。全关使用0xFFFFFFFF，写零不会有任何操作。
	CMD_PERISTALTIC_PUMP_OPT = 0x0C,	// 12 操作蠕动泵，参数1是执行圈数；参数2是速度，速度值为正则正转，为负则反转；
	CMD_DRAW_REAGENT = 0x0D,			// 13 抽取试剂。参数1：最大抽取圈数，超过此圈数报警，报警值01；参数2：抽取速度；参数3：抽取液位。
	CMD_DETERMIN_LEVEL_DOWN = 0x0E,		// 14 下行判定液位。参数1：最多执行圈数，超过此圈数报警，报警值02；参数2：下行速度；参数3：液位
	CMD_DETERMIN_LEVEL_UP = 0x0F,		// 15 上行判定液位。参数1：最多执行圈数，超过此圈数报警，报警值02；参数2：下行速度；参数3：液位
	CMD_START_TEMPER_CTRL = 0x10,		// 16 开启温控。模式1：按目标值和保温时间温控，温控函数为阻塞函数，直到完成温控之后才能解除阻塞，运行下一步骤。
	CMD_START_TEMPER_CTRL_CONST = 0x11, // 17 开启温控。模式2：按目标温度执行温控操作，一旦达到目标温度，程序即刻解除阻塞，但温控持续进行。
	CMD_STOP_TEMPER_CTRL = 0x12,		// 18 停止温控。对模式1和模式2均有效。
	CMD_READ_DARK_LED = 0x13,			// 19 读取暗电流
	CMD_READ_LED = 0x14,				// 20 读取光强值（AD值）

	CMD_SPECTRO_LIGHT_ON = 0x15,	 // 21 光谱仪开光源
	CMD_SPECTRO_LIGHT_OFF = 0x16,	 // 22 光谱仪关光源
	CMD_SPECTRO_SCAN = 0x17,		 // 23 光谱仪启动扫描
	CMD_SPECTRO_READ_DATA = 0x18,	 // 24 光谱仪读数据//数据存入保持寄存器29、30
	CMD_SYRINGE_RESET_ORIGIN = 0x19, // 25   //重新设置注射泵零点位
	CMD_SYRINGE_MOVE = 0x1A,		 // 26   //注射泵相对运动。相对于当前位置运动，正数推，负数抽。
	CMD_SYRINGE_MOVE_TO = 0x1B,		 // 27   //注射泵移动到绝对位置。绝对位置是相对于0点位置的数值。
	CMD_SYRINGE_STOP = 0x1C,		 // 28   //注射泵停止。
	CMD_STIR_ON = 0x1D,				 // 29	//开启搅拌
	CMD_STIR_OFF = 0x1E,			 // 30   //停止搅拌

	CMD_4_20_MA_CALIB_TEST = 0x1F, // 31 ////4-20ma输出测试。
	CMD_4_20_MA_CALIB_MAX = 0x20,  // 32   //4-20ma输出标定，标定上限电流。执行此命令后，将电流真实值写入寄存器
	CMD_4_20_MA_CALIB_MIN = 0x21,  // 33 //4-20ma输出标定，标定下限电流。执行此命令后，将电流真实值写入寄存器
	CMD_4_20_MA_NORMAL = 0x22,	   // 34
	CMD_SYRINGE_READ_POS = 0x23,   // 35 //读取注射泵当前位置

	CMD_RZ_HOLE = 0x29,  		 // 41 //润泽执行切换阀
	CMD_RZ_SPEED = 0x2A,  		 // 42 //润泽设置速度
	CMD_RZ_ML = 0x2B,  			 // 43 //润泽运行ml
	CMD_RZ_OPEN = 0x2C,			// 44 //润泽打开阀
	CMD_RZ_CLOSE = 0x2D,		// 45 //润泽关闭阀

	CMD_HC_HOLE = 0x2E,  		 // 46 //HC执行切换阀
	CMD_HC_SPEED = 0x2F,  		 // 47 //HC设置速度
	CMD_HC_ML = 0x30,  			 // 48 //HC运行ml
	CMD_HC_OPEN = 0x31,			// 49 //HC打开阀
	CMD_HC_CLOSE = 0x32,		// 50 //HC关闭阀

	CMD_START_TEMP_CTRL = 0x54, 	// 84 温控指令,配合"恒温温度"和"恒温时长"使用
	CMD_STOP_TEMP_CTRL = 0x55,		// 85
	CMD_START_ANALYSIS_PROC = 0x57, // 87 启动分析流程,配合"设备代码"、"工作量程"、"流程代码"、"恒温温度"、"恒温时长"、"水泵开启时长"使用
	CMD_START_CALIBRATE = 0x58,		// 88 自动标定
	CMD_START_CALIB_LOW = 0x59,		// 89 低点校准
	CMD_START_CALIB_HIGH = 0x5A,	// 90 高点校准
	CMD_START_CHECK_LOW = 0x5B,		// 91 低点核查
	CMD_START_CHECK_HIGH = 0x5C,	// 92 高点核查
	CMD_START_INIT_PROC = 0x5D,		// 93 清洗流程

	CMD_PERISTALTIC_PUMP_DRAW = 0x65,	// Decimal:101  	蠕动泵抽，配合“蠕动泵执行圈数”使用
	CMD_PERISTALTIC_PUMP_PUSH = 0x66,	// Decimal:102		蠕动泵推，配合“蠕动泵执行圈数”使用
	CMD_PERISTALTIC_PUMP_STOP = 0x67,	// Decimal:103		停止蠕动泵
	CMD_VALVE_OPEN = 0x68,				// Decimal:104		开启功能阀，配合“功能阀编号”使用
	CMD_VALVE_CLOSE = 0x69,				// Decimal:105		关闭功能阀，配合“功能阀编号”使用
	CMD_WASH_POOL_OPT = 0x6A,			// Decimal:106		清洗比色池，配合“试剂代码”和“试剂量”使用
	CMD_ADD_REAGENT_TO_POOL_OPT = 0x6B, // Decimal:107		向比色池加试剂，配合“试剂代码”和“试剂量”使用
	CMD_LL_AD_CALIB = 0x6C,				// Decimal:108		液位定量AD校准
	CMD_CLEAR_POOL = 0x6D,				// Decimal:109		清空比色池
	CMD_CLEAR_SAMPLE_PIPE = 0x6E,		// Decimal:110		清空样品管
	CMD_STIR_POOL = 0x6F,				// Decimal:111		搅拌比色池
	CMD_REAGENTS_PROFILL = 0x70,		// Decimal:112		试剂预充满
	CMD_INIT_SYRINGE_PUMP = 0x71,		// Decimal:113		测试
	CMD_UV_TEST = 0x72,					// Decimal:114		打开或关闭光谱仪测试
	CMD_FIT_CURVE = 0x73,				// Decimal:115		拟合曲线

	CMD_SET_PCB_DATE_TIME = 0xC8,	 // Decimal:200		设定时间
	CMD_GET_PCB_DATE_TIME = 0xC9,	 // Decimal:201		获取电路板时间
	CMD_SAVE_DEV_OPERAT_PARA = 0xFC, //
	CMD_SLAVE_HALT = 0xFD,			 // 停机不复位
	CMD_SAVE_DATA = 0xFE,			 // 保存控制板modbus数据
	CMD_SLAVE_RESET = 0xFF,			 // 控制板重启复位

	CMD_MAX_CODE = 0x100,

	CMD_START_NH3N = 0x78,			//120,启动主流程
	CMD_START_TP = 0x79,			//121
	CMD_START_COD = 0x7A,			//122
	CMD_START_TN = 0x7B,			//123
	CMD_START_CODMN = 0x7C,			//124

	CMD_RZSTART_NH3N = 0x7D,		//125,RZ启动主流程
	CMD_RZSTART_TP = 0x7E,			//126
	CMD_RZSTART_COD = 0x7F,			//127
	CMD_RZSTART_TN = 0x80,			//128
	CMD_RZSTART_CODMN = 0x81,		//129

	CMD_HCSTART_NH3N = 0x82,		//130,HC启动主流程
	CMD_HCSTART_TP = 0x83,			//131
	CMD_HCSTART_COD = 0x84,			//132
	CMD_HCSTART_TN = 0x85,			//133
	CMD_HCSTART_CODMN = 0x86,		//134

} HOST_CMD_t;

/****************************仪器代码（编号）*****************************************/
typedef enum
{
	DEV_NONE = (uint16_t)0x0000,
	DEV_COD_CR = 1,
	DEV_COD_CL = 2,
	DEV_TP = 3,
	DEV_TN = 4,
	DEV_NH_II = 5,
	DEV_COD_Mn = 6,
	DEV_TNR = 7,

	DEV_HCHO = 8,
	DEV_TMn = 9,
	DEV_Cr6 = 10,
	DEV_TCr = 11,
	DEV_Ni = 12,
	DEV_As_SURFACE_WATER = 13,
	DEV_As_SRC = 14,
	DEV_TFe = 15,
	DEV_TCu = 16,
	DEV_TCd = 17,
	DEV_THg = 18,
	DEV_VOLATILE_PHENOL = 19,
	DEV_Pb = 20,
	//	DEV_NH_I							= 21,
	//	DEV_Zn								= 22,
	//	DEV_ELECTROCHEMISTRY	= 23,
	//	DEV_CYANIDE						= 24,
	DEV_MAX = 20,
	DEV_SET = 0x01FF,

} DEVICE_CODE;

// typedef enum{
//	PROC_TYPE_NULL											= 0,
//	RROC_NORMAL										= 1,
//	RROC_AUTO_RROC_CALIBRAT_LOW					= 2,
//	RROC_AUTO_RROC_CALIBRAT_HIGH				= 3,
//	RROC_CALIBRAT_LOW							= 4,
//	PROC_TYPE_CALIBRAT_HIGH							= 5,
//	PROC_TYPE_CHECK_LOW               	= 6,
//	RROC_CALIBRAT_HIGH								= 7,
//	PROC_INIT											= 8,
//	PROC_TYPE_MAX												=0X100,
//
// }PROC_CODE;
typedef enum
{
	PROC_NULL = 0,
	RROC_NORMAL = 1,				 // 测量流程
	RROC_AUTO_RROC_CALIBRAT_LOW = 2, // 自动标定
	RROC_AUTO_RROC_CALIBRAT_HIGH = 3,
	RROC_CALIBRAT_LOW = 4,			// 低点校准
	RROC_CALIBRAT_HIGH = 5,			// 高点校准
	PROC_INIT = 6,					// 清洗流程
	RROC_VERIFY_LOW = 7,			// 零点核查
	RROC_VERIFY_HIGH = 8,			// 标液核查
	RROC_QC_VERIFY = 9,				// 质控核查
	RROC_RECOVER = 10,				// 加标回收
	PROC_VERIFY_BLIND = 11,			// 盲样核查
	PROC_DISTILLED_WATER_TEST = 12, // 零点液测试
	PROC_VERIFY_SPAN = 13,			// 跨度核查

} PROC_CODE; // 流程代码

#endif
