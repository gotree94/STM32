/**
 * @file    main.c
 * @brief   Mini Reed Switch Module Test for NUCLEO-F103RB
 * @author  Embedded Systems Lab
 * @date    2025
 * 
 * @details This program demonstrates how to interface a mini reed switch
 *          module with STM32F103 microcontroller. The reed switch detects
 *          magnetic field presence and triggers an interrupt.
 * 
 * Hardware Connections:
 *   - Reed Module VCC  -> 3.3V
 *   - Reed Module GND  -> GND
 *   - Reed Module DO   -> PA0 (Digital Output)
 *   - LED              -> PA5 (Built-in LED on NUCLEO board)
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private variables */
UART_HandleTypeDef huart2;
volatile uint8_t reed_state = 0;
volatile uint8_t reed_changed = 0;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void UART_SendString(char *str);

/**
 * @brief  Main program entry point
 * @retval int
 */
int main(void)
{
    char msg[100];
    uint8_t prev_state = 0;
    
    /* HAL Initialization */
    HAL_Init();
    
    /* Configure the system clock */
    SystemClock_Config();
    
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    
    /* Welcome message */
    UART_SendString("\r\n========================================\r\n");
    UART_SendString("  Mini Reed Switch Module Test\r\n");
    UART_SendString("  NUCLEO-F103RB\r\n");
    UART_SendString("========================================\r\n");
    UART_SendString("Bring a magnet close to the sensor...\r\n\r\n");
    
    /* Initial state read */
    reed_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    prev_state = reed_state;
    
    if (reed_state == GPIO_PIN_RESET)
    {
        UART_SendString("[INIT] Magnet DETECTED (Reed Switch CLOSED)\r\n");
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    }
    else
    {
        UART_SendString("[INIT] No magnet detected (Reed Switch OPEN)\r\n");
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    }
    
    /* Main loop */
    while (1)
    {
        /* Read current reed switch state */
        reed_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        
        /* Check for state change */
        if (reed_state != prev_state)
        {
            HAL_Delay(50);  /* Debounce delay */
            reed_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
            
            if (reed_state != prev_state)
            {
                if (reed_state == GPIO_PIN_RESET)
                {
                    UART_SendString("[EVENT] Magnet DETECTED - Reed Switch CLOSED\r\n");
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                }
                else
                {
                    UART_SendString("[EVENT] Magnet REMOVED - Reed Switch OPEN\r\n");
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                }
                prev_state = reed_state;
            }
        }
        
        HAL_Delay(10);
    }
}

/**
 * @brief  GPIO Initialization Function
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* Configure PA0 as input with pull-up (Reed Switch DO) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure PA5 as output (LED) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Initialize LED to OFF */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief  USART2 Initialization Function
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* USART2 GPIO Configuration: PA2 -> TX, PA3 -> RX */
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
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}

/**
 * @brief  Send string via UART
 * @param  str: String to send
 * @retval None
 */
void UART_SendString(char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * @brief  System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* Configure HSE Oscillator */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    /* Configure CPU, AHB and APB clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/**
 * @brief  SysTick Handler
 * @retval None
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
