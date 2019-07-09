#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdio.h>
#include <math.h>
#include "lcd.h"
#include "sht11.h"
#include "bt_serial.h"

#define TIMER1_FREQ 10

volatile uint32_t tick = 0;
volatile uint8_t progress = 0;
volatile uint16_t SOt, SOrh;
double t, rhlin, rhtrue;

char progress_char[] = { '|', '/', '-', '\\' };

//uint8_t req[] = "AT+VERSION";

uint8_t resp[2];
int recvcount = 0;

uint8_t sbuf[0x20];

void
lcd_setup(void) {
    lcd_cmd(HD44780_CMD_DISPLAY_CONTROL);
    lcd_cmd(HD44780_CMD_ENTRY_MODE_SET | HD44780_INCREMENT_CURSOR);
    lcd_cmd(HD44780_CMD_SHIFT_CONTROL);
    lcd_cmd(HD44780_CMD_CLEAR_DISPLAY);
    lcd_cmd(HD44780_CMD_DISPLAY_CONTROL | HD44780_DISPLAY_ON);
}

ISR(TIMER1_COMPA_vect) {
    tick++;
}

int
main(void)
{
    sei();
    lcd_init();
    lcd_setup();

    // set up timer1 for a TIMER1_FREQ interrupt
#define F_CPU_1      (F_CPU / 1)
#define F_CPU_8      (F_CPU / 8)
#define F_CPU_64     (F_CPU / 64)
#define F_CPU_256    (F_CPU / 256)
#define F_CPU_1024   (F_CPU / 1024)

#if TIMER1_FREQ > F_CPU
#   error TIMER1 > F_CPU!
#elif TIMER1_FREQ > (F_CPU_1 / 65536)
    TCCR1B = _BV(WGM12) | 1; // ctc mode up to OCR1A, clk = sysclk / 1
    OCR1A = F_CPU_1 / TIMER1_FREQ;
#elif TIMER1_FREQ > (F_CPU_8 / 65536)
    TCCR1B = _BV(WGM12) | 2; // ctc mode up to OCR1A, clk = sysclk / 1
    OCR1A = F_CPU_8 / TIMER1_FREQ;
#elif TIMER1_FREQ > (F_CPU_64 / 65536)
    TCCR1B = _BV(WGM12) | 3; // ctc mode up to OCR1A, clk = sysclk / 1
    OCR1A = F_CPU_64 / TIMER1_FREQ;
#elif TIMER1_FREQ > (F_CPU_256 / 65536)
    TCCR1B = _BV(WGM12) | 4; // ctc mode up to OCR1A, clk = sysclk / 1
    OCR1A = F_CPU_256 / TIMER1_FREQ;
#elif TIMER1_FREQ > (F_CPU_1024 / 65536)
    TCCR1B = _BV(WGM12) | 5; // ctc mode up to OCR1A, clk = sysclk / 1
    OCR1A = F_CPU_1024 / TIMER1_FREQ;
#else
#   error TIMER1 too low
#endif
    TCNT1 = 0; // timer reset
    TIMSK1 = _BV(OCIE1A);

    DDRC |= _BV(PC5);

    set_sleep_mode(SLEEP_MODE_IDLE);
    lcd_cmd(HD44780_CMD_CLEAR_DISPLAY);
    printf("lcd ok");

    sht11_init();
    lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR);
    printf("sensor ok");

    bt_serial_init();
    //bt_serial_set_recv_buffer(resp, sizeof(resp), &respptr);
    //bt_serial_send(req, sizeof(req));
    lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR);
    printf("serial ok");

    bt_serial_recv(resp, 1);
    while (1) {
        sleep_mode();

        // ---- sensor code begins ----
        do {
            while (sht11_send_byte(SHT11_CMD_GET_TEMP))
                ;
        } while (!sht11_wait_ready());
        SOt = sht11_read_word(); // read temperature

        do {
            while (sht11_send_byte(SHT11_CMD_GET_HUMIDITY))
                ;
        } while (!sht11_wait_ready());
        SOrh = sht11_read_word(); // read humidity

        t = -40.1 + 0.01 * SOt;
        rhlin = -2.0468 + (0.0367 * SOrh) + (-1.5955e-6 * SOrh * SOrh);
        rhtrue = rhlin + (t - 25)*(1e-2 + 8e-5 * SOrh);
        // ---- sensor code ends ----

        // ----
        PORTC ^= _BV(PC5);
        progress = (progress + 1) & 0x03;
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR);
        printf("%3.2f\xb2""C %3.2f%% %c", t, rhtrue, progress_char[progress]); // degree sign is sometimes \xdf, depends on the display

        if (bt_serial_send_len == 0) {
            sprintf(sbuf, "%3.2f'C %3.2f%% %c\r\n", t, rhtrue, progress_char[progress]); // degree sign is sometimes \xdf, depends on the display
            bt_serial_send(sbuf, 18);
        }

        if (bt_serial_recv_len == 0) {
            lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR + 0x40 + recvcount);
            lcd_data(resp[0]);
            recvcount = (recvcount + 1) & 0x0f;
            bt_serial_recv(resp, 1);
        }
    }
    return 0;
}
