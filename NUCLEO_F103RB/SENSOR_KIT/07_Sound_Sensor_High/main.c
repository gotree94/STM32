/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : High Sensitivity Sound Sensor Module Test for STM32F103
 * @author         : 
 * @date           : 2025
 ******************************************************************************
 * @description
 * 고감도 소리센서 모듈 테스트 (KY-037 또는 유사 모듈)
 * - 디지털 출력(DO): 소리 감지 시 LED 점등
 * - 아날로그 출력(AO): ADC로 소리 크기 측정
 * - 가변저항으로 감도 조절 가능
 * 
 * @pinout
 * - PA0  : Analog Output (ADC1_IN0)
 * - PA1  : Digital Output (EXTI)
 * - PA5  : LED Indicator (Output)
 * - PA2  : USART2 TX (Debug)
 * - PA3  : USART2 RX (Debug)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private defines */
#define SOUND_AO_PIN        GPIO_PIN_0      // Analog Output
#define SOUND_AO_PORT       GPIOA
#define SOUND_DO_PIN        GPIO_PIN_1      // Digital Output
#define SOUND_DO_PORT       GPIOA
#define LED_PIN             GPIO_PIN_5
#define LED_PORT            GPIOA

#define ADC_THRESHOLD_LOW   500             // 조용한 환경
#define ADC_THRESHOLD_MED   1500            // 보통 소리
#define ADC_THRESHOLD_HIGH  2500            // 큰 소리

/* Private variables */
UART_HandleTypeDef huart2;
ADC_HandleTypeDef hadc1;
volatile uint8_t sound_detected = 0;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
uint16_t Read_Sound_ADC(void);
void Print_Sound_Level(uint16_t adc_value);
void Print_Sound_Bar(uint16_t adc_value);

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
    MX_ADC1_Init();

    printf("\r\n============================================\r\n");
    printf("  High Sensitivity Sound Sensor Test\r\n");
    printf("  STM32F103 NUCLEO\r\n");
    printf("============================================\r\n");
    printf("PA0: Analog Output (ADC)\r\n");
    printf("PA1: Digital Output (Threshold Detection)\r\n");
    printf("PA5: LED Indicator\r\n\r\n");

    uint16_t adc_value = 0;
    uint16_t adc_max = 0;
    uint16_t adc_min = 4095;
    uint32_t sample_count = 0;
    uint32_t last_print_time = 0;

    while (1)
    {
        /* ADC 읽기 (아날로그 출력) */
        adc_value = Read_Sound_ADC();
        
        /* 최대/최소값 추적 */
        if (adc_value > adc_max) adc_max = adc_value;
        if (adc_value < adc_min) adc_min = adc_value;
        sample_count++;

        /* 디지털 출력 읽기 (임계값 감지) */
        if (HAL_GPIO_ReadPin(SOUND_DO_PORT, SOUND_DO_PIN) == GPIO_PIN_RESET)
        {
            /* 소리 감지됨 (Active Low) */
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
            sound_detected = 1;
        }
        else
        {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        }

        /* 100ms마다 결과 출력 */
        if (HAL_GetTick() - last_print_time >= 100)
        {
            last_print_time = HAL_GetTick();
            
            printf("ADC: %4d | ", adc_value);
            Print_Sound_Bar(adc_value);
            Print_Sound_Level(adc_value);
            
            if (sound_detected)
            {
                printf(" [DETECTED!]");
                sound_detected = 0;
            }
            printf("\r\n");

            /* 1초마다 통계 출력 */
            if (sample_count >= 100)
            {
                printf("--- Stats: Min=%d, Max=%d, Range=%d ---\r\n", 
                       adc_min, adc_max, adc_max - adc_min);
                adc_max = 0;
                adc_min = 4095;
                sample_count = 0;
            }
        }

        HAL_Delay(10);
    }
}

/**
 * @brief  Read Sound Sensor ADC Value
 * @retval ADC value (0-4095)
 */
uint16_t Read_Sound_ADC(void)
{
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        return HAL_ADC_GetValue(&hadc1);
    }
    return 0;
}

/**
 * @brief  Print sound level description
 */
void Print_Sound_Level(uint16_t adc_value)
{
    if (adc_value < ADC_THRESHOLD_LOW)
    {
        printf(" [Quiet]");
    }
    else if (adc_value < ADC_THRESHOLD_MED)
    {
        printf(" [Normal]");
    }
    else if (adc_value < ADC_THRESHOLD_HIGH)
    {
        printf(" [Loud]");
    }
    else
    {
        printf(" [Very Loud!]");
    }
}

/**
 * @brief  Print sound level bar graph
 */
void Print_Sound_Bar(uint16_t adc_value)
{
    uint8_t bar_length = adc_value / 200;  // 0-20 범위
    if (bar_length > 20) bar_length = 20;
    
    printf("[");
    for (uint8_t i = 0; i < 20; i++)
    {
        if (i < bar_length)
            printf("#");
        else
            printf("-");
    }
    printf("]");
}

/**
 * @brief System Clock Configuration (72MHz)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;  // 12MHz ADC clock
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
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

    /* Sound DO Pin - Input */
    GPIO_InitStruct.Pin = SOUND_DO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SOUND_DO_PORT, &GPIO_InitStruct);
}

/**
 * @brief ADC1 Initialization
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

    sConfig.Channel = ADC_CHANNEL_0;  // PA0
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /* ADC Calibration */
    HAL_ADCEx_Calibration_Start(&hadc1);
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
