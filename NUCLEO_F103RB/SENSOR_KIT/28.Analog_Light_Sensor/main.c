/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Analog Light Sensor Module Test for NUCLEO-F103RB
 * @author         : 나무
 * @date           : 2025
 ******************************************************************************
 * @description
 * 아날로그 광센서(조도센서) 모듈 테스트 프로그램
 * - 센서: KY-018 Photoresistor Module (LDR/CdS)
 * - ADC를 통한 밝기 측정
 * - 조도 레벨에 따른 LED 제어 및 UART 출력
 *
 * @pinout
 * - PA0: Light Sensor Analog Output (ADC1_IN0)
 * - PA5: On-board LED (LD2)
 * - PA2: USART2 TX
 * - PA3: USART2 RX
 *
 * @connection
 * Light Module   NUCLEO-F103RB
 * -----------    -------------
 * VCC        ->  3.3V
 * GND        ->  GND
 * AO/OUT     ->  PA0 (A0)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Light Level Thresholds */
#define LIGHT_DARK      500     /* 어두움 임계값 */
#define LIGHT_DIM       1500    /* 흐림 임계값 */
#define LIGHT_NORMAL    2500    /* 보통 임계값 */
#define LIGHT_BRIGHT    3500    /* 밝음 임계값 */

/* ADC Parameters */
#define ADC_RESOLUTION  4095
#define VREF            3.3f

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
ADC_HandleTypeDef hadc1;

/* Light Level Enumeration */
typedef enum {
    LEVEL_DARK,
    LEVEL_DIM,
    LEVEL_NORMAL,
    LEVEL_BRIGHT,
    LEVEL_VERY_BRIGHT
} LightLevel;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void GPIO_Init(void);
static void USART2_Init(void);
static void ADC1_Init(void);
uint16_t ADC_Read(void);
uint16_t ADC_Read_Average(uint8_t samples);
LightLevel Get_Light_Level(uint16_t adc_value);
const char* Get_Level_String(LightLevel level);
uint8_t Calculate_Percentage(uint16_t adc_value);

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
    printf("  Analog Light Sensor Test Program\r\n");
    printf("  NUCLEO-F103RB\r\n");
    printf("========================================\r\n");
    printf("Sensor: Photoresistor (LDR/CdS)\r\n\n");
    
    uint16_t adc_value = 0;
    float voltage = 0.0f;
    uint8_t percentage = 0;
    LightLevel level = LEVEL_DARK;
    LightLevel prev_level = LEVEL_DARK;
    
    /* Progress bar buffer */
    char bar[21];
    
    while (1)
    {
        /* Read averaged ADC value (8 samples) */
        adc_value = ADC_Read_Average(8);
        
        /* Calculate voltage and percentage */
        voltage = (adc_value * VREF) / ADC_RESOLUTION;
        percentage = Calculate_Percentage(adc_value);
        
        /* Get light level */
        level = Get_Light_Level(adc_value);
        
        /* Create progress bar */
        uint8_t filled = percentage / 5;  /* 20 chars = 100% */
        for (int i = 0; i < 20; i++) {
            bar[i] = (i < filled) ? '#' : '-';
        }
        bar[20] = '\0';
        
        /* Print results */
        printf("\rADC:%4d | V:%.2fV | [%s] %3d%% | %s       ", 
               adc_value, voltage, bar, percentage, Get_Level_String(level));
        
        /* LED control based on light level */
        if (level <= LEVEL_DIM)
        {
            /* Dark environment - LED ON */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        }
        else
        {
            /* Bright environment - LED OFF */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        }
        
        /* Print change notification */
        if (level != prev_level)
        {
            printf("\r\n>> Light Level Changed: %s -> %s\r\n", 
                   Get_Level_String(prev_level), Get_Level_String(level));
            prev_level = level;
        }
        
        HAL_Delay(200);  /* 200ms 간격 측정 */
    }
}

/**
 * @brief  Read single ADC value
 * @retval 12-bit ADC value
 */
uint16_t ADC_Read(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}

/**
 * @brief  Read averaged ADC value
 * @param  samples: Number of samples to average
 * @retval Averaged ADC value
 */
uint16_t ADC_Read_Average(uint8_t samples)
{
    uint32_t sum = 0;
    
    for (uint8_t i = 0; i < samples; i++)
    {
        sum += ADC_Read();
    }
    
    return (uint16_t)(sum / samples);
}

/**
 * @brief  Get light level from ADC value
 * @param  adc_value: ADC reading
 * @retval LightLevel enumeration
 */
LightLevel Get_Light_Level(uint16_t adc_value)
{
    if (adc_value < LIGHT_DARK)      return LEVEL_DARK;
    if (adc_value < LIGHT_DIM)       return LEVEL_DIM;
    if (adc_value < LIGHT_NORMAL)    return LEVEL_NORMAL;
    if (adc_value < LIGHT_BRIGHT)    return LEVEL_BRIGHT;
    return LEVEL_VERY_BRIGHT;
}

/**
 * @brief  Get string representation of light level
 * @param  level: Light level enumeration
 * @retval String pointer
 */
const char* Get_Level_String(LightLevel level)
{
    switch (level)
    {
        case LEVEL_DARK:        return "DARK       ";
        case LEVEL_DIM:         return "DIM        ";
        case LEVEL_NORMAL:      return "NORMAL     ";
        case LEVEL_BRIGHT:      return "BRIGHT     ";
        case LEVEL_VERY_BRIGHT: return "VERY BRIGHT";
        default:                return "UNKNOWN    ";
    }
}

/**
 * @brief  Calculate brightness percentage
 * @param  adc_value: ADC reading
 * @retval Percentage (0-100)
 */
uint8_t Calculate_Percentage(uint16_t adc_value)
{
    uint32_t percent = (adc_value * 100) / ADC_RESOLUTION;
    return (percent > 100) ? 100 : (uint8_t)percent;
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

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
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
 */
static void ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
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
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
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
