#ifndef GPIO_H
#define GPIO_H
#include "sys.h"

#define MAX_VALUE_TIME 4294967295u
#if 0
typedef enum tagsLED_ID
{
    LED_ID_ACT = 0,
    LED_ID_RUN,
    LED_ID_ERR,
    LED_ID_BUTT,

}LED_ID;

typedef enum tagsLED_STATE
{
    LED_STATE_OFF = 0,
    LED_STATE_ON,
    LED_STATE_BLINK,
    LED_STATE_BUTT,

}LED_STATE;
#endif

//void Gpio_setLed(LED_ID id, LED_STATE state);
void cpu_power_init(void);
void pwr_key_control(void);
void pwr_control(void);
void button_press_release(void);
void power_on_system(void);
void clean_uart_buf(void);
int greater_times(unsigned int begin_time, unsigned int end_time, unsigned int times);
void pse_reset(void);
/* detect power in by polling */
void detect_power_pin(void);

#endif
