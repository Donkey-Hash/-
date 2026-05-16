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
	pPid->fLastErr = pPid->fTargetValue - pPid->fActualValue; 	// 更新上一次误差
}
void ClearPidParam(PID_t* pPid)
{
	pPid->fIntegral = 0;
}
/*******************************************************************************
* Function Name  : PID迭代函数
* Description    : 设置温控PWM波的输出
* Input          : pPid：PID的三个参数结构体;	fActualValue：当前输出温度真实值（反馈值）
* Output         : PID调节之后的输出值 nUk
* Return         : nUk
*******************************************************************************/
int PIDRealize(PID_t* pPid,float fActualValue)
{
//	float error;
//	int result = 0;
	pPid->fActualValue = fActualValue;
	pPid->fError = pPid->fTargetValue - pPid->fActualValue;	// Error(t) = Target(t) - Actual(t)
	if(fabs(pPid->fError) > pPid->fDeadband)				// 如果误差绝对值大于死区则进入PID控制
	{
		pPid->fPropor = pPid->fError * pPid->fKp;			// 比例项：out = Kp * Error(t)
		
		if(fabs(pPid->fPropor) > pPid->nMaxUk)				// 积分项（如果比例项超过最大输出限制）
			pPid->fIntegral = pPid->nUkOffset;				// 积分限幅：把积分设置为偏移量防止过大
		else
		{
			pPid->fIntegral += pPid->fKi * pPid->fError;	// 积分项: out += Ki * Error(t)
			if(pPid->fIntegral > pPid->nMaxUk)				// 积分限幅
				pPid->fIntegral = pPid->nMaxUk;
			else if(pPid->fIntegral >= 0 && pPid->fIntegral < pPid->nUkOffset )
			{
				// 如果积分项在0和偏移量nUkOffset之间，则更新偏移量nUkOffset
				pPid->nUkOffset = pPid->fIntegral;
			}
			else if (pPid->fIntegral < 0)
			{
				// 如果积分项小于0，则将其设置为0，以避免积分项为负值
				pPid->fIntegral = 0;
			}
				
		}
		
		pPid->fdError = pPid->fError - pPid->fLastErr;		// 微分项：det(Error) = error(t) - error(t-1)
		pPid->fDiffer = pPid->fdError * pPid->fKd;			// 微分项：out = Ki * det(Error)
		
		pPid->nUk = (int)(pPid->fPropor + pPid->fIntegral + pPid->fDiffer + 0.5f);	// 最终输出 OUT = P_out + I_out + D_out;
	}
	else		// 如果误差绝对值在死区内
		pPid->nUk = (pPid->fIntegral + 0.5f);	// 控制输出 nUk 等于积分项加上0.5后四舍五入的结果
	
	pPid->fLastErr = pPid->fError;				// 更新上一次误差为当前误差
	if(pPid->nUk < 0)
		pPid->nUk = 0;
	if((pPid->nMaxUk > 0) && (pPid->nUk > pPid->nMaxUk))	// 输出限幅
		pPid->nUk = pPid->nMaxUk;
	
	return pPid->nUk;
}


