#include "delay.h"
#include "myiic.h"

static void i2c2_init(void);

#define CR1_PE_Set          ((uint16_t)0x0001)
#define OWN_ADDRESS			0x90				//�豸�����ַ
#define SLAVE_ADDRESS		0x90				//�ӻ���ַ
#define I2C_SPEED			400000				//I2C�ٶ�

static volatile unsigned char RxCount = 0;
static volatile unsigned char RxReg = 0;
unsigned char reg_val[I2C_REG_NUM] = {0}; 

/* ��ʼ��IIC */
void i2c_init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	   
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP  ; //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7); 	  //PB6,PB7 �����

	i2c2_init();
}

/* ����IIC��ʼ�ź� */
void IIC_Start(void)
{
	SDA_OUT();   //sda�����
	IIC_SDA = 1;	  	  
	IIC_SCL = 1;
	delay_us(4);
 	IIC_SDA = 0; //START:when CLK is high,DATA change form high to low
	delay_us(4);
	IIC_SCL = 0; //ǯסI2C���ߣ�׼�����ͻ��������
}

/* ����IICֹͣ�ź� */
void IIC_Stop(void)
{
	SDA_OUT();   //sda�����
	IIC_SCL = 0;
	IIC_SDA = 0; //STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;   //����I2C���߽����ź�
	delay_us(4);							   	
}

/*
 * �ȴ�Ӧ���źŵ���
 * ����ֵ��1������Ӧ��ʧ��
 *         0������Ӧ��ɹ�
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

/* ����ACKӦ�� */
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

/* ������ACKӦ�� */
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
 * IIC����һ���ֽ�
 * ���شӻ�����Ӧ��
 * 1����Ӧ��
 * 0����Ӧ��			  
 */
void IIC_SendByte(u8 txd)
{                        
    u8 t;   

	SDA_OUT(); 	    
    /* ����ʱ�ӿ�ʼ���ݴ��� */
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

/* ��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK */
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;		//���ÿ�©���
	//GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;	//�������  
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
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);		// ���ȼ�����

	// �ж� 
	NVIC_InitStructure.NVIC_IRQChannel = I2C2_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = I2C2_ER_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		
	NVIC_Init(&NVIC_InitStructure);

	//I2C_DeInit(I2C2);										///����Ϊ��ͨIICģʽ
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;				//I2Cģʽ
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; 		//ռ�ձ�(����ģʽʱ)
	I2C_InitStructure.I2C_OwnAddress1 = OWN_ADDRESS;		//�豸��ַ
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				//ʹ���Զ�Ӧ��
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;			//�ٶ�
	I2C_Init(I2C2, &I2C_InitStructure);

	//I2C_ITConfig(I2C2, I2C_IT_BUF | I2C_IT_EVT, ENABLE | I2C_IT_ERR);	//ʹ���ж�
	I2C_ITConfig(I2C2, I2C_IT_BUF, ENABLE);	//ʹ���ж�
	I2C_ITConfig(I2C2, I2C_IT_EVT, ENABLE);	//ʹ���ж�
	I2C_ITConfig(I2C2, I2C_IT_ERR, ENABLE);	//ʹ���ж�
	I2C_Cmd(I2C2, ENABLE); 												//ʹ��I2C
	//I2C_AcknowledgeConfig(I2C2, ENABLE);   							//����Ӧ��ģʽ
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

	SR1Register = I2C2->SR1; //��ȡ״̬
	SR2Register = I2C2->SR2;
	/* I2C2�Ǵӻ�(MSL = 0) */
	if((SR2Register &0x0001) != 0x0001)
	{	 	
		/* ��⵽��ַ(ADDR = 1: EV1) �ӷ��� */    
		if((SR1Register & 0x0082) == 0x0082)
		{			
			/* �����־��׼���������� */
			SR1Register = 0;
			SR2Register = 0;					
		}
		
		/* ��⵽��ַ(ADDR = 1: EV1) �ӽ��� */
		if((SR1Register & 0x0002) == 0x0002)
		{			
			/* �����־��׼���������� */
			SR1Register = 0;
			SR2Register = 0;			
			RxCount = 0;
			RxReg = 0;			
		}

		/* ����data EV3-1 EV3 */
		if ((SR1Register & 0x0080) == 0x0080)
		{
			I2C2->DR = reg_val[RxReg];
			//printf("qq %x \n", RxReg);
			SR1Register = 0;
			SR2Register = 0;				
		}		

		/* ��������(RXNE = 1: EV2) */
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

		/* ��⵽ֹͣ����(STOPF =1: EV4) */
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
    if ((I2C_ReadRegister(I2C2, I2C_Register_SR1) & 0xFF00) != 0x00)   //��״̬�Ĵ�������ȡI2C����  
    {  
        I2C2->SR1 &= 0x00FF;                     //��������־  
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

