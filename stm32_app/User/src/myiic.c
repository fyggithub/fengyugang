#include <core_cm3.h>
#include "delay.h"
#include "myiic.h"
#include "debug_usart.h"
#include "process.h"

#define CR1_PE_Set          ((uint16_t)0x0001)
#define OWN_ADDRESS			0x90				//设备自身地址
#define SLAVE_ADDRESS		0x90				//从机地址
#define I2C_SPEED			400000				//I2C速度
//#define I2C_SPEED			100000				//I2C速度

static volatile unsigned char RxCount = 0;
static volatile unsigned char RxReg = 0;
unsigned char reg_val[I2C_REG_NUM] = {0}; 

static void i2c2_init(void);

static void i2c1_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD; //开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7); 	  //PB6,PB7 输出高
}

// 初始化IIC 
void i2c_init(void)
{					     
	i2c1_init();  
	i2c2_init();
}

void SDA_OUT(IIC_E_BUS bus)
{
    if (IIC_E_BUS0 == bus)
    {   
        {GPIOB->CRL &=0X0FFFFFFF; GPIOB->CRL|=(u32)7<<28;}
    }
}

void SDA_IN(IIC_E_BUS bus)
{
    if (IIC_E_BUS0 == bus)
    {
        {GPIOB->CRL &=0X0FFFFFFF; GPIOB->CRL|=(u32)4<<28;}
    }
}

void IIC_SCL(IIC_E_BUS bus, u8 val)
{
    if (IIC_E_BUS0 == bus)
    {
        PBout(6) = val;
    }
}

void IIC_SDA(IIC_E_BUS bus, u8 val)
{
    if (IIC_E_BUS0 == bus)
    {
        PBout(7) = val;
    }
}

u8 READ_SDA(IIC_E_BUS bus)
{
    if (IIC_E_BUS0 == bus)
    {
        return PBin(7);
    }

    return 0;
}

/* 产生IIC起始信号 */
void IIC_Start(IIC_E_BUS bus)
{
	SDA_OUT(bus);   //sda线输出
	IIC_SDA(bus, 0x1);
	IIC_SCL(bus, 0x1);
	delay_us(4);
 	IIC_SDA(bus, 0x0); //START:when CLK is high,DATA change form high to low
	delay_us(4);
	IIC_SCL(bus, 0x0); //钳住I2C总线，准备发送或接收数据
}

/* 产生IIC停止信号 */
void IIC_Stop(IIC_E_BUS bus)
{
	SDA_OUT(bus);   //sda线输出
	IIC_SCL(bus, 0x0);
	IIC_SDA(bus, 0x0); //STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL(bus, 0x1); 
	IIC_SDA(bus, 0x1);   //发送I2C总线结束信号
	delay_us(4);							   	
}

/*
 * 等待应答信号到来
 * 返回值：1，接收应答失败
 *         0，接收应答成功
 */
u8 IIC_WaitAck(IIC_E_BUS bus)
{
	u8 ucErrTime = 0;   
	
	SDA_IN(bus);
    /* IIC_SDA(bus, 0x1); */
    delay_us(1);
	IIC_SCL(bus, 0x1);
    delay_us(1);	 

	while(READ_SDA(bus))
	{
		ucErrTime++;
		if(ucErrTime > 250)
		{	            
            //DebugPrint("WacitAck iic stop, 1234 \n");
            //printf("iic stop\n");
			IIC_Stop(bus);
			return 1;
		}
	}

	IIC_SCL(bus, 0);
	return 0;
}

/* 产生ACK应答 */
void IIC_Ack(IIC_E_BUS bus)
{
	IIC_SCL(bus, 0x0);
	SDA_OUT(bus);
	IIC_SDA(bus, 0x0);
	delay_us(2);
	IIC_SCL(bus, 0x1);
	delay_us(2);
	IIC_SCL(bus, 0x0);
}

/* 不产生ACK应答 */
void IIC_NAck(IIC_E_BUS bus)
{
	IIC_SCL(bus, 0x0);
	SDA_OUT(bus);
	IIC_SDA(bus, 0x1);
	delay_us(2);
	IIC_SCL(bus, 0x1);
	delay_us(2);
	IIC_SCL(bus, 0x0);
}

/*
 * IIC发送一个字节
 * 返回从机有无应答
 * 1，有应答
 * 0，无应答			  
 */
void IIC_SendByte(IIC_E_BUS bus, u8 txd)
{
    u8 t;   

	SDA_OUT(bus);
    /* 拉低时钟开始数据传输 */
    IIC_SCL(bus, 0x0);
    for(t = 0; t < 8; t++)
    {              
		if((txd & 0x80) >> 7)
        {
			IIC_SDA(bus, 0x1);
        }
		else
        {
			IIC_SDA(bus, 0x0);
        }

		txd <<= 1;
		delay_us(2);
		IIC_SCL(bus, 0x1);
		delay_us(2);
		IIC_SCL(bus, 0x0);
		delay_us(2);
    }
}

/* 读1个字节，ack=1时，发送ACK，ack=0，发送nACK */
u8 IIC_ReadByte(IIC_E_BUS bus, unsigned char ack)
{
	unsigned char i       = 0;
	unsigned char receive = 0;

	SDA_IN(bus);

    for(i = 0; i < 8; i++)
	{
        IIC_SCL(bus, 0x0);
        delay_us(2);
		IIC_SCL(bus, 0x1);
        receive <<= 1;
        if(READ_SDA(bus))
        {
            receive++;
        }
		delay_us(1);
    }

    if (!ack)
    {
        IIC_NAck(bus);
    }
    else
    {
        IIC_Ack(bus);
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

/*
static void i2c2_nvic_configuration(void)
{
    // 1 bit for pre-emption priority, 3 bits for subpriority 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    //NVIC_SetPriority(I2C1_EV_IRQn, 0x00); 
    //NVIC_EnableIRQ(I2C1_EV_IRQn);
    //NVIC_SetPriority(I2C1_ER_IRQn, 0x01); 
    //NVIC_EnableIRQ(I2C1_ER_IRQn);
        
    NVIC_SetPriority(I2C2_EV_IRQn, 0x00);
    NVIC_EnableIRQ(I2C2_EV_IRQn);
    NVIC_SetPriority(I2C2_ER_IRQn, 0x01); 
    NVIC_EnableIRQ(I2C2_ER_IRQn);
}
*/

static void i2c2_configuration(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);		// 优先级分组

	// 中断 
	NVIC_InitStructure.NVIC_IRQChannel = I2C2_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = I2C2_ER_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		
	NVIC_Init(&NVIC_InitStructure);

	I2C_DeInit(I2C2);										///配置为普通IIC模式
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;				//I2C模式
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; 		//占空比(快速模式时)
	I2C_InitStructure.I2C_OwnAddress1 = OWN_ADDRESS;		//设备地址
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				//使能自动应答
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;			//速度
	I2C_Init(I2C2, &I2C_InitStructure);
#if 1
	//I2C_ITConfig(I2C2, I2C_IT_BUF | I2C_IT_EVT | I2C_IT_ERR, ENABLE);	//使能中断
	I2C_ITConfig(I2C2, I2C_IT_BUF, ENABLE);	//使能中断
	I2C_ITConfig(I2C2, I2C_IT_EVT, ENABLE);	//使能中断
	I2C_ITConfig(I2C2, I2C_IT_ERR, ENABLE);	//使能中断
	I2C_Cmd(I2C2, ENABLE); 												//使能I2C
#endif
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
	//i2c2_nvic_configuration()
	i2c2_gpio_configuration();	
	i2c2_configuration();
   // i2c2_nvic_configuration();
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

/* my code 9-15 */
#if 1
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
#endif

/* the office code 9-15 */
#if 0
/**
  * @brief  This function handles I2C1 Event interrupt request.
  * @param  None
  * @retval : None
  */
void I2C2_EV_IRQHandler(void)
{

    __IO uint32_t SR1Register =0;
    __IO uint32_t SR2Register =0;
    

#ifdef SLAVE_DMA_USE

    /* Read SR1 register */
    SR1Register = I2C2->SR1;

    /* If ADDR is set */
    if ((SR1Register & 0x0002) == 0x0002)
    {
        /* In slave Transmitter/Receiver mode, when using DMA, the update of the buffer base address
          and the buffer size should be done before clearing ADDR flag. In fact, the only
          period when the slave has control  on the bus(SCL is stretched so master can not initiate 
          transfers) is the period between ADDR is set and ADDR is cleared. otherwise, the master can
          initiate transfers and the buffer size & the buffer address have not yet updated.*/

        /* Update the DMA channels memory base address and count */
        I2C_DMAConfig (I2C2, Buffer_Tx2, 0xFFFF, I2C_DIRECTION_TX);
        I2C_DMAConfig (I2C2, Buffer_Rx2, 0xFFFF, I2C_DIRECTION_RX);
        /* Clear ADDR by reading SR2 register */
        SR2Register = I2C2->SR2;
    }
#else
    /* Read the I2C1 SR1 and SR2 status registers */
    SR1Register = I2C2->SR1;
    SR2Register = I2C2->SR2;

    /* If I2C2 is slave (MSL flag = 0) */
    if ((SR2Register &0x0001) != 0x0001)
    {
        /* If ADDR = 1: EV1 */
        if ((SR1Register & 0x0002) == 0x0002)
        {
            /* Clear SR1Register SR2Register variables to prepare for next IT*/
            SR1Register = 0;
            SR2Register = 0;
            /* Initialize the transmit/receive counters for next transmission/reception
            using Interrupt  */
            // Tx_Idx2 = 0;
            // Rx_Idx2 = 0;
			RxCount = 0;
			RxReg = 0;
        }
        /* If TXE = 1: EV3 */
        if ((SR1Register & 0x0080) == 0x0080)
        {
            /* Write data in data register */
            I2C2->DR = reg_val[RxReg];
            SR1Register = 0;
            SR2Register = 0;
        }
        /* If RXNE = 1: EV2 */
        if ((SR1Register & 0x0040) == 0x0040)
        {
            /* Read data from data register */
            // Buffer_Rx2[Rx_Idx2++] = I2C2->DR;
			if (0 == RxCount)
			{
                RxReg = I2C2->DR;
                RxCount++;
            }
            else 
            {
                reg_val[RxReg] = I2C2->DR;
            }
            SR1Register = 0;
            SR2Register = 0;

        }
        /* If STOPF =1: EV4 (Slave has detected a STOP condition on the bus */
        if (( SR1Register & 0x0010) == 0x0010)
        {
            I2C2->CR1 |= CR1_PE_Set;
            SR1Register = 0;
            SR2Register = 0;
        }
    } /* End slave mode */

#endif
#if 0
    /* If SB = 1, I2C1 master sent a START on the bus: EV5) */
    if ((SR1Register &0x0001) == 0x0001)
    {

        /* Send the slave address for transmssion or for reception (according to the configured value
            in the write master write routine */
        I2C2->DR = Address;
        SR1Register = 0;
        SR2Register = 0;
    }
    /* If I2C2 is Master (MSL flag = 1) */

    if ((SR2Register &0x0001) == 0x0001)
    {
        /* If ADDR = 1, EV6 */
        if ((SR1Register &0x0002) == 0x0002)
        {
            /* Write the first data in case the Master is Transmitter */
            if (I2CDirection == I2C_DIRECTION_TX)
            {
                /* Initialize the Transmit counter */
                Tx_Idx2 = 0;
                /* Write the first data in the data register */
                I2C2->DR = Buffer_Tx1[Tx_Idx2++];
                /* Decrement the number of bytes to be written */
                NumbOfBytes2--;
                /* If no further data to be sent, disable the I2C BUF IT
                in order to not have a TxE  interrupt */
                if (NumbOfBytes2 == 0)
                {
                    I2C2->CR2 &= (uint16_t)~I2C_IT_BUF;
                }

            }
            /* Master Receiver */
            else

            {
                /* Initialize Receive counter */
                Rx_Idx2 = 0;
                /* At this stage, ADDR is cleared because both SR1 and SR2 were read.*/
                /* EV6_1: used for single byte reception. The ACK disable and the STOP
                Programming should be done just after ADDR is cleared. */
                if (NumbOfBytes2 == 1)
                {
                    /* Clear ACK */
                    I2C2->CR1 &= CR1_ACK_Reset;
                    /* Program the STOP */
                    I2C2->CR1 |= CR1_STOP_Set;
                }
            }
            SR1Register = 0;
            SR2Register = 0;

        }
        /* Master transmits the remaing data: from data2 until the last one.  */
        /* If TXE is set */
        if ((SR1Register &0x0084) == 0x0080)
        {
            /* If there is still data to write */
            if (NumbOfBytes2!=0)
            {
                /* Write the data in DR register */
                I2C2->DR = Buffer_Tx2[Tx_Idx2++];
                /* Decrment the number of data to be written */
                NumbOfBytes2--;
                /* If  no data remains to write, disable the BUF IT in order
                to not have again a TxE interrupt. */
                if (NumbOfBytes2 == 0)
                {
                    /* Disable the BUF IT */
                    I2C2->CR2 &= (uint16_t)~I2C_IT_BUF;
                }
            }
            SR1Register = 0;
            SR2Register = 0;
        }
        /* If BTF and TXE are set (EV8_2), program the STOP */
        if ((SR1Register &0x0084) == 0x0084)
        {

            /* Program the STOP */
            I2C2->CR1 |= CR1_STOP_Set;
            /* Disable EVT IT In order to not have again a BTF IT */
            I2C2->CR2 &= (uint16_t)~I2C_IT_EVT;
            SR1Register = 0;
            SR2Register = 0;
        }
        /* If RXNE is set */
        if ((SR1Register &0x0040) == 0x0040)
        {
            /* Read the data register */
            Buffer_Rx2[Rx_Idx2++] = I2C2->DR;
            /* Decrement the number of bytes to be read */
            NumbOfBytes2--;

            /* If it remains only one byte to read, disable ACK and program the STOP (EV7_1) */
            if (NumbOfBytes2 == 1)
            {
                /* Clear ACK */
                I2C2->CR1 &= CR1_ACK_Reset;
                /* Program the STOP */
                I2C2->CR1 |= CR1_STOP_Set;
            }
            SR1Register = 0;
            SR2Register = 0;
        }
    }
    #endif
}

/**
  * @brief  This function handles I2C2 Error interrupt request.
  * @param  None
  * @retval : None
  */
void I2C2_ER_IRQHandler(void)
{

    __IO uint32_t SR1Register =0;

    /* Read the I2C1 status register */
    SR1Register = I2C2->SR1;
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
#endif

/* the office code two */
#if 0
/**
  * @brief  This function handles I2C2 Event interrupt request.
  * @param  None
  * @retval None
  */
void I2C2_EV_IRQHandler(void)
{
  switch (I2C_GetLastEvent(I2C2))
  {
    /* Slave Transmitter ---------------------------------------------------*/
    case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:  /* EV1 */
    
      /* Transmit I2C2 data */
      // I2C_SendData(I2C2, I2C2_Buffer_Tx[Tx2_Idx++]);
      I2C_SendData(I2C2, reg_val[RxReg]);
      break;

   case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:             /* EV3 */
      /* Transmit I2C2 data */
      // I2C_SendData(I2C2, I2C2_Buffer_Tx[Tx2_Idx++]);
      I2C_SendData(I2C2, reg_val[RxReg]);
      break; 
  

    /* Slave Receiver ------------------------------------------------------*/
    case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:     /* EV1 */
      break;

    case I2C_EVENT_SLAVE_BYTE_RECEIVED:                /* EV2 */
      /* Store I2C2 received data */
      I2C2_Buffer_Rx[Rx2_Idx++] = I2C_ReceiveData(I2C2);

      if(Rx2_Idx == Tx2BufferSize)
      { 
        I2C_TransmitPEC(I2C2, ENABLE);  
        Direction = Receiver;
      }
      break; 

    case I2C_EVENT_SLAVE_STOP_DETECTED:                /* EV4 */
      /* Clear I2C2 STOPF flag: read of I2C_SR1 followed by a write on I2C_CR1 */
      (void)(I2C_GetITStatus(I2C2, I2C_IT_STOPF));
      I2C_Cmd(I2C2, ENABLE);
      break;
   
    default:
      break;
  }
}

/**
  * @brief  This function handles I2C2 Error interrupt request.
  * @param  None
  * @retval None
  */
void I2C2_ER_IRQHandler(void)
{
  /* Check on I2C2 AF flag and clear it */
  if (I2C_GetITStatus(I2C2, I2C_IT_AF)) 
  {
    I2C_ClearITPendingBit(I2C2, I2C_IT_AF);
  }
}
#endif

/* refer to website */
#if 0
    uint32_t  I2CFlagStatus;
     static uint8_t num = 0;
	 
     I2CFlagStatus = I2C_GetLastEvent(I2C2);  // =>  (SR2<<16|SR1)
 
     if ((I2CFlagStatus & 0x02) != 0){ //bit1:addr matched
           if(I2CFlagStatus & 0x80) //bit7 Data register empty (transmitters)
            {//read            
               num = 0;  
               I2C_SendData(I2C2, I2C_Buffer_Tx[num]);		
           }else{ 
               num = 1;  
              I2C_Buffer_Tx[0] = 0;
              I2C_Buffer_Rx[0] = 0;
             }
     }else if((I2CFlagStatus & 0x80) != 0){ // bit7  TxE  -Data register empty (transmitters)
           if((I2CFlagStatus & 0x04)==0){ //bit2  BTF (Byte transfer finished)
                num++;
                I2C_SendData(I2C2, I2C_Buffer_Tx[num]); //printf("I2C status:0x%x\r\n", I2CFlagStatus);
           }
     }else if((I2CFlagStatus & 0x40)&&(I2CFlagStatus & 0x10)){  //bit6(RxNE) +  bit4(STOPF) 
           I2C_Buffer_Rx[num] = I2C_ReceiveData(I2C2);    //g_debug_count1++;
            num++;   
           I2C_Buffer_Rx[0] = num-1;	
           I2C2->CR1 |= 0x1000;//CR1_PEC_Set;
     }else if((I2CFlagStatus & 0x40) != 0){ //bit6  RxNE    -Data register not empty (receivers))
           I2C_Buffer_Rx[num] = I2C_ReceiveData(I2C2);   
           num++;     
     }else if((I2CFlagStatus & 0x10) != 0){ //bit4  STOPF -Stop detection (slave mode)
           I2C_Buffer_Rx[0] = num-1;	
           I2C2->CR1 |= 0x1000;//CR1_PEC_Set;
     }else{
         printf("I2C error status:0x%x\r\n", I2CFlagStatus);
     }
 
	I2C2->SR1=0;
	I2C2->SR2=0;
}
#endif
