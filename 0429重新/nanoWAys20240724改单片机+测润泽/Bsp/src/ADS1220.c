
#include "ADS1220.h"
#include "leddef.h"
#include "STMcalendar.h"
// #include "SEGGER_RTT.h"

// char JScopeBuf[1024];

unsigned char CHENNEL[4] = {ADS1220_MUX_0_G, ADS1220_MUX_1_G, ADS1220_MUX_2_G, ADS1220_MUX_3_G};

void ADS1220Init(void)
{
}
void SetAdsSingleChennel(unsigned char ch)
{
	unsigned Temp = 0;
	if (ch > 3)
		ch = 0;
	//	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
	//	Temp &= 0x0f;									/* strip out old settings */
	Temp = 0x01 | CHENNEL[ch];
	ADS1220SetChannel(Temp);
	//	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
}

uint32_t ADsRead[4][128];
int ChangeTime = 0;
int Samples = 0;
int ReadADC(uint8_t ch, uint16_t count)
{
	uint16_t u2Samples = 0;
	int res = 0;
	int TempAD = 0;
	uint16_t i = 0;
	int Timeout = 0;
	SetAdsSingleChennel(ch);
	//	ADS1220SendStartCommand();
	while (u2Samples < count)
	{
		Timeout = 10000;
		//			LED2_OFF
		//			osDelay(1);
		while ((ADS1220_DRDY) && (Timeout-- >= 0))
		{
		}
		//				LED2_ON
		ChangeTime = 10000 - Timeout;
		//      if (Timeout < 0)
		//         return 0; /* ADS1220_TIMEOUT_WARNING; */
		TempAD = ADS1220ReadData();
		if (i == 0)
		{
			i++;
		}
		else
		{
			res += TempAD;
			ADsRead[ch][Samples++] = TempAD;
			// SEGGER_RTT_printf(0, "%d,%d,%d\r\n",ch,TempAD,ChangeTime);
			Samples &= 0x7F;
			u2Samples++;
			i++;
		}
	}
	res /= u2Samples;
	//		ADS1220SendShutdownCommand();
	return res; // >> 8;
}
void ReadADCs(uint16_t *pAD, uint16_t count)
{
	uint16_t u2Samples = 0;
	int res2 = 0;
	int res3 = 0;
	int TempAD = 0;
	//  uint16_t i = 0;
	int Timeout = 0;
	SetAdsSingleChennel(3);
	////////	Timeout = 10000;
	////////	while ( (ADS1220_DRDY) && (Timeout-- >= 0))
	////////	{}
	////////	ChangeTime = 10000 - Timeout;
	////////	TempAD = ADS1220ReadData();
	////////	ADS1220SendStartCommand();
	//	count = 100;
	while (u2Samples < count)
	{
		Timeout = 10000;
		while ((ADS1220_DRDY) && (Timeout-- >= 0)) /// 这部分应该改成中断方式
		{
		}
		ChangeTime = 10000 - Timeout;
		TempAD = ADS1220ReadData();
		//				LED1_TOGGLE
		//				SEGGER_RTT_printf(0, ",%d",TempAD);

		SetAdsSingleChennel(2); //			ADS1220SendStartCommand();
		res3 += TempAD;
		ADsRead[3][Samples] = TempAD;

		Timeout = 10000;
		while ((ADS1220_DRDY) && (Timeout-- >= 0))
		{
		}
		ChangeTime = 10000 - Timeout;
		TempAD = ADS1220ReadData();
		//				SEGGER_RTT_printf(0, ",%d",TempAD);
		SetAdsSingleChennel(3); //			ADS1220SendStartCommand();
		res2 += TempAD;
		ADsRead[2][Samples++] = TempAD;

		Samples &= 0x7F;
		u2Samples++;
		//			i++;
	}
	res2 /= u2Samples;
	res3 /= u2Samples;
	//		SEGGER_RTT_printf(0, ",%d",res2);
	//		SEGGER_RTT_printf(0, ",%d",res3);

	res2 >>= 8;
	res3 >>= 8;
	pAD[1] = res2;
	pAD[0] = res3; //		ADS1220SendShutdownCommand();
	//		SEGGER_RTT_printf(0, ",%d",res2);
	//		SEGGER_RTT_printf(0, ",%d\r\n",res3);
}

/* ADS1220 Initial Configuration */
void ADS1220Config(void)
{
	unsigned Temp;
	//	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
	/* clear prev value; */
	Temp = 0x00;
	Temp = ADS1220_MUX_3_G | ADS1220_PGA_BYPASS;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_0_REGISTER, 0x01, &Temp);
	Temp = 0x00;
	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);

	/* clear prev DataRate code; */
	Temp = 0x00;
	Temp = (ADS1220_DR_1000 | ADS1220_CC | ADS1220_MODE_NORMAL); /* Set default start mode to 600sps and continuous conversions */
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	Temp = 0x00;
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);

	Temp = 0x00;
	Temp = (ADS1220_REJECT_50); /* Set default start mode to 600sps and continuous conversions */
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	Temp = 0x00;
	ADS1220ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	ADS1220SendStartCommand();
	//	Temp = 0x00;
	//	Temp = (0x00);		/* Set default start mode to 600sps and continuous conversions */
	//	/* write the register value containing the new value back to the ADS */
	//	ADS1220WriteRegister(ADS1220_3_REGISTER, 0x01, &Temp);
	//	ADS1220ReadRegister(ADS1220_3_REGISTER, 0x01, &Temp);
}
/*  Polling Function */
int ADS1220WaitForDataReady(int Timeout)
{
	if (Timeout > 0)
	{
		/* wait for /DRDY = 1 */
		while (!(ADS1220_DRDY) && (Timeout-- >= 0))
			;
		/* wait for /DRDY = 0 */
		while ((ADS1220_DRDY) && (Timeout-- >= 0))
			;
		if (Timeout < 0)
			return 0; /* ADS1220_TIMEOUT_WARNING; */
	}
	else
	{
		/* wait for /DRDY = 1 */
		while (!(ADS1220_DRDY))
			;
		/* wait for /DRDY = 0 */
		while ((ADS1220_DRDY))
			;
	}
	return ADS1220_NO_ERROR;
}
void ADS1220AssertCS(bool fAssert)
{
	if (fAssert)
	{
		SEL_ADC
	}
	else
	{
		DIS_ADC
	}
}

uint16_t retry = 0;
uint16_t retry1 = 0;
uint16_t retry2 = 0;
uint16_t retry3 = 0;
uint16_t retry4 = 0;
void ADS1220SendByte(unsigned char TxData)
{
	retry = 0;
	//  = 0;
	/* Check if Tx buffer is empty */
	while (LL_SPI_IsActiveFlag_TXE(SPI2) == 0x00)
	{
		retry++;
		if (retry > 20000)
			return;
	}
	retry1 = retry;

	/* Write character in Data register.
	TXE flag is cleared by reading data in DR register */
	LL_SPI_TransmitData8(SPI2, TxData);
	retry = 0;

	/* Check if Rx buffer is not empty */
	while (LL_SPI_IsActiveFlag_RXNE(SPI2) == 0x00)
	{
		retry++;
		if (retry > 20000)
			return;
	}
	retry2 = retry;
	LL_SPI_ReceiveData8(SPI2);
}
unsigned char ADS1220ReceiveByte(uint8_t TxData)
{
	//	uint16_t retry = 0;
	retry = 0;
	// retry3 = 0;
	/* Check if Tx buffer is empty */
	while (LL_SPI_IsActiveFlag_TXE(SPI2) == 0x00)
	{
		retry++;
		if (retry > 20000)
			return 0;
	}
	retry3 = retry;
	/* Write character in Data register.
	TXE flag is cleared by reading data in DR register */
	LL_SPI_TransmitData8(SPI2, TxData);
	retry = 0;

	/* Check if Rx buffer is not empty */
	while (LL_SPI_IsActiveFlag_RXNE(SPI2) == 0x00)
	{
		retry++;
		if (retry > 20000)
			return 0;
	}
	retry4 = retry;
	/* received byte from SPI lines. */
	return LL_SPI_ReceiveData8(SPI2);
}
/*
******************************************************************************
 higher level functions
*/
long ADS1220ReadData(void)
{
	long Data;
	/* assert CS to start transfer */
	ADS1220AssertCS(1);
	/* send the command byte */
	ADS1220SendByte(ADS1220_CMD_RDATA);
	/* get the conversion result */
#ifdef ADS1120
	Data = ADS1220ReceiveByte();
	Data = (Data << 8) | ADS1220ReceiveByte();
	/* sign extend data */
	if (Data & 0x8000)
		Data |= 0xffff0000;
#else
	Data = ADS1220ReceiveByte(0xFF);
	Data = (Data << 8) | ADS1220ReceiveByte(0xFF);
	Data = (Data << 8) | ADS1220ReceiveByte(0xFF);
	/* sign extend data */
	if (Data & 0x800000)
		Data |= 0xff000000;
#endif
	/* de-assert CS */
	ADS1220AssertCS(0);
	return Data;
}

void ADS1220ReadRegister(int StartAddress, int NumRegs, unsigned *pData)
{
	int i;
	/* assert CS to start transfer */
	ADS1220AssertCS(1);
	/* send the command byte */
	ADS1220ReceiveByte(ADS1220_CMD_RREG | (((StartAddress << 2) & 0x0c) | ((NumRegs - 1) & 0x03)));
	/* get the register content */
	for (i = 0; i < NumRegs; i++)
	{
		*pData++ = ADS1220ReceiveByte(0xFF);
	}
	/* de-assert CS */

	ADS1220AssertCS(0);
	return;
}
void ADS1220WriteRegister(int StartAddress, int NumRegs, unsigned *pData)
{
	int i;
	/* assert CS to start transfer */
	ADS1220AssertCS(1);
	/* send the command byte */
	ADS1220SendByte(ADS1220_CMD_WREG | (((StartAddress << 2) & 0x0c) | ((NumRegs - 1) & 0x03)));
	/* send the data bytes */
	for (i = 0; i < NumRegs; i++)
	{
		ADS1220SendByte(*pData++);
	}
	/* de-assert CS */
	ADS1220AssertCS(0);
	return;
}
void ADS1220SendResetCommand(void)
{
	/* assert CS to start transfer */
	ADS1220AssertCS(1);
	/* send the command byte */
	ADS1220SendByte(ADS1220_CMD_RESET);
	/* de-assert CS */
	ADS1220AssertCS(0);
	return;
}
void ADS1220SendStartCommand(void)
{
	/* assert CS to start transfer */
	ADS1220AssertCS(1);
	/* send the command byte */
	ADS1220SendByte(ADS1220_CMD_SYNC);
	/* de-assert CS */
	ADS1220AssertCS(0);
	return;
}
void ADS1220SendShutdownCommand(void)
{
	/* assert CS to start transfer */
	ADS1220AssertCS(1);
	/* send the command byte */
	ADS1220SendByte(ADS1220_CMD_SHUTDOWN);
	/* de-assert CS */
	ADS1220AssertCS(0);
	return;
}
/*
******************************************************************************
register set value commands
*/
int ADS1220SetChannel(int Mux)
{
	unsigned int cMux = Mux;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_0_REGISTER, 0x01, &cMux);
	return ADS1220_NO_ERROR;
}
int ADS1220SetGain(int Gain)
{
	unsigned int cGain = Gain;
	/* write the register value containing the new code back to the ADS */
	ADS1220WriteRegister(ADS1220_0_REGISTER, 0x01, &cGain);
	return ADS1220_NO_ERROR;
}
int ADS1220SetPGABypass(int Bypass)
{
	unsigned int cBypass = Bypass;
	/* write the register value containing the new code back to the ADS */
	ADS1220WriteRegister(ADS1220_0_REGISTER, 0x01, &cBypass);
	return ADS1220_NO_ERROR;
}
int ADS1220SetDataRate(int DataRate)
{
	unsigned int cDataRate = DataRate;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_1_REGISTER, 0x01, &cDataRate);
	return ADS1220_NO_ERROR;
}
int ADS1220SetClockMode(int ClockMode)
{
	unsigned int cClockMode = ClockMode;
	/* write the register value containing the value back to the ADS */
	ADS1220WriteRegister(ADS1220_1_REGISTER, 0x01, &cClockMode);
	return ADS1220_NO_ERROR;
}
int ADS1220SetPowerDown(int PowerDown)
{
	unsigned int cPowerDown = PowerDown;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_1_REGISTER, 0x01, &cPowerDown);
	return ADS1220_NO_ERROR;
}
int ADS1220SetTemperatureMode(int TempMode)
{
	unsigned int cTempMode = TempMode;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_1_REGISTER, 0x01, &cTempMode);
	return ADS1220_NO_ERROR;
}
int ADS1220SetBurnOutSource(int BurnOut)
{
	unsigned int cBurnOut = BurnOut;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_1_REGISTER, 0x01, &cBurnOut);
	return ADS1220_NO_ERROR;
}
int ADS1220SetVoltageReference(int VoltageRef)
{
	unsigned int cVoltageRef = VoltageRef;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_2_REGISTER, 0x01, &cVoltageRef);
	return ADS1220_NO_ERROR;
}
int ADS1220Set50_60Rejection(int Rejection)
{
	unsigned int cRejection = Rejection;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_2_REGISTER, 0x01, &cRejection);
	return ADS1220_NO_ERROR;
}
int ADS1220SetLowSidePowerSwitch(int PowerSwitch)
{
	unsigned int cPowerSwitch = PowerSwitch;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_2_REGISTER, 0x01, &cPowerSwitch);
	return ADS1220_NO_ERROR;
}
int ADS1220SetCurrentDACOutput(int CurrentOutput)
{
	unsigned int cCurrentOutput = CurrentOutput;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_2_REGISTER, 0x01, &cCurrentOutput);
	return ADS1220_NO_ERROR;
}
int ADS1220SetIDACRouting(int IDACRoute)
{
	unsigned int cIDACRoute = IDACRoute;
	/* write the register value containing the new value back to the ADS */
	ADS1220WriteRegister(ADS1220_3_REGISTER, 0x01, &cIDACRoute);
	return ADS1220_NO_ERROR;
}
int ADS1220SetDRDYMode(int DRDYMode)
{
	unsigned int cDRDYMode = DRDYMode;
	/* write the register value containing the new gain code back to the ADS */
	ADS1220WriteRegister(ADS1220_3_REGISTER, 0x01, &cDRDYMode);
	return ADS1220_NO_ERROR;
}
/*
******************************************************************************
register get value commands
*/
int ADS1220GetChannel(void)
{
	unsigned Temp;
	/* Parse Mux data from register */
	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return (Temp >> 4);
}
int ADS1220GetGain(void)
{
	unsigned Temp;
	/* Parse Gain data from register */
	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return ((Temp & 0x0e) >> 1);
}
int ADS1220GetPGABypass(void)
{
	unsigned Temp;
	/* Parse Bypass data from register */
	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return (Temp & 0x01);
}
int ADS1220GetDataRate(void)
{
	unsigned Temp;
	/* Parse DataRate data from register */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return (Temp >> 5);
}
int ADS1220GetClockMode(void)
{
	unsigned Temp;
	/* Parse ClockMode data from register */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return ((Temp & 0x18) >> 3);
}
int ADS1220GetPowerDown(void)
{
	unsigned Temp;
	/* Parse PowerDown data from register */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return ((Temp & 0x04) >> 2);
}
int ADS1220GetTemperatureMode(void)
{
	unsigned Temp;
	/* Parse TempMode data from register */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return ((Temp & 0x02) >> 1);
}
int ADS1220GetBurnOutSource(void)
{
	unsigned Temp;
	/* Parse BurnOut data from register */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return (Temp & 0x01);
}
int ADS1220GetVoltageReference(void)
{
	unsigned Temp;
	/* Parse VoltageRef data from register */
	ADS1220ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return (Temp >> 6);
}
int ADS1220Get50_60Rejection(void)
{
	unsigned Temp;
	/* Parse Rejection data from register */
	ADS1220ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return ((Temp & 0x30) >> 4);
}
int ADS1220GetLowSidePowerSwitch(void)
{
	unsigned Temp;
	/* Parse PowerSwitch data from register */
	ADS1220ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return ((Temp & 0x08) >> 3);
}
int ADS1220GetCurrentDACOutput(void)
{
	unsigned Temp;
	/* Parse IDACOutput data from register */
	ADS1220ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return (Temp & 0x07);
}
int ADS1220GetIDACRouting(int WhichOne)
{
	/* Check WhichOne sizing */
	if (WhichOne > 1)
		return ADS1220_ERROR;
	unsigned Temp;
	/* Parse Mux data from register */
	ADS1220ReadRegister(ADS1220_3_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	if (WhichOne)
		return ((Temp & 0x1c) >> 2);
	else
		return (Temp >> 5);
}
int ADS1220GetDRDYMode(void)
{
	unsigned Temp;
	/* Parse DRDYMode data from register */
	ADS1220ReadRegister(ADS1220_3_REGISTER, 0x01, &Temp);
	/* return the parsed data */
	return ((Temp & 0x02) >> 1);
}
/* Useful Functions within Main Program for Setting Register Contents
 *
 *  	These functions show the programming flow based on the header definitions.
 *  	The calls are not made within the demo example, but could easily be used by calling the function
 *  		defined within the program to complete a fully useful program.
 *	Similar function calls were made in the firwmare design for the ADS1220EVM.
 *
 *  The following function calls use ASCII data sent from a COM port to control settings
 *	on the ADS1220.  The data is recontructed from ASCII and then combined with the
 *	register contents to save as new configuration settings.
 *
 * 	Function names correspond to datasheet register definitions
 */
void set_MUX(char c)
{
	int mux = (int)c - 48;
	int dERROR;
	unsigned Temp;
	if (mux >= 49 && mux <= 54)
		mux -= 39;
	/* The MUX value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
	Temp &= 0x0f; /* strip out old settings */
	/* Change Data rate */
	switch (mux)
	{
	case 0:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_0_1);
		break;
	case 1:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_0_2);
		break;
	case 2:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_0_3);
		break;
	case 3:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_1_2);
		break;
	case 4:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_1_3);
		break;
	case 5:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_2_3);
		break;
	case 6:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_1_0);
		break;
	case 7:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_3_2);
		break;
	case 8:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_0_G);
		break;
	case 9:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_1_G);
		break;
	case 10:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_2_G);
		break;
	case 11:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_3_G);
		break;
	case 12:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_EX_VREF);
		break;
	case 13:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_AVDD);
		break;
	case 14:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_DIV2);
		break;
	case 15:
		dERROR = ADS1220SetChannel(Temp + ADS1220_MUX_DIV2);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_GAIN(char c)
{
	int pga = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* The GAIN value is only part of the register, so we have to read it back
	   and massage the new value into it*/
	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
	Temp &= 0xf1; /* strip out old settings */
	/* Change gain rate */
	switch (pga)
	{
	case 0:
		dERROR = ADS1220SetGain(Temp + ADS1220_GAIN_1);
		break;
	case 1:
		dERROR = ADS1220SetGain(Temp + ADS1220_GAIN_2);
		break;
	case 2:
		dERROR = ADS1220SetGain(Temp + ADS1220_GAIN_4);
		break;
	case 3:
		dERROR = ADS1220SetGain(Temp + ADS1220_GAIN_8);
		break;
	case 4:
		dERROR = ADS1220SetGain(Temp + ADS1220_GAIN_16);
		break;
	case 5:
		dERROR = ADS1220SetGain(Temp + ADS1220_GAIN_32);
		break;
	case 6:
		dERROR = ADS1220SetGain(Temp + ADS1220_GAIN_64);
		break;
	case 7:
		dERROR = ADS1220SetGain(Temp + ADS1220_GAIN_128);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_PGA_BYPASS(char c)
{
	int buff = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the PGA Bypass value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
	Temp &= 0xfe; /* strip out old settings */
	/* Change PGA Bypass */
	switch (buff)
	{
	case 0:
		dERROR = ADS1220SetPGABypass(Temp);
		break;
	case 1:
		dERROR = ADS1220SetPGABypass(Temp + ADS1220_PGA_BYPASS);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_DR(char c)
{
	int spd = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the DataRate value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	Temp &= 0x1f; /* strip out old settings */
	/* Change Data rate */
	switch (spd)
	{
	case 0:
		dERROR = ADS1220SetDataRate(Temp + ADS1220_DR_20);
		break;
	case 1:
		dERROR = ADS1220SetDataRate(Temp + ADS1220_DR_45);
		break;
	case 2:
		dERROR = ADS1220SetDataRate(Temp + ADS1220_DR_90);
		break;
	case 3:
		dERROR = ADS1220SetDataRate(Temp + ADS1220_DR_175);
		break;
	case 4:
		dERROR = ADS1220SetDataRate(Temp + ADS1220_DR_330);
		break;
	case 5:
		dERROR = ADS1220SetDataRate(Temp + ADS1220_DR_600);
		break;
	case 6:
		dERROR = ADS1220SetDataRate(Temp + ADS1220_DR_1000);
		break;
	case 7:
		dERROR = ADS1220SetDataRate(Temp + ADS1220_DR_1000);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_MODE(char c)
{
	int spd = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the MODE value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	Temp &= 0xe7; /* strip out old settings */
	/* Change the operating Mode */
	switch (spd)
	{
	case 0:
		dERROR = ADS1220SetClockMode(Temp + ADS1220_MODE_NORMAL);
		break;
	case 1:
		dERROR = ADS1220SetClockMode(Temp + ADS1220_MODE_DUTY);
		break;
	case 2:
		dERROR = ADS1220SetClockMode(Temp + ADS1220_MODE_TURBO);
		break;
	case 3:
		dERROR = ADS1220SetClockMode(Temp + ADS1220_MODE_DCT);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_CM(char c)
{
	int pwrdn = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the Conversion Mode value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	Temp &= 0xfb; /* strip out old settings */
	/* Change power down mode */
	switch (pwrdn)
	{
	case 0:
		dERROR = ADS1220SetPowerDown(Temp);
		break;
	case 1:
		dERROR = ADS1220SetPowerDown(Temp + ADS1220_CC);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_TS(char c)
{
	int tmp = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the Temperature Sensor mode value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	Temp &= 0xfd; /* strip out old settings */
	/* Change Temp Diode Setting */
	switch (tmp)
	{
	case 0:
		dERROR = ADS1220SetTemperatureMode(Temp);
		break;
	case 1:
		dERROR = ADS1220SetTemperatureMode(Temp + ADS1220_TEMP_SENSOR);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_BCS(char c)
{
	int bo = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the Burnout Current Source value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
	Temp &= 0xfe; /* strip out old settings */
	/* Change Burnout Current */
	switch (bo)
	{
	case 0:
		dERROR = ADS1220SetBurnOutSource(Temp);
		break;
	case 1:
		dERROR = ADS1220SetBurnOutSource(Temp + ADS1220_BCS);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_VREF(char c)
{
	int ref = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the Voltage Reference value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	Temp &= 0x3f; /* strip out old settings */
	/* Change Reference */
	switch (ref)
	{
	case 0:
		dERROR = ADS1220SetVoltageReference(Temp + ADS1220_VREF_INT);
		break;
	case 1:
		dERROR = ADS1220SetVoltageReference(Temp + ADS1220_VREF_EX_DED);
		break;
	case 2:
		dERROR = ADS1220SetVoltageReference(Temp + ADS1220_VREF_EX_AIN);
		break;
	case 3:
		dERROR = ADS1220SetVoltageReference(Temp + ADS1220_VREF_SUPPLY);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_50_60(char c)
{
	int flt = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the Digital Filter value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	Temp &= 0xcf; /* strip out old settings */
	/* Change Filter Setting */
	switch (flt)
	{
	case 0:
		dERROR = ADS1220Set50_60Rejection(Temp + ADS1220_REJECT_OFF);
		break;
	case 1:
		dERROR = ADS1220Set50_60Rejection(Temp + ADS1220_REJECT_BOTH);
		break;
	case 2:
		dERROR = ADS1220Set50_60Rejection(Temp + ADS1220_REJECT_50);
		break;
	case 3:
		dERROR = ADS1220Set50_60Rejection(Temp + ADS1220_REJECT_60);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_PSW(char c)
{
	int sw = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the Low-side Switch value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	Temp &= 0xf7; /* strip out old settings */
				  /* Change low-side switch mode */
	switch (sw)
	{
	case 0:
		dERROR = ADS1220SetLowSidePowerSwitch(Temp);
		break;
	case 1:
		dERROR = ADS1220SetLowSidePowerSwitch(Temp + ADS1220_PSW_SW);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_IDAC(char c)
{
	int current = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the IDAC Current value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
	Temp &= 0xf8; /* strip out old settings */
	/* Change IDAC Current Output */
	switch (current)
	{
	case 0:
		dERROR = ADS1220SetCurrentDACOutput(Temp + ADS1220_IDAC_OFF);
		break;
	case 1:
#ifdef ADS1120
		dERROR = ADS1220SetCurrentDACOutput(Temp + ADS1220_IDAC_OFF);
#else
		dERROR = ADS1220SetCurrentDACOutput(Temp + ADS1220_IDAC_10);
#endif
		break;
	case 2:
		dERROR = ADS1220SetCurrentDACOutput(Temp + ADS1220_IDAC_50);
		break;
	case 3:
		dERROR = ADS1220SetCurrentDACOutput(Temp + ADS1220_IDAC_100);
		break;
	case 4:
		dERROR = ADS1220SetCurrentDACOutput(Temp + ADS1220_IDAC_250);
		break;
	case 5:
		dERROR = ADS1220SetCurrentDACOutput(Temp + ADS1220_IDAC_500);
		break;
	case 6:
		dERROR = ADS1220SetCurrentDACOutput(Temp + ADS1220_IDAC_1000);
		break;
	case 7:
		dERROR = ADS1220SetCurrentDACOutput(Temp + ADS1220_IDAC_2000);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_IMUX(char c, int i)
{
	int mux = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the IDAC Mux value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_3_REGISTER, 0x01, &Temp);
	if (i == 1)
	{
		Temp &= 0xe3; /* strip out old settings */
		/* Change IDAC2 MUX Output */
		switch (mux)
		{
		case 0:
			Temp |= ADS1220_IDAC2_OFF;
			break;
		case 1:
			Temp |= ADS1220_IDAC2_AIN0;
			break;
		case 2:
			Temp |= ADS1220_IDAC2_AIN1;
			break;
		case 3:
			Temp |= ADS1220_IDAC2_AIN2;
			break;
		case 4:
			Temp |= ADS1220_IDAC2_AIN3;
			break;
		case 5:
			Temp |= ADS1220_IDAC2_REFP0;
			break;
		case 6:
			Temp |= ADS1220_IDAC2_REFN0;
			break;
		case 7:
			Temp |= ADS1220_IDAC2_REFN0;
			break;
		default:
			dERROR = ADS1220_ERROR;
			break;
		}
	}
	else
	{
		Temp &= 0x1f;
		/* Change IDAC1 MUX Output */
		switch (mux)
		{
		case 0:
			Temp |= ADS1220_IDAC1_OFF;
			break;
		case 1:
			Temp |= ADS1220_IDAC1_AIN0;
			break;
		case 2:
			Temp |= ADS1220_IDAC1_AIN1;
			break;
		case 3:
			Temp |= ADS1220_IDAC1_AIN2;
			break;
		case 4:
			Temp |= ADS1220_IDAC1_AIN3;
			break;
		case 5:
			Temp |= ADS1220_IDAC1_REFP0;
			break;
		case 6:
			Temp |= ADS1220_IDAC1_REFN0;
			break;
		case 7:
			Temp |= ADS1220_IDAC1_REFN0;
			break;
		default:
			dERROR = ADS1220_ERROR;
			break;
		}
	}
	if (dERROR == ADS1220_NO_ERROR)
		dERROR = ADS1220SetIDACRouting(Temp);
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_DRDYM(char c)
{
	int drdy = (int)c - 48;
	int dERROR;
	unsigned Temp;
	/* the DRDY output mode value is only part of the register, so we have to read it back
	   and massage the new value into it */
	ADS1220ReadRegister(ADS1220_3_REGISTER, 0x01, &Temp);
	Temp &= 0xfd; /* strip out old settings */
	/* Change DRDY Mode Setting */
	switch (drdy)
	{
	case 0:
		dERROR = ADS1220SetDRDYMode(Temp);
		break;
	case 1:
		dERROR = ADS1220SetDRDYMode(Temp + ADS1220_DRDY_MODE);
		break;
	default:
		dERROR = ADS1220_ERROR;
		break;
	}
	if (dERROR == ADS1220_ERROR)
		set_ERROR();
}
void set_ERROR(void)
{
	/* Add some error routine here is desired */
}
