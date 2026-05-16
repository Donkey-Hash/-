/******************************************************************************
*文件名：ADC1119.c
*描述：16位外部AD
*实验平台：
*库版本：
*
*作者：
*******************************************************************************/
//#include "includes.h"

#include "ADC1119.h"

const char Channel[4] = {	ADS1119_CH0,	ADS1119_CH1,	ADS1119_CH2,	ADS1119_CH3};

static ADS1119_InitTypeDefine ADS1119_InitType;
void I2C_Virtual_Init(void);
void ADS1119_START(void);
uint8_t I2C_Virtual_ack;	//应答标志位
int16_t ADS1119_RawData[4] = {0};
uint8_t ADS1119_ConfigData[2] = {0};
uint16_t ADS1119_Buff[6]={0};
uint8_t ADS1119_err=0;



/******************************************************************************
** 函数名: delay_us();
** 功能: 软件延时1uS
** 输入: dat--延时mS数
** 输出:无
*******************************************************************************/
void delay_us(uint8_t dat)
{
    uint8_t i = 0,n = 0;
    for(n=0;n<dat;n++)
    {
        i = 4;
        while(i--);
    }
}
/*******************************************************************************
* 函数名: delay_1mS
* 描述:软件延时1mS
* 输入: dat--延时mS数
* 输出: 无
*******************************************************************************/
void delay_ms(uint16_t dat)
{
    uint16_t i = 0,n = 0;
    for(n=0;n<dat;n++)
    {
        i = 6400;
        while(i--);
    }
}
/*******************************************************************************
* 函数名: delay_1mS
* 描述:软件延时1mS
* 输入: dat--延时mS数
* 输出: 无
*******************************************************************************/
void I2C_Virtual_Init(void)
{
	SET_SDA_OUT();
	SDA_H() ;	
	SCL_H(); 
	RESET_ADS_L;
	delay_us(1);
	RESET_ADS_H;
          
}

/******************************************************************************
** 函数名: I2C_Virtual_Start();
** 功能: IIC开始
** 输入: 无
** 输出:无
*******************************************************************************/
void I2C_Virtual_Start(void)
{
	SET_SDA_OUT();
	SDA_H()       //发送起始条件的数据信号
	SCL_H()
	delay_us(4); //起始条件建立时间大于4.7us,延时   
	SDA_L()       //发送起始信号
	delay_us(4); //起始条件锁定时间大于4μ   
	SCL_L()
	delay_us(2);  //钳住I2C总线，准备发送或接收数据
}
/******************************************************************************
** 函数名: I2C_Virtual_Stop();
** 功能: IIC开始
** 输入: 无
** 输出:无
*******************************************************************************/
void I2C_Virtual_Stop(void)
{
	SET_SDA_OUT();
  SCL_L()       
  SDA_L()        //发送结束条件的数据信号
  delay_us(4);   
  SCL_H()        //发送结束条件的时钟信号
  delay_us(4);  //结束条件建立时间大于4μ  
  SDA_H() 
  delay_us(4);  //发送I2C总线结束信号
}
/******************************************************************************
** 函数名: I2C_Virtual_Ack();
** 功能: IIC开始
** 输入: 无
** 输出:无
*******************************************************************************/
void I2C_Virtual_Ack()
{  
  SCL_L()
  SET_SDA_OUT();
  SDA_L()
  delay_us(2);    
  SCL_H() 
  delay_us(2);                  
  SCL_L()                  
  delay_us(2);   
}
/**
 * @brief 主机不应答函数
 */
void I2C_Virtual_NoAck(void)
{
  SCL_L()
  SET_SDA_OUT();
  SDA_H()
  delay_us(2);
  SCL_H()
  delay_us(2);	//时钟低电平周期大于4μ
  SCL_L() 	    //清时钟线，钳住I2C总线以便继续接收
  delay_us(2);
}

/******************************************************************************
** 函数名: I2C_Virtual_WaitAck();
** 功能: IIC等待心跳
** 输入: 无
** 输出:结果
*******************************************************************************/
uint8_t I2C_Virtual_WaitAck(void)
{
	uint16_t t = 1000;
	SET_SDA_IN();
	SDA_H()//8位发送完后释放数据线，准备接收应答位     
	delay_us(2);    
	SCL_H()   
	delay_us(1); 
	while(SDA_FLAG)//等待SHT30应答
	{
		t--;
		delay_us(1);  
		if(t==0)
		{
			I2C_Virtual_Stop();
			return 1;
		}
	} 
	SCL_L()           
	delay_us(1);     
	return 0;
}
/**
 * @brief 主机字节数据发送函数
 * 		将数据c发送出去,可以是地址,也可以是数据,发完后等待应答,
 * 		并对此状态位进行操作.(不应答或非应答都使ack=0 假)
 * 		发送数据正常，ack=1; ack=0表示被控器无应答或损坏。
 * @retval 应答位
 * 		@arg 0:有应答
 * 		@arg 1:无应答
 */
uint8_t I2C_Virtual_SendByte(uint8_t c)
{
	SET_SDA_OUT();
	SDA_L()
	delay_us(2);//延时 2us   
	for (uint8_t BitCnt = 0; BitCnt < 8; BitCnt++)	//要传送的数据长度为8位
	{
		if ((c) & 0x80)	//判断发送位
			SDA_H()
		else
			SDA_L() 
		delay_us(3);	//保证时钟高电平周期大于4μ
		SCL_H()                  	//置时钟线为高，通知被控器开始接收数据位
		delay_us(3);	//保证时钟高电平周期大于4μ
		SCL_L()
		delay_us(3);  
		c<<=1;
	}
	return 1;
}
/**
 * @brief 主机字节数据接收函数
 * 		用来接收从器件传来的数据,并判断总线错误(不发应答信号)，
 * 		发完后请用应答函数。
 */
uint8_t I2C_Virtual_RcvByte(void)
{   
	uint8_t i,val=0;
	SET_SDA_IN();
	delay_us(3);
	for(i=0x80;i>0;i>>=1)//高位为1，循环右移
	{
		SCL_L()
		delay_us(2);
		SCL_H()	
		delay_us(1);
		if(SDA_FLAG) 
			val = (val|i);//读一位数据线的值
	}
	return val;
}
 
 void StartADS1119(void)
{
	I2C_Virtual_Start();											//写寄存器之后需要重新启动总线
	I2C_Virtual_SendByte(ADS1119_ADDRESS_W);						//发送器件地址（读）
	ADS1119_err=I2C_Virtual_WaitAck();
	if(ADS1119_err)
		return;	  
	I2C_Virtual_SendByte(0x08);		//发送数据
	ADS1119_err=I2C_Virtual_WaitAck();
	if(ADS1119_err==0)
	{
			I2C_Virtual_Stop();
	}

}
/**
 * @brief Configuration of ADS1115, continuous conversion
 */


uint8_t ADS1119ChConf(uint8_t ch)
{
	uint8_t temp = Channel[ch];
	temp |= CONST_CONFIG_DR20_SHOT;     //单次转换
	I2C_Virtual_Start();					 //启动总线
	I2C_Virtual_SendByte(ADS1119_ADDRESS_W);		//发送器件地址（写）
	ADS1119_err = I2C_Virtual_WaitAck();
	if(ADS1119_err)
		return 1;
	I2C_Virtual_SendByte(0x40);		//发送数据 写寄存器指令
	ADS1119_err=I2C_Virtual_WaitAck();
	if(ADS1119_err)
		return 2;
	I2C_Virtual_SendByte(temp);		//发送数据
	ADS1119_err=I2C_Virtual_WaitAck();
	if(ADS1119_err)
		return 3;
	I2C_Virtual_Stop();
	return 0;
}

/**
 * @brief 读取ADS1119当前通道下的原始数据
 * @param rawData: 传入一个int16_t整型变量的指针，ADS1115的原始数据将保存在这个变量中
 * @return 读取结果
 * 		@arg 0: fail
 * 		@arg 1: success
 */
uint8_t ADS1119_ReadRawData(int16_t *rawData)
{
				
	I2C_Virtual_Start();											//写寄存器之后需要重新启动总线
	I2C_Virtual_SendByte(ADS1119_ADDRESS_W);						//发送器件地址（读）
	ADS1119_err=I2C_Virtual_WaitAck();
  if(ADS1119_err)
		return 1;
	I2C_Virtual_SendByte(0x10);
	ADS1119_err=I2C_Virtual_WaitAck();
	if(ADS1119_err)
		return 2;
	I2C_Virtual_Start();
	I2C_Virtual_SendByte(ADS1119_ADDRESS_R);
	ADS1119_err=I2C_Virtual_WaitAck();
	if(ADS1119_err)
		return 3;
	ADS1119_Buff[0]=I2C_Virtual_RcvByte();
	I2C_Virtual_Ack();
	ADS1119_Buff[1]=I2C_Virtual_RcvByte();
	I2C_Virtual_NoAck();
	*rawData = (int16_t) (((ADS1119_Buff[0] << 8) & 0xFF00) | (ADS1119_Buff[1] & 0xFF));
	I2C_Virtual_Stop(); 
	return 0;
}


/**
 * @brief 将传感器的原始采样数据转化为电压数据，
 * 			根据ADS1115_InitType结构体中包含的增益信息计算
 * @param rawData: 待转换的原始数据
 * @retval 返回经过计算的电压值
 */
float ADS1119_RawDataToVoltage(int16_t rawData)
{
	float voltage;
	switch (ADS1119_InitType.GAIN)
	{
		case ADS1119_Gain_Configuration_4:
				voltage = rawData * 0.015625;
				break;
		case ADS1119_Gain_Configuration_1:
				voltage = rawData * 0.0625;
				break;
		default:
				voltage = 0;
				break;
	}
 
	return voltage;
}
/**
 * @brief 直接获取ADS1119当前通道的电压采样值
 * @return 电压采样值
 */
float ADS1119_GetVoltage()
{
		int16_t rawData;
		float voltage;
		ADS1119_ReadRawData(&rawData);

		voltage=ADS1119_RawDataToVoltage(rawData);
		return voltage;
 
	
}

/**
 * @brief 获取并计算ADC采样的平均电压值
 * @param num: 计算平均值的数量
 * @retval 电压采样的平均值
 */
float ADS1119_GetAverageVoltage(uint16_t num)
{
	int32_t sum = 0;
	int16_t rawData;
 
	if(num == 0)
	{
		return ADS1119_GetVoltage( );
	}
 
	for(uint16_t i =0; i< num;i++)
	{
		ADS1119_ReadRawData(&rawData);
		sum += rawData;
	}
 
	return ADS1119_RawDataToVoltage(sum/num);
}


uint8_t ADS1119_ReadConfigure(uint8_t r,uint8_t* ret)
{
	I2C_Virtual_Start();											//写寄存器之后需要重新启动总线
	I2C_Virtual_SendByte(ADS1119_ADDRESS_W);						//发送器件地址（读）

	ADS1119_err=I2C_Virtual_WaitAck();
  if(ADS1119_err)
		return 1;
	if(r == 0)
		I2C_Virtual_SendByte(0x20);		//读配置寄存器
	else
		I2C_Virtual_SendByte(0x24);		//读状态寄存器
	ADS1119_err=I2C_Virtual_WaitAck();
	if(ADS1119_err)
		return 2;
	I2C_Virtual_Start();
	I2C_Virtual_SendByte(ADS1119_ADDRESS_R);
	ADS1119_err=I2C_Virtual_WaitAck();
	if(ADS1119_err)
		return 3;
	*ret = I2C_Virtual_RcvByte();
	I2C_Virtual_NoAck();
	I2C_Virtual_Stop(); 
	return 0;	
}



bool ADS1119ReadCh(uint8_t ch,int16_t *res)
{
	uint8_t rt;
	rt = ADS1119ChConf(ch);
	if(rt)
		return false;
	StartADS1119();
//	rt = ADS1119_ReadConfigure(0,ADS1119_ConfigData);
//	if(rt)
//		return false;
//	rt = ADS1119_ReadConfigure(1,ADS1119_ConfigData + 1);
//	if(rt)
//		return false;
	while(ADS_CONV_DONE_FLAG)
			delay_us(5);
//	rt = ADS1119_ReadConfigure(1,ADS1119_ConfigData + 1);
//	if(rt)
//		return false;
	rt = ADS1119_ReadRawData(res);
	if(rt)
		return false;
	return true;
}

