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
 * File: $Id: portevent.c,v 1.1 2007/04/24 23:15:18 wolti Exp $
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "cmsis_os.h"
/* ----------------------- Variables ----------------------------------------*/
//extern osSemaphoreId ModSEMHandle;
extern osSemaphoreId Comm3RxHandle; 
extern osSemaphoreId USART3RxHandle;
extern osSemaphoreId USART6RxHandle;

static eMBEventType eQueuedEvent;
static BOOL     xEventInQueue;
static eMBEventType eQueuedEvent3;
static BOOL     xEventInQueue3;
static eMBEventType eQueuedEvent6;
static BOOL     xEventInQueue6;

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortEventInit( void )
{
    xEventInQueue = FALSE;
    return TRUE;
}

BOOL
xMBPortEventInit3( void )
{
    xEventInQueue3 = FALSE;
    return TRUE;
}

BOOL
xMBPortEventInit6( void )
{
    xEventInQueue6 = FALSE;
    return TRUE;
}

BOOL
xMBPortEventPost( eMBEventType eEvent )
{
    /* 先写入事件，再释放信号量，避免任务被唤醒后读到“无事件”造成丢事件。 */
    eQueuedEvent = eEvent;
    xEventInQueue = TRUE;
	//		osSemaphoreRelease(ModSEMHandle); 
	osSemaphoreRelease(Comm3RxHandle);
    return TRUE;
}

BOOL
xMBPortEventPost3( eMBEventType eEvent )
{
    eQueuedEvent3 = eEvent;
    xEventInQueue3 = TRUE;
	osSemaphoreRelease(USART3RxHandle);
    return TRUE;
}

BOOL
xMBPortEventPost6( eMBEventType eEvent )
{
    eQueuedEvent6 = eEvent;
    xEventInQueue6 = TRUE;
	osSemaphoreRelease(USART6RxHandle);
    return TRUE;
}

BOOL
xMBPortEventGet( eMBEventType * eEvent )
{
    BOOL            xEventHappened = FALSE;

    if( xEventInQueue )
    {
        *eEvent = eQueuedEvent;
        xEventInQueue = FALSE;
        xEventHappened = TRUE;
    }
    return xEventHappened;
}

BOOL
xMBPortEventGet3( eMBEventType * eEvent )
{
    BOOL xEventHappened = FALSE;

    if( xEventInQueue3 )
    {
        *eEvent = eQueuedEvent3;
        xEventInQueue3 = FALSE;
        xEventHappened = TRUE;
    }
    return xEventHappened;
}

BOOL
xMBPortEventGet6( eMBEventType * eEvent )
{
    BOOL xEventHappened = FALSE;

    if( xEventInQueue6 )
    {
        *eEvent = eQueuedEvent6;
        xEventInQueue6 = FALSE;
        xEventHappened = TRUE;
    }
    return xEventHappened;
}
