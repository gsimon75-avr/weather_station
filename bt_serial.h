#ifndef BT_SERIAL_H
#define BT_SERIAL_H

void bt_serial_init(void);
void bt_serial_recv(uint8_t *data, int len);
void bt_serial_send(const uint8_t *data, int len);
extern int bt_serial_send_len;
extern int bt_serial_recv_len;
#endif
