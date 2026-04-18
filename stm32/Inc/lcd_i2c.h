/* lcd_i2c.h – HD44780 over PCF8574 I2C (HAL) */
#ifndef __LCD_I2C_H
#define __LCD_I2C_H

#include "stm32f1xx_hal.h"

#define LCD_ADDR (0x27 << 1)   /* shift for HAL – try 0x3F<<1 if blank */
#define LCD_COLS 16
#define LCD_ROWS  2

void LCD_Init     (I2C_HandleTypeDef *hi2c);
void LCD_Clear    (void);
void LCD_SetCursor(uint8_t col, uint8_t row);
void LCD_Print    (const char *str);
void LCD_Printf   (const char *fmt, ...);

#endif
