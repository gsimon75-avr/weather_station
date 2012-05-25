#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdio.h>
#include <math.h>
#include "lcd.h"
#include "sht11.h"

//#define HWTEST    1
//#define SHT11TEST   1

const uint8_t userchars[] =
{
    0x1f,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x1f,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x1f,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x1f,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x1f,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x1f,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x1f,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x1f, 
};

#define DEW_A 17.27
#define DEW_B 237.7

uint16_t SOt, SOrh;
double t, tdew, rhlin, rhtrue, dew_gamma;
uint8_t phase = 0;

int
lcdwrite(char c, FILE *f) {
    lcd_data(c);
    return 0;
}

void
lcd_setup(void) {
    int i;

    // lcd setup
    lcd_cmd(HD44780_CMD_DISPLAY_CONTROL);
    lcd_cmd(HD44780_CMD_ENTRY_MODE_SET | HD44780_INCREMENT_CURSOR);
    lcd_cmd(HD44780_CMD_SHIFT_CONTROL);
    lcd_cmd(HD44780_CMD_CLEAR_DISPLAY);
    lcd_cmd(HD44780_CMD_DISPLAY_CONTROL | HD44780_DISPLAY_ON);
    //lcd_cmd(HD44780_CMD_DISPLAY_CONTROL | HD44780_DISPLAY_ON | HD44780_CURSOR_ON | HD44780_BLINK_ON);

    // set up the progress indicator chars to 0..4
    lcd_cmd(HD44780_CMD_SET_CGRAM_ADDR | 0x00);
    for (i = 0; i < sizeof(userchars); i++)
        lcd_data(userchars[i]);
    lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x00);

    // let printf just write to the lcd :)
    stdout = fdevopen(lcdwrite, NULL);
}

#ifdef HWTEST
int
main(void)
{
    uint32_t n = 0;

    sei();
    lcd_init();
    lcd_setup();

    while (1) {
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x00);
        printf("%lu", n++);
    }

    return(0);
}

#else

void
disp_phase(void) {
    if (phase < 8) {
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x0f);
        lcd_data(phase);
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x4f);
        lcd_data(' ');
    }
    else if (phase < 16) {
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x0f);
        lcd_data(' ');
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x4f);
        lcd_data(phase - 8);
    }
    else if (phase < 23) {
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x0f);
        lcd_data(' ');
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x4f);
        lcd_data(22 - phase);
    }
    else if (phase < 30) {
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x0f);
        lcd_data(30 - phase);
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x4f);
        lcd_data(' ');
    }
    phase = (phase + 1) % 30;
}

int
main(void)
{
    DDRD |= 0x40;
    PORTD |= 0x40;

    sei();
    lcd_init();
    lcd_setup();
    sht11_init();

    while (1)
    {
        PORTD ^= 0x40;
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
        dew_gamma = DEW_A * t / (DEW_B + t) + log(rhtrue / 100.0);
        tdew = DEW_B * dew_gamma / (DEW_A - dew_gamma);

#ifdef SHT11TEST
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x00);
        printf("t=%5u", SOt);
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x40);
        printf("h=%5u", SOrh);
#else
        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x00);
        printf("%3.2f\xdf""C %3.2f%%", t, rhtrue);

        lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x40);
        printf("dew: %3.2f\xdf""C", tdew);
#endif
        disp_phase();
    }
    return(0);
}
#endif
