/**
 * @file    main.c
 * @brief   Heartbeat/Pulse Sensor Module Test for NUCLEO-F103RB
 * @author  Embedded Systems Lab
 * @date    2025
 * 
 * @details This program demonstrates how to interface a heartbeat/pulse sensor
 *          module with STM32F103 microcontroller. The sensor outputs an analog
 *          signal proportional to heartbeat, which is read via ADC.
 * 
 * Hardware Connections:
 *   - Pulse Sensor VCC  -> 3.3V (or 5V depending on module)
 *   - Pulse Sensor GND  -> GND
 *   - Pulse Sensor S    -> PA0 (ADC1_IN0)
 *   - LED               -> PA5 (Built-in LED on NUCLEO board)
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines */
#define SAMPLE_RATE_MS      10      /* Sampling interval in ms */
#define THRESHOLD_HIGH      2200    /* Upper threshold for beat detection */
#define THRESHOLD_LOW       1800    /* Lower threshold for beat detection */
#define MIN_BEAT_INTERVAL   300     /* Minimum ms between beats (200 BPM max) */
#define MAX_BEAT_INTERVAL   2000    /* Maximum ms between beats (30 BPM min) */
#define BPM_AVERAGE_COUNT   5       /* Number of beats to average */

/* Private variables */
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;

volatile uint16_t adc_value = 0;
volatile uint8_t beat_detected = 0;
volatile uint32_t last_beat_time = 0;
volatile uint32_t beat_intervals[BPM_AVERAGE_COUNT] = {0};
volatile uint8_t interval_index = 0;
volatile uint16_t bpm = 0;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
void UART_SendString(char *str);
uint16_t Read_ADC(void);
uint16_t Calculate_BPM(void);
void Detect_Heartbeat(uint16_t value);

/**
 * @brief  Main program entry point
 * @retval int
 */
int main(void)
{
    char msg[100];
    uint32_t last_print_time = 0;
    uint16_t current_adc = 0;
    uint8_t above_threshold = 0;
    
    /* HAL Initialization */
    HAL_Init();
    
    /* Configure the system clock */
    SystemClock_Config();
    
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();
    
    /* Welcome message */
    UART_SendString("\r\n========================================\r\n");
    UART_SendString("  Heartbeat/Pulse Sensor Module Test\r\n");
    UART_SendString("  NUCLEO-F103RB\r\n");
    UART_SendString("========================================\r\n");
    UART_SendString("Place your finger on the sensor...\r\n");
    UART_SendString("Wait for stable readings (10-20 sec)\r\n\r\n");
    
    /* Calibration delay */
    HAL_Delay(1000);
    
    /* Main loop */
    while (1)
    {
        /* Read ADC value */
        current_adc = Read_ADC();
        
        /* Detect heartbeat using threshold crossing */
        if (!above_threshold && current_adc > THRESHOLD_HIGH)
        {
            above_threshold = 1;
            
            uint32_t current_time = HAL_GetTick();
            uint32_t interval = current_time - last_beat_time;
            
            /* Valid beat detection */
            if (interval > MIN_BEAT_INTERVAL && interval < MAX_BEAT_INTERVAL)
            {
                /* Store interval for averaging */
                beat_intervals[interval_index] = interval;
                interval_index = (interval_index + 1) % BPM_AVERAGE_COUNT;
                
                /* Calculate BPM */
                bpm = Calculate_BPM();
                
                /* Visual feedback - blink LED */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                
                /* Print beat detection */
                sprintf(msg, "[BEAT] ADC: %4d | Interval: %4lu ms | BPM: %3d\r\n", 
                        current_adc, interval, bpm);
                UART_SendString(msg);
            }
            
            last_beat_time = current_time;
        }
        else if (above_threshold && current_adc < THRESHOLD_LOW)
        {
            above_threshold = 0;
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        }
        
        /* Print raw data periodically (every 500ms) */
        if (HAL_GetTick() - last_print_time >= 500)
        {
            sprintf(msg, "[RAW] ADC Value: %4d | Current BPM: %3d\r\n", 
                    current_adc, bpm);
            UART_SendString(msg);
            last_print_time = HAL_GetTick();
        }
        
        HAL_Delay(SAMPLE_RATE_MS);
    }
}

/**
 * @brief  Read ADC value
 * @retval ADC conversion result (0-4095)
 */
uint16_t Read_ADC(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}

/**
 * @brief  Calculate BPM from stored intervals
 * @retval Calculated BPM value
 */
uint16_t Calculate_BPM(void)
{
    uint32_t sum = 0;
    uint8_t count = 0;
    
    for (int i = 0; i < BPM_AVERAGE_COUNT; i++)
    {
        if (beat_intervals[i] > 0)
        {
            sum += beat_intervals[i];
            count++;
        }
    }
    
    if (count > 0)
    {
        uint32_t avg_interval = sum / count;
        return (uint16_t)(60000 / avg_interval);  /* Convert to BPM */
    }
    
    return 0;
}

/**
 * @brief  ADC1 Initialization Function
 * @retval None
 */
static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    /* Enable ADC1 Clock */
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    /* Configure ADC */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);
    
    /* Configure ADC Channel */
    sConfig.Channel = ADC_CHANNEL_0;
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
    
    /* Configure PA0 as analog input (ADC) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
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
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;  /* 72MHz/6 = 12MHz */
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
