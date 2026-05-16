

#include "bspuart1.h"



int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}


void StartUart1Rx(void)
{
	if(huart1.State == HAL_UART_STATE_BUSY_TX_RX) 
	{
		huart1.State = HAL_UART_STATE_BUSY_TX;
	}
	else if(huart1.State == HAL_UART_STATE_BUSY_RX) 
	{
		huart1.State = HAL_UART_STATE_READY;
	}
	HAL_UART_Receive_IT(&huart1,RS232RxBuff,10);
}

/**
  * @brief  This function handles UART interrupt request.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void UART1_IRQHandler(void)
{
  uint32_t tmp_flag = 0, tmp_it_source = 0;

//  tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_PE);
//  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_PE);  
//  /* UART parity error interrupt occurred ------------------------------------*/
//  if((tmp_flag != RESET) && (tmp_it_source != RESET))
//  { 
//    huart1.ErrorCode |= HAL_UART_ERROR_PE;
//  }
//  
//  tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_FE);
//  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_ERR);
//  /* UART frame error interrupt occurred -------------------------------------*/
//  if((tmp_flag != RESET) && (tmp_it_source != RESET))
//  { 
//    huart1.ErrorCode |= HAL_UART_ERROR_FE;
//  }
//  
//  tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_NE);
//  /* UART noise error interrupt occurred -------------------------------------*/
//  if((tmp_flag != RESET) && (tmp_it_source != RESET))
//  { 
//    huart1.ErrorCode |= HAL_UART_ERROR_NE;
//  }
  
//  tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE);
//  /* UART Over-Run interrupt occurred ----------------------------------------*/
//  if((tmp_flag != RESET) && (tmp_it_source != RESET))
//  { 
//    huart1.ErrorCode |= HAL_UART_ERROR_ORE;
//  }
  
  tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    UART1_Receive_IT(&huart1);
  }
	
	tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE);
  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
		__HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
  }

  tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TC);
  /* UART in mode Transmitter end --------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {  
		/* Check if a receive process is ongoing or not */
		if(huart1.State == HAL_UART_STATE_BUSY_TX_RX) 
		{
			huart1.State = HAL_UART_STATE_BUSY_RX;
		}
		else
		{
			huart1.State = HAL_UART_STATE_READY;
		}
//    UART_EndTransmit_IT(&huart1);
		  __HAL_UART_DISABLE_IT(&huart1, UART_IT_TC);

  }  

  if(huart1.ErrorCode != HAL_UART_ERROR_NONE)
  {
    /* Clear all the error flag at once */
    __HAL_UART_CLEAR_PEFLAG(&huart1);
    
    /* Set the UART state ready to be able to start again the process */
    huart1.State = HAL_UART_STATE_READY;
    
    HAL_UART_ErrorCallback(&huart1);
  }  
}

HAL_StatusTypeDef UART1_Receive_IT(UART_HandleTypeDef *huart)
{
//  uint16_t* tmp;
  uint32_t tmp_state = 0;
  
  tmp_state = huart->State; 
  if((tmp_state == HAL_UART_STATE_BUSY_RX) || (tmp_state == HAL_UART_STATE_BUSY_TX_RX))
  {
//    if(huart->Init.WordLength == UART_WORDLENGTH_9B)
//    {
//      tmp = (uint16_t*) huart->pRxBuffPtr;
//      if(huart->Init.Parity == UART_PARITY_NONE)
//      {
//        *tmp = (uint16_t)(huart->Instance->DR & (uint16_t)0x01FF);
//        huart->pRxBuffPtr += 2;
//      }
//      else
//      {
//        *tmp = (uint16_t)(huart->Instance->DR & (uint16_t)0x00FF);
//        huart->pRxBuffPtr += 1;
//      }
//    }
//    else
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
//		EnableRS232Timers();
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

void osUart1Tx(uint8_t *pData, uint16_t Size)
{


}
static	uint8_t ch[8];
void SendIntByString(uint8_t channel ,uint16_t data)
{

	ch[7] = '\t';
	ch[6] = '0' + data % 10;
	data /= 10;
	ch[5] = '0' + data % 10;
	data /= 10;
	ch[4] = '0' + data % 10;
	data /= 10;
	ch[3] = '0' + data % 10;
	data /= 10;
	ch[2] = '0' + data % 10;
	ch[1] = ':';
	ch[0] = '0' + channel;
	HAL_UART_Transmit_DMA(&huart1,ch,8);
//	osRS232Tx(ch,8);
}


