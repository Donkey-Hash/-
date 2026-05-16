# Modbus（MB3 独立寄存器表）运行一段时间卡死/复位问题排查记录

日期：2026-05-03  
工程位置（本次排查的工作区）：`.\0429重新\nanoWAys20240724改单片机+测润泽`

## 1. 背景与现象

在按 `USART3_MB3_NewRegisterMap_Example.txt` / `USART3_Modbus_Design_Notes.txt` 的思路，将不同 Modbus 总线（例如 USART1/USART3/USART6）映射到“各自独立的寄存器组”之后，现场出现如下现象：

- 程序运行一段时间后卡死，随后复位（常见表现为 IWDG/HardFault 触发复位）
- 复位不一定立刻发生，具有“随机性/跑一段时间才出现”的特征

这类症状高度符合 **内存被破坏（越界写/读）**：先破坏某个任务栈/OS 控制块/全局变量，随后在某个时刻引发异常或喂狗失败。

## 2. 关键结论（根因）

根因是 **MB3（USART3）独立寄存器缓冲区长度不足**，但 Modbus 回调仍按 512 个寄存器的范围进行读写，导致越界访问。

### 2.1 回调函数使用的“合法寄存器范围”

工程里寄存器回调的地址范围判定使用了 `Modbus.h` 中的宏：

- `REG_INPUT_NREGS = 512`
- `REG_HOLDING_NREGS = 512`

位置：`.\0429重新\nanoWAys20240724改单片机+测润泽\FreeModbus\modbus\include\Modbus.h:1`

也就是说：协议栈认为输入寄存器/保持寄存器都允许访问 **1~512**（各 512 个）。

### 2.2 MB3 独立表实际只分配了 110/400（严重不足）

在问题版本中，MB3 的独立缓冲区定义为：

- Holding：`static USHORT _mb3_holding_buf[110]`
- Input：`static USHORT _mb3_input_buf[400]`

位置（已备份保留）：`.\0429重新\nanoWAys20240724改单片机+测润泽\MDK-ARM\modbusdata_mb3.c.bak_test:1`

当主站对 MB3 访问超过这些上限的地址时（例如读/写较高地址，或一次读写较多寄存器），回调会访问：

- `pHoldBuf[iRegIndex]`
- `pInpBuf[iRegIndex]`

从而发生 **越界写/越界读**，直接破坏 RAM，最终导致卡死/复位。

### 2.3 为什么“改完映射后才出现”

因为改动后 `FreeModbus/modbus/Modbus.c` 会根据 `g_mb_inst` 把寄存器缓冲区切到 MB3 独立表：

- `g_mb_inst == 3` -> 使用 `usRegHoldingBuf3/usRegInputBuf3`
- 否则使用原来的共享表 `usRegHoldingBuf/usRegInputBuf`

位置：`.\0429重新\nanoWAys20240724改单片机+测润泽\FreeModbus\modbus\Modbus.c:1`

也就是说：只有当 USART3（MB3）被轮询/处理到时，才会走到那套更小的数组，更容易触发越界。

## 3. 修复方案（已在当前工程落地）

### 3.1 将 MB3 缓冲区长度与回调边界一致（核心修复）

新增（或恢复）UTF-8 编码的 `MDK-ARM/modbusdata_mb3.c`，将 MB3 的 Holding/Input 缓冲区长度改为：

- `REG_HOLDING_NREGS`（512）
- `REG_INPUT_NREGS`（512）

文件：`.\0429重新\nanoWAys20240724改单片机+测润泽\MDK-ARM\modbusdata_mb3.c:1`

这样无论主站怎么读写 1~512 范围内寄存器，都不会发生越界。

> 说明：之前的 `modbusdata_mb3.c` 带中文注释（GBK/CP936 编码），在某些自动化 patch 流程里会出现 “invalid utf-8 sequence” 的问题，所以本次用新增 UTF-8 文件的方式落地修复，同时保留原文件为 `.bak_test` 以便对照。

### 3.2 补齐 MB6 轮询时的实例标记（降低误用风险）

工程采用 `g_mb_inst` 来区分当前在处理的 Modbus 实例。`eMBPoll()` / `eMB3Poll()` 已设置为 1/3，但 `eMB6Poll()` 原本未设置，存在“上一轮残留为 3”的风险。

修复：在 `eMB6Poll()` 开头加入 `g_mb_inst = 6;`

文件：`.\0429重新\nanoWAys20240724改单片机+测润泽\FreeModbus\modbus\mb.c:741`

这能避免 USART6 的请求在 `g_mb_inst` 残留时错误使用 MB3 的寄存器表（尤其在 MB3 表较小时会放大越界概率）。

## 4. 如何验证修复是否生效

建议按“最容易触发越界”的方式验证：

1. 让上位机/主站对 MB3 做较大范围的读写（例如读 input/holding 的高地址段，或一次读取很多寄存器）
2. 保持与现场一致的轮询周期与并发（USART1/USART3/USART6 同时工作更接近真实场景）
3. 连续运行超过之前“跑一段时间就复位”的时间窗口

如果根因是越界破坏内存，修复后复位现象应明显消失。

## 5. 仍需注意的潜在问题（不一定导致复位，但会导致“行为怪异”）

以下点不一定造成“卡死复位”，但可能造成业务逻辑异常，建议后续按需求确认：

1. **`HaveNewCMDFlag` 的触发语义**  
   `eMBRegHoldingCB()` 在写入寄存器时会置位 `HaveNewCMDFlag`，并且部分逻辑仍写回 `usRegHoldingBuf[0]`（共享表）。  
   当 MB3 使用独立 holding 表时，可能出现“MB3 写寄存器触发了 USART1 的命令处理”的错配。  
   文件：`.\0429重新\nanoWAys20240724改单片机+测润泽\FreeModbus\modbus\Modbus.c:1`

2. **回调函数里存在与实际宏不一致的二次边界判断**  
   当前 `Modbus.c` 里有 `MAX_REGS = 1024` 之类的检查，这个数值与 `REG_*_NREGS=512` 并不一致，容易造成“检查通过但逻辑范围不一致”的维护风险。  
   建议后续统一以 `REG_INPUT_NREGS/REG_HOLDING_NREGS` 为准（本次修复已先从根因入手，确保不会越界）。

## 7. 二次排查补充：仍出现“卡死 -> IWDG 复位”

在修复 MB3 缓冲区越界后，如果现场仍出现“卡死，约 30+ 秒后触发 IWDG 复位”，更像是 **系统被某个无限等待阻塞**，导致三条 Modbus 任务无法继续执行/喂狗。

本工程的 IWDG 配置为：
- Prescaler = 256，Reload = 4095（典型约 32 秒量级超时）
- 文件：`.\0429重新\nanoWAys20240724改单片机+测润泽\Src\iwdg.c`

### 7.1 高风险点：串口发送完成等待（TC）无限 while

在 `FreeModbus` 的 RTU 发送状态机中，发送完最后一个字节后会调用 `xMBPortSerialWaitTC*()` 等待 TC 置位。
本工程原实现为：

- `while(!LL_USART_IsActiveFlag_TC(pModbusUart3)){}`
- `while(!LL_USART_IsActiveFlag_TC(pModbusUart6)){}`

一旦出现“TC 异常不置位”，CPU 会卡在 while 中，进而导致：
- Modbus 轮询任务拿不到时间片
- 互斥锁无法释放（或其它任务无法继续）
- 最终 IWDG 复位

修复（已落地）：
- 给 `xMBPortSerialWaitTC/xMBPortSerialWaitTC3/xMBPortSerialWaitTC6` 增加 guard 超时，避免无限阻塞  
  文件：`.\0429重新\nanoWAys20240724改单片机+测润泽\FreeModbus\port\portserial.c`

### 7.2 高风险点：三条总线共用互斥锁且等待为 osWaitForever

`UartsRxTask()/USART3Task()/USART6Task()` 在进入 `eMB*Poll()` 前会获取同一个互斥锁 `ModbusBusMutexHandle`，原来等待策略为 `osWaitForever`。

如果某次出现“持锁区异常卡住”，其它两路任务会永久等待，连带导致喂狗路径全部停摆。

修复（已落地）：
- 将互斥锁获取改为有限超时（200ms），超时则跳过本次 poll 并继续喂狗  
  文件：`.\0429重新\nanoWAys20240724改单片机+测润泽\Tasks\src\uartsrxtask.c`

### 7.3 高风险点：USART 产生 ORE/FE/NE/PE 错误后未清除，导致中断风暴

在 RS485/多总线高负载场景下，USART 可能出现：
- ORE（Overrun）
- FE（Framing error）
- NE（Noise）
- PE（Parity error）

如果这些错误标志未被清除，IRQ 可能被错误标志持续触发，CPU 长时间陷入中断处理，任务无法运行并最终触发 IWDG 复位。

修复（已落地）：
- 在 `USART1_IRQHandler/USART3_IRQHandler/USART6_IRQHandler` 的 MB 从站分支中，增加对 ORE/FE/NE/PE 的清除（读 SR/DR 序列）  
  文件：`.\0429重新\nanoWAys20240724改单片机+测润泽\Src\stm32f4xx_it.c`

### 7.4 风险点：事件投递与信号量释放的先后顺序（可能导致丢事件）

原实现中先 `osSemaphoreRelease()` 再设置 `xEventInQueue/eQueuedEvent`，存在竞态：任务被唤醒后可能读到“无事件”，导致丢事件。

修复（已落地）：
- 调整为“先写事件，再释放信号量”  
  文件：`.\0429重新\nanoWAys20240724改单片机+测润泽\FreeModbus\port\portevent.c`

## 6. 本次改动清单（便于回溯）

- 新增：`.\0429重新\nanoWAys20240724改单片机+测润泽\MDK-ARM\modbusdata_mb3.c:1`
- 修改：`.\0429重新\nanoWAys20240724改单片机+测润泽\FreeModbus\modbus\mb.c:741`
- 修改：`.\0429重新\nanoWAys20240724改单片机+测润泽\FreeModbus\port\portserial.c:1`
- 修改：`.\0429重新\nanoWAys20240724改单片机+测润泽\Tasks\src\uartsrxtask.c:1`
- 修改：`.\0429重新\nanoWAys20240724改单片机+测润泽\Src\stm32f4xx_it.c:1`
- 修改：`.\0429重新\nanoWAys20240724改单片机+测润泽\FreeModbus\port\portevent.c:1`
- 备份保留（问题版本）：`.\0429重新\nanoWAys20240724改单片机+测润泽\MDK-ARM\modbusdata_mb3.c.bak_test:1`
