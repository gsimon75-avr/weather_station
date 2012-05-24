#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include "lcd.h"

// NOTE: for 4-bit operation, bit0..3 must be pulled to 1 on the
// lcd device...
//
// Wiring:
// PB1          RS
// PB0          E
// PB5..2       DB7..4
//      GND     R/#W
// Vcc -4k7-    DB3..0  (PULL THEM UP OR IT WON'T WORK!)

// NOTE: PB7,6 are used for external crystal
// PB3..4 can be used for ICSP as well

const unsigned char hexchar[] = 
{
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

void
delay_ms(unsigned int ms)
{
  while(ms--)
    _delay_ms(0.96);
}

static void e_pulse(unsigned char c) {
  PORTB = c | 0x01;   /* E=0->1 */
  _delay_us(10); //_NOP();
  PORTB = c; /* E=1->0 */
  _delay_us(10);
}

void
lcd_cmd(unsigned char cmd)
{
  e_pulse((cmd >> 4) << 2); /* RS=0=instrmode */
  // perhaps some delay here, 1 us or so...
  e_pulse((cmd & 0x0f) << 2);
  _delay_ms(2);
}

void
lcd_data(unsigned char n)
{
  e_pulse(((n >> 4) << 2) | 0x02); /* RS=1=datamode */
  // perhaps some delay here, 1 us or so...
  e_pulse(((n & 0x0f) << 2) | 0x02);
  _delay_us(50);
}

static void
lcd_cmd_byte(unsigned char cmd)
{
  e_pulse(((cmd >> 4) << 2)); /* RS=0=instrmode */
  // perhaps some delay here, 1 us or so...
}

void
lcd_init(void)
{
  PORTB = 0xc0;
  DDRB = 0x3f; // enable used PORT bits as output
  _delay_ms(15);
  lcd_cmd_byte(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT);
  lcd_cmd_byte(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT);
  lcd_cmd_byte(HD44780_CMD_FUNCTION_SET | HD44780_8_BIT);
  lcd_cmd_byte(HD44780_CMD_FUNCTION_SET);
  lcd_cmd(HD44780_CMD_FUNCTION_SET | HD44780_2_LINES);
}

void
lcd_debug(const unsigned char *p)
{
  int n;
  
  lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x40);
  for (n = 0; n < 16; n++)
    lcd_data(' ');
  lcd_cmd(HD44780_CMD_SET_DDRAM_ADDR | 0x40);
  while (*p)
    {
      lcd_data(*p);
      p++;
    }
  delay_ms(50);
}
