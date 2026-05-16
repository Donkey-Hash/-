/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_dac.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_iwdg.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_rtc.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Y4_Pin LL_GPIO_PIN_2
#define Y4_GPIO_Port GPIOE
#define Y3_Pin LL_GPIO_PIN_3
#define Y3_GPIO_Port GPIOE
#define Y2_Pin LL_GPIO_PIN_4
#define Y2_GPIO_Port GPIOE
#define Y1_Pin LL_GPIO_PIN_5
#define Y1_GPIO_Port GPIOE
#define X1_Pin LL_GPIO_PIN_6
#define X1_GPIO_Port GPIOE
#define X2_Pin LL_GPIO_PIN_13
#define X2_GPIO_Port GPIOC
#define ADC_TEMPER1_Pin LL_GPIO_PIN_0
#define ADC_TEMPER1_GPIO_Port GPIOA
#define ADC_TEMPER2_Pin LL_GPIO_PIN_1
#define ADC_TEMPER2_GPIO_Port GPIOA
#define DAC_IOUT_Pin LL_GPIO_PIN_4
#define DAC_IOUT_GPIO_Port GPIOA
#define DAC_ABS_Pin LL_GPIO_PIN_5
#define DAC_ABS_GPIO_Port GPIOA
#define PWM_LL1_Pin LL_GPIO_PIN_6
#define PWM_LL1_GPIO_Port GPIOA
#define AD_LL1_Pin LL_GPIO_PIN_7
#define AD_LL1_GPIO_Port GPIOA
#define PWM_LL2_Pin LL_GPIO_PIN_0
#define PWM_LL2_GPIO_Port GPIOB
#define AD_LL2_Pin LL_GPIO_PIN_1
#define AD_LL2_GPIO_Port GPIOB
#define ADC_RD_Pin LL_GPIO_PIN_14
#define ADC_RD_GPIO_Port GPIOE
#define FRAM_WP_Pin LL_GPIO_PIN_15
#define FRAM_WP_GPIO_Port GPIOE
#define FRAM_SCL_Pin LL_GPIO_PIN_10
#define FRAM_SCL_GPIO_Port GPIOB
#define FRAM_SDA_Pin LL_GPIO_PIN_11
#define FRAM_SDA_GPIO_Port GPIOB
#define AD_INV_Pin LL_GPIO_PIN_12
#define AD_INV_GPIO_Port GPIOB
#define AD_SCK_Pin LL_GPIO_PIN_13
#define AD_SCK_GPIO_Port GPIOB
#define AD_MISO_Pin LL_GPIO_PIN_14
#define AD_MISO_GPIO_Port GPIOB
#define AD_MOSI_Pin LL_GPIO_PIN_15
#define AD_MOSI_GPIO_Port GPIOB
#define MbMaster_TX_Pin LL_GPIO_PIN_8
#define MbMaster_TX_GPIO_Port GPIOD
#define MbMaster_RX_Pin LL_GPIO_PIN_9
#define MbMaster_RX_GPIO_Port GPIOD
#define MbMaster_DIR_Pin LL_GPIO_PIN_10
#define MbMaster_DIR_GPIO_Port GPIOD
#define LED1_Pin LL_GPIO_PIN_11
#define LED1_GPIO_Port GPIOD
#define LED2_Pin LL_GPIO_PIN_12
#define LED2_GPIO_Port GPIOD
#define MOTOR_CLK_Pin LL_GPIO_PIN_13
#define MOTOR_CLK_GPIO_Port GPIOD
#define MOTOR_DIR_Pin LL_GPIO_PIN_14
#define MOTOR_DIR_GPIO_Port GPIOD
#define MOTOR_EN_Pin LL_GPIO_PIN_15
#define MOTOR_EN_GPIO_Port GPIOD
#define ExDEV_TX_Pin LL_GPIO_PIN_6
#define ExDEV_TX_GPIO_Port GPIOC
#define ExDEV_RX_Pin LL_GPIO_PIN_7
#define ExDEV_RX_GPIO_Port GPIOC
#define Buzzer_Pin LL_GPIO_PIN_8
#define Buzzer_GPIO_Port GPIOC
#define OTG_FS_PWR_EN_Pin LL_GPIO_PIN_9
#define OTG_FS_PWR_EN_GPIO_Port GPIOC
#define MbSlave_DIR_Pin LL_GPIO_PIN_8
#define MbSlave_DIR_GPIO_Port GPIOA
#define HMI_TX_Pin LL_GPIO_PIN_9
#define HMI_TX_GPIO_Port GPIOA
#define HMI_RX_Pin LL_GPIO_PIN_10
#define HMI_RX_GPIO_Port GPIOA
#define PWM_HOT_Pin LL_GPIO_PIN_15
#define PWM_HOT_GPIO_Port GPIOA
#define FLASH_SCK_Pin LL_GPIO_PIN_10
#define FLASH_SCK_GPIO_Port GPIOC
#define FLASH_MISO_Pin LL_GPIO_PIN_11
#define FLASH_MISO_GPIO_Port GPIOC
#define FLASH_MOSI_Pin LL_GPIO_PIN_12
#define FLASH_MOSI_GPIO_Port GPIOC
#define FLASH_NSS_Pin LL_GPIO_PIN_0
#define FLASH_NSS_GPIO_Port GPIOD
#define FLASH_WP_Pin LL_GPIO_PIN_1
#define FLASH_WP_GPIO_Port GPIOD
#define Y19_Pin LL_GPIO_PIN_2
#define Y19_GPIO_Port GPIOD
#define Y18_Pin LL_GPIO_PIN_3
#define Y18_GPIO_Port GPIOD
#define Y17_Pin LL_GPIO_PIN_4
#define Y17_GPIO_Port GPIOD
#define Y16_Pin LL_GPIO_PIN_5
#define Y16_GPIO_Port GPIOD
#define Y15_Pin LL_GPIO_PIN_6
#define Y15_GPIO_Port GPIOD
#define Y14_Pin LL_GPIO_PIN_7
#define Y14_GPIO_Port GPIOD
#define Y13_Pin LL_GPIO_PIN_3
#define Y13_GPIO_Port GPIOB
#define Y12_Pin LL_GPIO_PIN_4
#define Y12_GPIO_Port GPIOB
#define Y11_Pin LL_GPIO_PIN_5
#define Y11_GPIO_Port GPIOB
#define Y10_Pin LL_GPIO_PIN_6
#define Y10_GPIO_Port GPIOB
#define Y9_Pin LL_GPIO_PIN_7
#define Y9_GPIO_Port GPIOB
#define Y8_Pin LL_GPIO_PIN_8
#define Y8_GPIO_Port GPIOB
#define Y7_Pin LL_GPIO_PIN_9
#define Y7_GPIO_Port GPIOB
#define Y6_Pin LL_GPIO_PIN_0
#define Y6_GPIO_Port GPIOE
#define Y5_Pin LL_GPIO_PIN_1
#define Y5_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/*
 * USART3 串口角色选择
 * -------------------
 * 0: USART3 作为外设 Modbus 主站口（DMA + IDLE + TC），由 Bsp/src/ModbusMaster.c 使用。
 * 1: USART3 作为第二条 Modbus-RTU 从站口（MB3），与 USART1 共享同一套寄存器映射/回调。
 *
 * 说明：
 * - Modbus 总线通常只允许一个主站；当 USART3 作为从站口时，必须避免同时启用旧的 ModbusMaster 主站逻辑，
 *   否则会出现中断入口/方向控制/DMA 资源冲突，导致通信异常。
 */
#define APP_USART3_MODBUS_SLAVE_MB3 1

/*
 * USART6 串口角色选择
 * -------------------
 * 0: USART6 用于外设通信（DMA + IDLE），例如光谱仪等外设链路。
 * 1: USART6 作为第三条 Modbus-RTU 从站口（MB6），与 USART1/USART3 共用同一套寄存器映射/回调。
 *
 * 说明：
 * - MB6 采用与 MB3 相同的“第二实例”扩展方式（事件/RTU 状态机/定时器/port层均独立），因此可与 USART1/MB3 并行运行。
 * - 当 USART6 作为 MB6 从站口时，必须避免同时启用外设通信的 DMA+IDLE 接收逻辑（ExDevDMAConfig 等），否则会资源冲突。
 */
#define APP_USART6_MODBUS_SLAVE_MB6 1

/*
 * USART6 Modbus 任务开关
 * -------------------
 * 0: 不创建 USART6Task
 * 1: 创建 USART6Task（MB6 从站轮询任务）
 */
#define APP_USART6_MODBUS_TASK_ENABLE APP_USART6_MODBUS_SLAVE_MB6

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
