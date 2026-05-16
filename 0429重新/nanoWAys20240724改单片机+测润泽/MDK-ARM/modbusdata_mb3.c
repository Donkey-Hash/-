#include "modbusdata_mb3.h"

/*
 * MB3 (USART3) 独立寄存器缓冲区
 *
 * 注意：回调函数 eMBRegInputCB/eMBRegHoldingCB 的边界检查使用
 * REG_INPUT_NREGS / REG_HOLDING_NREGS（在 Modbus.h 中为 512）。
 * 因此这里的缓冲区长度必须至少覆盖这些范围，否则会发生越界访问，
 * 进而导致随机卡死/HardFault/看门狗复位等问题。
 */

static USHORT _mb3_holding_buf[REG_HOLDING_NREGS] = {0};
static USHORT _mb3_input_buf[REG_INPUT_NREGS] = {0};
static UCHAR _mb3_coil_buf[COIL_NCOILS / 8] = {0};
static UCHAR _mb3_discrete_buf[DISCRETE_INPUT_NDISCRETES / 8] = {0};

/* 全局指针（供 Modbus 回调函数与业务逻辑使用） */
USHORT *usRegHoldingBuf3 = _mb3_holding_buf;
USHORT *usRegInputBuf3 = _mb3_input_buf;
UCHAR *usCoilBuf3 = _mb3_coil_buf;
UCHAR *usDiscreteInputBuf3 = _mb3_discrete_buf;

/* 结构体映射（按 16bit 寄存器对齐） */
MODBUS3_HOLDING_REG_t *pMb3HoldData = (MODBUS3_HOLDING_REG_t *)_mb3_holding_buf;
MODBUS3_INPUT_REG_t *pMb3InputData = (MODBUS3_INPUT_REG_t *)_mb3_input_buf;

/* 当前正在处理的 Modbus 实例（1/3/6...），由 eMBPoll/eMB3Poll 等设置 */
volatile uint8_t g_mb_inst = 0;

