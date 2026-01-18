/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Linear Hall Sensor Module Test for STM32F103 NUCLEO Board
 * @author         : Embedded Systems Lab
 * @date           : 2025
 ******************************************************************************
 * @attention
 *
 * Linear Hall Sensor Module Interface:
 * - DO (Digital Output) -> PA0 (Digital Input, Active LOW when magnetic field detected)
 * - AO (Analog Output)  -> PA1 (ADC1_IN1, varies with magnetic field strength)
 * - VCC -> 3.3V or 5V
 * - GND -> GND
 *
 * The linear Hall sensor (e.g., 49E) outputs a voltage proportional to the
 * magnetic field strength. The analog output varies around Vcc/2 at rest,
 * increasing/decreasing based on magnetic pole orientation.
 *
 * UART2 is used for debug output (connected to ST-Link Virtual COM Port)
 * - TX: PA2, RX: PA3, Baudrate: 115200
 *
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private variables */
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;

/* Calibration values */
#define ADC_CENTER_VALUE    2048    // Center value at no magnetic field
#define ADC_THRESHOLD       200     // Threshold for magnetic detection

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);

/* Printf redirect to UART */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  Read digital output from hall sensor
 * @retval 1 if magnetic field detected, 0 otherwise
 */
uint8_t Hall_ReadDigital(void) {
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) ? 1 : 0;
}

/**
 * @brief  Read analog value from hall sensor
 * @retval ADC value (0-4095)
 */
uint16_t Hall_ReadAnalog(void) {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        return HAL_ADC_GetValue(&hadc1);
    }
    return 0xFFFF;
}

/**
 * @brief  Get averaged ADC reading for stability
 * @param  samples: Number of samples to average
 * @retval Averaged ADC value
 */
uint16_t Hall_ReadAnalogAvg(uint8_t samples) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++) {
        sum += Hall_ReadAnalog();
        HAL_Delay(2);
    }
    return (uint16_t)(sum / samples);
}

/**
 * @brief  Determine magnetic pole based on ADC value
 * @param  adc_value: Current ADC reading
 * @retval 1 for North pole, -1 for South pole, 0 for no significant field
 */
int8_t Hall_GetPolarity(uint16_t adc_value) {
    int16_t deviation = (int16_t)adc_value - ADC_CENTER_VALUE;
    
    if (deviation > ADC_THRESHOLD) {
        return 1;   // North pole (or South depending on sensor orientation)
    } else if (deviation < -ADC_THRESHOLD) {
        return -1;  // South pole (or North depending on sensor orientation)
    }
    return 0;       // No significant magnetic field
}

/**
 * @brief  Convert ADC value to approximate Gauss
 * @param  adc_value: Current ADC reading
 * @retval Magnetic field strength in approximate Gauss
 */
int16_t Hall_GetGauss(uint16_t adc_value) {
    // 49E Hall sensor: ~1.3mV/Gauss
    // 3.3V / 4096 = 0.8mV per ADC count
    // Approximate conversion factor
    int16_t deviation = (int16_t)adc_value - ADC_CENTER_VALUE;
    return (int16_t)(deviation * 0.6f);  // Approximate Gauss value
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();

    printf("\r\n========================================\r\n");
    printf("  Linear Hall Sensor Module Test Program\r\n");
    printf("  NUCLEO-F103RB Board\r\n");
    printf("========================================\r\n\n");
    printf("DO Pin: PA0 (Digital Input)\r\n");
    printf("AO Pin: PA1 (ADC1_IN1)\r\n");
    printf("Center ADC Value: %d\r\n\n", ADC_CENTER_VALUE);

    // Calibration phase
    printf("Calibrating... Remove magnets from sensor\r\n");
    HAL_Delay(2000);
    uint16_t calibration_value = Hall_ReadAnalogAvg(10);
    printf("Calibration complete. Rest value: %d\r\n\n", calibration_value);

    uint16_t adc_value;
    uint8_t digital_state;
    int8_t polarity;
    int16_t gauss;
    uint16_t min_value = 4095;
    uint16_t max_value = 0;

    while (1) {
        // Read sensor values
        adc_value = Hall_ReadAnalogAvg(5);
        digital_state = Hall_ReadDigital();
        polarity = Hall_GetPolarity(adc_value);
        gauss = Hall_GetGauss(adc_value);

        // Track min/max
        if (adc_value < min_value) min_value = adc_value;
        if (adc_value > max_value) max_value = adc_value;

        // Print status
        printf("ADC: %4d | ", adc_value);
        printf("Gauss: %+5d | ", gauss);
        printf("Digital: %d | ", digital_state);

        // Visual bar graph
        int16_t deviation = (int16_t)adc_value - ADC_CENTER_VALUE;
        printf("[");
        for (int i = -10; i <= 10; i++) {
            if (i == 0) {
                printf("|");
            } else if ((deviation > 0 && i > 0 && i <= deviation / 100) ||
                       (deviation < 0 && i < 0 && i >= deviation / 100)) {
                printf("=");
            } else {
                printf(" ");
            }
        }
        printf("] ");

        // Polarity indicator
        switch (polarity) {
            case 1:
                printf("NORTH (S approaching)");
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                break;
            case -1:
                printf("SOUTH (N approaching)");
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                break;
            default:
                printf("No field");
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        }

        printf("\r\n");

        // Periodic min/max report
        static uint32_t report_counter = 0;
        if (++report_counter >= 20) {
            printf("--- Range: Min=%d, Max=%d, Span=%d ---\r\n", 
                   min_value, max_value, max_value - min_value);
            report_counter = 0;
        }

        HAL_Delay(200);
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

/**
 * @brief GPIO Initialization Function
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PA0 - Hall sensor DO (Digital Input)
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA5 - Onboard LED (Output)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief ADC1 Initialization Function
 * @retval None
 */
static void MX_ADC1_Init(void) {
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

    sConfig.Channel = ADC_CHANNEL_1;  // PA1
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADCEx_Calibration_Start(&hadc1);
}

/**
 * @brief USART2 Initialization Function
 * @retval None
 */
static void MX_USART2_UART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART2_CLK_ENABLE();

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

void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
