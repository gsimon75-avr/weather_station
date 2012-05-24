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

const unsigned char userchars[] =
{
    0x10,  0x10,  0x10,  0x10,  0x10,  0x10,  0x10,  0x10, 
    0x04,  0x04,  0x04,  0x04,  0x04,  0x04,  0x04,  0x04, 
    0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01, 
};

#define DEW_A 17.27
#define DEW_B 237.7

unsigned int SOt, SOrh;
double t, tdew, rhlin, rhtrue, dew_gamma;

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

int
main(void)
{
    char first_run;

    lcd_init();
    lcd_setup();

    lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x00);
    printf("init sht11");
    sht11_init();
    lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x00);
    printf("sht11 up  ");

    first_run = 1;
    while (1)
    {
        do {
            // send GET_TEMP
            while (sht11_send_byte(SHT11_CMD_GET_TEMP))
                ;

            // while measuring temperature, calculate RH & RHreal
            if (!first_run)
            {
                rhlin = -4 + (0.0405 * SOrh) - (2.8e-6 * SOrh * SOrh);
                rhtrue = rhlin + (t - 25)*(1e-2 + 8e-5 * SOrh);

                dew_gamma = DEW_A * t / (DEW_B + t) + log(rhtrue / 100.0);
                tdew = DEW_B * dew_gamma / (DEW_A - dew_gamma);
            }
        } while (!sht11_wait_ready());
        SOt = sht11_read_word(1); // read temperature

        do {
            // send GET_HUMIDITY
            while (sht11_send_byte(SHT11_CMD_GET_HUMIDITY))
                ;

            // while reading humidity, display results
            if (!first_run)
            {
                // calculate T
                t = -40 + 0.01 * SOt;
#ifdef SHT11TEST
                lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x00);
                printf("SOt=%u    ", SOt);
                lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x40);
                printf("SOrh=%u    ", SOrh);
#else
                lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x00);
                printf("%3.2f\xdf""C %3.2f%%", t, rhtrue);

                lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x40);
                printf("dew: %3.2f\xdf""C", tdew);
#endif
            }

            first_run = 0;

        } while (!sht11_wait_ready());
        SOrh = sht11_read_word(1); // read humidity
    }
    return(0);
}
#endif
