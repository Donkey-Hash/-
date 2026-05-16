
#include "MCP4725.h" 
#include "i2c.h"
#include "modbusdata.h"
#include "FlashUserData.h"
//#include "delay.h"


extern I2C_HandleTypeDef hi2c1;


void InitI2c1(void)
{					     
	GPIO_InitTypeDef GPIO_InitStruct;
//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//使能GPIOB时钟
	   
	GPIO_InitStruct.Pin = DA2_SDA_Pin|DA2_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB,DA2_SDA_Pin|DA2_SCL_Pin,GPIO_PIN_SET);
	
}


void I2C_delay(void)
{
	uint16_t i=500;	//这里可以优化速度，经测试最低到5还能写入
	while(i--);
}

//void SDA_IN (void)
//{
//  GPIO_InitTypeDef GPIO_InitStruct;	
//	GPIO_InitStruct.Pin = DA2_SDA_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//	
//}


//void SDA_OUT(void)
//{ 
//	GPIO_InitTypeDef GPIO_InitStruct;	
//	GPIO_InitStruct.Pin = DA2_SDA_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//}
//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	I2C_delay();
	IIC_SDA_H;	  	  
	IIC_SCL_H;
	I2C_delay();
 	IIC_SDA_L;     //START:when CLK is high,DATA change form high to low 
	I2C_delay();
	IIC_SCL_L;     //钳住I2C总线，准备发送或接收数据 
	I2C_delay();
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL_L;
	IIC_SDA_L;//STOP:when CLK is high DATA change form low to high
 	I2C_delay();
	IIC_SCL_H; 
	I2C_delay();
	IIC_SDA_H;//发送I2C总线结束信号
	I2C_delay();							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t IIC_Wait_Ack(void)
{
	uint16_t ucErrTime=0;
	
	SDA_IN();  //SDA设置为输入		
	IIC_SDA_H;
	I2C_delay();		 	  
	IIC_SCL_H;
//	IIC_SCL_L;	
	I2C_delay();

//	SDA_OUT();
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>500)
		{
			IIC_Stop();
//			IIC_SCL_L;
			return 1;
		}
	}
	IIC_SCL_L;//时钟输出0 	   
	 
	return 0; 
	
} 
//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL_L;
	SDA_OUT();
	IIC_SDA_L;
	I2C_delay();
	IIC_SCL_H;
	I2C_delay();
	IIC_SCL_L;
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL_L;
	SDA_OUT();
	IIC_SDA_H;
	I2C_delay();
	IIC_SCL_H;
	I2C_delay();
	IIC_SCL_L;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(uint8_t txd)
{                        
	uint8_t t;   
	SDA_OUT(); 
	I2C_delay();	
	IIC_SCL_L;//拉低时钟开始数据传输
	I2C_delay();
	for(t=0;t<8;t++)
	{
		if((txd&0x80)>>7)
		{IIC_SDA_H;}
		else
		{IIC_SDA_L;}
		txd<<=1; 	  
		I2C_delay();   //对TEA5767这三个延时都是必须的
		IIC_SCL_H;
		I2C_delay(); 
		IIC_SCL_L;	
		I2C_delay();
	}	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;

	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL_L; 
        I2C_delay();
		IIC_SCL_H;
        receive<<=1;
        if(READ_SDA)receive++;   
		I2C_delay(); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}





void MCP4725_WriteData_Digital(uint16_t data)   //12位数字量
{
//  uint8_t data_H=0,data_L=0;
	uint8_t dat[3];
//	dat[0] = 0xC0;
	dat[0] = ( 0x0F00 & data) >> 8;
	dat[1] = 0x00FF & data ;
//////	HAL_I2C_Master_Transmit(&hi2c1,0xC0,dat,2,500);
	IIC_Start();
//	IIC_Wait_Ack();//	I2C_delay();//  
	IIC_Send_Byte(0XC4);      //器件寻址，器件代吗：1100； 
	IIC_Wait_Ack();	// I2C_delay();//  
//  IIC_Send_Byte(0x40); 	
//  IIC_Wait_Ack();	// I2C_delay();//  
  IIC_Send_Byte(dat[0]); 	
  IIC_Wait_Ack();	// I2C_delay();//  
  IIC_Send_Byte(dat[1]);
	IIC_Wait_Ack();	//I2C_delay();//
  IIC_Stop();//产生一个停止条件 
	I2C_delay(); 	
	
}

uint8_t IICDataIn[6];
void MCP4725ReadData(void)   //12位数字量
{
	SDA_OUT(); 
	IIC_Start();
	IIC_Send_Byte(0XC1);      //器件寻址，器件代吗：1100； 地址位A2，A1，A0为 0 ， 0 ， 1；-> 1100 0010
    IIC_Wait_Ack();	
		IICDataIn[0] = IIC_Read_Byte(1);
	IICDataIn[1] = IIC_Read_Byte(1);
	IICDataIn[2] = IIC_Read_Byte(1);
	IICDataIn[3] = IIC_Read_Byte(1);
	IICDataIn[4] = IIC_Read_Byte(0);
    IIC_Stop();//产生一个停止条件  	
//	delay_ms(10);	
}

void MCP4725_WriteData_Voltage(uint16_t Vout)   //电压单位mV
{
  uint16_t 	Dh;
	uint32_t 	Dn;
	Dn = ( 4095 * Vout) / 3300;
	Dh = 0x0FFF & Dn ;
	MCP4725_WriteData_Digital(Dh);
}

void ResultIOut(bool betest)   //电压单位mV
{
	uint32_t temp;
	uint16_t rangset;
	uint16_t 	Dh;
//	MCP4725_WriteData_Voltage(pMbHoldData->nRange1Set);
	if(pMbHoldData->nWorkRange > 3)
		MCP4725_WriteData_Voltage(0);
	else
	{
		
		if(betest)
		{
			rangset = pMbHoldData->fCurrentTestRange;
			temp = 4095 * pMbHoldData->fCurrentTestSet;
		}
		else
		{
			rangset = (&pMbHoldData->nRange1Set)[pMbHoldData->nWorkRange];
			temp = 4095 * pMbHoldData->fMeasureResult;
		}
		temp /= rangset;
		Dh = 0x0FFF & temp;
		MCP4725_WriteData_Digital(Dh);
//		temp += (3300/5);
//		MCP4725_WriteData_Voltage(temp);
	}
}


 









