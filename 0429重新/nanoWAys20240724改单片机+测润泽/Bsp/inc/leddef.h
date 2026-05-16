#ifndef	__LED_DEFINE__
#define  __LED_DEFINE__

#include "main.h"

#define LED1_ON					HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);
#define LED1_OFF 				HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
#define LED1_TOGGLE 		HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);

#define LED2_ON					HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_RESET);
#define LED2_OFF 				HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_SET);
#define LED2_TOGGLE 		HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);

#define Buzzer_ON					HAL_GPIO_WritePin(Buzzer_GPIO_Port,Buzzer_Pin,GPIO_PIN_SET);
#define Buzzer_OFF 				HAL_GPIO_WritePin(Buzzer_GPIO_Port,Buzzer_Pin,GPIO_PIN_RESET);
#define Buzzer_TOGGLE 		HAL_GPIO_TogglePin(Buzzer_GPIO_Port,Buzzer_Pin);

//#define LED3_ON					HAL_GPIO_WritePin(LED3_GPIO_Port,LED3_Pin,GPIO_PIN_RESET);
//#define LED3_OFF 				HAL_GPIO_WritePin(LED3_GPIO_Port,LED3_Pin,GPIO_PIN_SET);
//#define LED3_TOGGLE 		HAL_GPIO_TogglePin(LED3_GPIO_Port,LED3_Pin);

#define LED_RUN_ON  			LED1_ON
#define LED_RUN_OFF  			LED1_OFF
#define LED_RUN_TOGGLE  	LED1_TOGGLE

#define LED_WARN_ON  			LED2_ON
#define LED_WARN_OFF  		LED2_OFF
#define LED_WARN_TOGGLE  	LED2_TOGGLE

//#define LED_TEST_ON  			LED3_ON
//#define LED_TEST_OFF  		LED3_OFF
//#define LED_TEST_TOGGLE  	LED3_TOGGLE

//#define ENABLE_PUMP_DRIVER  			LED3_ON
//#define DISABLE_PUMP_DRIVER	  		LED3_OFF



#endif
