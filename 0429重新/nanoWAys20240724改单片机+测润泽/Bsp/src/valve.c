
#include "valvedef.h"
#include "TestLEDCtr.h"
#include "peristalticpump.h"

extern UCHAR *usCoilBuf;

bool VirtualRelay1 = false;
bool VirtualRelay2 = false;
bool VirtualRelay3 = false;
bool VirtualRelay4 = false;
bool VirtualRelay5 = false;
bool VirtualRelay6 = false;
bool VirtualRelay7 = false;
bool VirtualRelay8 = false;
bool VirtualRelay9 = false;
bool VirtualRelay10 = false;
bool *pvrCurrenttest = &VirtualRelay1;
bool *pvrAmaxOut = &VirtualRelay2;
bool *pvrAminOut = &VirtualRelay3;
bool *pvrAutoRang = &VirtualRelay4;

void UpdataValveState(UCHAR *pCoilBuf)
{
	UCHAR tempreg;
	UCHAR bits = 0x00;
	unsigned int valves = 1; // MAX_MB_VALVES;
	while (valves <= MAX_MB_VALVES)
	{
		bits = 0;
		//		ENTER_CRITICAL_SECTION();
		tempreg = *pCoilBuf++;
		//		EXIT_CRITICAL_SECTION( );
		while ((valves <= MAX_MB_VALVES) && (bits < 8))
		{
			if (tempreg & 0x01)
				OpenValveX((VALVE_NAME)valves);
			else
				CloseValveX((VALVE_NAME)valves);
			tempreg >>= 1;
			valves++;
			bits++;
		}
	}
}

void OpenValveX(VALVE_NAME valve)
{
	switch (valve)
	{
	case VALVE_1:
		Y1_ON
		break;
	case VALVE_2:
		Y2_ON
		break;
	case VALVE_3:
		Y3_ON
		break;
	case VALVE_4:
		Y4_ON
		break;
	case VALVE_5:
		Y5_ON
		break;
	case VALVE_6:
		Y6_ON
		break;
	case VALVE_7:
		Y7_ON
		break;
	case VALVE_8:
		Y8_ON
		break;
	case VALVE_9:
		Y9_ON
		break;
	case VALVE_10:
		Y10_ON
		break;
	case VALVE_11:
		Y11_ON
		break;
	case VALVE_12:
		Y12_ON
		break;
	case VALVE_13:
		Y13_ON
		break;
	case VALVE_14:
		Y14_ON
		break;
	case VALVE_15:
		Y15_ON
		break;
	case VALVE_16:
		Y16_ON
		break;
	case VALVE_17:
		Y17_ON
		break;
	case VALVE_18:
		Y18_ON
		break;
	case VALVE_19:
		Y19_ON
		break;
		//		case VALVE_20:
		//			Y20_ON
		//			break;
		//		case VALVE_21:
		//			Y21_ON
		//			break;
		//		case VALVE_22:
		//			Y22_ON
		//			break;
		//		case VALVE_23:
		//			Y23_ON
		//			break;
		//		case VALVE_24:
		//			Y24_ON
		//			break;
	case VIRTUAL_RELAY_1:
		VirtualRelay1 = true;
		break;
	case VIRTUAL_RELAY_2:
		VirtualRelay2 = true;
		break;
	case VIRTUAL_RELAY_3:
		VirtualRelay3 = true;
		break;
	case VIRTUAL_RELAY_4:
		VirtualRelay4 = true;
		break;
	case VIRTUAL_RELAY_5:
		VirtualRelay5 = true;
		break;
	case VIRTUAL_RELAY_6:
		VirtualRelay6 = true;
		break;
	case VIRTUAL_RELAY_7:
		VirtualRelay7 = true;
		break;
	case VIRTUAL_RELAY_8:
		VirtualRelay8 = true;
		break;
	case POOL_LIGHT:
		OpenAbsorbLED();
		break;
		//		case PERISTALTIC_PUMP_PUSH:
		//			StopPump();
		//			StartPump(PUSH_LIQUID,SPEED_SAMPLE,15);
		//			break;
		//		case PERISTALTIC_PUMP_PULL:
		//			StopPump();
		//			StartPump(DRAW_LIQUID,SPEED_SAMPLE,10);
		//			break;
	default:
		break;
	}
}

void CloseValveX(VALVE_NAME valve)
{
	switch (valve)
	{
	case VALVE_1:
		Y1_OFF
		break;
	case VALVE_2:
		Y2_OFF
		break;
	case VALVE_3:
		Y3_OFF
		break;
	case VALVE_4:
		Y4_OFF
		break;
	case VALVE_5:
		Y5_OFF
		break;
	case VALVE_6:
		Y6_OFF
		break;
	case VALVE_7:
		Y7_OFF
		break;
	case VALVE_8:
		Y8_OFF
		break;
	case VALVE_9:
		Y9_OFF
		break;
	case VALVE_10:
		Y10_OFF
		break;
	case VALVE_11:
		Y11_OFF
		break;
	case VALVE_12:
		Y12_OFF
		break;
	case VALVE_13:
		Y13_OFF
		break;
	case VALVE_14:
		Y14_OFF
		break;
	case VALVE_15:
		Y15_OFF
		break;
	case VALVE_16:
		Y16_OFF
		break;
	case VALVE_17:
		Y17_OFF
		break;
	case VALVE_18:
		Y18_OFF
		break;
	case VALVE_19:
		Y19_OFF
		break;
		//		case VALVE_20:
		//			Y20_OFF
		//			break;
		//		case VALVE_21:
		//			Y21_OFF
		//			break;
		//		case VALVE_22:
		//			Y22_OFF
		//			break;
		//		case VALVE_23:
		//			Y23_OFF
		//			break;
		//		case VALVE_24:
		//			Y24_OFF
		//			break;
	case VIRTUAL_RELAY_1:
		VirtualRelay1 = false;
		break;
	case VIRTUAL_RELAY_2:
		VirtualRelay2 = false;
		break;
	case VIRTUAL_RELAY_3:
		VirtualRelay3 = false;
		break;
	case VIRTUAL_RELAY_4:
		VirtualRelay4 = false;
		break;
	case VIRTUAL_RELAY_5:
		VirtualRelay5 = false;
		break;
	case VIRTUAL_RELAY_6:
		VirtualRelay6 = false;
		break;
	case POOL_LIGHT:
		// CloseAbsorbLED();
		break;
		//		case PERISTALTIC_PUMP_PUSH:
		//			StopPump();
		//			break;
		//		case PERISTALTIC_PUMP_PULL:
		//			StopPump();
		//			break;
	default:
		break;
	}
}
void OpenAllValves(void)
{
	*(uint32_t *)usCoilBuf = 0xFFFFFFFF;
	Y1_ON
	Y2_ON
	Y3_ON
	Y4_ON
	Y5_ON
	Y6_ON
	Y7_ON
	Y8_ON
	Y9_ON
	Y10_ON
	Y11_ON
	Y12_ON
	Y13_ON
	Y14_ON
	Y15_ON
	Y16_ON
	Y17_ON
	Y18_ON
	Y19_ON
	//	Y20_ON
	//	Y21_ON
	//	Y22_ON
	//	Y23_ON
	//	Y24_ON
}
void CloseAllValves(void)
{
	*(uint32_t *)usCoilBuf = 0x00;
	Y1_OFF
	Y2_OFF
	Y3_OFF
	Y4_OFF
	Y5_OFF
	Y6_OFF
	Y7_OFF
	Y8_OFF
	Y9_OFF
	Y10_OFF
	Y11_OFF
	Y12_OFF
	Y13_OFF
	Y14_OFF
	Y15_OFF
	Y16_OFF
	Y17_OFF
	// Y18_OFF
	// Y19_OFF
	
	//	Y20_OFF
	//	Y21_OFF
	//	Y22_OFF
	//	Y23_OFF
	//	Y24_OFF
}

// void UpdataValveState(uint16_t valveReg)
//{
//	uint16_t valve = 1;
//	while((valve <= MAX_Y_VALVES) )
//	{
//		if(valveReg & 0x01)
//			OpenValveX((VALVE_NAME)valve);
//		else
//			CloseValveX((VALVE_NAME)valve);
//		valveReg >>= 1;
//		valve++;
//
//	}
// }

void OpenValves(uint32_t valveReg)
{
	uint16_t valve = 1;
	*(uint32_t *)usCoilBuf |= valveReg;
	while ((valve <= MAX_Y_VALVES))
	{
		if (valveReg & 0x01)
			OpenValveX((VALVE_NAME)valve);
		valveReg >>= 1;
		valve++;
	}
}

void CloseValves(uint32_t valveReg)
{
	uint16_t valve = 1;
	*(uint32_t *)usCoilBuf &= (~valveReg); // ¸üÐÂÏßÈ¦¼Ä´æÆ÷
	while ((valve <= MAX_Y_VALVES))
	{
		if (valveReg & 0x01)
			CloseValveX((VALVE_NAME)valve);
		valveReg >>= 1;
		valve++;
	}
}
