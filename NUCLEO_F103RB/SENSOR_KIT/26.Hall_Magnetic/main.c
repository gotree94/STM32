/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Hall Magnetic Sensor Module Test for NUCLEO-F103RB
 * @author         : 나무
 * @date           : 2025
 ******************************************************************************
 * @description
 * 홀 마그네틱 센서 모듈 테스트 프로그램
 * - 센서: KY-003 Hall Magnetic Sensor Module (또는 호환 모듈)
 * - 자석 감지 시 디지털 출력 (LOW/HIGH)
 * - LED 상태 표시 및 UART 출력
 *
 * @pinout
 * - PA0: Hall Sensor Digital Output (GPIO Input)
 * - PA5: On-board LED (LD2)
 * - PA2: USART2 TX (ST-Link Virtual COM)
 * - PA3: USART2 RX
 *
 * @connection
 * Hall Module    NUCLEO-F103RB
 * -----------    -------------
 * VCC        ->  3.3V
 * GND        ->  GND
 * DO/OUT     ->  PA0 (A0)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void GPIO_Init(void);
static void USART2_Init(void);
void UART_SendString(char *str);

/* Redirect printf to UART ---------------------------------------------------*/
#ifdef __GNUC__
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

/**
 * @brief  Main program
 * @retval int
 */
int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    GPIO_Init();
    USART2_Init();
    
    /* Welcome message */
    printf("\r\n========================================\r\n");
    printf("  Hall Magnetic Sensor Test Program\r\n");
    printf("  NUCLEO-F103RB\r\n");
    printf("========================================\r\n\n");
    
    uint8_t prev_state = 1;
    uint8_t curr_state = 1;
    uint32_t detect_count = 0;
    
    while (1)
    {
        /* Read Hall sensor state */
        curr_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        
        /* Detect state change */
        if (curr_state != prev_state)
        {
            if (curr_state == GPIO_PIN_RESET)
            {
                /* Magnet detected (sensor output LOW) */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                detect_count++;
                printf("[%lu] Magnet DETECTED! (Sensor: LOW)\r\n", detect_count);
            }
            else
            {
                /* Magnet removed (sensor output HIGH) */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                printf("[%lu] Magnet REMOVED  (Sensor: HIGH)\r\n", detect_count);
            }
            prev_state = curr_state;
        }
        
        HAL_Delay(10);  /* Debounce delay */
    }
}

/**
 * @brief System Clock Configuration
 *        - HSI 8MHz -> PLL -> 64MHz System Clock
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * @brief GPIO Initialization
 *        - PA0: Input (Hall Sensor)
 *        - PA5: Output (LED)
 */
static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Configure PA0 as Input (Hall Sensor) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure PA5 as Output (LED) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* LED off by default */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief USART2 Initialization
 *        - 115200 baud, 8N1
 */
static void USART2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable Clocks */
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Configure PA2 (TX), PA3 (RX) */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* USART2 Configuration */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}

/**
 * @brief Send string via UART
 */
void UART_SendString(char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}
