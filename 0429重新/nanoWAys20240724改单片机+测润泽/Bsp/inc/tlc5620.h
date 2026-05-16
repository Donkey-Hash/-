#ifndef __TLC_5620_H__
#define __TLC_5620_H__
#include "stm32f4xx_hal.h"
#include "main.h"
//#define DA_DATA_Pin GPIO_PIN_1
//#define DA_DATA_GPIO_Port GPIOB
//#define DA_CLK_Pin GPIO_PIN_2
//#define DA_CLK_GPIO_Port GPIOB
//#define DA_LOAD_Pin GPIO_PIN_6
//#define DA_LOAD_GPIO_Port GPIOC
//#define DA_LDAC_Pin GPIO_PIN_7
//#define DA_LDAC_GPIO_Port GPIOC

#define	V_REF_DAC					2.5
#define I_MAX_LED					30

#define LL_BLANK_AD_MIN				12000
#define LL_BLANK_AD_MAX				15000

#define ADCA							0
#define ADCB							1
#define ADCC							2
#define ADCD							3

#define ADC_CH_ABS_LED				ADCA
#define ADC_CH_LL1_LED				ADCB
#define	ADC_CH_LL2_LED				ADCC
#define ADC_CH_TEST						ADCD

#define DA_DATA_LOW	 			{HAL_GPIO_WritePin(DA_DATA_GPIO_Port,DA_DATA_Pin, GPIO_PIN_RESET);}
#define DA_DATA_HIGH	 		{HAL_GPIO_WritePin(DA_DATA_GPIO_Port,DA_DATA_Pin, GPIO_PIN_SET);}

#define DA_CLK_LOW	 			{HAL_GPIO_WritePin(DA_CLK_GPIO_Port,DA_CLK_Pin, GPIO_PIN_RESET);}
#define DA_CLK_HIGH	 			{HAL_GPIO_WritePin(DA_CLK_GPIO_Port,DA_CLK_Pin, GPIO_PIN_SET);}

#define DA_LOAD_LOW	 			{HAL_GPIO_WritePin(DA_LOAD_GPIO_Port,DA_LOAD_Pin, GPIO_PIN_RESET);}
#define DA_LOAD_HIGH	 		{HAL_GPIO_WritePin(DA_LOAD_GPIO_Port,DA_LOAD_Pin, GPIO_PIN_SET);}

#define DA_LDAC_LOW	 			{HAL_GPIO_WritePin(DA_LDAC_GPIO_Port,DA_LDAC_Pin,	GPIO_PIN_RESET);}
#define DA_LDAC_HIGH	 		{HAL_GPIO_WritePin(DA_LDAC_GPIO_Port,DA_LDAC_Pin, GPIO_PIN_SET);}


extern float CurrentLEDAbs;

void InitTLC5620(void);
void SetTlc5620byDigit(uint8_t channel,uint8_t value);
void SetAbsorbLEDCurrent(float current);
void OpenAbsorbLED(void);
void CloseAbsorbLED(void);
void OpenLLLED(void);
void CloseLLLED(void);
void SetLL1LEDCurrent(float current);
void SetLL2LEDCurrent(float current);

#endif
