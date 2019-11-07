#ifndef __FAN_H
#define __FAN_H

#define FAN_ARR			99	/* 设置自动重装载寄存器周期的值 */
#define FAN_PRESCALER	24		/* 设置时钟频率除数的预分频值 */
#define FAN_DUTY		300		/* 占空比 */

void fan_init(u16 arr, u16 psc);
void fan_setduty(uint16_t duty);
void update_fan_speed(void);

#endif
