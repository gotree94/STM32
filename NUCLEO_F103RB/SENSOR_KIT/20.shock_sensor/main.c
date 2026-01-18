/**
 * @file    main.c
 * @brief   Shock/Vibration Sensor Module Test for NUCLEO-F103RB
 * @author  Embedded Systems Lab
 * @date    2025
 * 
 * @details This program demonstrates how to interface a shock/vibration
 *          sensor module with STM32F103 microcontroller. The sensor detects
 *          physical impacts and vibrations using both digital and analog outputs.
 * 
 * Hardware Connections:
 *   - Shock Sensor VCC  -> 3.3V
 *   - Shock Sensor GND  -> GND
 *   - Shock Sensor DO   -> PA0 (Digital Output - Interrupt)
 *   - Shock Sensor AO   -> PA1 (Analog Output - ADC, if available)
 *   - LED               -> PA5 (Built-in LED on NUCLEO board)
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines */
#define SHOCK_THRESHOLD     100     /* ADC threshold for shock detection */
#define COOLDOWN_MS         200     /* Minimum time between shock events */
#define SENSITIVITY_LEVELS  5       /* Number of sensitivity levels */

/* Private variables */
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;

volatile uint32_t shock_count = 0;
volatile uint32_t last_shock_time = 0;
volatile uint8_t shock_detected = 0;
volatile uint16_t max_shock_value = 0;

/* Shock intensity history */
#define HISTORY_SIZE 10
volatile uint16_t shock_history[HISTORY_SIZE] = {0};
volatile uint8_t history_index = 0;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_EXTI_Init(void);
void UART_SendString(char *str);
uint16_t Read_ADC(void);
const char* Get_Intensity_String(uint16_t adc_value);
void Print_Shock_Event(uint16_t intensity);
void Print_Statistics(void);

/**
 * @brief  Main program entry point
 * @retval int
 */
int main(void)
{
    char msg[100];
    uint32_t last_status_time = 0;
    uint16_t adc_value = 0;
    uint8_t digital_state = 0;
    uint8_t prev_digital_state = 1;
    
    /* HAL Initialization */
    HAL_Init();
    
    /* Configure the system clock */
    SystemClock_Config();
    
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();
    MX_EXTI_Init();
    
    /* Welcome message */
    UART_SendString("\r\n========================================\r\n");
    UART_SendString("  Shock/Vibration Sensor Module Test\r\n");
    UART_SendString("  NUCLEO-F103RB\r\n");
    UART_SendString("========================================\r\n");
    UART_SendString("Tap or shake the sensor to detect impacts.\r\n");
    UART_SendString("Intensity levels: Light, Medium, Strong, Very Strong, Extreme\r\n\r\n");
    
    /* Initial state */
    UART_SendString("[READY] Waiting for shock events...\r\n\r\n");
    
    /* Main loop */
    while (1)
    {
        /* Read digital output (for modules with DO) */
        digital_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        
        /* Digital trigger detection */
        if (digital_state == GPIO_PIN_RESET && prev_digital_state == GPIO_PIN_SET)
        {
            uint32_t current_time = HAL_GetTick();
            
            /* Cooldown check to prevent multiple triggers */
            if ((current_time - last_shock_time) >= COOLDOWN_MS)
            {
                /* Read analog value for intensity */
                adc_value = Read_ADC();
                
                /* Update statistics */
                shock_count++;
                last_shock_time = current_time;
                
                /* Track max value */
                if (adc_value > max_shock_value)
                {
                    max_shock_value = adc_value;
                }
                
                /* Store in history */
                shock_history[history_index] = adc_value;
                history_index = (history_index + 1) % HISTORY_SIZE;
                
                /* Visual feedback */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                
                /* Print event */
                Print_Shock_Event(adc_value);
            }
        }
        else if (digital_state == GPIO_PIN_SET)
        {
            /* Turn off LED when no shock */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        }
        
        prev_digital_state = digital_state;
        
        /* Periodic analog monitoring (every 100ms) */
        static uint32_t last_analog_read = 0;
        if (HAL_GetTick() - last_analog_read >= 100)
        {
            adc_value = Read_ADC();
            
            /* Check for analog threshold trigger */
            if (adc_value > SHOCK_THRESHOLD)
            {
                uint32_t current_time = HAL_GetTick();
                if ((current_time - last_shock_time) >= COOLDOWN_MS)
                {
                    shock_count++;
                    last_shock_time = current_time;
                    
                    if (adc_value > max_shock_value)
                    {
                        max_shock_value = adc_value;
                    }
                    
                    shock_history[history_index] = adc_value;
                    history_index = (history_index + 1) % HISTORY_SIZE;
                    
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                    Print_Shock_Event(adc_value);
                    HAL_Delay(50);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                }
            }
            
            last_analog_read = HAL_GetTick();
        }
        
        /* Print statistics every 10 seconds */
        if (HAL_GetTick() - last_status_time >= 10000)
        {
            Print_Statistics();
            last_status_time = HAL_GetTick();
        }
        
        HAL_Delay(5);
    }
}

/**
 * @brief  Read ADC value from shock sensor
 * @retval ADC conversion result (0-4095)
 */
uint16_t Read_ADC(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}

/**
 * @brief  Get intensity description string
 * @param  adc_value: ADC reading
 * @retval Intensity description string
 */
const char* Get_Intensity_String(uint16_t adc_value)
{
    if (adc_value < 500)        return "Light";
    else if (adc_value < 1000)  return "Medium";
    else if (adc_value < 2000)  return "Strong";
    else if (adc_value < 3000)  return "Very Strong";
    else                        return "EXTREME";
}

/**
 * @brief  Print shock event details
 * @param  intensity: ADC value representing shock intensity
 * @retval None
 */
void Print_Shock_Event(uint16_t intensity)
{
    char msg[100];
    const char* level = Get_Intensity_String(intensity);
    
    sprintf(msg, "[SHOCK #%lu] Intensity: %s (ADC: %d)\r\n", 
            shock_count, level, intensity);
    UART_SendString(msg);
    
    /* Visual bar graph */
    UART_SendString("         |");
    int bars = intensity / 200;  /* Scale to 20 max bars */
    if (bars > 20) bars = 20;
    for (int i = 0; i < bars; i++)
    {
        UART_SendString("█");
    }
    UART_SendString("|\r\n\r\n");
}

/**
 * @brief  Print shock statistics
 * @retval None
 */
void Print_Statistics(void)
{
    char msg[100];
    
    UART_SendString("─────────── Statistics ───────────\r\n");
    sprintf(msg, "  Total shocks: %lu\r\n", shock_count);
    UART_SendString(msg);
    sprintf(msg, "  Max intensity: %d (ADC)\r\n", max_shock_value);
    UART_SendString(msg);
    
    /* Calculate average from history */
    uint32_t sum = 0;
    uint8_t count = 0;
    for (int i = 0; i < HISTORY_SIZE; i++)
    {
        if (shock_history[i] > 0)
        {
            sum += shock_history[i];
            count++;
        }
    }
    
    if (count > 0)
    {
        sprintf(msg, "  Avg intensity: %lu (last %d events)\r\n", 
                sum / count, count);
        UART_SendString(msg);
    }
    
    UART_SendString("──────────────────────────────────\r\n\r\n");
}

/**
 * @brief  ADC1 Initialization Function
 * @retval None
 */
static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);
    
    /* Configure ADC Channel 1 (PA1) */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    /* Calibrate ADC */
    HAL_ADCEx_Calibration_Start(&hadc1);
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
    
    /* Configure PA0 as input with pull-up (Digital Output from sensor) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure PA1 as analog input (Analog Output from sensor) */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
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
 * @brief  External Interrupt Initialization (Optional)
 * @retval None
 */
static void MX_EXTI_Init(void)
{
    /* Enable AFIO clock for EXTI */
    __HAL_RCC_AFIO_CLK_ENABLE();
    
    /* Note: For interrupt-based detection, reconfigure PA0 */
    /* This is optional - the main loop handles polling */
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
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    
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
    
    /* Configure ADC Clock (max 14MHz) */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

/**
 * @brief  SysTick Handler
 * @retval None
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
