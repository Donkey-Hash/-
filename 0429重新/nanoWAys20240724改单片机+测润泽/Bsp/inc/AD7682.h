

#ifndef __AD_7682_H__
#define __AD_7682_H__
#include "main.h"
#include "spi.h"
//#include "elechemirelaysdef.h"
#include "gpio.h"

#define SEL_ADC LL_GPIO_ResetOutputPin(AD_INV_GPIO_Port,AD_INV_Pin);//(PORTB &= ~_BV(PORTB1))
#define DIS_ADC LL_GPIO_SetOutputPin(AD_INV_GPIO_Port,AD_INV_Pin);//(PORTB |= _BV(PORTB1))

#define SEL_DAC LL_GPIO_ResetOutputPin(DAC_SYN_GPIO_Port,DAC_SYN_Pin);//(PORTB &= ~_BV(PORTB2))
#define DIS_DAC LL_GPIO_SetOutputPin(DAC_SYN_GPIO_Port,DAC_SYN_Pin);//(PORTB |= _BV(PORTB2))

#define GET_DAC_BY_VOL(vol) (uint16_t)((long)32768 - ((int16_t)(vol) << 4))
#define GET_VOL_BY_ADC(uni) (int16_t)((long)32768 - (int32_t)(uni))

#define DEFAULT_AD_CHANNEL 0

#define AD_CACHE_METHOD 0

void vSPI_config(void);
void SetDACVol(int32_t i4Vol);
void SetDAC(uint16_t u2dac);
uint16_t ReadADC(uint8_t ch, uint16_t count);
int32_t i4CalcCurrentByADC(uint16_t u2ADC);
int32_t CalcCurrentByADC(uint16_t u2ADC);



#endif

