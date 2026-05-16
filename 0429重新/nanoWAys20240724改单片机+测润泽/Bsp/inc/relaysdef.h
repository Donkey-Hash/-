
#ifndef	__RELAYS_DEFINE__
#define  __RELAYS_DEFINE__

#include "main.h"


#define Y1_ON				LL_GPIO_ResetOutputPin(Y1_GPIO_Port,Y1_Pin);
#define Y1_OFF 			LL_GPIO_SetOutputPin(Y1_GPIO_Port,Y1_Pin);
#define Y1_TOGGLE 	LL_GPIO_TogglePin(Y1_GPIO_Port,Y1_Pin);

#define Y2_ON				LL_GPIO_ResetOutputPin(Y2_GPIO_Port,Y2_Pin);
#define Y2_OFF 			LL_GPIO_SetOutputPin(Y2_GPIO_Port,Y2_Pin);
#define Y2_TOGGLE 	LL_GPIO_TogglePin(Y2_GPIO_Port,Y2_Pin);

#define Y3_ON				LL_GPIO_ResetOutputPin(Y3_GPIO_Port,Y3_Pin);
#define Y3_OFF 			LL_GPIO_SetOutputPin(Y3_GPIO_Port,Y3_Pin);
#define Y3_TOGGLE 	LL_GPIO_TogglePin(Y3_GPIO_Port,Y3_Pin);

#define Y4_ON				LL_GPIO_ResetOutputPin(Y4_GPIO_Port,Y4_Pin);
#define Y4_OFF 			LL_GPIO_SetOutputPin(Y4_GPIO_Port,Y4_Pin);
#define Y4_TOGGLE 	LL_GPIO_TogglePin(Y4_GPIO_Port,Y4_Pin);

#define Y5_ON				LL_GPIO_ResetOutputPin(Y5_GPIO_Port,Y5_Pin);
#define Y5_OFF 			LL_GPIO_SetOutputPin(Y5_GPIO_Port,Y5_Pin);
#define Y5_TOGGLE 	LL_GPIO_TogglePin(Y5_GPIO_Port,Y5_Pin);

#define Y6_ON				LL_GPIO_ResetOutputPin(Y6_GPIO_Port,Y6_Pin);
#define Y6_OFF 			LL_GPIO_SetOutputPin(Y6_GPIO_Port,Y6_Pin);
#define Y6_TOGGLE 	LL_GPIO_TogglePin(Y6_GPIO_Port,Y6_Pin);

#define Y7_ON				LL_GPIO_ResetOutputPin(Y7_GPIO_Port,Y7_Pin);
#define Y7_OFF 			LL_GPIO_SetOutputPin(Y7_GPIO_Port,Y7_Pin);
#define Y7_TOGGLE 	LL_GPIO_TogglePin(Y7_GPIO_Port,Y7_Pin);

#define Y8_ON				LL_GPIO_ResetOutputPin(Y8_GPIO_Port,Y8_Pin);
#define Y8_OFF 			LL_GPIO_SetOutputPin(Y8_GPIO_Port,Y8_Pin);
#define Y8_TOGGLE 	LL_GPIO_TogglePin(Y8_GPIO_Port,Y8_Pin);

#define Y9_ON				LL_GPIO_ResetOutputPin(Y9_GPIO_Port,Y9_Pin);
#define Y9_OFF 			LL_GPIO_SetOutputPin(Y9_GPIO_Port,Y9_Pin);
#define Y9_TOGGLE 	LL_GPIO_TogglePin(Y9_GPIO_Port,Y9_Pin);

#define Y10_ON				LL_GPIO_ResetOutputPin(Y10_GPIO_Port,Y10_Pin);
#define Y10_OFF 			LL_GPIO_SetOutputPin(Y10_GPIO_Port,Y10_Pin);
#define Y10_TOGGLE 		LL_GPIO_TogglePin(Y10_GPIO_Port,Y10_Pin);

#define Y11_ON				LL_GPIO_ResetOutputPin(Y11_GPIO_Port,Y11_Pin);
#define Y11_OFF 			LL_GPIO_SetOutputPin(Y11_GPIO_Port,Y11_Pin);
#define Y11_TOGGLE 		LL_GPIO_TogglePin(Y11_GPIO_Port,Y11_Pin);

#define Y12_ON				LL_GPIO_ResetOutputPin(Y12_GPIO_Port,Y12_Pin);
#define Y12_OFF 			LL_GPIO_SetOutputPin(Y12_GPIO_Port,Y12_Pin);
#define Y12_TOGGLE 		LL_GPIO_TogglePin(Y12_GPIO_Port,Y12_Pin);

#define Y13_ON				LL_GPIO_ResetOutputPin(Y13_GPIO_Port,Y13_Pin);
#define Y13_OFF 			LL_GPIO_SetOutputPin(Y13_GPIO_Port,Y13_Pin);
#define Y13_TOGGLE 		LL_GPIO_TogglePin(Y13_GPIO_Port,Y13_Pin);

#define Y14_ON				LL_GPIO_ResetOutputPin(Y14_GPIO_Port,Y14_Pin);
#define Y14_OFF 			LL_GPIO_SetOutputPin(Y14_GPIO_Port,Y14_Pin);
#define Y14_TOGGLE 		LL_GPIO_TogglePin(Y14_GPIO_Port,Y14_Pin);

#define Y15_ON				LL_GPIO_ResetOutputPin(Y15_GPIO_Port,Y15_Pin);
#define Y15_OFF 			LL_GPIO_SetOutputPin(Y15_GPIO_Port,Y15_Pin);
#define Y15_TOGGLE 		LL_GPIO_TogglePin(Y15_GPIO_Port,Y15_Pin);

#define Y16_ON				LL_GPIO_ResetOutputPin(Y16_GPIO_Port,Y16_Pin);
#define Y16_OFF 			LL_GPIO_SetOutputPin(Y16_GPIO_Port,Y16_Pin);
#define Y16_TOGGLE 		LL_GPIO_TogglePin(Y16_GPIO_Port,Y16_Pin);

#define Y17_ON				LL_GPIO_ResetOutputPin(Y17_GPIO_Port,Y17_Pin);
#define Y17_OFF 			LL_GPIO_SetOutputPin(Y17_GPIO_Port,Y17_Pin);
#define Y17_TOGGLE 		LL_GPIO_TogglePin(Y17_GPIO_Port,Y17_Pin);

#define Y18_ON				LL_GPIO_ResetOutputPin(Y18_GPIO_Port,Y18_Pin);
#define Y18_OFF 			LL_GPIO_SetOutputPin(Y18_GPIO_Port,Y18_Pin);
#define Y18_TOGGLE 		LL_GPIO_TogglePin(Y18_GPIO_Port,Y18_Pin);

#define Y19_ON				LL_GPIO_ResetOutputPin(Y19_GPIO_Port,Y19_Pin);
#define Y19_OFF 			LL_GPIO_SetOutputPin(Y19_GPIO_Port,Y19_Pin);
#define Y19_TOGGLE 		LL_GPIO_TogglePin(Y19_GPIO_Port,Y19_Pin);

/* 以下端口没有被初始化定义，注释掉
#define Y20_ON				LL_GPIO_ResetOutputPin(Y20_GPIO_Port,Y20_Pin);
#define Y20_OFF 			LL_GPIO_SetOutputPin(Y20_GPIO_Port,Y20_Pin);
#define Y20_TOGGLE 		LL_GPIO_TogglePin(Y20_GPIO_Port,Y20_Pin);

#define Y21_ON				LL_GPIO_ResetOutputPin(Y21_GPIO_Port,Y21_Pin);
#define Y21_OFF 			LL_GPIO_SetOutputPin(Y21_GPIO_Port,Y21_Pin);
#define Y21_TOGGLE 		LL_GPIO_TogglePin(Y21_GPIO_Port,Y21_Pin);

#define Y22_ON				LL_GPIO_ResetOutputPin(Y22_GPIO_Port,Y22_Pin);
#define Y22_OFF 			LL_GPIO_SetOutputPin(Y22_GPIO_Port,Y22_Pin);
#define Y22_TOGGLE 		LL_GPIO_TogglePin(Y22_GPIO_Port,Y22_Pin);

#define Y23_ON				LL_GPIO_ResetOutputPin(Y23_GPIO_Port,Y23_Pin);
#define Y23_OFF 			LL_GPIO_SetOutputPin(Y23_GPIO_Port,Y23_Pin);
#define Y23_TOGGLE 		LL_GPIO_TogglePin(Y23_GPIO_Port,Y23_Pin);

#define Y24_ON				LL_GPIO_ResetOutputPin(Y24_GPIO_Port,Y24_Pin);
#define Y24_OFF 			LL_GPIO_SetOutputPin(Y24_GPIO_Port,Y24_Pin);
#define Y24_TOGGLE 		LL_GPIO_TogglePin(Y24_GPIO_Port,Y24_Pin);


#define OpenY(n)   		Yn_ON
#define CloseY(n)		Yn_OFF
*/

#endif

