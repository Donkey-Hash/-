


#include <stdbool.h>
#include "mtimers.h"
#include "tlc5620.h"
#include "modbusdata.h"
#include "FlashUserData.h"
extern float LL1InitCurrent;
extern float LL2InitCurrent;

float CurrentLEDAbs  = 2;   //单位：mA

/*******************************************************************************
* Function Name  : 通过数字量设置大DAC通道输出
* Description    : SetTlc5620byDigit.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetTlc5620byDigit(uint8_t channel,uint8_t value)
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
	SetTlc5620byDigit(0,0);
	SetTlc5620byDigit(1,0);
	SetTlc5620byDigit(2,0);
	SetTlc5620byDigit(3,0);
}
/*******************************************************************************
* Function Name  : 通过电压值设置大DAC通道输出
* Description    : SetTlc5620byVoltage.
* Input          : 通道号，电压值（0-2.5V）
* Output         : None
* Return         : None
*******************************************************************************/
void SetTlc5620byVoltage(uint8_t channel,float voltage)
{
	uint8_t value;
	if(voltage > V_REF_DAC)
		voltage = V_REF_DAC;
	if(voltage < 0)
		voltage = 0.0;
	voltage /= V_REF_DAC;
	voltage *= 255;
	value += 0.5;
	value = (uint8_t)voltage;
	SetTlc5620byDigit(channel,value);
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
	if(cur > I_MAX_LED)
		cur = 3;
	SetLL1LEDCurrent(LL1InitCurrent);
	cur = (float)pMbHoldData->nLL2LdCur / 10;
	if(cur > I_MAX_LED)
		cur = 3;
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
	if(current > I_MAX_LED)
		current = I_MAX_LED;
	if(current < 0)
		current = 0.0;
	current *= 60000;
	current /= I_MAX_LED;
	current += 0.5;
	value = (uint16_t)current;
	LL_TIM_OC_SetCompareCH4(TIM2,value);
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
	if(current > I_MAX_LED)
		current = I_MAX_LED;
	if(current < 0)
		current = 0.0;
	current *= 60000;
	current /= I_MAX_LED;
	current += 0.5;
	value = (uint16_t)current;
	LL_TIM_OC_SetCompareCH1(TIM9,value);
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
	if(current > I_MAX_LED)
		current = I_MAX_LED;
	if(current < 0)
		current = 0.0;
	current *= 60000;
	current /= I_MAX_LED;
	current += 0.5;
	value = (uint16_t)current;
	LL_TIM_OC_SetCompareCH2(TIM9,1800);
}

