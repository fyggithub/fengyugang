#ifndef GPIO_H
#define GPIO_H

#define LED_ON     1
#define LED_OFF    0
#define BEEP_ON    1
#define BEEP_OFF   0


extern void GPIO_Configuration(void);
extern void Gpio_setPb14(void);
extern void Gpio_allBoardRest(void);

#endif
