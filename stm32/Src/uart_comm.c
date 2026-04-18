/* uart_comm.c – interrupt-driven single-byte RX with ring buffer */
#include "uart_comm.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

void UART_Ch_Init(UART_Ch_t *ch, UART_HandleTypeDef *huart)
{
    ch->huart      = huart;
    ch->head       = 0;
    ch->tail       = 0;
    ch->line_ready = false;
    memset(ch->ring, 0, UART_RX_BUF_SIZE);
    memset(ch->line, 0, UART_RX_BUF_SIZE);
    HAL_UART_Receive_IT(huart, &ch->rx_byte, 1);
}

void UART_Ch_RxCallback(UART_Ch_t *ch)
{
    uint16_t next = (ch->head + 1) % UART_RX_BUF_SIZE;
    if (next != ch->tail) {
        ch->ring[ch->head] = ch->rx_byte;
        ch->head = next;
    }
    HAL_UART_Receive_IT(ch->huart, &ch->rx_byte, 1);
}

bool UART_Ch_GetLine(UART_Ch_t *ch)
{
    static uint16_t pos = 0;
    while (ch->tail != ch->head) {
        uint8_t b = ch->ring[ch->tail];
        ch->tail = (ch->tail + 1) % UART_RX_BUF_SIZE;
        if (b == '\r') continue;
        if (b == '\n') {
            ch->line[pos] = '\0';
            pos = 0;
            ch->line_ready = true;
            return true;
        }
        if (pos < UART_RX_BUF_SIZE - 1)
            ch->line[pos++] = (char)b;
    }
    return false;
}

void UART_SendLine(UART_HandleTypeDef *huart, const char *str)
{
    HAL_UART_Transmit(huart, (uint8_t *)str,    strlen(str), 100);
    HAL_UART_Transmit(huart, (uint8_t *)"\r\n", 2,           100);
}

void UART_SendLinef(UART_HandleTypeDef *huart, const char *fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    UART_SendLine(huart, buf);
}
