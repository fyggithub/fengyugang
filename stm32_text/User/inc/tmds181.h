#ifndef __TMDS181_H
#define __TMDS181_H

#define HDMI_OUT1_DXSEL		GPIO_Pin_1
#define HDMI_OUT2_DXSEL		GPIO_Pin_5

void tmds_init(void);
void hd3ss215_init(void);
void hds3ss215_check_switch(void);
void hds3ss215_switch_rk3339(void);
void hds3ss215_switch_pc(void);

void hds3ss215_switch_test(void);
u8 tmds_ReadOneByte(u8 DeviceAddr, u8 ReadAddr);		// this is test
void tmds181_register_ctl(void);

#endif
