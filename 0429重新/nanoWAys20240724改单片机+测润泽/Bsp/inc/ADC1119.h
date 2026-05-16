#ifndef __ADC1119_H
#define __ADC1119_H
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "main.h"
#define I2C_Virtual_SDA_Port GPIOC
#define I2C_Virtual_SCL_Port GPIOC
#define I2C_Virtual_SDA_Pin  GPIO_Pin_0
#define I2C_Virtual_SCL_Pin  GPIO_Pin_2

#define ADS1119_ADDRESS		0x40		                                //ADDR PIN ->GND
#define ADS1119_ADDRESS_W	ADS1119_ADDRESS<<1|0x00	                        //write address
#define ADS1119_ADDRESS_R	ADS1119_ADDRESS<<1|0x01	                        //read address


/************POINTER REGISTER*****************/
#define ADS1119_Pointer_ConverReg			0x00	                //Convertion register
#define ADS1119_Pointer_ConfigReg			0x01	                //Config register
#define ADS1119_Pointer_LoThreshReg			0x02	                //Lo_thresh register
#define ADS1119_Pointer_HiThreshReg			0x03	                //Hi_thresh register


//Bits[7:5]
#define ADS1119_MUX_Channel_0				0x60		        //AINp=AIN0, AINn=GND
#define ADS1119_MUX_Channel_1				0x80		        //AINp=AIN1, AINn=GND
#define ADS1119_MUX_Channel_2				0xA0    	        //AINp=AIN2, AINn=GND
#define ADS1119_MUX_Channel_3				0xC0		        //AINp=AIN3, AINn=GND

//Bits[4]
#define ADS1119_Gain_Configuration_1		        0x00                    //Gain=1(default),2.048
#define ADS1119_Gain_Configuration_4                    0x10                    //Gain=4,0.512

//Bits[3:2]
#define ADS1119_DataRate_20				0x00		        //Data Rate = 20(default)
#define ADS1119_DataRate_90				0x04		        //Data Rate = 90
#define ADS1119_DataRate_330				0x08		        //Data Rate = 330
#define ADS1119_DataRate_1000				0x0C		        //Data Rate = 1000

//Bits[1]
#define ADS1119_CM_Shot				        0x00		        //Single-shot conversion mode (default)
#define ADS1119_CM_CON				        0x02		        //Continuous conversion mode

//Bits[0]
#define ADS1119_VREF_IN				        0x00		        //Internal 2.048-V reference selected (default)
#define ADS1119_VREF_EXTERN				0x01		        //External reference selected using the REFP and REFN inputs

#define ADS1119_MAX_CHANNEL                              4
typedef enum {
	ADS1119_CH01	=  0x00,
	ADS1119_CH23	=  0x20,
	ADS1119_CH12	=  0x40,	
	ADS1119_CH0		=  0x60,
	ADS1119_CH1		=  0x80,
	ADS1119_CH2		=  0xA0,
	ADS1119_CH3		=  0xC0,
	ADS1119_CHV		=  0xE0,
}ADS1119_CH;

#define CONST_CONFIG_DR20_SHOT    0x01
#define CONST_CONFIG_DR90_SHOT    0x0D
#define CONST_CONFIG_DR1000_SHOT    0x0D
#define CONST_CONFIG_DR20_CONTINUOUS    0x03
#define CONST_CONFIG_DR1000_CONTINUOUS    0x0F
typedef struct
{
    uint16_t MUX;
    uint16_t GAIN;
    uint16_t DataRate;
    uint16_t CM;
    uint16_t VREF;
} ADS1119_InitTypeDefine;

void ADS1119_Init(void);
void ADS1119_UserConfig1(void);
void ADS1119_UserConfig2(void);
uint8_t ADS1119_Config(ADS1119_InitTypeDefine* ADS1119_InitStruct);
 
uint8_t ADS1119_ReadRawData(int16_t* rawData);
void ADS1119_ScanChannel(uint8_t channel);
float ADS1119_RawDataToVoltage(int16_t rawData);
 
float ADS1119_GetVoltage(void);
float ADS1119_GetAverageVoltage(uint16_t num);
void ADC1119_GPIO_Config(void); 
void ADS1119_RefreshAllChannel(void);

void I2C_Virtual_Init(void);
uint8_t ADS1119ChConf(uint8_t ch);
bool ADS1119ReadCh(uint8_t ch,int16_t *res);
uint8_t ADS1119_ReadConfigure(uint8_t r,uint8_t* ret);

#define SET_SDA_IN() LL_GPIO_SetPinMode(ADC1119_SDA_GPIO_Port,ADC1119_SDA_Pin,LL_GPIO_MODE_INPUT);
#define SET_SDA_OUT() LL_GPIO_SetPinMode(ADC1119_SDA_GPIO_Port,ADC1119_SDA_Pin,LL_GPIO_MODE_OUTPUT);

#define SDA_H()     LL_GPIO_SetOutputPin(ADC1119_SDA_GPIO_Port, ADC1119_SDA_Pin);
#define SDA_L()     LL_GPIO_ResetOutputPin(ADC1119_SDA_GPIO_Port, ADC1119_SDA_Pin);

#define SCL_H()      LL_GPIO_SetOutputPin(ADC1119_SCL_GPIO_Port, ADC1119_SCL_Pin);
#define SCL_L()      LL_GPIO_ResetOutputPin(ADC1119_SCL_GPIO_Port, ADC1119_SCL_Pin);

#define RESET_ADS_L      LL_GPIO_ResetOutputPin(ADC1119_RESET_GPIO_Port, ADC1119_RESET_Pin);
#define RESET_ADS_H      LL_GPIO_SetOutputPin(ADC1119_RESET_GPIO_Port, ADC1119_RESET_Pin);

#define SDA_FLAG    LL_GPIO_IsInputPinSet(ADC1119_SDA_GPIO_Port, ADC1119_SDA_Pin)//SDA的数据输入
#define ADS_CONV_DONE_FLAG 		LL_GPIO_IsInputPinSet(ADC1119_READY_GPIO_Port, ADC1119_READY_Pin)//SDA的数据输入

#endif
