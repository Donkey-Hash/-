/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOI);

  /**/
  LL_GPIO_SetOutputPin(GPIOF, LL_PW_EN_Pin|ADC1119_RESET_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOC, ADC1119_SDA_Pin|ADC1119_SCL_Pin);

  /**/
  LL_GPIO_SetOutputPin(Y23_GPIO_Port, Y23_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOH, Y19_Pin|Y18_Pin|Y17_Pin|Y16_Pin
                          |Y15_Pin|Y13_Pin|MOTOR_EN_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOB, Y14_Pin|Y5_Pin|Y4_Pin|Y3_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOD, Y22_Pin|Y21_Pin|NAND_PW_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOG, Y12_Pin|Y11_Pin|Y10_Pin|Y9_Pin
                          |Y8_Pin|Y7_Pin|Y6_Pin);

  /**/
  LL_GPIO_SetOutputPin(OTG_FS_PWR_GPIO_Port, OTG_FS_PWR_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOI, Y1_Pin|Y2_Pin);

  /**/
  LL_GPIO_ResetOutputPin(GPIOH, DA2_SCL_Pin|DA2_SDA_Pin|ABS_PW_EN_Pin|LED1_Pin
                          |MOTOR_DIR_Pin);

  /**/
  LL_GPIO_ResetOutputPin(Y24_GPIO_Port, Y24_Pin);

  /**/
  LL_GPIO_ResetOutputPin(GPIOI, MOTOR_STDBY_Pin|UART5_RD_Pin);

  /**/
  LL_GPIO_ResetOutputPin(UART1_RD_GPIO_Port, UART1_RD_Pin);

  /**/
  LL_GPIO_ResetOutputPin(UART1_PWR_OUT_GPIO_Port, UART1_PWR_OUT_Pin);

  /**/
  LL_GPIO_ResetOutputPin(GPIOB, UART8_RD_Pin|UART8_PWR_OUT_Pin);

  /**/
  GPIO_InitStruct.Pin = LL_PW_EN_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LL_PW_EN_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = ADC1119_RESET_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(ADC1119_RESET_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = ADC1119_SDA_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(ADC1119_SDA_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = ADC1119_SCL_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(ADC1119_SCL_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DA2_SCL_Pin|DA2_SDA_Pin|ABS_PW_EN_Pin|LED1_Pin
                          |Y19_Pin|Y18_Pin|Y17_Pin|Y16_Pin
                          |Y15_Pin|Y13_Pin|MOTOR_DIR_Pin|MOTOR_EN_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = Y24_Pin|Y23_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = Y14_Pin|Y5_Pin|Y4_Pin|Y3_Pin
                          |UART8_RD_Pin|UART8_PWR_OUT_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = Y22_Pin|Y21_Pin|NAND_PW_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = Y12_Pin|Y11_Pin|Y10_Pin|Y9_Pin
                          |Y8_Pin|Y7_Pin|UART1_PWR_OUT_Pin|Y6_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = MOTOR_DIAG_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(MOTOR_DIAG_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = OTG_FS_PWR_FAULT_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(OTG_FS_PWR_FAULT_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = OTG_FS_PWR_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(OTG_FS_PWR_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = MOTOR_INDEX_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(MOTOR_INDEX_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = MOTOR_STDBY_Pin|Y1_Pin|UART5_RD_Pin|Y2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = UART1_RD_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(UART1_RD_GPIO_Port, &GPIO_InitStruct);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTF, LL_SYSCFG_EXTI_LINE10);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_10;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  LL_GPIO_SetPinPull(ADC1119_READY_GPIO_Port, ADC1119_READY_Pin, LL_GPIO_PULL_UP);

  /**/
  LL_GPIO_SetPinMode(ADC1119_READY_GPIO_Port, ADC1119_READY_Pin, LL_GPIO_MODE_INPUT);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
  NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
