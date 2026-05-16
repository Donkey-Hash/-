DrawLiquidERROR DrawLiquid(DEVICE_CODE dev,FUN_VALVE_CODE valve,uint16_t speed,uint8_t quanty,bool fine)
{
	uint8_t ch;
	uint16_t timeth;
	bool beDetected = false;

	Bubbles = 0;	
	LiquidDetTimes = 0;
	SampleCount = 0;
	LLCheckState = 0;
//	MinRiseAD = 0;
//	count1 = count2 = count3 = count4 = 0;
	if(fine)					//如果是精确定量，判断次数为100
		timeth = 100;
	else
		timeth = 50;
	
	
	if(quanty == 1)
	{
		ch = LL1_CH;
		SetLL1LEDCurrent(LL1InitCurrent);
		SetLL2LEDCurrent(0);
		EmptyLLAD = LL1ADBlank;
	}	
	else 
	{
		ch = LL2_CH;
		SetLL1LEDCurrent(0);
		SetLL2LEDCurrent(LL2InitCurrent);
		EmptyLLAD = LL2ADBlank;
	}	
	OpenFunValve(dev,valve);													//打开阀门
	osDelay(3000);
	
//EmptyLLAD = osLTCReadADx(ch,1024);								//读取空管初始值



	ThrTotalRef = (EmptyLLAD*4/5);											//设置全反射点AD阈值
	ThrHalfEmpty = EmptyLLAD/2;
	StartPump(DRAW_LIQUID,speed,MAX_PULL_LIQUID_CIRCLE);								//DRAW_LIQUID //50
	do{
		osDelay(5);															//5ms判断一次    //如果AD值的变化超过阈值，接着判断，如果连续两次的值比较平稳则视为检测到液位。
		PracticalLLAD = osLTCReadADx(ch,10);							//读取当前值
//		DevADs[(SampleCount % 500)] = PracticalLLAD;
		SampleCount++;
#ifdef RAW_DATA_DEBUG
		LED_WARN_ON
		if(fine && DrawTestFlag)
			SendIntByString(LLCheckState,PracticalLLAD);
		LED_WARN_OFF
#endif		
		switch(LLCheckState)
		{
			case 0:
				if((PracticalLLAD <= ThrTotalRef) && (PracticalLLAD <= LastLLAD))       //如果小于阈值,且AD值在下降,认为探测到上液面
				{					
					LiquidDetTimes ++;
					if(LiquidDetTimes >= 1)       //只成功一次即可进入下一阶段
					{
						LLCheckState = 1;
						LiquidDetTimes = 0;				
					}
				}
				else
				{
//					count1++;
					LiquidDetTimes = 0;
				}
				LastLLAD = PracticalLLAD;
				break;
			case 1:
				if((PracticalLLAD >= ThrTotalRef) && (PracticalLLAD >= LastLLAD))    //探测到AD回升到空管值,
				{
					LiquidDetTimes ++;
					if(LiquidDetTimes >= 3)
					{
						LLCheckState = 2;
						LiquidDetTimes = 0;
//						MaxRiseAD = PracticalLLAD;
						ButtonCheckedCount = SampleCount;
						AvgRiseAD = PracticalLLAD;						
					}
				}
				else
					{
//						count2++;
						LiquidDetTimes = 0;
					}	
				LastLLAD = PracticalLLAD;
				break;
			case 2:	
				if(PracticalLLAD <= ThrTotalRef)
				{
					LLCheckState = 0;
					Bubbles ++;
					LiquidDetTimes = 0;
					ChangePumpSpeed(speed);
					break;       // 如果检测到全反射点，重新找上升沿
				}	

				if(PracticalLLAD - EmptyLLAD >= LEVEL_THRESHOLD)
	//			if(temp < 256)
					{
//						count3++;
						LiquidDetTimes ++;
						if(fine && (LiquidDetTimes >= 3))
							ChangePumpSpeed(speed/10);
						if(LiquidDetTimes >= timeth)
						{
							beDetected = true;	
							LLCheckState = 3;							
							ButtonCheckedCount = SampleCount - ButtonCheckedCount;
						}							
					}
					else
					{					
//						MaxRiseAD = PracticalLLAD;
						LiquidDetTimes = 0;
//						count4++;
					}
//				}				
				LastLLAD = PracticalLLAD;	
				break;
			default:
				break;
		}

	}while(!TimeoutFlag && !beDetected);
	if(beDetected)
		osDelay(500);	
	StopPump();	

	if(TimeoutFlag)
	{
		CloseFunValve(dev,valve);	
		osDelay(500);	
		SetLL1LEDCurrent(0);
		SetLL2LEDCurrent(0);
//		if(Bubbles > 0)
//			return FULL_BUBBLES;
		return DRAW_TIMEOUT;											//抽取液体超时,返回
	}
	
	if(fine == false)                           //不是精确定量,液面定位完成
	{
		CloseFunValve(dev,valve);	
		osDelay(500);	
		SetLL1LEDCurrent(0);
		SetLL2LEDCurrent(0);
		return DRAW_OK;														//不定量抽取完成
	}

	osDelay(1500);	
	
//	ref = preLLAD								//精细定量.先读取参考值,满液值
//	dref = (EmptyLLAD - ref)/2;	
//	multispeed = speed;
	if(speed<100)
		speed = 10;
	else 
		speed /= 10;
//	multispeed /= speed;
	StartPump(PUSH_LIQUID,speed,5);								//往回推,找液面底
//	ButtonCheckedCount *= (5*multispeed*10);
//	ButtonCheckedCount += 2000;
//StartPumpWithXms(PUSH_LIQUID,speed,ButtonCheckedCount);	
	do{
		osDelay(1);
		PracticalLLAD = osLTCReadADx(ch,128);
		#ifdef RAW_DATA_DEBUG
		if(fine && DrawTestFlag)
		SendIntByString(LLCheckState,PracticalLLAD);
		#endif
	}while(!TimeoutFlag && PracticalLLAD > ThrHalfEmpty);//dref);	
	StopPump();	
	CloseFunValve(dev,valve);	
	if(TimeoutFlag)
	{
		osDelay(100);																		//
		
		osDelay(10);	
		SetLL1LEDCurrent(0);
		SetLL2LEDCurrent(0);
		if(quanty == 3)
		return DETECT_LEVEL2_FALSE;											//定量失败	
		else
			return DETECT_LEVEL1_FALSE;											//定量失败	
	}

	osDelay(800);
//	OpenFunValve(dev,AIR_VALVE);														//平衡定量管压力
//	osDelay(1500);
//	CloseFunValve(dev,AIR_VALVE);	
//	osDelay(1500);	
	SetLL1LEDCurrent(0);
	SetLL2LEDCurrent(0);
	return DRAW_OK;
}