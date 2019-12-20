#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"

/* IO�������� */
/*
 * #define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF; GPIOB->CRL|=(u32)8<<28;}
 * #define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF; GPIOB->CRL|=(u32)3<<28;}
 */

/* IO�������� */
/*
 * #define IIC_SCL    PBout(6) //SCL
 * #define IIC_SDA    PBout(7) //SDA	 
 * #define READ_SDA   PBin(7)  //����SDA 
 */

typedef enum IIC_E_BUS
{
    IIC_E_BUS0 = 0x0,
    IIC_E_BUS1,
    IIC_E_BUS2,
    IIC_E_BUS_BUTT,

}IIC_E_BUS;

/* IIC���в������� */
extern void Config_I2cInit(void);
extern void IIC_Start(IIC_E_BUS bus);
extern void IIC_Stop(IIC_E_BUS bus);
extern void IIC_SendByte(IIC_E_BUS bus, u8 txd);
extern void IIC_Ack(IIC_E_BUS bus);
extern void IIC_NAck(IIC_E_BUS bus);
extern u8   IIC_ReadByte(IIC_E_BUS bus, unsigned char ack);
extern u8   IIC_WaitAck(IIC_E_BUS bus);
extern void IIC_SCL(IIC_E_BUS bus, u8 val);
extern void IIC_SDA(IIC_E_BUS bus, u8 val);
extern void SDA_IN(IIC_E_BUS bus);
extern void SDA_OUT(IIC_E_BUS bus);
extern u8 READ_SDA(IIC_E_BUS bus);
extern u8 IIC_ReadStr(u8 val);


#define BIT0		0x01
#define BIT1		0x02
#define BIT2		0x04
#define BIT3		0x08
#define BIT4		0x10
#define BIT5		0x20
#define BIT6		0x40
#define BIT7		0x80

#if 1	//DMA
/* I2C SPE mask */
#define CR1_PE_Set              ((uint16_t)0x0001)
#define CR1_PE_Reset            ((uint16_t)0xFFFE)

/* I2C START mask */
#define CR1_START_Set           ((uint16_t)0x0100)
#define CR1_START_Reset         ((uint16_t)0xFEFF)

#define CR1_POS_Set           ((uint16_t)0x0800)
#define CR1_POS_Reset         ((uint16_t)0xF7FF)

/* I2C STOP mask */
#define CR1_STOP_Set            ((uint16_t)0x0200)
#define CR1_STOP_Reset          ((uint16_t)0xFDFF)

/* I2C ACK mask */
#define CR1_ACK_Set             ((uint16_t)0x0400)
#define CR1_ACK_Reset           ((uint16_t)0xFBFF)

/* I2C ENARP mask */
#define CR1_ENARP_Set           ((uint16_t)0x0010)
#define CR1_ENARP_Reset         ((uint16_t)0xFFEF)

/* I2C NOSTRETCH mask */
#define CR1_NOSTRETCH_Set       ((uint16_t)0x0080)
#define CR1_NOSTRETCH_Reset     ((uint16_t)0xFF7F)

/* I2C registers Masks */
#define CR1_CLEAR_Mask          ((uint16_t)0xFBF5)

/* I2C DMAEN mask */
#define CR2_DMAEN_Set           ((uint16_t)0x0800)
#define CR2_DMAEN_Reset         ((uint16_t)0xF7FF)

/* I2C LAST mask */
#define CR2_LAST_Set            ((uint16_t)0x1000)
#define CR2_LAST_Reset          ((uint16_t)0xEFFF)

/* I2C FREQ mask */
#define CR2_FREQ_Reset          ((uint16_t)0xFFC0)

/* I2C ADD0 mask */
#define OAR1_ADD0_Set           ((uint16_t)0x0001)
#define OAR1_ADD0_Reset         ((uint16_t)0xFFFE)

/* I2C ENDUAL mask */
#define OAR2_ENDUAL_Set         ((uint16_t)0x0001)
#define OAR2_ENDUAL_Reset       ((uint16_t)0xFFFE)

/* I2C ADD2 mask */
#define OAR2_ADD2_Reset         ((uint16_t)0xFF01)

/* I2C F/S mask */
#define CCR_FS_Set              ((uint16_t)0x8000)

/* I2C CCR mask */
#define CCR_CCR_Set             ((uint16_t)0x0FFF)

/* I2C FLAG mask */
#define FLAG_Mask               ((uint32_t)0x00FFFFFF)

/* I2C Interrupt Enable mask */
#define ITEN_Mask               ((uint32_t)0x07000000)




#define I2C_DIRECTION_TX 0
#define I2C_DIRECTION_RX 1

#define I2C1_DMA_CHANNEL_TX           DMA1_Channel6
#define I2C1_DMA_CHANNEL_RX           DMA1_Channel7

#define I2C2_DMA_CHANNEL_TX           DMA1_Channel4
#define I2C2_DMA_CHANNEL_RX           DMA1_Channel5

#define I2C1_DR_Address              0x40005410
#define I2C2_DR_Address              0x40005810

#define  ClockSpeed            400000
#define OwnAddress1 0x28
#define OwnAddress2 0x90


/* I2C DMAEN mask */
#define CR2_DMAEN_Set           ((uint16_t)0x0800)
#define CR2_DMAEN_Reset         ((uint16_t)0xF7FF)


typedef enum
{
    Polling = 0x00,
    Interrupt = 0x01,
    DMA = 0x02
} I2C_ProgrammingModel;

typedef enum
{
    Error = 0,
    Success = !Error
}Status;


#endif

/* I2C �Ĵ��� */
#define SW_VERSION			0x00	// ��Ƭ������汾
#define HW_VERSION			0x01	// Ӳ���汾 
#define SYS_TEMP_H			0x02	// �¶ȴ�������8λ
#define SYS_TEMP_L			0x03	// �¶ȴ�������8λ
#define SYS_TEMP_CTL		0x04	// �¶ȿ��ƼĴ�����BIT0: 1-��ȡ�¶�
#define SYS_FAN_STATUS      0x05	// ����״̬�Ĵ���
#define SYS_FAN_SPEED       0x06	// ����
#define SYS_CTL_FAN			0x07	// �����ٶȿ��ƼĴ���

#define SYS_RTC_SEC			0x08	// ��ȡʱ��-��
#define SYS_RTC_MIN			0x09	// ��ȡʱ��-��
#define SYS_RTC_HOUR		0x0a	// ��ȡʱ��-ʱ
#define SYS_RTC_WEEK		0x0b	
#define SYS_RTC_DAY		    0x0c
#define SYS_RTC_MONTH		0x0d
#define SYS_RTC_YEAR		0x0e
#define SYS_RTC_SEC_W		0x0f	// ����ʱ��-��
#define SYS_RTC_MIN_W		0x10	
#define SYS_RTC_HOUR_W		0x11
#define SYS_RTC_WEEK_W		0x12
#define SYS_RTC_DAY_W		0x13	
#define SYS_RTC_MONTH_W		0x14
#define SYS_RTC_YEAR_W		0x15
#define SYS_RTC_AMPM		0x16
#define SYS_RTC_AMPM_W		0x17	// 1: AM; 0: PM
#define SYS_RTC_12_24		0x18
#define SYS_RTC_12_24_W		0x19	// 1: 12Сʱ��; 0: 24Сʱ��
#define SYS_RTC_CENTURY		0x1a
#define SYS_RTC_CENTURY_W	0x1c
#define SYS_RTC_CTL			0x1d	// ʱ�����ÿ��ƼĴ�����BIT0: 1-����/����ʱ��

#define SYS_IR_VAL			0x1e	// ����
#define SYS_IR_CTL			0x1f	// ������ƼĴ���
#define SYS_HDS3SS215		0x20	// ��Ƶ���绷��
#define SYS_HDS3SS215_CTL	0x21	// �����л�
#define RED_LED_CTL			0x22	// panel led red ���ƼĴ���
#define GREEN_LED_CTL		0x23	
#define BLUE_LED_CTL		0x24

#define OLED_IP				0x25
#define OLED_IP_1			0x26
#define OLED_IP_2			0x27
#define OLED_IP_3			0x28
#define OLED_IP_4			0x29
#define OLED_REQ_PIC		0x2A
#define OLED_PIC			0x2C

#define OLED_DISPLAY_OFF	0x2E  //oled����
#define OLED_CONTRAST		0x2F   //�Աȶ�����
#define SYS_INIT_COM         0x47  //ϵͳ��ʼ���������
//#define oled_pic             0x31    //�ַ�����
#define OLED_ASC			0x2D

#define OLED_1  	         0x32    
#define OLED_2     		     0x33   
#define OLED_3	             0x34    
#define OLED_4          	 0x35    
#define OLED_5               0x36    
#define OLED_6               0x37    
#define OLED_7               0x38    
#define OLED_8               0x39    
#define OLED_9               0x3a    
#define OLED_10              0x3b    
#define OLED_11              0x3c    
#define OLED_12              0x3d    
#define OLED_13              0x3e   
#define OLED_14              0x3f   
#define OLED_15              0x41   
#define OLED_16              0x42    
#define OLED_17              0x43  
#define OLED_18              0x44   
#define OLED_19              0x45    
#define OLED_20              0x46 

#define OLED_Logo  	         0x48

#define OLED_Logo_1  	    0x49    
#define OLED_Logo_2     	0x4a   
#define OLED_Logo_3	        0x4b    
#define OLED_Logo_4         0x4c    
#define OLED_Logo_5         0x4d    
#define OLED_Logo_6         0x4e    
#define OLED_Logo_7         0x4f    
#define OLED_Logo_8         0x50    
#define OLED_Logo_9         0x51    
#define OLED_Logo_10        0x52    
#define OLED_Logo_11        0x53    
#define OLED_Logo_12        0x54    
#define OLED_Logo_13        0x55   
#define OLED_Logo_14        0x56 

#define SYS_PWR_CTL         0x57

#define REQ_SHUTDOWN        0x60    //req shutdown reg
#define REBOOT_REG          0x62    // reboot ctrl reg
#define ENABLE_VERSION      0x63    // bit0: sw, bit1: hw
#define TMDS_REG            0x64
#define TMDS_REG_VALUE      0x65
#define TMDS_CTL            0x66

#define IT6662_CTL          0x67
#define IT6662_REG          0x68
#define IT6662_OFFSET       0x69
#define IT6662_VALUE        0x6a

#define LED_COUNT_R         0x6b   
#define LED_COUNT_G         0x6c
#define LED_COUNT_B         0x6d
#define LED_TEST            0x6e    // for test
#define LED_ADD_1           0x6f    // for test
#define LED_ADD_2           0x70    // for test
#define LED_TYPE            0x71    // for test
#define LED_TEST1           0x72    // for test
#define LED_BRIGHT          0x73    // for test
#define LED_INTERVAL        0x74    // for test
    
#define TMDS_HDMI1_REG            0x75
#define TMDS_HDMI1_REG_VALUE      0x76
#define TMDS_HDMI1_CTL            0x77
#define TMDS_HDMI2_REG            0x78
#define TMDS_HDMI2_REG_VALUE      0x79
#define TMDS_HDMI2_CTL            0x7a

//add by wzl
#define SYS_FAN_AUTO_CTRL_DISABLE 0x7b

#define MIC1_CTL			0x80
#define MIC1_TYPE			0x81
#define MIC2_CTL			0x82
#define MIC2_TYPE			0x83
#define MIC1_ENABLE			0x84
#define MIC2_ENABLE			0x85

/* I2C�Ĵ������� */
#define I2C_REG_NUM			0xbd

/* �Ĵ�������λ����
 * SYS_TEMP_H �¶ȸ�8λ�Ĵ���
 *	˵����read only 
 *		ϵͳ�¶ȸ�8λSYS_TEMP_H����SYS_TEMP_L����¶ȵ�ֵ, LSB BIT7��λ��ʾ����0.5
 *		BIT15--BIT8��ɸ�8λ��BIT7--BIT0��8λ��BIT7��ʾ���ȣ�BIT6--BIT0 Ԥ��
 *		ʵ���¶�printf("temp = %d.%d\n", val_reg[sys_tempH],val_reg[sys_tempL] == 0 ? 0:5);
 *		���� val_reg[sys_tempH] = 0x21��val_reg[sys_tempL] = 0x10����ϵͳ�¶�Ϊ33.5 ��C
 *      
 * SYS_CTL_FAN
 *	˵����
 *		R/W
 * ���ȿ���ʹ��,
 * BIT0     enable 0x1 ��ʾsys_fan_speed�Ĵ�����д,ʹ���ⲿ���Ʒ��ȣ� 
 *				0x0 ��ʾ����д,ʹ���ڲ��¶ȵ����
 * BIT1     enalbe 0x1 ��ʾ�����ٶȸı���Ҫ����
 *				0x0 ��ʾ����û���ٶȸı䲻��Ҫ����
 *	
 * SYS_RTC_CTL
 *		BIT0��1 ����/����ʱ�� 
 *		BIT1: 1 ��ȡʱ��
 *		BIT2��1 ��ȡAMPM����
 *		BIT3��1 ����AMPM
 * SYS_HDS3SS215	
 *		BIT0: 1 �ϵ�
 *		BIT0��0 �ϵ�
 * SYS_HDS3SS215_CTL
 * 		BIT0��1 ȷ���л�
 *		BIT7: 1: �ϵ�;	0:�ϵ�
 */

#endif
