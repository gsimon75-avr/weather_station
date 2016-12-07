#include "lcd.h"

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#if HD44780_BIT == 8
// Wiring:
// PB4          RS
// PB3          E
// PB2          R/#W
// PD7..0       DB7..0
#define RS          _BV(PB4)
#define E           _BV(PB3)
#define RW          _BV(PB2)
#elif HD44780_BIT == 4
// Wiring:
// PB1          RS
// PB0          E
// PB5..2       DB7..4
// PD7          R/#W
// NOTE: for 4-bit operation, bit0..3 must be pulled to 1 on the
// lcd device...
#define RS          _BV(PB1)
#define E           _BV(PB0)
#define RW          _BV(PD7)
#else
#error You have to define HD44780_BIT to one of {8, 4}
#endif

// NOTE: PB7,6 are used for external crystal
// PB3..4 can be used for ICSP as well


// T = width of E clk high/low states (> 250 ns) in cpu cycles
#if F_CPU < 4000000
#define T 1
#else
#define T (1 + ((F_CPU - 1) / 4000000))
#endif

void 
lcd_wait_busy(void) {
    uint8_t lcd_status;

#if HD44780_BIT == 8
    PORTB &= ~RS;
    DDRD = 0; // D[7..0] are inputs
    PORTB |= RW;

    do {
        _delay_loop_1(T);
        PORTB |= E;
        _delay_loop_1(T);
        lcd_status = PIND;
        PORTB &= ~E;
    } while (lcd_status & 0x80);

    PORTB &= ~RW;
    DDRD = 0xff; // D[7..0] are outputs
#elif HD44780_BIT == 4
    PORTB &= ~RS;
    DDRB &= 0xc3; // B[2..5] are inputs
    PORTD |= RW;

    do {
        _delay_loop_1(T);
        PORTB |= E;
        _delay_loop_1(T);
        lcd_status = (PINB << 2) & 0xf0;
        PORTB &= ~E;

        _delay_loop_1(T);
        PORTB |= E;
        _delay_loop_1(T);
        PORTB &= ~E;
    } while (lcd_status & 0x80);

    PORTD &= ~RW;
    DDRB |= 0x3c; // B[2..5] are outputs
#endif
}

static void
e_pulse(uint8_t c) {
#if HD44780_BIT == 8
    PORTD = c;
    _delay_loop_1(T);
    PORTB |= E;
    _delay_loop_1(T);
    PORTB &= ~E;
#elif HD44780_BIT == 4
    PORTB = c | E;
    _delay_loop_1(T);
    PORTB &= ~E;
    _delay_loop_1(T);
#endif
}

void
lcd_cmd(uint8_t n) {
    lcd_wait_busy();
#if HD44780_BIT == 8
    //PORTB &= ~RS; // lcd_wait_busy leaves RS at 0
    e_pulse(n);
#elif HD44780_BIT == 4
    _delay_loop_1(T);
    PORTB = ((n & 0xf0) >> 2) | E;   /* RS=0 */
    _delay_loop_1(T);
    PORTB &= ~E;
    
    _delay_loop_1(T);
    PORTB = ((n & 0x0f) << 2) | E;   /* RS=0 */
    _delay_loop_1(T);
    PORTB &= ~E;
#endif
}

void
lcd_data(uint8_t n) {
    lcd_wait_busy();
#if HD44780_BIT == 8
    PORTB |= RS;
    e_pulse(n);
#elif HD44780_BIT == 4
    _delay_loop_1(T);
    PORTB = ((n & 0xf0) >> 2) | RS | E;   /* RS=1 */
    _delay_loop_1(T);
    PORTB &= ~E;
    
    _delay_loop_1(T);
    PORTB = ((n & 0x0f) << 2) | RS | E;   /* RS=1 */
    _delay_loop_1(T);
    PORTB &= ~E;
#endif
}

int
lcdwrite(char c, FILE *f) {
    lcd_data(c);
    return 0;
}

void
lcd_init(void) {
#if HD44780_BIT == 8
    PORTB = 0;
    DDRB = RW | RS | E; // enable used PORT bits as output
    DDRD = 0xff;

    _delay_ms(15);  e_pulse(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT);
    _delay_ms(5);   e_pulse(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT);
    _delay_us(150); e_pulse(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT);
    _delay_us(150); // not specified

    lcd_cmd(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT | HD44780_2_LINES);
#elif HD44780_BIT == 4
    PORTB = 0;
    DDRB = 0x3c | RS | E; // enable used PORT bits as output
    PORTD &= ~RW; // PD7 is low
    DDRD |= RW; // enable PD7 as output

    _delay_ms(15);
    e_pulse((((HD44780_CMD_FUNCTION_SET | HD44780_8_BIT) >> 4) << 2)); /* RS=0=instrmode */
    _delay_ms(5); // > 4.1 ms
    e_pulse((((HD44780_CMD_FUNCTION_SET | HD44780_8_BIT) >> 4) << 2)); /* RS=0=instrmode */
    _delay_us(150); // > 100 us
    e_pulse((((HD44780_CMD_FUNCTION_SET | HD44780_8_BIT) >> 4) << 2)); /* RS=0=instrmode */
    _delay_us(150); // not specified
    e_pulse(((HD44780_CMD_FUNCTION_SET >> 4) << 2)); /* RS=0=instrmode */
    _delay_us(150); // not specified

    lcd_cmd(HD44780_CMD_FUNCTION_SET | HD44780_2_LINES);
#endif

    // let printf just write to the lcd :)
    stdout = fdevopen(lcdwrite, NULL);
}

