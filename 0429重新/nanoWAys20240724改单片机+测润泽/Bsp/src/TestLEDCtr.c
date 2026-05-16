

#include <stdbool.h>
#include "mtimers.h"
#include "TestLEDCtr.h"
#include "modbusdata.h"
#include "FlashUserData.h"
extern float LL1InitCurrent;
extern float LL2InitCurrent;

float CurrentLEDAbs = 10; // 单位：mA

/*******************************************************************************
 * Function Name  : 通过数字量设置大DAC通道输出
 * Description    : SetTlc5620byDigit.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void SetTlc5620byDigit(uint8_t channel, uint8_t value)
{
	//	uint8_t i;
	////	tmp = channel;
	////	tmp=0x03&tmp;
	////	tmp=tmp<<1;//-2
	////	tmp=tmp+1;//-2
	////	tmp=tmp<<5;//-2
	////	tmp=tmp<<6;
	//	channel &= 0x03;
	//	channel <<= 6;
	//	DA_CLK_HIGH;
	//	DA_DATA_LOW;
	//
	//	DA_LOAD_HIGH;
	//	DA_LDAC_HIGH;
	//
	//	for(i=0;i<3;i++)//输出路数号
	//	{
	//		DA_CLK_HIGH;
	//		if((channel & 0x80) == 0x80)
	//			DA_DATA_HIGH
	//		else
	//			DA_DATA_LOW
	//		DA_CLK_LOW;
	//		channel <<= 1;
	//	}
	//	for(i=0;i<8;i++)//输出DA值
	//	{
	//		DA_CLK_HIGH;
	//		if((value & 0x80) == 0x80)
	//			DA_DATA_HIGH
	//		else
	//			DA_DATA_LOW
	//		DA_CLK_LOW
	//		value <<= 1;
	//	}
	//
	//	DA_LOAD_LOW
	//	DA_LOAD_LOW
	//	DA_LOAD_HIGH
	//	DA_LDAC_LOW
	//	DA_LDAC_LOW
	//	DA_LDAC_HIGH
}
/*******************************************************************************
 * Function Name  : InitTLC5620
 * Description    : 初始化TLC5620
 * Input          : 通道号，电压值（0-2.5V）
 * Output         : None
 * Return         : None
 *******************************************************************************/
void InitTLC5620(void)
{
	SetTlc5620byDigit(0, 0);
	SetTlc5620byDigit(1, 0);
	SetTlc5620byDigit(2, 0);
	SetTlc5620byDigit(3, 0);
}
/*******************************************************************************
 * Function Name  : 通过电压值设置大DAC通道输出
 * Description    : SetTlc5620byVoltage.
 * Input          : 通道号，电压值（0-2.5V）
 * Output         : None
 * Return         : None
 *******************************************************************************/
void SetTlc5620byVoltage(uint8_t channel, float voltage)
{
	uint8_t value;
	if (voltage > V_REF_DAC)
		voltage = V_REF_DAC;
	if (voltage < 0)
		voltage = 0.0;
	voltage /= V_REF_DAC;
	voltage *= 255;
	value += 0.5;
	value = (uint8_t)voltage;
	SetTlc5620byDigit(channel, value);
}
/*******************************************************************************
 * Function Name  : OpenAbsorbLED
 * Description    : SetTlc5620byVoltage.
 * Input          :
 * Output         : None
 * Return         : None
 *******************************************************************************/
void OpenAbsorbLED(void)
{
	//	SetAbsorbLEDCurrent(10);
	CurrentLEDAbs = (float)(pMbHoldData->nPoolLdCur) / 10;	// 从保持寄存器获取已设置好的吸光度电流值并换算
	SetAbsorbLEDCurrent(CurrentLEDAbs);
}
/*******************************************************************************
 * Function Name  : OpenAbsorbLED
 * Description    : SetTlc5620byVoltage.
 * Input          :
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CloseAbsorbLED(void)
{
	SetAbsorbLEDCurrent(0);
}
/*******************************************************************************
 * Function Name  : OpenAbsorbLED
 * Description    : SetTlc5620byVoltage.
 * Input          :
 * Output         : None
 * Return         : None
 *******************************************************************************/
void OpenLLLED(void)
{
	float cur = (float)pMbHoldData->nLL1LdCur / 10;
	if (cur > I_MAX_LED)
		cur = I_MAX_LED;
	//	LL1InitCurrent = 4;
	SetLL1LEDCurrent(LL1InitCurrent);
	cur = (float)pMbHoldData->nLL2LdCur / 10;
	if (cur > I_MAX_LED)
		cur = I_MAX_LED;
	//	LL2InitCurrent = 4;
	SetLL2LEDCurrent(LL2InitCurrent);
}
/*******************************************************************************
 * Function Name  : OpenAbsorbLED
 * Description    : SetTlc5620byVoltage.
 * Input          :
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CloseLLLED(void)
{
	SetLL1LEDCurrent(0);
	SetLL2LEDCurrent(0);
}
/*******************************************************************************
 * Function Name  : 设置吸光度用LED电流
 * Description    : SetTlc5620byVoltage.
 * Input          : 电流值（0-25mA）
 * Output         : None
 * Return         : None
 *******************************************************************************/
void SetAbsorbLEDCurrent(float current)
{
	uint16_t value;
	if (current > I_MAX_LED)
		current = I_MAX_LED;
	if (current < 0)
		current = 0.0;
	current *= 4095;
	current /= I_MAX_LED;
	current += 0.5f;
	value = (uint16_t)current;

	LL_DAC_ConvertData12RightAligned(DAC, LL_DAC_CHANNEL_2, value);
	LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_2);			// DAC1输出对应电压
}
/*******************************************************************************
 * Function Name  : 设置液位1用LED电流
 * Description    : SetLL1LEDCurrent.
 * Input          : 电流值（0-25mA）
 * Output         : None
 * Return         : None
 *******************************************************************************/
void SetLL1LEDCurrent(float current)
{
	uint16_t value;
	if (current > I_MAX_LED)
		current = I_MAX_LED;
	if (current < 0)
		current = 0.0;
	current *= 6000;
	current /= I_MAX_LED;
	current += 0.5f;
	value = (uint16_t)current;
	//	value = 10000;
	if (value == 0)
	{
		LL_TIM_OC_SetCompareCH1(TIM3, value);
	}
	else
	{
		LL_TIM_OC_SetCompareCH1(TIM3, value);
		LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1);
		LL_TIM_EnableCounter(TIM3);
	}
}
/*******************************************************************************
 * Function Name  : 设置液位2用LED电流
 * Description    : SetLL2LEDCurrent.
 * Input          : 电流值（0-25mA）
 * Output         : None
 * Return         : None
 *******************************************************************************/
void SetLL2LEDCurrent(float current)
{
	uint16_t value;
	if (current > I_MAX_LED)
		current = I_MAX_LED;
	if (current < 0)
		current = 0.0;
	current *= 6000;
	current /= I_MAX_LED;
	current += 0.5f;
	value = (uint16_t)current;
	if (value == 0)
	{
		LL_TIM_OC_SetCompareCH3(TIM3, value);
	}
	else
	{
		LL_TIM_OC_SetCompareCH3(TIM3, value);
		LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH3);
		LL_TIM_EnableCounter(TIM3);
	}
}
