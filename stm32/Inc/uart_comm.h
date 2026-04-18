/* uart_comm.h – Non-blocking UART RX (interrupt + ring buffer) */
#ifndef __UART_COMM_H
#define __UART_COMM_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define UART_RX_BUF_SIZE 128

typedef struct {
    UART_HandleTypeDef *huart;
    uint8_t  rx_byte;
    uint8_t  ring[UART_RX_BUF_SIZE];
    uint16_t head, tail;
    char     line[UART_RX_BUF_SIZE];
    bool     line_ready;
} UART_Ch_t;

void UART_Ch_Init      (UART_Ch_t *ch, UART_HandleTypeDef *huart);
void UART_Ch_RxCallback(UART_Ch_t *ch);
bool UART_Ch_GetLine   (UART_Ch_t *ch);
void UART_SendLine     (UART_HandleTypeDef *huart, const char *str);
void UART_SendLinef    (UART_HandleTypeDef *huart, const char *fmt, ...);

#endif
