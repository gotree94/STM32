/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Temperature Sensor Module Test for NUCLEO-F103RB
 * @author         : 나무
 * @date           : 2025
 ******************************************************************************
 * @description
 * 아날로그 온도 센서 모듈 테스트 프로그램
 * - 센서: KY-013 NTC Thermistor Module 또는 LM35/TMP36
 * - ADC를 통한 아날로그 값 읽기
 * - 온도 변환 및 UART 출력
 *
 * @pinout
 * - PA0: Temperature Sensor Analog Output (ADC1_IN0)
 * - PA5: On-board LED (LD2)
 * - PA2: USART2 TX
 * - PA3: USART2 RX
 *
 * @connection
 * Temp Module    NUCLEO-F103RB
 * -----------    -------------
 * VCC        ->  3.3V
 * GND        ->  GND
 * AO/OUT     ->  PA0 (A0)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* NTC Thermistor Parameters (KY-013) */
#define THERMISTOR_NOMINAL    10000   /* 25도에서의 저항값 (10K) */
#define TEMPERATURE_NOMINAL   25      /* 기준 온도 */
#define B_COEFFICIENT         3950    /* B 상수 */
#define SERIES_RESISTOR       10000   /* 직렬 저항값 (10K) */

/* ADC Parameters */
#define ADC_RESOLUTION        4095    /* 12-bit ADC */
#define VREF                  3.3f    /* 기준 전압 */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
ADC_HandleTypeDef hadc1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void GPIO_Init(void);
static void USART2_Init(void);
static void ADC1_Init(void);
uint16_t ADC_Read(void);
float Get_Temperature_NTC(uint16_t adc_value);
float Get_Temperature_LM35(uint16_t adc_value);

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
    ADC1_Init();
    
    /* Welcome message */
    printf("\r\n========================================\r\n");
    printf("  Temperature Sensor Test Program\r\n");
    printf("  NUCLEO-F103RB\r\n");
    printf("========================================\r\n");
    printf("Sensor Type: NTC Thermistor (KY-013)\r\n\n");
    
    uint16_t adc_value = 0;
    float temperature = 0.0f;
    float voltage = 0.0f;
    
    while (1)
    {
        /* Read ADC value */
        adc_value = ADC_Read();
        
        /* Calculate voltage */
        voltage = (adc_value * VREF) / ADC_RESOLUTION;
        
        /* Calculate temperature (NTC method) */
        temperature = Get_Temperature_NTC(adc_value);
        
        /* Print results */
        printf("ADC: %4d | Voltage: %.2fV | Temp: %.1f°C\r\n", 
               adc_value, voltage, temperature);
        
        /* Temperature warning LED */
        if (temperature > 30.0f)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        }
        
        HAL_Delay(1000);  /* 1초 간격 측정 */
    }
}

/**
 * @brief  Read ADC value
 * @retval 12-bit ADC value (0-4095)
 */
uint16_t ADC_Read(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}

/**
 * @brief  Calculate temperature from NTC thermistor
 * @param  adc_value: ADC reading
 * @retval Temperature in Celsius
 * @note   Uses Steinhart-Hart equation (simplified B-parameter form)
 */
float Get_Temperature_NTC(uint16_t adc_value)
{
    float resistance;
    float steinhart;
    
    /* Prevent division by zero */
    if (adc_value == 0) return -273.15f;
    
    /* Calculate thermistor resistance */
    /* Voltage divider: Vout = Vcc * R_thermistor / (R_series + R_thermistor) */
    resistance = SERIES_RESISTOR * ((float)adc_value / (ADC_RESOLUTION - adc_value));
    
    /* Steinhart-Hart equation (B-parameter form) */
    /* 1/T = 1/T0 + (1/B) * ln(R/R0) */
    steinhart = resistance / THERMISTOR_NOMINAL;        /* R/R0 */
    steinhart = log(steinhart);                          /* ln(R/R0) */
    steinhart /= B_COEFFICIENT;                          /* 1/B * ln(R/R0) */
    steinhart += 1.0f / (TEMPERATURE_NOMINAL + 273.15f); /* + 1/T0 */
    steinhart = 1.0f / steinhart;                        /* Invert */
    steinhart -= 273.15f;                                /* Convert to Celsius */
    
    return steinhart;
}

/**
 * @brief  Calculate temperature from LM35 sensor
 * @param  adc_value: ADC reading
 * @retval Temperature in Celsius
 * @note   LM35 outputs 10mV/°C
 */
float Get_Temperature_LM35(uint16_t adc_value)
{
    float voltage = (adc_value * VREF) / ADC_RESOLUTION;
    return voltage * 100.0f;  /* 10mV/°C = 100°C/V */
}

/**
 * @brief System Clock Configuration
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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

    /* ADC Clock Configuration */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;  /* 64MHz/6 = 10.67MHz */
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

/**
 * @brief GPIO Initialization
 */
static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Configure PA5 as Output (LED) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief ADC1 Initialization
 *        - PA0 (ADC1_IN0)
 *        - 12-bit resolution
 */
static void ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable Clocks */
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Configure PA0 as Analog Input */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* ADC1 Configuration */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);
    
    /* Channel Configuration */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    /* ADC Calibration */
    HAL_ADCEx_Calibration_Start(&hadc1);
}

/**
 * @brief USART2 Initialization
 */
static void USART2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
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
