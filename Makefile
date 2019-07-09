PROJECT=station

CC=avr-gcc
OC=avr-objcopy
AD=avrdude

CFLAGS=-g -mmcu=atmega168 -DF_CPU=14318000UL -Os -Wall -Wstrict-prototypes -mcall-prologues -DHD44780_BIT=4
LDFLAGS=-g -mmcu=atmega168 -lprintf_flt -lm -uvfprintf

ADFLAGS=-c stk500v2 -p m168 -P /dev/ttyUSB0
OCFLAGS=-j .text -j .data -O ihex 

all:		$(PROJECT).hex

.PHONY:		all help clean install

all:		$(PROJECT).hex

clean:
		rm -f *.o *.map *.elf *.hex *.out

install:	$(PROJECT).hex
		$(AD) $(ADFLAGS) -e -U flash:w:$^

%.o:		%.c 
		$(CC) $(CFLAGS) -c $^

station.elf:	station.o lcd.o sht11.o
		$(CC) $(LDFLAGS) -o $@ $^
#"$(TOOLPATHWIN)\\avr\\lib\\libm.a" "$(TOOLPATHWIN)\\avr\\lib\\libprintf_flt.a"

%.hex:		%.elf 
	    	$(OC) $(OCFLAGS) $^ $@

# vim: set sw=8 ts=8 noet:
