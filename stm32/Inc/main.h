#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef  hi2c1;

/* RGB LED pins */
#define LED_R_PIN    GPIO_PIN_1   /* PA1 — Red   */
#define LED_R_PORT   GPIOA
#define LED_G_PIN    GPIO_PIN_0   /* PB0 — Green */
#define LED_G_PORT   GPIOB
#define LED_B_PIN    GPIO_PIN_1   /* PB1 — Blue  */
#define LED_B_PORT   GPIOB

void Error_Handler(void);

#ifdef __cplusplus
}
#endif
#endif
