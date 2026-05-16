
#ifndef __PROGRESS_BAR_H__
#define __PROGRESS_BAR_H__

#include "instrumentinfo.h"
#include "replyhost.h"

void SetCrrentStepName(STEP_NAME_CODE stepname);
void UpdateStepStatus(void);
void UpdateStepName(STEP_NAME_CODE stepname);
void InitStepStatus(STEP_NAME_CODE initstepname,uint16_t updatecycle);
void StopUpdateStepStatus(void);
void StopReportStepTimer(void);

#endif

