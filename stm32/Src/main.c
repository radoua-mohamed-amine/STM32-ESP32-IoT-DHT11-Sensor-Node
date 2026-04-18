#include "main.h"
#include "uart_comm.h"
#include "lcd_i2c.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Peripheral handles ─────────────────────────────────────────────── */
UART_HandleTypeDef huart1;
I2C_HandleTypeDef  hi2c1;

/* ── UART channel ───────────────────────────────────────────────────── */
UART_Ch_t ch_esp;

/* ── Application state ──────────────────────────────────────────────── */
static float s_temp = 0.0f;
static float s_hum  = 0.0f;

/* ── Forward declarations ───────────────────────────────────────────── */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);

static void parse_json(const char *line);
static void update_leds(float temp);
static void update_lcd(void);

/* ════════════════════════════════════════════════════════════════════ */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();

    UART_Ch_Init(&ch_esp, &huart1);

    LCD_Init(&hi2c1);
    LCD_Clear();
    LCD_SetCursor(0, 0); LCD_Print("STM32 ready");
    LCD_SetCursor(0, 1); LCD_Print("Wait ESP32...");

    while (1)
    {
        if (UART_Ch_GetLine(&ch_esp))
        {
            parse_json(ch_esp.line);
            ch_esp.line_ready = false;

            update_leds(s_temp);
            update_lcd();

            UART_SendLine(&huart1, "ACK");
        }
    }
}

/* ── Parse JSON: {"t":24.5,"h":62.0} ───────────────────────── */
static void parse_json(const char *line)
{
    const char *pt = strstr(line, "\"t\":");
    const char *ph = strstr(line, "\"h\":");

    if (pt) s_temp = strtof(pt + 4, NULL);
    if (ph) s_hum  = strtof(ph + 4, NULL);
}

/* ── RGB control ───────────────────────────────────────────── */
static void rgb_set(uint8_t r, uint8_t g, uint8_t b)
{
    HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, r ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_G_PORT, LED_G_PIN, g ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_B_PORT, LED_B_PIN, b ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void update_leds(float temp)
{
    if      (temp < 20.0f) rgb_set(0, 0, 1);   /* Blue  */
    else if (temp < 30.0f) rgb_set(0, 1, 0);   /* Green */
    else                   rgb_set(1, 0, 0);   /* Red   */
}

/* ── LCD update ────────────────────────────────────────────── */
static void update_lcd(void)
{
    LCD_SetCursor(0, 0);
    LCD_Printf("Temp: %.1f%cC   ", s_temp, 0xDF);

    LCD_SetCursor(0, 1);
    LCD_Printf("Hum:  %.0f %%   ", s_hum);
}

/* ── UART RX callback ──────────────────────────────────────── */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
        UART_Ch_RxCallback(&ch_esp);
}

/* ── Clock config ──────────────────────────────────────────── */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState        = RCC_HSE_ON;
    osc.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
    osc.PLL.PLLState    = RCC_PLL_ON;
    osc.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL      = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();

    clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                       | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

/* ── UART init ─────────────────────────────────────────────── */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

/* ── I2C init ──────────────────────────────────────────────── */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance             = I2C1;
    hi2c1.Init.ClockSpeed      = 100000;
    hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1     = 0;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

/* ── GPIO init (FIXED) ─────────────────────────────────────── */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef g = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    g.Pull  = GPIO_NOPULL;

    /* Red LED (PA1) */
    g.Pin = LED_R_PIN;
    HAL_GPIO_Init(LED_R_PORT, &g);

    /* Green + Blue (PB0, PB1) */
    g.Pin = LED_G_PIN | LED_B_PIN;
    HAL_GPIO_Init(LED_G_PORT, &g);

    /* All OFF */
    HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_G_PORT, LED_G_PIN | LED_B_PIN, GPIO_PIN_RESET);
}

/* ── Error handler ─────────────────────────────────────────── */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
