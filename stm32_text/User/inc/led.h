#ifndef __LED_H
#define __LED_H

#define RED_LED		0x01
#define GREEN_LED	0x02
#define BLUE_LED	0x03 
#define LED_ON      1
#define LED_OFF     0

void update_led(void);
void led_ctl(int index, int on);
void led_blink_ir_timer(void);
void led_blink_ir(void);
void led_loop_blink_timer(void);
void led_breathing_test(void); // for test

#endif
