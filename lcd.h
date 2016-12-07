#ifndef LCD_H
#define LCD_H

#include <stdint.h>
/*                                  dontcare, applicable modifiers */
#define HD44780_CMD_CLEAR_DISPLAY       0x01
#define HD44780_CMD_CURSOR_HOME         0x02 /* 0x01 - */
#define HD44780_CMD_ENTRY_MODE_SET      0x04 /* -    INCREMENT_CURSOR, DISPLAY_SHIFT */
#define HD44780_CMD_DISPLAY_CONTROL     0x08 /* 0x03 DISPLAY_ON, CURSOR_ON, BLINK*/
#define HD44780_CMD_SHIFT_CONTROL       0x10 /* 0x03 SHIFT_DISPLAY, SHIFT_LEFT */
#define HD44780_CMD_FUNCTION_SET        0x20 /* 0x03 8_BIT, 2_LINES, 10_DOTS */
#define HD44780_CMD_SET_CGRAM_ADDR      0x40 /* -    lower 6 bits = addr */
#define HD44780_CMD_SET_DDRAM_ADDR      0x80 /* -    lower 7 bits = addr */

#define HD44780_INCREMENT_CURSOR        0x02
#define HD44780_DISPLAY_SHIFT           0x01

#define HD44780_DISPLAY_ON              0x04
#define HD44780_CURSOR_ON               0x02
#define HD44780_BLINK_ON                0x01

#define HD44780_SHIFT_DISPLAY           0x08
#define HD44780_SHIFT_LEFT              0x04

#define HD44780_8_BIT                   0x10
#define HD44780_2_LINES                 0x08
#define HD44780_10_DOT                  0x04

#define HD44780_BUSY                    0x80

void lcd_init(void);
void lcd_cmd(uint8_t cmd);
void lcd_data(uint8_t n);
#endif
