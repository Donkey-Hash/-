
#include "port.h"
#include "mbcrc.h"
#include "ModbusMaster.h"
#include "yssyringepump.h"
#include "cmsis_os.h"
#include "FlashUserData.h"
// #include "softtimer.h"

extern bool SetMFCFlowFlag;
// extern SET_FLOWS_t tSetFlows;
/******************************************************/

// MB_MFC_DEV_INFO MbMFCDevInfo[8];

// SET_FLOWS_t tMFCCurrentSet = {0};

// MODBUS_MFC_UPDATE_FLAG MFCUpdateFlag;
uint16_t temp[4] = {0};
/******************************************************/
uint8_t RZTxBuff[64];

volatile float last_ml = 99;
volatile u16 last_speed = 0;
volatile u8 last_num = 99;
volatile u8 RZdelayflag = 0;	//0不做处理。置1则取消延时

int32_t ReadPos(void)
{
	temp[0] = 0x55;
	temp[1] = 0xAA;
	int ret = MbMasterReadInputRegs(HOST_MB_YS_SYRINGE, 0, 2, temp);
	if (ret >= 0)
		return *(int32_t *)temp;
	else
		return ret;
}

void StopYsSyringe(void)
{
	MbMasterWriteHoldingReg(HOST_MB_YS_SYRINGE, 0, YS_SYRINGE_CMD_STOP_MOTOR);
}
void ResetYsSyringeOrigin(void)
{
	MbMasterWriteHoldingReg(HOST_MB_YS_SYRINGE, 0, YS_SYRINGE_CMD_RESET_SYS_ORIGIN);
}

bool SetYsSyringeSpeed(void)
{
	return MbMasterWriteHoldingReg(HOST_MB_YS_SYRINGE, 1, pMbHoldData->nSyringePumpSpeed);
}

void MoveYsSyringeTo(int32_t steps)
{
	MbMasterWriteHoldingRegs(HOST_MB_YS_SYRINGE, 2, 2, (uint16_t *)&steps);
}

void MoveYsSyringe(int32_t steps)
{
	MbMasterWriteHoldingRegs(HOST_MB_YS_SYRINGE, 4, 2, (uint16_t *)&steps);
}

void RUNZE_SETHOLE_1(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x45;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xee;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SETHOLE_2(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x44;
	RZTxBuff[j++] = 0x02;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xef;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SETHOLE_3(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x44;
	RZTxBuff[j++] = 0x03;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xf0;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SETHOLE_4(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x44;
	RZTxBuff[j++] = 0x04;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xf1;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SETHOLE_5(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x44;
	RZTxBuff[j++] = 0x05;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xf2;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SETHOLE_6(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x44;
	RZTxBuff[j++] = 0x06;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xf3;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SETHOLE_7(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x44;
	RZTxBuff[j++] = 0x07;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xf4;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SETHOLE_8(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x44;
	RZTxBuff[j++] = 0x08;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xf5;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SETHOLE_9(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x44;
	RZTxBuff[j++] = 0x09;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xf6;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SETHOLE_10(void)
{	//CC 00 44 01 00 DD EE 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x44;
	RZTxBuff[j++] = 0x0a;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xf7;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}


// void RUNZE_SET_0ML(void)
// {
// 	//CC 01 45 00 00 DD EF 01
// 	int j = 0;
// 	RZTxBuff[j++] = 0xCC;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0x45;
// 	RZTxBuff[j++] = 0x00;
// 	RZTxBuff[j++] = 0x00;
// 	RZTxBuff[j++] = 0xdd;
// 	RZTxBuff[j++] = 0xef;
// 	RZTxBuff[j++] = 0x01;
// 	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
// 	HostModbusSend(RZTxBuff, j);
// }
// void RUNZE_SET_05ML(void)
// {
// 	//CC 01 4E 3E 01 DD 37 02
// 	int j = 0;
// 	RZTxBuff[j++] = 0xCC;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0x4E;
// 	RZTxBuff[j++] = 0x3E;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0xdd;
// 	RZTxBuff[j++] = 0x37;
// 	RZTxBuff[j++] = 0x02;
// 	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
// 	HostModbusSend(RZTxBuff, j);
// }

// void RUNZE_SET_1ML(void)
// {
// 	//CC 01 4E 7D 02 DD 77 02
// 	int j = 0;
// 	RZTxBuff[j++] = 0xCC;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0x4E;
// 	RZTxBuff[j++] = 0x7D;
// 	RZTxBuff[j++] = 0x02;
// 	RZTxBuff[j++] = 0xdd;
// 	RZTxBuff[j++] = 0x77;
// 	RZTxBuff[j++] = 0x02;
// 	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
// 	HostModbusSend(RZTxBuff, j);
// }

// void RUNZE_SET_15ML(void)
// {
// 	//CC 01 4E BB 03 DD B6 02
// 	int j = 0;
// 	RZTxBuff[j++] = 0xCC;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0x4E;
// 	RZTxBuff[j++] = 0xBB;
// 	RZTxBuff[j++] = 0x03;
// 	RZTxBuff[j++] = 0xdd;
// 	RZTxBuff[j++] = 0xB6;
// 	RZTxBuff[j++] = 0x02;
// 	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
// 	HostModbusSend(RZTxBuff, j);
// }


// void RUNZE_SET_2ML(void)
// {
// 	//CC 01 4E F9 04 DD F5 02
// 	int j = 0;
// 	RZTxBuff[j++] = 0xCC;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0x4E;
// 	RZTxBuff[j++] = 0xF9;
// 	RZTxBuff[j++] = 0x04;
// 	RZTxBuff[j++] = 0xdd;
// 	RZTxBuff[j++] = 0xF5;
// 	RZTxBuff[j++] = 0x02;
// 	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
// 	HostModbusSend(RZTxBuff, j);
// }

// void RUNZE_SET_3ML(void)
// {
// 	//CC 01 4E 76 07 DD 75 02
// 	int j = 0;
// 	RZTxBuff[j++] = 0xCC;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0x4E;
// 	RZTxBuff[j++] = 0x76;
// 	RZTxBuff[j++] = 0x07;
// 	RZTxBuff[j++] = 0xdd;
// 	RZTxBuff[j++] = 0x75;
// 	RZTxBuff[j++] = 0x02;
// 	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
// 	HostModbusSend(RZTxBuff, j);
// }

// void RUNZE_SET_4ML(void)
// {
// 	//CC 01 4E F3 09 DD F4 02
// 	int j = 0;
// 	RZTxBuff[j++] = 0xCC;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0x4E;
// 	RZTxBuff[j++] = 0xF3;
// 	RZTxBuff[j++] = 0x09;
// 	RZTxBuff[j++] = 0xdd;
// 	RZTxBuff[j++] = 0xF4;
// 	RZTxBuff[j++] = 0x02;
// 	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
// 	HostModbusSend(RZTxBuff, j);
// }


// void RUNZE_SET_5ML(void)
// {
// 	//CC 01 4E 6F 0C DD 73 02
// 	int j = 0;
// 	RZTxBuff[j++] = 0xCC;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0x4E;
// 	RZTxBuff[j++] = 0x6F;
// 	RZTxBuff[j++] = 0x0C;
// 	RZTxBuff[j++] = 0xdd;
// 	RZTxBuff[j++] = 0x73;
// 	RZTxBuff[j++] = 0x02;
// 	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
// 	HostModbusSend(RZTxBuff, j);
// }


// void RUNZE_SET_6ML(void)
// {
// 	//CC 01 4E EC 0E DD F2 02
// 	int j = 0;
// 	RZTxBuff[j++] = 0xCC;
// 	RZTxBuff[j++] = 0x01;
// 	RZTxBuff[j++] = 0x4E;
// 	RZTxBuff[j++] = 0xEC;
// 	RZTxBuff[j++] = 0x0E;
// 	RZTxBuff[j++] = 0xdd;
// 	RZTxBuff[j++] = 0xF2;
// 	RZTxBuff[j++] = 0x02;
// 	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
// 	HostModbusSend(RZTxBuff, j);
// }



void RUNZE_SET_STOP(void)
{
	//CC 01 49 00 00 DD F3 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x49;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xF3;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SET_NC(void)	//开阀
{
	//CC 01 60 01 00 DD 0B 02
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x60;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0x0b;
	RZTxBuff[j++] = 0x02;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
	osDelay(200);
}

void RUNZE_SET_NO(void)	//关阀。默认是关
{
	//CC 01 61 01 00 DD 0C 02
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x61;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0x0C;
	RZTxBuff[j++] = 0x02;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
	osDelay(200);
}

void RUNZE_SET_SPEED_500(void)	//设置速度为500
{
	//CC 01 4B F4 01 DD EA 02
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x4B;
	RZTxBuff[j++] = 0xF4;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xEA;
	RZTxBuff[j++] = 0x02;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SET_SPEED_200(void)	//设置速度为200
{
	//CC 01 4B C8 00 DD BD 02
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x4B;
	RZTxBuff[j++] = 0xC8;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xBD;
	RZTxBuff[j++] = 0x02;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SET_SPEED_100(void)	//设置速度为100
{
	//CC 01 4B 64 00 DD 59 02
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x4B;
	RZTxBuff[j++] = 0x64;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0x59;
	RZTxBuff[j++] = 0x02;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SET_SPEED_10(void)	//设置速度为10
{
	//CC 01 4B 0A 00 DD FF 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x4B;
	RZTxBuff[j++] = 0x0A;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xFF;
	RZTxBuff[j++] = 0x02;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SET_SPEED_3(void)	//设置速度为3
{
	//CC 01 4B 03 00 DD F8 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x4B;
	RZTxBuff[j++] = 0x03;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xF8;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

void RUNZE_SET_SPEED_1(void)	//设置速度为1
{
	//CC 01 4B 01 00 DD F6 01
	int j = 0;
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x4B;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0xdd;
	RZTxBuff[j++] = 0xF6;
	RZTxBuff[j++] = 0x01;
	// memset(MbRxBuff, 0, sizeof(MbRxBuff));
	HostModbusSend(RZTxBuff, j);
}

/////////////////////////////////////////////////////////////////////////////////////
uint16_t Calculate_rzsum(uint8_t * inbuf,uint8_t len)
{
	uint16_t calsum = 0;
	while(len --)
	{
		calsum += *(inbuf ++);
	}
	return calsum;
}

void RUNZE_SET_ML(float ml)	//0~6,0.5,1.5
{
	int j = 0;
	uint16_t sum = 0;
	float tmp = 0;
	if(ml < 0)
	{
		ml = 0;
	}
	else if(ml > 6)
	{
		ml = 6;
	}
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	if(ml == 0)
	{
		//RUNZE_SET_0ML();
		RZTxBuff[j++] = 0x45;
		RZTxBuff[j++] = 0x00;
		RZTxBuff[j++] = 0x00;
	}
	else
	{
		tmp = ml * 3820;
		tmp /= 6;
		sum = tmp;
		RZTxBuff[j++] = 0x4E;
		RZTxBuff[j++] = sum & 0xff;
		RZTxBuff[j++] = sum >> 8;
	}
	RZTxBuff[j++] = 0xDD;
	sum = 0;
	sum = Calculate_rzsum((uint8_t *)&RZTxBuff, j);
	RZTxBuff[j++] = sum & 0xff;
	RZTxBuff[j++] = sum >> 8;

	HostModbusSend(RZTxBuff, j);
	//////////////////////////////////////////

	if(last_ml != ml && RZdelayflag == 0)
	{
		if(last_ml == 0 && ml >= 4 && last_speed >= 200)
		{
			osDelay(18000);
		}
		else if(ml < last_ml && last_speed >= 200)
		{
			osDelay(15000);
		}
		else if(last_speed < 200)
		{
			osDelay(20000);
		}
		else
		{
			osDelay(15000);
		}
	}
	else
	{
		osDelay(2000);
	}
	last_ml = ml;
}

void RUNZE_SETHOLE(u8 num)	//1~10
{
	osDelay(200);
	// RUNZE_SETHOLE_10();
	// osDelay(5000);
	switch(num)
	{
		case 0:
		case 1:RUNZE_SETHOLE_1();break;
		case 2:RUNZE_SETHOLE_2();break;
		case 3:RUNZE_SETHOLE_3();break;
		case 4:RUNZE_SETHOLE_4();break;
		case 5:RUNZE_SETHOLE_5();break;
		case 6:RUNZE_SETHOLE_6();break;
		case 7:RUNZE_SETHOLE_7();break;
		case 8:RUNZE_SETHOLE_8();break;
		case 9:RUNZE_SETHOLE_9();break;
		case 10:RUNZE_SETHOLE_10();break;
		default:RUNZE_SETHOLE_1();break;
	}
	if(last_num != num && RZdelayflag == 0)
		osDelay(5000);
	else
		osDelay(500);
	last_num = num;
}

void RUNZE_SET_SPEED(u16 speed)
{
	int j = 0;
	uint16_t sum = 0;

	osDelay(200);
	if(speed < 1)
	{
		speed = 1;
	}
	else if(speed > 500)
	{
		speed = 500;
	}
	RZTxBuff[j++] = 0xCC;
	RZTxBuff[j++] = 0x01;
	RZTxBuff[j++] = 0x4B;
	RZTxBuff[j++] = speed & 0xff;
	RZTxBuff[j++] = speed >> 8;
	RZTxBuff[j++] = 0xDD;
	sum = Calculate_rzsum((uint8_t *)&RZTxBuff, j);
	RZTxBuff[j++] = sum & 0xff;
	RZTxBuff[j++] = sum >> 8;

	HostModbusSend(RZTxBuff, j);


	// else if(speed <= 10)
	// {
	// 	RUNZE_SET_SPEED_10();	//设置速度为10
	// }
	// else if(speed <= 100)
	// {
	// 	RUNZE_SET_SPEED_100();	//设置速度为100
	// }
	// else if(speed <= 200)
	// {
	// 	RUNZE_SET_SPEED_200();	//设置速度为200
	// }
	// else
	// {
	// 	RUNZE_SET_SPEED_500();	//设置速度为200
	// }
	osDelay(500);
	last_speed = speed;
}


////////////////////////////////////////////////////////////////////////
//恒创

void HC_SET_ML(float ml)	//0.0~5.0
{	//22 06 00 14 13 88	//5ml = 5000 = 0x1388
	int j = 0;
	uint16_t crc = 0;
	uint16_t tmp = 0;
	osDelay(200);
	if(ml > 5)
		ml = 5;
	RZTxBuff[j++] = 0x22;
	RZTxBuff[j++] = 0x06;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x14;
	if(ml == 0)
	{
		RZTxBuff[j++] = 0xff;
		RZTxBuff[j++] = 0xff;
	}
	else
	{
		tmp = ml * 1000;
		RZTxBuff[j++] = tmp >> 8;
		RZTxBuff[j++] = tmp & 0xff;

	}
	crc = usMBCRC16((uint8_t *)&RZTxBuff, j);
	RZTxBuff[j++] = crc & 0xff;
	RZTxBuff[j++] = crc >> 8;

	HostModbusSend(RZTxBuff, j);

	if(last_ml != ml && RZdelayflag == 0)
	{
		if(last_ml == 0 && ml >= 4 && last_speed >= 200)
		{
			osDelay(18000);
		}
		else if(ml < last_ml && last_speed >= 200)
		{
			osDelay(15000);
		}
		else if(last_speed < 200)
		{
			osDelay(20000);
		}
		else
		{
			osDelay(15000);
		}
	}
	else
	{
		osDelay(2000);
	}
	last_ml = ml;

}

void HC_SETHOLE(u8 num)	//0~10,0时阻塞
{	//21 05 00 01 FF 00 xx xx
	int j = 0;
	uint16_t crc = 0;
	osDelay(200);
	if(num > 10)
		num = 10;

	RZTxBuff[j++] = 0x21;
	RZTxBuff[j++] = 0x05;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = num;
	RZTxBuff[j++] = 0xff;
	RZTxBuff[j++] = 0x00;
	crc = usMBCRC16((uint8_t *)&RZTxBuff, j);
	RZTxBuff[j++] = crc & 0xff;
	RZTxBuff[j++] = crc >> 8;

	HostModbusSend(RZTxBuff, j);
	if(last_num != num && RZdelayflag == 0)
		osDelay(5000);
	else
		osDelay(500);
	last_num = num;
	
}

void HC_SET_SPEED(u16 speed)	//1~1500
{	//22 06 00 0C 00 01
	int j = 0;
	uint16_t crc = 0;
	osDelay(200);
	speed *= 2;
	if(speed > 1500)
		speed = 1500;
	if(speed == 0)
		speed = 1;
	
	RZTxBuff[j++] = 0x22;
	RZTxBuff[j++] = 0x06;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x0c;
	RZTxBuff[j++] = speed >> 8;
	RZTxBuff[j++] = speed & 0xff;
	crc = usMBCRC16((uint8_t *)&RZTxBuff, j);
	RZTxBuff[j++] = crc & 0xff;
	RZTxBuff[j++] = crc >> 8;

	HostModbusSend(RZTxBuff, j);
	osDelay(500);
}

void HC_SET_STOP(void)
{	//22 06 00 14 FF FF
	int j = 0;
	uint16_t crc = 0;
	RZTxBuff[j++] = 0x22;
	RZTxBuff[j++] = 0x06;
	RZTxBuff[j++] = 0x00;
	RZTxBuff[j++] = 0x14;
	RZTxBuff[j++] = 0xff;
	RZTxBuff[j++] = 0xff;
	crc = usMBCRC16((uint8_t *)&RZTxBuff, j);
	RZTxBuff[j++] = crc & 0xff;
	RZTxBuff[j++] = crc >> 8;

	HostModbusSend(RZTxBuff, j);
}

void HC_SET_NC(void)	//开阀
{
	Y17_ON
}

void HC_SET_NO(void)	//关阀。默认是关
{
	Y17_OFF
}
