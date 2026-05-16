
#ifndef __ENCRYPT_MCU_H__
#define __ENCRYPT_MCU_H__
#include <stdbool.h>
#include "main.h"

extern const __IO uint32_t wDeviceID[3];

void EncryptAlgo(uint8_t *destin, uint8_t *source, uint16_t n );
void DecryptAlgo(uint8_t *destin, uint8_t *source, uint16_t n );
int GetMcuEncryptState(void);
void ProgramEncryptCode(void);
bool ProgramDevSNCode(uint8_t* sn,uint32_t len);
void ReReadDevID(uint8_t * rIDstr);
#endif
