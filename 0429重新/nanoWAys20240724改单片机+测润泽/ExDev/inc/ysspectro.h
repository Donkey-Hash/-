#ifndef __YS_SPECTRO_H__
#define __YS_SPECTRO_H__
#include <stdbool.h>
#include "port.h"

void ExDevDMAConfig(void);
void ExDevRxIdleCallback(void);

bool OpenSpectroLight(void);
bool CloseSpectroLight(void);
bool StartSpectroScan(void);
int16_t ReadSpectroData(uint16_t* pdata);
bool SetAverageTimes(void);
bool SetIntegralTime(void);
#endif

