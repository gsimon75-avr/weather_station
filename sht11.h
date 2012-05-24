#ifndef SHT11_H
#define SHT11_H

#define SHT11_CMD_GET_TEMP      0x03
#define SHT11_CMD_GET_HUMIDITY  0x05
#define SHT11_CMD_GET_STATUS    0x07
#define SHT11_CMD_SET_STATUS    0x06
#define SHT11_CMD_SOFT_RESET    0x1e

#define SHT11_STATUS_BATTERY_LOW        0x40
#define SHT11_STATUS_HEATER             0x04
#define SHT11_STATUS_NO_RELOAD_OTP      0x02
#define SHT11_STATUS_LOW_RESOLUTION     0x01

extern unsigned char sht11_error;

void sht11_init(void);
void sht11_hard_reset(void);

unsigned char sht11_send_byte(unsigned char cmd);
unsigned char sht11_wait_ready(void);
unsigned int sht11_read_word(void);
#endif
