/*
 * FreeModbus Libary: LPC214X Port
 * Copyright (C) 2007 Tiago Prado Lone <tiago@maxwellbohr.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.1 2007/04/24 23:15:18 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "stm32f4xx_ll_tim.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "tim.h"
#include "cmsis_os.h"
/* ----------------------- Start implementation -----------------------------*/

//extern osSemaphoreId ModSEMHandle;


BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
	  TIM_InitStruct.Prescaler = 1199;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 50 * usTim1Timerout50us;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM11, &TIM_InitStruct);
	
	LL_TIM_EnableIT_UPDATE(TIM11);
	LL_TIM_EnableCounter(TIM11);	
	return TRUE;
}

BOOL
xMBPortTimersInit3( USHORT usTim1Timerout50us )
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  TIM_InitStruct.Prescaler = 1199;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 50 * usTim1Timerout50us;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM7, &TIM_InitStruct);

	LL_TIM_EnableIT_UPDATE(TIM7);
	LL_TIM_EnableCounter(TIM7);
	return TRUE;
}

BOOL
xMBPortTimersInit6( USHORT usTim1Timerout50us )
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  TIM_InitStruct.Prescaler = 1199;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 50 * usTim1Timerout50us;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM6, &TIM_InitStruct);

	LL_TIM_EnableIT_UPDATE(TIM6);
	LL_TIM_EnableCounter(TIM6);
	return TRUE;
}


void
vMBPortTimersEnable(  )
{	
	LL_TIM_ClearFlag_UPDATE(TIM11);
	LL_TIM_SetCounter(TIM11,0);
	LL_TIM_EnableIT_UPDATE(TIM11);
	LL_TIM_EnableCounter(TIM11);
}

void
vMBPortTimersEnable3(  )
{
	LL_TIM_ClearFlag_UPDATE(TIM7);
	LL_TIM_SetCounter(TIM7,0);
	LL_TIM_EnableIT_UPDATE(TIM7);
	LL_TIM_EnableCounter(TIM7);
}

void
vMBPortTimersEnable6(  )
{
	LL_TIM_ClearFlag_UPDATE(TIM6);
	LL_TIM_SetCounter(TIM6,0);
	LL_TIM_EnableIT_UPDATE(TIM6);
	LL_TIM_EnableCounter(TIM6);
}

void
vMBPortTimersDisable(  )
{
	LL_TIM_SetCounter(TIM11,0);
	LL_TIM_DisableCounter(TIM11);
	LL_TIM_ClearFlag_UPDATE(TIM11);
}

void
vMBPortTimersDisable3(  )
{
	LL_TIM_SetCounter(TIM7,0);
	LL_TIM_DisableCounter(TIM7);
	LL_TIM_ClearFlag_UPDATE(TIM7);
}

void
vMBPortTimersDisable6(  )
{
	LL_TIM_SetCounter(TIM6,0);
	LL_TIM_DisableCounter(TIM6);
	LL_TIM_ClearFlag_UPDATE(TIM6);
}


void
TIMERExpiredISR( void )
{
	(void)pxMBPortCBTimerExpired();
//	osSemaphoreRelease(ModSEMHandle); 
}

void
TIMER3ExpiredISR( void )
{
	(void)pxMBPortCBTimerExpired3();
}

void
TIMER6ExpiredISR( void )
{
	(void)pxMBPortCBTimerExpired6();
}
