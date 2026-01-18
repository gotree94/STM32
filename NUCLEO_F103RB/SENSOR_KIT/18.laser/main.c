/**
 * @file    main.c
 * @brief   Laser Module Test for NUCLEO-F103RB
 * @author  Embedded Systems Lab
 * @date    2025
 * 
 * @details This program demonstrates how to control a laser module
 *          with STM32F103 microcontroller. Includes various operating
 *          modes: continuous, blink, and PWM dimming.
 * 
 * Hardware Connections:
 *   - Laser Module VCC  -> 5V (or 3.3V depending on module)
 *   - Laser Module GND  -> GND
 *   - Laser Module S    -> PA0 (PWM capable via TIM2_CH1)
 *   - User Button       -> PC13 (Built-in on NUCLEO board)
 *   - LED               -> PA5 (Built-in LED on NUCLEO board)
 * 
 * @warning NEVER point laser at eyes! Use proper laser safety precautions.
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines */
#define MODE_OFF        0
#define MODE_ON         1
#define MODE_BLINK      2
#define MODE_PWM        3
#define MODE_SOS        4
#define NUM_MODES       5

#define BLINK_PERIOD_MS 500
#define PWM_PERIOD      1000    /* PWM period (ARR value) */
#define PWM_STEP        100     /* PWM duty cycle step */

/* Private variables */
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;

volatile uint8_t current_mode = MODE_OFF;
volatile uint8_t button_pressed = 0;
volatile uint16_t pwm_duty = 500;  /* 50% duty cycle */
volatile uint8_t pwm_direction = 1;  /* 1: increasing, 0: decreasing */

/* SOS pattern: ... --- ... (3 short, 3 long, 3 short) */
const uint16_t sos_pattern[] = {200, 200, 200, 200, 200, 600, 
                                600, 200, 600, 200, 600, 600,
                                200, 200, 200, 200, 200, 1000};
const uint8_t sos_length = 18;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
void UART_SendString(char *str);
void Laser_On(void);
void Laser_Off(void);
void Laser_SetPWM(uint16_t duty);
void Run_Mode_Off(void);
void Run_Mode_On(void);
void Run_Mode_Blink(void);
void Run_Mode_PWM(void);
void Run_Mode_SOS(void);
void Print_Mode(void);

/**
 * @brief  Main program entry point
 * @retval int
 */
int main(void)
{
    /* HAL Initialization */
    HAL_Init();
    
    /* Configure the system clock */
    SystemClock_Config();
    
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_USART2_UART_Init();
    
    /* Start PWM */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    
    /* Welcome message */
    UART_SendString("\r\n========================================\r\n");
    UART_SendString("  Laser Module Test\r\n");
    UART_SendString("  NUCLEO-F103RB\r\n");
    UART_SendString("========================================\r\n");
    UART_SendString("WARNING: Never point laser at eyes!\r\n\r\n");
    UART_SendString("Modes (Press User Button to change):\r\n");
    UART_SendString("  0: OFF\r\n");
    UART_SendString("  1: ON (Continuous)\r\n");
    UART_SendString("  2: BLINK\r\n");
    UART_SendString("  3: PWM (Breathing)\r\n");
    UART_SendString("  4: SOS\r\n\r\n");
    
    /* Initial state */
    Laser_Off();
    Print_Mode();
    
    /* Main loop */
    while (1)
    {
        /* Check for button press (mode change) */
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
        {
            HAL_Delay(50);  /* Debounce */
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
            {
                /* Wait for button release */
                while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET);
                HAL_Delay(50);
                
                /* Change mode */
                current_mode = (current_mode + 1) % NUM_MODES;
                Print_Mode();
                
                /* Reset PWM for mode transitions */
                pwm_duty = 500;
                pwm_direction = 1;
            }
        }
        
        /* Execute current mode */
        switch (current_mode)
        {
            case MODE_OFF:
                Run_Mode_Off();
                break;
            case MODE_ON:
                Run_Mode_On();
                break;
            case MODE_BLINK:
                Run_Mode_Blink();
                break;
            case MODE_PWM:
                Run_Mode_PWM();
                break;
            case MODE_SOS:
                Run_Mode_SOS();
                break;
            default:
                Run_Mode_Off();
                break;
        }
    }
}

/**
 * @brief  Turn laser ON (100% duty)
 * @retval None
 */
void Laser_On(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, PWM_PERIOD);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

/**
 * @brief  Turn laser OFF (0% duty)
 * @retval None
 */
void Laser_Off(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief  Set laser PWM duty cycle
 * @param  duty: Duty cycle value (0-1000)
 * @retval None
 */
void Laser_SetPWM(uint16_t duty)
{
    if (duty > PWM_PERIOD) duty = PWM_PERIOD;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
    
    /* LED brightness indication */
    if (duty > 500)
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief  Mode OFF - Laser completely off
 * @retval None
 */
void Run_Mode_Off(void)
{
    Laser_Off();
    HAL_Delay(100);
}

/**
 * @brief  Mode ON - Laser continuously on
 * @retval None
 */
void Run_Mode_On(void)
{
    Laser_On();
    HAL_Delay(100);
}

/**
 * @brief  Mode BLINK - Simple on/off blinking
 * @retval None
 */
void Run_Mode_Blink(void)
{
    static uint32_t last_toggle = 0;
    static uint8_t state = 0;
    
    if (HAL_GetTick() - last_toggle >= BLINK_PERIOD_MS)
    {
        state = !state;
        if (state)
            Laser_On();
        else
            Laser_Off();
        last_toggle = HAL_GetTick();
    }
}

/**
 * @brief  Mode PWM - Breathing effect with PWM
 * @retval None
 */
void Run_Mode_PWM(void)
{
    Laser_SetPWM(pwm_duty);
    
    /* Update duty cycle for breathing effect */
    if (pwm_direction)
    {
        pwm_duty += PWM_STEP / 5;
        if (pwm_duty >= PWM_PERIOD)
        {
            pwm_duty = PWM_PERIOD;
            pwm_direction = 0;
        }
    }
    else
    {
        if (pwm_duty >= PWM_STEP / 5)
            pwm_duty -= PWM_STEP / 5;
        else
        {
            pwm_duty = 0;
            pwm_direction = 1;
        }
    }
    
    HAL_Delay(20);
}

/**
 * @brief  Mode SOS - Morse code SOS pattern
 * @retval None
 */
void Run_Mode_SOS(void)
{
    static uint8_t pattern_index = 0;
    static uint8_t laser_state = 0;
    
    if (laser_state == 0)
    {
        Laser_On();
        laser_state = 1;
    }
    else
    {
        Laser_Off();
        laser_state = 0;
    }
    
    HAL_Delay(sos_pattern[pattern_index]);
    pattern_index = (pattern_index + 1) % sos_length;
}

/**
 * @brief  Print current mode to UART
 * @retval None
 */
void Print_Mode(void)
{
    char msg[50];
    const char* mode_names[] = {"OFF", "ON", "BLINK", "PWM", "SOS"};
    
    sprintf(msg, "[MODE] Current: %s\r\n", mode_names[current_mode]);
    UART_SendString(msg);
}

/**
 * @brief  TIM2 Initialization Function (PWM)
 * @retval None
 */
static void MX_TIM2_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 71;          /* 72MHz / 72 = 1MHz */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = PWM_PERIOD - 1;  /* 1MHz / 1000 = 1kHz PWM */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim2);
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
    
    /* Configure PA0 for TIM2_CH1 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
    
    /* Configure PA5 as output (LED) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure PC13 as input (User Button) */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  /* External pull-up on NUCLEO */
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
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
