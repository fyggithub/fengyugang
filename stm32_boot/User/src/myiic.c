#include "delay.h"
#include "myiic.h"

static void i2c2_init(void);

#define CR1_PE_Set          ((uint16_t)0x0001)
#define OWN_ADDRESS			0x90				//设备自身地址
#define SLAVE_ADDRESS		0x90				//从机地址
#define I2C_SPEED			400000				//I2C速度

static volatile unsigned char RxCount = 0;
static volatile unsigned char RxReg = 0;
unsigned char reg_val[I2C_REG_NUM] = {0}; 

/* 初始化IIC */
void i2c_init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	   
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP  ; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7); 	  //PB6,PB7 输出高

	i2c2_init();
}

/* 产生IIC起始信号 */
void IIC_Start(void)
{
	SDA_OUT();   //sda线输出
	IIC_SDA = 1;	  	  
	IIC_SCL = 1;
	delay_us(4);
 	IIC_SDA = 0; //START:when CLK is high,DATA change form high to low
	delay_us(4);
	IIC_SCL = 0; //钳住I2C总线，准备发送或接收数据
}

/* 产生IIC停止信号 */
void IIC_Stop(void)
{
	SDA_OUT();   //sda线输出
	IIC_SCL = 0;
	IIC_SDA = 0; //STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;   //发送I2C总线结束信号
	delay_us(4);							   	
}

/*
 * 等待应答信号到来
 * 返回值：1，接收应答失败
 *         0，接收应答成功
 */
u8 IIC_WaitAck(void)
{
	u8 ucErrTime = 0;
	SDA_IN();
	IIC_SDA = 1;
    delay_us(1);
	IIC_SCL = 1;
    delay_us(1);	 

	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}

	IIC_SCL = 0;
	return 0;
}

/* 产生ACK应答 */
void IIC_Ack(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 0;
	delay_us(2);
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
}

/* 不产生ACK应答 */
void IIC_NAck(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 1;
	delay_us(2);
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
}

/*
 * IIC发送一个字节
 * 返回从机有无应答
 * 1，有应答
 * 0，无应答			  
 */
void IIC_SendByte(u8 txd)
{                        
    u8 t;   

	SDA_OUT(); 	    
    /* 拉低时钟开始数据传输 */
    IIC_SCL = 0;
    for(t = 0; t < 8; t++)
    {              
		if((txd & 0x80) >> 7)
        {
			IIC_SDA = 1;
        }
		else
        {
			IIC_SDA = 0;
        }

		txd <<= 1;
		delay_us(2);
		IIC_SCL = 1;
		delay_us(2);
		IIC_SCL = 0;
		delay_us(2);
    }
}

/* 读1个字节，ack=1时，发送ACK，ack=0，发送nACK */
u8 IIC_ReadByte(unsigned char ack)
{
	unsigned char i       = 0;
	unsigned char receive = 0;

	SDA_IN();

    for(i = 0; i < 8; i++)
	{
        IIC_SCL = 0;
        delay_us(2);
		IIC_SCL = 1;
        receive <<= 1;
        if(READ_SDA)
        {
            receive++;
        }
		delay_us(1);
    }

    if (!ack)
    {
        IIC_NAck();
    }
    else
    {
        IIC_Ack();
    }

    return receive;
}

static void i2c2_gpio_configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;		//复用开漏输出
	//GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;	//推挽输出  
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

    /* Enable I2C2 reset state */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
    /* Release I2C2 from reset state */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);	
}

static void i2c2_configuration(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);		// 优先级分组

	// 中断 
	NVIC_InitStructure.NVIC_IRQChannel = I2C2_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = I2C2_ER_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		
	NVIC_Init(&NVIC_InitStructure);

	//I2C_DeInit(I2C2);										///配置为普通IIC模式
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;				//I2C模式
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; 		//占空比(快速模式时)
	I2C_InitStructure.I2C_OwnAddress1 = OWN_ADDRESS;		//设备地址
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				//使能自动应答
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;			//速度
	I2C_Init(I2C2, &I2C_InitStructure);

	//I2C_ITConfig(I2C2, I2C_IT_BUF | I2C_IT_EVT, ENABLE | I2C_IT_ERR);	//使能中断
	I2C_ITConfig(I2C2, I2C_IT_BUF, ENABLE);	//使能中断
	I2C_ITConfig(I2C2, I2C_IT_EVT, ENABLE);	//使能中断
	I2C_ITConfig(I2C2, I2C_IT_ERR, ENABLE);	//使能中断
	I2C_Cmd(I2C2, ENABLE); 												//使能I2C
	//I2C_AcknowledgeConfig(I2C2, ENABLE);   							//允许应答模式
	/*
    // Enable Event IT needed for ADDR and STOPF events ITs
    I2C2->CR2 |= I2C_IT_EVT ;
    // Enable Error IT 
    I2C2->CR2 |= I2C_IT_ERR;	
    // Enable Buffer IT (TXE and RXNE ITs) 
    I2C2->CR2 |= I2C_IT_BUF;    	//interrupt
    //I2C2->CR2 |= CR2_DMAEN_Set;	//dma
    */
}

static void i2c2_init(void)
{
	//i2c2_nvic_configuration();
	i2c2_gpio_configuration();	
	i2c2_configuration();
	//i2c2_dma_config();
}

/*
void I2C_clear_ADDR(I2C_TypeDef* I2Cx) 
{
	I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR);
	((void)(I2Cx->SR2));
	//I2C_Cmd(I2Cx, ENABLE);
}

void I2C_clear_STOPF(I2C_TypeDef* I2Cx) 
{
	I2C_GetFlagStatus(I2Cx, I2C_FLAG_STOPF);
	I2C_Cmd(I2Cx, ENABLE);
}
*/

void I2C2_EV_IRQHandler(void)
{
	__IO uint32_t SR1Register =0;
	__IO uint32_t SR2Register =0;
	//uint32_t temp = 0;

	SR1Register = I2C2->SR1; //读取状态
	SR2Register = I2C2->SR2;
	/* I2C2是从机(MSL = 0) */
	if((SR2Register &0x0001) != 0x0001)
	{	 	
		/* 检测到地址(ADDR = 1: EV1) 从发送 */    
		if((SR1Register & 0x0082) == 0x0082)
		{			
			/* 清除标志，准备接收数据 */
			SR1Register = 0;
			SR2Register = 0;					
		}
		
		/* 检测到地址(ADDR = 1: EV1) 从接收 */
		if((SR1Register & 0x0002) == 0x0002)
		{			
			/* 清除标志，准备接收数据 */
			SR1Register = 0;
			SR2Register = 0;			
			RxCount = 0;
			RxReg = 0;			
		}

		/* 发送data EV3-1 EV3 */
		if ((SR1Register & 0x0080) == 0x0080)
		{
			I2C2->DR = reg_val[RxReg];
			//printf("qq %x \n", RxReg);
			SR1Register = 0;
			SR2Register = 0;				
		}		

		/* 接收数据(RXNE = 1: EV2) */
		if((SR1Register & 0x0040) == 0x0040)
		{	
			if (0 == RxCount) 
			{
				RxReg = I2C2->DR;				
				RxCount++;
			}				 
			else
			{
				reg_val[RxReg] = I2C2->DR;
			}			

			/*
			if (0 == RxCount) 
			{
				RxReg = temp;				
				RxCount++;
				if (RxReg != 0x10)
				{
					printf("err \n");
				}
			}				 
			else
			{
				reg_val[RxReg] = temp;
				RxCount++;
				if (reg_val[RxReg] == 0xff)
				{
					printf("error \n");
				}
			}			
			*/
			SR1Register = 0;
			SR2Register = 0;
		}

		/* 检测到停止条件(STOPF =1: EV4) */
		if(( SR1Register & 0x0010) == 0x0010)
		{
			I2C2->CR1 |= CR1_PE_Set;
			SR1Register = 0;
			SR2Register = 0;
		}	
 	}		
}       

void I2C2_ER_IRQHandler(void)
{
	#if 0
	//printf("err\n");
    if ((I2C_ReadRegister(I2C2, I2C_Register_SR1) & 0xFF00) != 0x00)   //读状态寄存器，获取I2C错误  
    {  
        I2C2->SR1 &= 0x00FF;                     //清除错误标志  
    }  	
    #endif   

	__IO uint32_t SR1Register =0;

	/* Read the I2C1 status register */
	SR1Register = I2C2->SR1;

	 //printf("e %x\n", SR1Register);	
	/* If AF = 1 */
	if ((SR1Register & 0x0400) == 0x0400)
	{
	   I2C2->SR1 &= 0xFBFF;
	   SR1Register = 0;
	}
	/* If ARLO = 1 */
	if ((SR1Register & 0x0200) == 0x0200)
	{
	   I2C2->SR1 &= 0xFBFF;
	   SR1Register = 0;
	}
	/* If BERR = 1 */
	if ((SR1Register & 0x0100) == 0x0100)
	{
	   I2C2->SR1 &= 0xFEFF;
	   SR1Register = 0;
	}

	/* If OVR = 1 */
	if ((SR1Register & 0x0800) == 0x0800)
	{
	   I2C2->SR1 &= 0xF7FF;
	   SR1Register = 0;
	}
}

