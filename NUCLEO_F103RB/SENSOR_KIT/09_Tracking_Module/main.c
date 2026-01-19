/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Line Tracking Module Test for STM32F103 NUCLEO
 * @author         : 
 * @date           : 2025
 ******************************************************************************
 * @description
 * 라인 트래킹(추적) 모듈 테스트 (TCRT5000 기반)
 * - 적외선 센서를 이용한 흑/백 라인 감지
 * - 단일 채널 디지털 출력
 * - 3핀 모듈 (VCC, GND, OUT)
 * 
 * @pinout
 * - PA0  : Sensor Output (Digital Input)
 * - PA5  : LED Indicator (Output)
 * - PA2  : USART2 TX (Debug)
 * - PA3  : USART2 RX (Debug)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private defines */
#define SENSOR_PIN          GPIO_PIN_0
#define SENSOR_PORT         GPIOA
#define LED_PIN             GPIO_PIN_5
#define LED_PORT            GPIOA

/* Sensor state macros */
#define LINE_DETECTED       0   // Active Low (흑색 라인 감지)
#define LINE_NOT_DETECTED   1   // 백색 바닥

/* Private variables */
UART_HandleTypeDef huart2;

/* Statistics */
uint32_t detect_count = 0;
uint32_t total_samples = 0;
uint32_t last_transition_time = 0;
uint8_t prev_state = LINE_NOT_DETECTED;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
uint8_t Read_Sensor(void);
void Print_Sensor_Bar(uint8_t state);

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

    printf("\r\n============================================\r\n");
    printf("  Line Tracking Sensor Test (Single Channel)\r\n");
    printf("  STM32F103 NUCLEO\r\n");
    printf("============================================\r\n");
    printf("Module: 3-Pin (VCC, GND, OUT)\r\n");
    printf("PA0: Sensor Output\r\n");
    printf("(0=Black Line, 1=White Surface)\r\n\r\n");

    uint8_t sensor_state;
    uint32_t last_print_time = 0;
    uint32_t stats_time = 0;

    while (1)
    {
        /* 센서 읽기 */
        sensor_state = Read_Sensor();
        total_samples++;

        /* 라인 감지 시 LED ON */
        if (sensor_state == LINE_DETECTED)
        {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
            detect_count++;
        }
        else
        {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        }

        /* 상태 전환 감지 */
        if (sensor_state != prev_state)
        {
            uint32_t current_time = HAL_GetTick();
            uint32_t duration = current_time - last_transition_time;
            
            printf("[TRANSITION] %s -> %s (after %lu ms)\r\n",
                   prev_state == LINE_DETECTED ? "BLACK" : "WHITE",
                   sensor_state == LINE_DETECTED ? "BLACK" : "WHITE",
                   duration);
            
            last_transition_time = current_time;
            prev_state = sensor_state;
        }

        /* 100ms마다 상태 출력 */
        if (HAL_GetTick() - last_print_time >= 100)
        {
            last_print_time = HAL_GetTick();

            printf("Sensor: [%s] ", 
                   sensor_state == LINE_DETECTED ? "BLACK ###" : "WHITE ---");
            Print_Sensor_Bar(sensor_state);
            printf("\r\n");
        }

        /* 5초마다 통계 출력 */
        if (HAL_GetTick() - stats_time >= 5000)
        {
            stats_time = HAL_GetTick();
            uint32_t detect_percent = (detect_count * 100) / total_samples;
            
            printf("\r\n--- Statistics (5s) ---\r\n");
            printf("Total samples: %lu\r\n", total_samples);
            printf("Line detected: %lu (%lu%%)\r\n", detect_count, detect_percent);
            printf("------------------------\r\n\r\n");
            
            detect_count = 0;
            total_samples = 0;
        }

        HAL_Delay(10);
    }
}

/**
 * @brief  Read Tracking Sensor
 * @retval LINE_DETECTED (0) or LINE_NOT_DETECTED (1)
 */
uint8_t Read_Sensor(void)
{
    return HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_PIN);
}

/**
 * @brief  Print visual sensor bar
 */
void Print_Sensor_Bar(uint8_t state)
{
    if (state == LINE_DETECTED)
    {
        printf("[##########]  <- LINE DETECTED");
    }
    else
    {
        printf("[----------]  <- NO LINE");
    }
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

    /* LED Pin - Output */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    /* Sensor Pin - Input with Pull-up */
    GPIO_InitStruct.Pin = SENSOR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SENSOR_PORT, &GPIO_InitStruct);
}

/**
 * @brief USART2 Initialization (115200 baud)
 */
static void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2;
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
```
