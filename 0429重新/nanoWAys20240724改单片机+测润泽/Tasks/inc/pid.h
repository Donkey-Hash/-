#ifndef  __PID_H__
#define  __PID_H__

typedef struct _pid
{
	float	fKp;									//比例系数
	float	fKi;									//积分系数
	float	fKd;									//微分系数
	float	fDeadband;						//死区
	float	fActualValue;					//实际测量值
	float	fTargetValue;					//设定值	
	float fError;
	float	fLastErr;							//上一次误差
	float fdError;							//当前误差
	float fPropor;							//比例放大量
	float	fIntegral;						//积分量	
	float fDiffer;							//微分放大量
	int		nUk;									//控制量
	unsigned int nMaxUk;
	unsigned int nUkOffset;
}PID_t;

void SetPidKp(PID_t* pPid,float fKp);
void SetPidKi(PID_t* pPid,float fKi);
void SetPidKd(PID_t* pPid,float fKd);
void SetPidDeadband(PID_t* pPid,float fDeadband);
void SetPidMaxUk(PID_t* pPid,unsigned int nMaxUk);
void SetTargetValue(PID_t* pPid,float fValue);
void SetPidParam(PID_t* pPid,float fKp,float fKi,float fKd,float fDeadband);
void SetPidIntegral(PID_t* pPid,float fIntegral);
void SetPidUkOffset(PID_t* pPid,unsigned int nUkOffset);
void SetPidfActualValue(PID_t* pPid,float fActualValue);
void PidBumpless(PID_t* pPid);	
void ClearPidParam(PID_t* pPid);
int PIDRealize(PID_t* pPid,float fActualValue);

#endif
