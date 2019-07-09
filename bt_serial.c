#include "bt_serial.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define RESET   _BV(PD2)
#define CMDMODE _BV(PD3)

int bt_serial_send_len;
int bt_serial_recv_len;
const uint8_t *send_ptr;
uint8_t *recv_ptr;

ISR(USART_RX_vect) {
    if (bt_serial_recv_len > 0) {
        *(recv_ptr++) = UDR0;
        bt_serial_recv_len--;
    }
}

ISR(USART_TX_vect) {
    if (bt_serial_send_len > 0) {
        UDR0 = *(send_ptr++);
        bt_serial_send_len--;
    }
}

void
bt_serial_init(void) {
    //PORTD &= ~RESET;
    //PORTD |= CMDMODE;
    //DDRD |= RESET | CMDMODE;

    // U2X0     = 0 no doublespeed
    UBRR0 = (F_CPU / (16*9600UL)) - 1; // = 119
    UCSR0B = _BV(RXCIE0) | _BV(TXCIE0) | _BV(RXEN0) | _BV(TXEN0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8bits, no pty, 1 stop

    //PORTD |= RESET; // enable normal operation
}

void
bt_serial_recv(uint8_t *data, int len) {
    recv_ptr = data;
    bt_serial_recv_len = len;
}

void
bt_serial_send(const uint8_t *data, int len) {
    send_ptr = data + 1;
    bt_serial_send_len = len - 1;
    UDR0 = *data;
}

