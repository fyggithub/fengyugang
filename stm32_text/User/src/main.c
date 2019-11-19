#include "common_init.h"
#include "process.h"
#include "delay.h"
#include "sys.h"
#include "gpio.h"
#include "debug_usart.h"
#include "wdg.h"
#include "lm75.h"
#include "ds1339c.h"
#include "ir.h"
#include "tmds181.h"
#include "fan.h"
#include "led.h"
#include "timer.h"
#include "oled.h"
#include "stm32f10x_i2c.h"
#include "myiic.h"
#include "usart.h"

static void sys_update()
{	
	//ir_check_get_data();        /* 红外 */
	lm75_check_read_temp();     /* 温度，每5s读取一次值 */
	update_led();
	oled_update_display();
	update_fan_speed();

	pwr_control();              //系统重启或关机
	detect_power_pin();         // detect power on/off by polling 
	clean_uart_buf();           //重新上电，串口缓冲区清0
	ir_decode_data();           /* cpu断电后，红外开机 */
}

int main(void)  
{ 
    sys_init();	
    cpu_power_init();
    delay_ms(100);
    printf("enter app\n");

    while(1)
    {
		hostBoardProc();
        sys_update();
        Ir_Deal();
    }
}
