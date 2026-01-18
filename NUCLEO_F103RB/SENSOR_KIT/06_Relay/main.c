/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Relay Module Test for STM32F103 NUCLEO
 * @author         : 
 * @date           : 2025
 ******************************************************************************
 * @description
 * 릴레이 모듈 제어 테스트
 * - 1초 간격으로 릴레이 ON/OFF 토글
 * - 버튼(B1)으로 릴레이 수동 제어
 * 
 * @pinout
 * - PA5  : Relay Signal (Output)
 * - PC13 : User Button B1 (Input, Active Low)
 * - PA2  : USART2 TX (Debug)
 * - PA3  : USART2 RX (Debug)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private defines */
#define RELAY_PIN           GPIO_PIN_5
#define RELAY_PORT          GPIOA
#define USER_BUTTON_PIN     GPIO_PIN_13
#define USER_BUTTON_PORT    GPIOC

/* Private variables */
UART_HandleTypeDef huart2;
volatile uint8_t relay_state = 0;
volatile uint8_t button_pressed = 0;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void Relay_On(void);
void Relay_Off(void);
void Relay_Toggle(void);

/* Printf redirect */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  Main program
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    printf("\r\n========================================\r\n");
    printf("  Relay Module Test - STM32F103 NUCLEO\r\n");
    printf("========================================\r\n");
    printf("PA5: Relay Signal Output\r\n");
    printf("PC13: User Button (Manual Control)\r\n\r\n");

    uint32_t last_toggle_time = 0;
    uint32_t toggle_interval = 1000;  // 1초 간격

    while (1)
    {
        /* 자동 토글 모드 (1초 간격) */
        if (HAL_GetTick() - last_toggle_time >= toggle_interval)
        {
            last_toggle_time = HAL_GetTick();
            Relay_Toggle();
            printf("Relay State: %s\r\n", relay_state ? "ON" : "OFF");
        }

        /* 버튼 수동 제어 */
        if (HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN) == GPIO_PIN_RESET)
        {
            HAL_Delay(50);  // Debounce
            if (HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN) == GPIO_PIN_RESET)
            {
                if (!button_pressed)
                {
                    button_pressed = 1;
                    Relay_Toggle();
                    printf("[BUTTON] Relay State: %s\r\n", relay_state ? "ON" : "OFF");
                }
            }
        }
        else
        {
            button_pressed = 0;
        }

        HAL_Delay(10);
    }
}

/**
 * @brief  Relay ON
 */
void Relay_On(void)
{
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);
    relay_state = 1;
}

/**
 * @brief  Relay OFF
 */
void Relay_Off(void)
{
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
    relay_state = 0;
}

/**
 * @brief  Relay Toggle
 */
void Relay_Toggle(void)
{
    HAL_GPIO_TogglePin(RELAY_PORT, RELAY_PIN);
    relay_state = !relay_state;
}

/**
 * @brief System Clock Configuration (72MHz)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * @brief GPIO Initialization
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Relay Pin - Output */
    GPIO_InitStruct.Pin = RELAY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RELAY_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);

    /* User Button - Input */
    GPIO_InitStruct.Pin = USER_BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USER_BUTTON_PORT, &GPIO_InitStruct);
}

/**
 * @brief USART2 Initialization (115200 baud)
 */
static void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart2);
}
