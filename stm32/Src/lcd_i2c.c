/* lcd_i2c.c – HD44780 over PCF8574 (4-bit mode, HAL I2C)
 * PCF8574 bit map: P0=RS P1=RW P2=EN P3=BL P4=D4 P5=D5 P6=D6 P7=D7
 */
#include "lcd_i2c.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define RS 0x01
#define EN 0x04
#define BL 0x08

static I2C_HandleTypeDef *_hi2c;
static uint8_t _bl = BL;

static void i2c_write(uint8_t d)
{
    HAL_I2C_Master_Transmit(_hi2c, LCD_ADDR, &d, 1, 10);
}

static void lcd_nibble(uint8_t nib, uint8_t flags)
{
    uint8_t d = (nib & 0xF0) | flags | _bl;
    i2c_write(d | EN); HAL_Delay(1);
    i2c_write(d & ~EN); HAL_Delay(1);
}

static void lcd_byte(uint8_t byte, uint8_t flags)
{
    lcd_nibble(byte & 0xF0,        flags);
    lcd_nibble((byte << 4) & 0xF0, flags);
}

static void lcd_cmd (uint8_t c) { lcd_byte(c, 0);  }
static void lcd_data(uint8_t c) { lcd_byte(c, RS); }

void LCD_Init(I2C_HandleTypeDef *hi2c)
{
    _hi2c = hi2c;
    HAL_Delay(50);
    lcd_nibble(0x30, 0); HAL_Delay(5);
    lcd_nibble(0x30, 0); HAL_Delay(1);
    lcd_nibble(0x30, 0); HAL_Delay(1);
    lcd_nibble(0x20, 0); HAL_Delay(1);
    lcd_cmd(0x28); lcd_cmd(0x0C);
    lcd_cmd(0x06); lcd_cmd(0x01); HAL_Delay(2);
}

void LCD_Clear(void)          { lcd_cmd(0x01); HAL_Delay(2); }
void LCD_SetCursor(uint8_t col, uint8_t row)
{
    uint8_t offs[] = {0x00, 0x40};
    lcd_cmd(0x80 | (col + offs[row & 1]));
}
void LCD_Print(const char *s) { while (*s) lcd_data((uint8_t)*s++); }
void LCD_Printf(const char *fmt, ...)
{
    char buf[LCD_COLS + 1];
    va_list a; va_start(a, fmt);
    vsnprintf(buf, sizeof(buf), fmt, a);
    va_end(a);
    LCD_Print(buf);
}
