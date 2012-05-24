#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include "sht11.h"

// Wiring:
// PB7          Data (sht11)
// PB6          SClk (sht11)

// temp&humidity sensor lines
#define VALUE_DATA              _BV(PB7)
#define VALUE_SCLK              _BV(PB6)

static void
sclk_on(void)
{
  PORTB |= VALUE_SCLK;
  _delay_us(1); // Tclh >= 100 ns
}

static void
sclk_off(void)
{
  PORTB &= ~VALUE_SCLK;
  _delay_us(1); // Tv >= 250 ns
}

static void
set_input(void)
{
  DDRB &= ~VALUE_DATA;
  PORTB |= VALUE_DATA;
  _delay_us(1);
}

static void
set_output(void)
{
  PORTB |= VALUE_DATA;
  DDRB |= VALUE_DATA;
  _delay_us(1); // Tsu >= 100 ns
}

static void
send_bit(unsigned char n)
{
  if (n)
    PORTB |= VALUE_DATA;
  else
    PORTB &= ~VALUE_DATA;
  _delay_us(1); // Tsu >= 100 ns

  sclk_on();
  sclk_off();
}

static unsigned char
read_bit(void)
{
  unsigned char n;

  sclk_on();
  n = PINB & VALUE_DATA;
  sclk_off();
  return n;
}


unsigned char
sht11_send_byte(unsigned char cmd) // NOTE: bit7..5 of cmd is in fact unit address
{
  unsigned char i;

  // Transmission Start
  set_output();
  sclk_on();
  PORTB &= ~VALUE_DATA; // data off
  sclk_off();
  sclk_on();
  PORTB |= VALUE_DATA; // data on
  sclk_off();

  // send address(3 bits), cmd(5 bits)
  for (i = 0x80; i; i >>= 1)
    send_bit(cmd & i);

  // check ACK condition during the 9th high pulse of sclk
  set_input();
  return read_bit();
}

unsigned char
sht11_wait_ready(void)
{
#define READ_RETRY 350
  unsigned int i;

  // wait until sht11 pulls data down
  for (i = 0; i < READ_RETRY; i++)
    {
      if (!(PINB & VALUE_DATA))
        break;
      _delay_ms(1);
    }
  return (i != READ_RETRY);
}

unsigned char
sht11_read_byte(unsigned char ack) // ack: 0=go on, 1=end transmission
{
  unsigned char i, n;

  // read from bit7 to bit0
  n = 0;
  for (i = 0x80; i; i >>= 1)
    {
      if (read_bit())
        n |= i;
    }

  // send ack
  set_output();
  send_bit(ack);
  set_input();
  return n;
}

unsigned int
sht11_read_word(unsigned char ack)
{
  unsigned int n;

  n = sht11_read_byte(0);
  n = (n << 8) + sht11_read_byte(ack);
  return n;
}

void
sht11_hard_reset(void)
{
  unsigned char i;

  for (i = 0; i < 10; i++)
    {
      PORTB |= VALUE_SCLK;
      PORTB &= ~VALUE_SCLK;
    }
}

void
sht11_init(void)
{
  DDRB |= VALUE_SCLK; // sclk line is output
  PORTB |= VALUE_DATA;
  DDRB &= ~VALUE_DATA; // data line is input
}
