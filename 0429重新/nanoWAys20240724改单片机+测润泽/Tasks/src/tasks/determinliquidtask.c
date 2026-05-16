
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "determinliquidtask.h"

#include "usart.h"
#include "relaysdef.h"
#include "ltc1867.h"
#include "tlc5620.h"


uint16_t ADref,ADabs;
void UartsTxTask(void const * argument)
{
//	SetAbsorbLEDCurrent(10);
	  for(;;)
  {
//		
		
		osDelay(1000);
		ADref = osLTCReadADx(LED_REF_CH,1024);
		ADabs = osLTCReadADx(LED_ABS_CH,1024);
//		SetAbsorbLEDCurrent(0);
//		SendIntByString(LED_REF_CH,ADref);
		osDelay(20);
//		SendIntByString(LED_ABS_CH,ADabs);
    osDelay(1000);
  }
}

