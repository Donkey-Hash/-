
#ifndef __YS_SYRINGE_PUMP_H___
#define __YS_SYRINGE_PUMP_H___
#include <stdint.h>
#include <stdbool.h>
// #include "communicate.h"

// #define MODBUS_MOTOR
#include "uartsrxtask.h"

extern volatile u8 RZdelayflag;	//0不做处理。置1则取消延时

typedef enum
{
	YS_SYRINGE_CMD_NONE = 0x00, // 无指令，用于指令清?
	YS_SYRINGE_CMD_SET_RUN_SPEED = 1,
	YS_SYRINGE_CMD_MOVE_TO = 2,
	YS_SYRINGE_CMD_MOVE_TO_B = 3,
	YS_SYRINGE_CMD_STEPS_MOVE = 4,
	YS_SYRINGE_CMD_STEPS_MOVE_B = 5,
	YS_SYRINGE_CMD_SET_FULL_SCALE_STEPS = 6,
	YS_SYRINGE_CMD_SET_FULL_SCALE_STEPS_B = 7,

	YS_SYRINGE_CMD_SET_BAUD = 10,
	YS_SYRINGE_CMD_SET_ADDR = 11,
	YS_SYRINGE_CMD_SET_MICRO_STEPS = 12,
	YS_SYRINGE_CMD_RUNNING_MORTOR = 13,
	YS_SYRINGE_CMD_SET_WORK_MODEL = 14,
	YS_SYRINGE_CMD_SET_ANGLES = 15,

	YS_SYRINGE_CMD_STOP_MOTOR = 20,
	YS_SYRINGE_CMD_RESET_SYS_ORIGIN = 21,

	YS_SYRINGE_CMD_SWITCH_VALVE_OPEN = 101,		  // 打开切换阀   目标位
	YS_SYRINGE_CMD_SWITCH_VALVE_CLOSE = 102,	  // 关闭切换阀   常态位
	YS_SYRINGE_CMD_SWITCH_VALVE_PARAM_SAVE = 103, // 切换阀参数保存

	YS_SYRINGE_CMD_SLAVE_RESET = 0xFF, // 控制板重启复位
	YS_SYRINGE_CMD_SAVE_DATA = 0xFE,   // 保存控制板modbus数据
	YS_SYRINGE_CMD_MAX_CODE = 0x100,

} YS_SYRINGE_CMD_t; /// 上位机指令

int32_t ReadPos(void);
void StopYsSyringe(void);
void ResetYsSyringeOrigin(void);
bool SetYsSyringeSpeed(void);
void MoveYsSyringeTo(int32_t steps);
void MoveYsSyringe(int32_t steps);


/*
1：废液阀、复位位置
2：清洗费液阀
3：消解下阀
4：蒸馏水阀
5：水样阀
6：核查阀
7：量程阀
8：试剂一
9：试剂二
10：试剂三(或者空)
*/

//TP_flag:
void RUNZE_SETHOLE(u8 num);	//1~10

void RUNZE_SET_ML(float ml);	//0~6

void RUNZE_SET_SPEED(u16 speed);	//1~500

void RUNZE_SET_STOP(void);
void RUNZE_SET_NC(void);	//开阀
void RUNZE_SET_NO(void);	//关阀。默认是关

//恒创
void HC_SET_ML(float ml);	//0.0~5.0
void HC_SETHOLE(u8 num);	//0~10,0时阻塞
void HC_SET_SPEED(u16 speed);	//1~1500
void HC_SET_STOP(void);
void HC_SET_NC(void);	//开阀
void HC_SET_NO(void);	//关阀。默认是关




#endif
