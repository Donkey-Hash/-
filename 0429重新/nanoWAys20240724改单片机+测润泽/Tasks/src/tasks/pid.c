#include <math.h> 
#include "pid.h"

/*******************************************************************************
* Function Name  : SetTargetValue
* Description    : 设置目标值.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetTargetValue(PID_t* pPid,float fValue)
{
	pPid->fTargetValue = fValue;
}
/*******************************************************************************
* Function Name  : SetPidParam
* Description    : 设置PID参数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/******************************************************************************
void SetPid(PID_t* pPid,)
{
	pPid-> = ;
	pPid-> = 0.0;
} 

*******************************************************************************/


void SetPidParam(PID_t* pPid,float fKp,float fKi,float fKd,float fDeadband)
{
	pPid->fKp = fKp;
	pPid->fKi = fKi;
	pPid->fKd = fKd;
	pPid->fDeadband = fDeadband;
}
void SetPidKp(PID_t* pPid,float fKp)
{
	pPid->fKp = fKp;
} 
void SetPidKi(PID_t* pPid,float fKi)
{
	pPid->fKi = fKi;
} 
void SetPidKd(PID_t* pPid,float fKd)
{
	pPid->fKd = fKd;
} 
void SetPidDeadband(PID_t* pPid,float fDeadband)
{
	pPid->fDeadband = fDeadband;
} 
void SetPidMaxUk(PID_t* pPid,unsigned int nMaxUk)
{
	pPid->nMaxUk = nMaxUk;
} 
void SetPidUkOffset(PID_t* pPid,unsigned int nUkOffset)
{
	pPid->nUkOffset = nUkOffset;
	pPid->fIntegral = pPid->nUkOffset;
} 
void SetPidIntegral(PID_t* pPid,float fIntegral)
{
	pPid->fIntegral = fIntegral;
} 
void SetPidfActualValue(PID_t* pPid,float fActualValue)
{
	pPid->fActualValue = fActualValue;
} 
  
void PidBumpless(PID_t* pPid)
{
	pPid->fLastErr = pPid->fTargetValue - pPid->fActualValue; 
}
void ClearPidParam(PID_t* pPid)
{
	pPid->fIntegral = 0;
}
int PIDRealize(PID_t* pPid,float fActualValue)
{
//	float error;
//	int result = 0;
	pPid->fActualValue = fActualValue;
	pPid->fError = pPid->fTargetValue - pPid->fActualValue;
	if(fabs(pPid->fError) > pPid->fDeadband)
	{
		pPid->fPropor = pPid->fError * pPid->fKp;
		if(fabs(pPid->fPropor) > pPid->nMaxUk)
			pPid->fIntegral = pPid->nUkOffset;
		else
		{
			pPid->fIntegral += pPid->fKi * pPid->fError;
			if(pPid->fIntegral > pPid->nMaxUk)
				pPid->fIntegral = pPid->nMaxUk;
			else if(pPid->fIntegral >= 0 && pPid->fIntegral < pPid->nUkOffset )
			{
				pPid->nUkOffset = pPid->fIntegral;
				
			}
			else if (pPid->fIntegral < 0)
			{
				pPid->fIntegral = 0;
			}
				
		}
		pPid->fdError = pPid->fError - pPid->fLastErr;
		pPid->fDiffer = pPid->fdError * pPid->fKd;
		pPid->nUk = (int)(pPid->fPropor + pPid->fIntegral + pPid->fDiffer + 0.5);
	}
	else
		pPid->nUk = (pPid->fIntegral + 0.5);
	pPid->fLastErr = pPid->fError;
	if(pPid->nUk < 0)
		pPid->nUk = 0;
	if((pPid->nMaxUk > 0) && (pPid->nUk > pPid->nMaxUk))
		pPid->nUk = pPid->nMaxUk;
	
	return pPid->nUk;
}


