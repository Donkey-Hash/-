

#include "bspuart3.h"
#include "leddef.h"
#include "FlashUserData.h"

//extern osSemaphoreId Comm3TxSEMHandle;

bool Uart3BuzyFlag = false;

void StartRS232Rx(void)
{
	if(huart3.State == HAL_UART_STATE_BUSY_TX_RX) 
	{
		huart3.State = HAL_UART_STATE_BUSY_TX;
	}
	else if(huart3.State == HAL_UART_STATE_BUSY_RX) 
	{
		huart3.State = HAL_UART_STATE_READY;
	}
	HAL_UART_Receive_IT(&huart3,RS232RxBuff,10);
}

/**
  * @brief  This function handles UART interrupt request.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void UART3_IRQHandler(void)
{
  uint32_t tmp_flag = 0, tmp_it_source = 0;

  tmp_flag = __HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    //UART3_Receive_IT(&huart3);
		LED_WARN_ON
		prvvUARTRxISR();
		LED_WARN_OFF
  }
	
	tmp_flag = __HAL_UART_GET_FLAG(&huart3, UART_FLAG_TXE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_TXE);
  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
//		__HAL_UART_DISABLE_IT(&huart3, UART_IT_TXE);
		prvvUARTTxReadyISR();
  }

  tmp_flag = __HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_TC);
  /* UART in mode Transmitter end --------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {  
		
		__HAL_UART_DISABLE_IT(&huart3, UART_IT_TC);
//		/* Check if a receive process is ongoing or not */
//		if(huart3.State == HAL_UART_STATE_BUSY_TX_RX) 
//		{
//			huart3.State = HAL_UART_STATE_BUSY_RX;
//		}
//		else
//		{
//			huart3.State = HAL_UART_STATE_READY;
//		}
////    UART_EndTransmit_IT(&huart3);
//		  __HAL_UART_DISABLE_IT(&huart3, UART_IT_TC);
//		osSemaphoreRelease(Comm3TxSEMHandle); 
//		Uart3BuzyFlag = true;
  }  

  if(huart3.ErrorCode != HAL_UART_ERROR_NONE)
  {
    /* Clear all the error flag at once */
    __HAL_UART_CLEAR_PEFLAG(&huart3);
    
    /* Set the UART state ready to be able to start again the process */
    huart3.State = HAL_UART_STATE_READY;
    
    HAL_UART_ErrorCallback(&huart3);
  }  
}

HAL_StatusTypeDef UART3_Receive_IT(UART_HandleTypeDef *huart)
{
//  uint16_t* tmp;
  uint32_t tmp_state = 0;
  
  tmp_state = huart->State; 
  if((tmp_state == HAL_UART_STATE_BUSY_RX) || (tmp_state == HAL_UART_STATE_BUSY_TX_RX))
  {

    {
      if(huart->Init.Parity == UART_PARITY_NONE)
      {
        *huart->pRxBuffPtr++ = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
      }
      else
      {
        *huart->pRxBuffPtr++ = (uint8_t)(huart->Instance->DR & (uint8_t)0x007F);
      }
    }
		EnableRS232Timers();
    if(--huart->RxXferCount == 0)
    {
      __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);

      /* Check if a transmit process is ongoing or not */
      if(huart->State == HAL_UART_STATE_BUSY_TX_RX) 
      {
        huart->State = HAL_UART_STATE_BUSY_TX;
      }
      else
      {
        /* Disable the UART Parity Error Interrupt */
        __HAL_UART_DISABLE_IT(huart, UART_IT_PE);

        /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
        __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

        huart->State = HAL_UART_STATE_READY;
      }
      HAL_UART_RxCpltCallback(huart);

      return HAL_OK;
    }
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY; 
  }
}
/* USER CODE END 1 */

/**
  * @}
  */
//itoa

void osRS232Tx(uint8_t *pData, uint16_t Size)
{
//	if(osSemaphoreWait(Comm3TxSEMHandle , 1000) == osOK)
//	{
//		HAL_UART_Transmit_DMA(&huart3,pData,Size);
//	  Uart3BuzyFlag = true;
//	}
//	else
//	{
//		LED_WARN_ON
//		AddErrRecord(UART_3_TX_FALSE,pData[1]);
//	}
}


