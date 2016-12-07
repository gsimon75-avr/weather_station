#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include "sht11.h"

// Wiring:
// PD5          Data (sht11)
// PD6          SClk (sht11)

// temp&humidity sensor lines
#define VALUE_DATA              _BV(PD5)
#define VALUE_SCLK              _BV(PD6)

static void
sclk_on(void) {
    PORTD |= VALUE_SCLK; // sclk on
    _delay_us(1); // Tclh >= 100 ns
}

static void
sclk_off(void) {
    PORTD &= ~VALUE_SCLK; // sclk off
    _delay_us(1); // Tv >= 250 ns
}

unsigned char
sht11_send_byte(unsigned char cmd) { // NOTE: bit7..5 of cmd is in fact unit address
    unsigned char i;

    // Transmission Start
    PORTD |= VALUE_DATA;
    DDRD |= VALUE_DATA; //set output
    _delay_us(1); // Tsu >= 100 ns

    sclk_on();
    PORTD &= ~VALUE_DATA; // data off
    _delay_us(1); // Tsu >= 100 ns
    sclk_off();
    sclk_on();
    PORTD |= VALUE_DATA; // data on
    _delay_us(1); // Tsu >= 100 ns
    sclk_off();
    PORTD &= ~VALUE_DATA; // data off

    // send cmd
    for (i = 0x80; i; i >>= 1) {
        _delay_us(1); // Tv >= 250 ns
        if (cmd & i)
            PORTD |= VALUE_DATA;
        else
            PORTD &= ~VALUE_DATA;

        _delay_us(1); // Tsu >= 100 ns
        PORTD |= VALUE_SCLK; // sclk on

        _delay_us(1); // Tclh >= 100 ns
        PORTD &= ~VALUE_SCLK; // sclk off
    }

    // check ACK condition during the 9th high pulse of sclk
    PORTD |= VALUE_DATA;
    DDRD &= ~VALUE_DATA; // set input
    _delay_us(1);

    sclk_on();
    i = PIND & VALUE_DATA;
    sclk_off();
    return i;
}

unsigned char
sht11_wait_ready(void) {
#define READ_RETRY 350
    unsigned int i;

    // wait until sht11 pulls data down
    for (i = 0; i < READ_RETRY; i++)
    {
        if (!(PIND & VALUE_DATA))
            break;
        _delay_ms(3);
    }
    return (i != READ_RETRY);
}

unsigned char
sht11_read_byte(unsigned char ack) { // ack: 0=go on, 1=end transmission
    unsigned char i, n;

    // read from bit7 to bit0
    n = 0;
    for (i = 0x80; i; i >>= 1) {
        sclk_on();
        if (PIND & VALUE_DATA)
            n |= i;
        sclk_off();
    }

    // send ack
    PORTD |= VALUE_DATA;
    DDRD |= VALUE_DATA; //set output
    _delay_us(1); // Tsu >= 100 ns

    if (ack)
        PORTD |= VALUE_DATA;
    else
        PORTD &= ~VALUE_DATA;
    _delay_us(1); // Tsu >= 100 ns

    PORTD |= VALUE_SCLK; // sclk on

    _delay_us(1); // Tclh >= 100 ns
    PORTD &= ~VALUE_SCLK; // sclk off

    PORTD |= VALUE_DATA;
    DDRD &= ~VALUE_DATA; // set input
    _delay_us(1);
    return n;
}

unsigned int
sht11_read_word(void) {
    unsigned int n;

    n = sht11_read_byte(0);
    n = (n << 8) + sht11_read_byte(1);
    return n;
}

void
sht11_hard_reset(void) {
    unsigned char i;

    for (i = 0; i < 10; i++)
    {
        PORTD |= VALUE_SCLK;
        PORTD &= ~VALUE_SCLK;
    }
}

void
sht11_init(void) {
    PORTD &= ~VALUE_SCLK;
    DDRD |= VALUE_SCLK; // sclk line is output and zero by default
    PORTD |= VALUE_DATA;
    DDRD &= ~VALUE_DATA; // data line is input
}
