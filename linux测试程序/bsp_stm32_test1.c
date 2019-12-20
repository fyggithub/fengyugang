#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>
//#include "./../include/bsp_stm32_public.h"
//#include "bsp_stm32_public.h"

#define STM32_SERIAL_AM57XX		"/dev/ttyXRUSB2"

/*定义串口数据协议各字段长度*/
#define UART_HEAD_LEN        2
#define UART_CMD_LEN         2
#define UART_DATALEN_LEN     2
#define UART_DATA_LEN        16
#define UART_STATE_LEN       1
#define UART_CS_LEN          1

#define CPLD_POWER_STATE_REG (0x10)
#define CPLD_POWER_UP_REG    (0x11)
#define CPLD_POWER_CPU_REG   (0x12)
#define CPLD_BOARD_SLOT_REG  (0x13)
#define CPLD_FAN_STATE_REG   (0x15)
#define POWER_UP             (0x1)
#define POWER_OFF            (0x0)
#define BIT_MASK(x)          (~(0x1<<x))
#define STM32_BIT(x)         (0x1<<x)
#define SW_CTRL_PAGE         (0x00)
#define TXFlow Control Enable
#define TXFLOW_CTRL_ENABLE   (1<<5)
#define RXFLOW_CTRL_ENABLE   (1<<4)

#define SW_MODE_REG          (0xb)
/*
 * #define SW_FWDG_EN           BIT(1)
 * #define SW_FWDG_MODE         BIT(0)
 */

#define BASE_IOC_PRIVATE   100
#define FREE_REQ_PIN       _IOWR     ('F', BASE_IOC_PRIVATE + 0, int)
#define SET_REQ_PIN        _IOWR     ('F', BASE_IOC_PRIVATE + 1, int)
#define GET_GNT_PIN        _IOWR     ('F', BASE_IOC_PRIVATE + 2, int)
#define SW_IMP_PORT_CTRL_REG (0x8)
#define RX_UCST_EN           (0x1<<4)
#define RX_MCST_EN           (0x1<<3)
#define RX_BCST_EN           (0x1<<2)

#define SW_STATUS_PAGE       (0x01)

#define SW_MANAGE_PAGE       (0x2)
#define MIRROR_CAP_CTRL_REG  (0x10)
#define MIRROR_IN_CTRL_REG   (0x12)
#define MIRROR_OUT_CTRL_REG  (0x1c)

#define SW_IMP_PORTID_REG    (0x3)
#define BRCM_HDR_DISABLE     (0x00)
#define SW_GMANAGE_CONF_REG  (0x00)
#define EN_IMP_PORT          (0x2 << 6)

#define SW_INT_CTRL_PAGE     (0x3)
#define ARL_CTRL_PAGE        (0x4)
#define ARL_ACCESS_PAGE      (0x5)
#define VLAN_TABLE_INDEX_REG (0x81)
#define VTBL_ADDR_INDEX1     (0x1)
#define VTBL_ADDR_INDEX2     (0x2)
#define VTBL_ADDR_INDEX3     (0x2)
#define VLAN_TABLE_ENTRY_REG (0x83)

#define VLAN_TABLE_RW_CTRL_REG (0x80)
#define START_DONE_INIT        (0x1<<7)
#define VTBL_W_INIT            (0x00)

#define PORT0_PHY_PAGE         (0x10)
#define PORT1_PHY_PAGE         (0x11)
#define PORT2_PHY_PAGE         (0x12)
#define PORT3_PHY_PAGE         (0x13)
#define PORT4_PHY_PAGE         (0x14)
#define PORT5_PHY_PAGE         (0x15)
#define PORT6_PHY_PAGE         (0x16)
#define PORT7_PHY_PAGE         (0x17)

#define SW_VLAN_PAGE           (0x34)
#define GLOBAL_REG             (0x00)
/*
 * 0xe7  1110 0111
 * bit7:1 enable Vlan
 * bit6_5:11 IVL
 * bit3:0 No change for 1Q/ISP tag if VID is not 0
 */
#define GLOBAL_1Q_VAL              (0xe7)
#define PORT0_TAG_REG              (0x10)
#define PORT1_TAG_REG              (0x12)
#define PORT2_TAG_REG              (0x14)
#define PORT3_TAG_REG              (0x16)
#define PORT4_TAG_REG              (0x18)
#define PORT5_TAG_REG              (0x1A)
#define PORT6_TAG_REG              (0x1C)
#define PORT7_TAG_REG              (0x1E)
#define IMP_TAG_REG                (0x20)
#define VLAN_ID_1                  (0x1)
#define VLAN_ID_2                  (0x2)
#define VLAN_ID_3                  (0x2)

#define SW_GVLAN_CTRL4_REG         (0x5)
#define SRC_MEMBER_CHECK           (0x1<<7)

#define SW_GVLAN_CTRL5_REG         (0x6)

#define PORTINFO_BASEPAGE          0x01
#define PORTSPEED_OFFSET           0x04
#define PORTDUPLEX_OFFSET          0x08
#define PORTLINKINFO_OFFSET        0x00

#define PORT0_STAT_OVERRIDE_REG    (0x58)
#define PORT1_STAT_OVERRIDE_REG    (0x59)
#define PORT2_STAT_OVERRIDE_REG    (0x5a)
#define PORT3_STAT_OVERRIDE_REG    (0x5b)
#define PORT4_STAT_OVERRIDE_REG    (0x5c)
#define PORT5_STAT_OVERRIDE_REG    (0x5d)
#define PORT6_STAT_OVERRIDE_REG    (0x5e)
#define PORT7_STAT_OVERRIDE_REG    (0x5f)
#define PORT_IMP_STAT_OVERRIDE_REG (0x0e)
#define N_ARRAY(ar) (sizeof(ar) / sizeof(ar[0]))

#define PHY_LINK_STATUS_REG         (0x1)
#define PHY_AUX_STATUS_REG          (0x19)

#define	   UPDATE_INPUT_CMD           0x01
#define    MAX_BOOT_CMD               0x02

#define    LED_RED_INPUT_CMD          0x06
#define    LED_GREEN_INPUT_CMD        0x07
#define    LED_BLUE_INPUT_CMD         0x08

#define    OLED_CLEAR_INPUT_CMD       0x0a
#define    OLED_LOGO_INPUT_CMD        0x0b
#define    OLED_LOGO_UPDATE_INPUT_CMD 0x0c
#define    OLED_IP_INPUT_CMD          0x0d
#define    OLED_CTL_INPUT_CMD         0x0e
#define    OLED_STRINGS_INPUT_CMD     0x0f

#define    FAN_AUTO_INPUT_CMD         0x16
#define    FAN_SET_SPEED_INPUT_CMD    0x17

#define    SYS_SHUTDOWN_INPUT_CMD     0x20
#define    SYS_POWER_INPUT_CMD        0x21
    


/*定义最大接收字节数*/
#define USART_REC_LEN  	(UART_HEAD_LEN + UART_CMD_LEN + UART_DATALEN_LEN + \
	                     UART_DATA_LEN + UART_STATE_LEN + UART_CS_LEN)
#define HEAD            (0xEBAA)

const char *pUpdageFilePath[1] = 
{
    //"./stm32_app.bin",
    "/opt/stm32_upgrade/stm32_app.bin"
};

/*串口数据协议定义*/
typedef struct tagsUART_CMD
{
    unsigned short head;
    unsigned short cmd;
    unsigned char  data[UART_DATA_LEN];
	unsigned short  dataLen;
    unsigned char  state;
    unsigned char  cs;

}UART_CMD;

typedef enum
{
	UART_ID0 = 0,
	UART_ID1,	
}UART_ID;

typedef enum
{
	APP_UPDATE_START_CMD = 3,
	APP_UPDATE_CMD,
	APP_UPDATE_FINISH_CMD,
    FIRMWARE_VER_CMD,
    MAX_VER_CMD,

    LED_RED_CMD = 0x22,
    LED_GREEN_CMD,
    LED_BLUE_CMD,

    OLED_CLEAR_CMD = 0x47,
    OLED_LOGO_CMD = 0x48,
	OLED_LOGO_UPDATE_CMD = 0x2A,
	OLED_IP_CMD = 0x25,
	OLED_CTL_CMD = 0x2E,
	OLED_STRINGS_CMD = 0x2D,

	FAN_AUTO_CMD = 0x7B,
	FAN_SET_SPEED_CMD = 0x07,

	SYS_SHUTDOWN_CMD = 0x61,
	SYS_POWER_CMD = 0x62,
    
}PROC_CMD;

typedef enum
{
    READ_FLAG  = 0,
    WRITE_FLAG ,
    DIR_FLAG_BUTT,
}UART_DIR;

typedef enum
{
    STM32_RES_OK = 0,
    STM32_RES_UNKNOWN,
    STM32_RES_FAIL,   
    STM32_RES_DATA_ERR,
}Respond;


UART_CMD g_UartSend;
/* int serial_fd[UART_ID_BUTT] = {0x0,}; */
int serial_fd[1] = {0x0,};


static void stm32_serial_flush(int fd)
{	
	tcflush(fd, TCIOFLUSH);
	sleep(1);
	return;
}

static void stm32_set_blocking (int fd, int should_block)
{
    struct termios tty;

    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0)
    {
        printf("error from tggetattr\n");
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        printf("error setting term attributes\n");
    }

    return;
}

static int stm32_set_interface_attribs(int fd, int speed, int parity)
{
    struct termios tty;

    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0)
    {
        printf("error  from tcgetattr\n");
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL); // shut off xon/xoff ctrl

    /* 不占用串口，可读串口  */
    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    //tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    //tty.c_cflag |= parity;

    /* 无奇偶校验 */
    tty.c_cflag &=~PARENB;
    tty.c_iflag &= ~INPCK;

    /* 1bit stop */
    tty.c_cflag &= ~CSTOPB;
    /* 无流控 */
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        printf("error from tcsetattr\n");
        return -1;
    }

    return 0;
}

int stm32_serial_init(UART_ID id)
{
    int fd = -1;

//	if (UART_ID_AM57XX == id)
//	{
    	fd = open(STM32_SERIAL_AM57XX, O_RDWR | O_NOCTTY | O_SYNC);
//    }
//    else
//    {
//        printf("%s %d input param is err\n", __FUNCTION__, __LINE__);
//    }

	if (fd < 0)
	{
		printf("the open is fail\n"); 
		return -1; 
	}
	printf("fd = %d\n",fd);
    serial_fd[id] = fd; 

    stm32_set_interface_attribs(serial_fd[id], B115200, 0);
	stm32_set_blocking (serial_fd[id], 0);
	stm32_serial_flush(serial_fd[id]);
	
	printf("init ok!\n");
	return 0;
}

void stm32_serial_exit(UART_ID id)
{
    if(0 < serial_fd[id])
    {
        close(serial_fd[id]);
    }
}
  
// 同步读串口  
int ReadComm(int fd, unsigned char *buf, unsigned int size)  
{
    int      nread       =     0;
    int      read_result =     0;
    unsigned char *pdst  =  buf;

    if ((size <= 0) || (buf == NULL))
    {
        printf("the input param is err\n"); 
        return -1;
    }

    for (nread = 0; nread < size; nread += read_result)
    {
        read_result = read(fd, pdst, size - nread); 
        if (read_result < 0)
        {
			read_result = 0;
        }
		else
		{
			pdst += read_result;
		}
    }
	
    return size;
}
  
static int WriteComm(int fd, unsigned char *buf, unsigned int size)
{
	int      nwrite       =     0;
	int      write_result =     0;
	unsigned char   *pdst =  buf;
	
	if ((size <= 0) || (buf == NULL))
	{
		printf("the input param is err\n");
		return -1;
	}
	
	for (nwrite = 0; nwrite < size; nwrite += write_result)
	{
		write_result = write(fd, pdst, size - nwrite);
		if (write_result < 0)
		{
			printf("serial: the write is fial, continue------\n");
			break;
		}

		pdst += write_result;
	}

	return 0;
}

static unsigned char Proc_CheckSum(unsigned char *pData, unsigned short len)
{
    unsigned short i;
    unsigned char  val = 0;
	
	for(i = 0; i < len; i++)
	{
        val +=pData[i];
	}

	return val;
}

static int readFileData(int fd, unsigned char *pData, int len)
{
    int nLen        = 0;
    int readLen     = 0;
    int needReadLen = len;

	while(1)
	{
		readLen = read(fd, &pData[nLen], needReadLen);
		if(readLen == needReadLen)
		{
			return 0;
		}
		else if(readLen > 0 && readLen < needReadLen)
		{
		    nLen += readLen;
			needReadLen -= readLen;
			lseek(fd, nLen, SEEK_SET);
		}
		else if(readLen > needReadLen)
		{
            printf("%s %d\n", __FUNCTION__, __LINE__);
		    return -1;
		}
	}
}

int bsp_uartTransfer(UART_ID id, UART_DIR dirFlag, PROC_CMD cmd, unsigned char *sendBuf, 
        unsigned char *rcvBuf, unsigned char len)
{
    int     fd  = -1;
    int      ret      = -1;
    UART_CMD uartSend = {0x0};
    UART_CMD uartRes  = {0x0};
    unsigned char checkSum = 0;

    if ((UART_DATA_LEN < len) || (NULL == sendBuf) || (NULL == rcvBuf) 
            || (DIR_FLAG_BUTT < dirFlag))
    {
        printf("--%s--the input param len is err \n", __FUNCTION__);
        return -1;
    }

	uartSend.head = HEAD;
	uartSend.cmd  = cmd;
	uartSend.dataLen = UART_DATA_LEN;
    memcpy(&uartSend.data[0], sendBuf, UART_DATA_LEN);

	uartSend.state = 0xff;
	checkSum = Proc_CheckSum((unsigned char *)(&uartSend), sizeof(UART_CMD) - 1);
	uartSend.cs = 0 - checkSum;
	//printf("checkSum=%d, uartSend.cs=%d, cmd=%d\n", checkSum, uartSend.cs, cmd);

    fd = serial_fd[id];
    if (0 > fd)
    {
        printf("%s %d err\n", __FUNCTION__, __LINE__);
        return -1;
    }

    ret = WriteComm(fd, (unsigned char *)(&uartSend), USART_REC_LEN);
    if (0 > ret)
    {
        printf("%s %d WriteComm is fail\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = ReadComm(fd, (unsigned char *)(&uartRes), USART_REC_LEN);
    if (0 > ret)
    {
        printf("%s %d ReadComm is fail\n", __FUNCTION__, __LINE__);       
        return ret;
    }

	checkSum = Proc_CheckSum((unsigned char *)(&uartRes), sizeof(UART_CMD));
    if(0x0 != checkSum)
    {
        printf("%s %d the checkSum is err\n", __FUNCTION__, __LINE__);
        return -1;
    }

    /*
     * printf("checkSum=%d cs=%d head=0x%x cmd=%d dataLen=%d state=%d\n", checkSum, 
     *         uartRes.cs, uartRes.head, uartRes.cmd, uartRes.dataLen, uartRes.state);
     */

    /* *pVal = uartRes.data[3]; */
    /* printf("%s %d val=%d\n", __FUNCTION__, __LINE__, uartRes.data[3]); */
    if (READ_FLAG == dirFlag)
    {
        memcpy(rcvBuf, uartRes.data, len);
    }

    return 0;
}

int bsp_getVer(UART_ID uartId, unsigned char *pVal)
{
    int ret = -1;
    unsigned char sendBuf[UART_DATA_LEN];

    if (NULL == pVal)
    {
        printf("%s %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    memset(sendBuf, 0x0, UART_DATA_LEN);

    ret = bsp_uartTransfer(uartId, READ_FLAG, FIRMWARE_VER_CMD, sendBuf, pVal, 
            sizeof(unsigned char));
    if (0 > ret)
    {
        printf("%s %d fail\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}

int sendOnePacketData(int id, UART_CMD *pUartSend, unsigned short cmd, unsigned short dataLen)
{
    int ret = -1;
    unsigned char sendBuf[UART_DATA_LEN];

    if (NULL == pUartSend)
    {
        printf("%s %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    memset(sendBuf, 0x0, UART_DATA_LEN);
    memcpy(sendBuf, pUartSend->data, UART_DATA_LEN);

    ret = bsp_uartTransfer(id, WRITE_FLAG, cmd, sendBuf, sendBuf, dataLen);
    if (0 > ret)
    {
        printf("%s %d fail\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}

int stm32_upgrade(UART_ID id)
{
    struct stat buf;
	int      i            = 0;
	int      fd           = -1;
	int      ret          = -1;
	int      fileSize     = 0;
	int      seekSize     = 0;
	int      sendCnt      = 0;
	int      lastFramSize = 0;
	UART_CMD *pUartSend   = &g_UartSend;
    //unsigned char ver = 0;
    //int  tryCnt = 3;

    /* printf("%s %d file:%s\n", __FUNCTION__, __LINE__, pUpdageFilePath[id]); */
    fd = open(pUpdageFilePath[id], O_RDWR);
	if(0 > fd)
	{
	    printf("%s is fail\n", pUpdageFilePath[id]);
		return -1;
	}

    stat(pUpdageFilePath[id], &buf);
	printf("file size:%ld\n", buf.st_size);

    fileSize = buf.st_size;
	sendCnt = fileSize / UART_DATA_LEN;
    lastFramSize =  fileSize % UART_DATA_LEN;

	memset(pUartSend->data, 0, UART_DATA_LEN);
	//sendOnePacketData(id, pUartSend, 10, UART_DATA_LEN);
	sendOnePacketData(id, pUartSend, APP_UPDATE_START_CMD, UART_DATA_LEN);
    printf("start upgrade\n");
    //sleep(5);
    //printf("start sendCnt=%d \n", sendCnt);

	for(i = 0; i < sendCnt; i++)
	{
        memset(pUartSend->data, 0, UART_DATA_LEN);
        ret = readFileData(fd, pUartSend->data, UART_DATA_LEN);
        if(ret != 0)
        {
            printf("read file data failed\n");
            break;
        }
        seekSize += UART_DATA_LEN;
        lseek(fd, seekSize, SEEK_SET);
    
	 sendOnePacketData(id, pUartSend, APP_UPDATE_CMD, UART_DATA_LEN);
	/* printf("send data frame %d ok\n", i); */
        printf("#");
        fflush(stdout);
	}

	if(lastFramSize != 0)
	{
        memset(pUartSend->data, 0, UART_DATA_LEN);
		ret = readFileData(fd, pUartSend->data, lastFramSize);
		if(ret != 0)
		{
		    printf("read file data failed\n");
		}
		else
		{
            sendOnePacketData(id, pUartSend, APP_UPDATE_CMD, UART_DATA_LEN);
            printf("#");
            fflush(stdout);
		}
	}
	close(fd);

    sendOnePacketData(id, pUartSend, APP_UPDATE_FINISH_CMD, UART_DATA_LEN);
	return 0;
}


int Uart_Init(void)
{
	int ret = -1;
	ret = stm32_serial_init(UART_ID0);
	if(ret < 0)
	{
		printf("stm32_serial_init is fail!\n");
		return -1;
	}
	return 0;
}

int AppStm32Update(void)
{
	int ret; 
	printf("start upgrade!\n");
	ret = stm32_upgrade(UART_ID0);
	if(0 > ret)
	{
		printf("update is fail!\n");
		return -1;
	}
	printf("updata is ok!\n");
	return 0;
}

unsigned char Led_Ctl(char module,char on_off)
{
	unsigned char val;
	switch(module)
	{
		case 1: 
			if(on_off)
				val = 0x81; 
			else
				val = 0x80; 
			break;
		case 2:
			if(on_off)
				val = 0x42; 
			else
				val = 0x40; 
			break;
		case 3:
			if(on_off)
				val = 0x24; 
			else
				val = 0x20; 
			break;					
		default:break;
	}
	return val;
}

int Text_Led(PROC_CMD cmd,char module,char on_off)
{
	int ret;
	unsigned char pSendBuff[UART_DATA_LEN];
	switch(cmd)
	{
		case LED_RED_CMD:
					{
						pSendBuff[0] = Led_Ctl(module,on_off);
						ret = bsp_uartTransfer(UART_ID0, 0, LED_RED_CMD,pSendBuff,pSendBuff,UART_DATA_LEN); 
					}break;				
		case LED_GREEN_CMD:
					{
						pSendBuff[0] = Led_Ctl(module,on_off);
						ret = bsp_uartTransfer(UART_ID0, 0, LED_GREEN_CMD,pSendBuff,pSendBuff,UART_DATA_LEN); 
					}break;
		case LED_BLUE_CMD:
					{					
						pSendBuff[0] = Led_Ctl(module,on_off);												
						ret = bsp_uartTransfer(UART_ID0, 0, LED_BLUE_CMD,pSendBuff,pSendBuff,UART_DATA_LEN); 
					}break;
		default:break;
	}	
	return ret;
}

int Oled_Logo(PROC_CMD cmd)
{
	int ret,len,i;
	unsigned char pSendBuff[UART_DATA_LEN];
	char logo_buf[10] = "ifreecomm";
	
	pSendBuff[0] = 0x89;
	len = pSendBuff[0] & 0x7f;

	if(len > (sizeof(logo_buf) - 1) )
		return -1;
	
	memcpy(pSendBuff+1,logo_buf,len);	
	ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN); 

	return ret;
}

int Oled_Clear(PROC_CMD cmd)
{
	int ret;
	unsigned char pSendBuff[UART_DATA_LEN];
	pSendBuff[0] = 0x01;
	ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN); 
	return ret;
}

int Oled_Pattern(PROC_CMD cmd,char value)
{
	int ret;
	unsigned char pSendBuff[UART_DATA_LEN];
	pSendBuff[0] = 0x01;
	pSendBuff[1] = value;
	ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN); 
	return ret;
}

int Oled_Set_Ip(PROC_CMD cmd,char *ip)
{
	int i,ret;
	unsigned char pSendBuff[UART_DATA_LEN];
	pSendBuff[0] = 0x01;
	memcpy((pSendBuff + 1),ip,4);
	ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN); 
	return ret;
}

int Oled_On_Off(PROC_CMD cmd,char val,char duty)
{
	int i,ret;
	unsigned char pSendBuff[UART_DATA_LEN];
	
	pSendBuff[0] = val;
	if(pSendBuff[0] & 0x04)
	{
		pSendBuff[1] = duty;
	}
	ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN); 
	return ret;
}

int Oled_Strings(PROC_CMD cmd,char *strings)
{
	int ret,len,i;
	unsigned char pSendBuff[UART_DATA_LEN];
	
	pSendBuff[0] = strings[0];
	len = pSendBuff[0] & 0x7f;
	
	if(len < 15)
	{	
		memcpy((pSendBuff + 1),strings+1,len);
		ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN);
	}
	else
	{
		printf("parameter is error! the data length rang is 0~15.\n");
		ret = -1;
	}
	return ret;
}

int Fan_Auto(PROC_CMD cmd,char val)
{
	int ret;
	unsigned char pSendBuff[UART_DATA_LEN];

	switch(val)
	{
		case 0:
			pSendBuff[0] = 0x00;    //run auto fan
			ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN);
			break;
		case 1:
			pSendBuff[0] = 0x80;    //stop fan auto
			ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN);
			break;
		default:
			printf("parameter is error!,the last parameter is 0 or 1.\n");
			ret = -1;
			break;
	}
	
	return ret;
}

int Fan_Set(PROC_CMD cmd,char val)
{
	int ret;
	unsigned char pSendBuff[UART_DATA_LEN];
	pSendBuff[0] = 0x80;
	pSendBuff[1] = val;
	if((pSendBuff[1] >= 0) && (pSendBuff[1] <= 100))	
	{
		ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN);
	}
	else
	{	
		printf("parameter is error! parameter adjustment rang 0~100.\n");
	}
	return ret;
}

int Sys_Shutdown(PROC_CMD cmd)
{
	int ret;
	unsigned char pSendBuff[UART_DATA_LEN];

	pSendBuff[0] = 0x44;	
	ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN);

	return ret;
}

int Sys_Power(PROC_CMD cmd)
{
	int ret;
	unsigned char pSendBuff[UART_DATA_LEN];

	pSendBuff[0] = 0x55;	
	ret = bsp_uartTransfer(UART_ID0, 0, cmd,pSendBuff,pSendBuff,UART_DATA_LEN);

	return ret;
}

void *recv_buf(void *arg)
{
	int ret;
	int cfd = (int)arg;
	unsigned char pSendBuff[UART_DATA_LEN];

	while(1)
	{
		memset(pSendBuff,0,UART_DATA_LEN);
		ret = ReadComm(cfd, pSendBuff, UART_DATA_LEN) ;
		if(ret <= 0)
			break;
		printf("rcv = %s ",pSendBuff);
	}
}


int main(int argc,char *argv[])
{
	int ret,i,cmd,len;
	char val[10];

	Uart_Init();
	cmd = atoi(argv[1]);
	printf("input cmd = %d\n",cmd);

	pthread_t thread;
	pthread_create(&thread, NULL, recv_buf, (void*)serial_fd[UART_ID0]);
	
	switch(cmd)
	{
		case UPDATE_INPUT_CMD:   AppStm32Update();break;
		case LED_RED_INPUT_CMD:
						{
							val[0] = atoi(argv[2]);
							val[1] = atoi(argv[3]);
							Text_Led(LED_RED_CMD,val[0],val[1]);
						}break;						
		case LED_GREEN_INPUT_CMD:
						{
							val[0] = atoi(argv[2]);
							val[1] = atoi(argv[3]);
							Text_Led(LED_GREEN_CMD,val[0],val[1]);
						}break;
		case LED_BLUE_INPUT_CMD:
						{
							val[0] = atoi(argv[2]);
							val[1] = atoi(argv[3]);			
							Text_Led(LED_BLUE_CMD,val[0],val[1]);
						}break;
		case OLED_LOGO_INPUT_CMD: Oled_Logo(OLED_LOGO_CMD);break;
		case OLED_CLEAR_INPUT_CMD:Oled_Clear(OLED_CLEAR_CMD); break;		
		case OLED_LOGO_UPDATE_INPUT_CMD:
						{
							val[0] = atoi(argv[2]);
							Oled_Pattern(OLED_LOGO_UPDATE_CMD,val[0]);
						}break;		
		case OLED_IP_INPUT_CMD:
						{
							val[0] = atoi(argv[2]);
							val[1] = atoi(argv[3]);
							val[2] = atoi(argv[4]);
							val[3] = atoi(argv[5]);
							Oled_Set_Ip(OLED_IP_CMD,val);
						}break;		
		case OLED_CTL_INPUT_CMD:
						{
							val[0] = atoi(argv[2]);
							if(val[0] & 0x04)
							{
								val[1] = atoi(argv[3]);
							}
							else
							{
								val[1] = 0;	
							}
							Oled_On_Off(OLED_CTL_CMD,val[0],val[1]);
						}break;		
		case OLED_STRINGS_INPUT_CMD:
						{
							val[0] = atoi(argv[2]);
							len = val[0] & 0x7f;
							memcpy(val+1,argv[3],len);
							Oled_Strings(OLED_STRINGS_CMD,val);
						}break;
		case FAN_AUTO_INPUT_CMD:
						{
							val[0] = atoi(argv[2]);
							Fan_Auto(FAN_AUTO_CMD,val[0]);
						}break;						
		case FAN_SET_SPEED_INPUT_CMD:
						{
							val[0] = atoi(argv[2]);
							Fan_Set(FAN_SET_SPEED_CMD,val[0]);
						}break;
		case SYS_SHUTDOWN_INPUT_CMD: Sys_Shutdown(SYS_SHUTDOWN_CMD); break;
		case SYS_POWER_INPUT_CMD:    Sys_Power(SYS_POWER_CMD);       break;
		default:printf("cmd is not exist!\n");break;
	}
	
	stm32_serial_exit(UART_ID0);
	
	return 0;
}








