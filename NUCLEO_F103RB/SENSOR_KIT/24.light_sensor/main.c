/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : 조도 센서 모듈 테스트 (Light Sensor / Photoresistor Module)
 * @board          : NUCLEO-F103RB
 * @sensor         : KY-018 Photoresistor Module (LDR)
 ******************************************************************************
 * @description
 * 조도 센서 모듈은 CdS(황화카드뮴) 광저항을 이용하여 주변 밝기를 측정합니다.
 * 빛이 밝으면 저항이 낮아지고, 어두우면 저항이 높아집니다.
 * ADC를 통해 아날로그 값을 읽어 조도를 측정합니다.
 * 
 * @pinout
 * - Module S (Signal) -> PA0 (ADC1_CH0)
 * - Module +          -> 3.3V
 * - Module -          -> GND
 * 
 * @note
 * DMA를 사용한 연속 ADC 변환으로 안정적인 측정
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
#define ADC_RESOLUTION      4096    /* 12-bit ADC */
#define SAMPLES_TO_AVG      16      /* 평균 샘플 수 */

/* 조도 레벨 임계값 (ADC 값 기준, 밝으면 값이 낮음) */
#define LIGHT_VERY_BRIGHT   500
#define LIGHT_BRIGHT        1500
#define LIGHT_NORMAL        2500
#define LIGHT_DIM           3200
#define LIGHT_DARK          3800

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
UART_HandleTypeDef huart2;

volatile uint16_t adc_buffer[SAMPLES_TO_AVG];
volatile uint8_t adc_complete = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);

uint16_t Get_Average_ADC(void);
const char* Get_Light_Level_String(uint16_t adc_value);
void Print_Light_Bar(uint16_t adc_value);

/* Printf redirect -----------------------------------------------------------*/
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* DMA Complete Callback -----------------------------------------------------*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        adc_complete = 1;
    }
}

/* Main program --------------------------------------------------------------*/
int main(void) {
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();
    
    printf("\r\n========================================\r\n");
    printf("  Light Sensor (LDR) Module Test\r\n");
    printf("  Board: NUCLEO-F103RB\r\n");
    printf("========================================\r\n\n");
    printf("ADC Resolution: 12-bit (0-4095)\r\n");
    printf("Samples averaged: %d\r\n\n", SAMPLES_TO_AVG);
    
    /* ADC Calibration */
    HAL_ADCEx_Calibration_Start(&hadc1);
    
    /* Start DMA-based ADC conversion */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLES_TO_AVG);
    
    uint16_t adc_value = 0;
    uint16_t prev_adc_value = 0;
    uint32_t last_print_time = 0;
    
    while (1) {
        if (adc_complete) {
            adc_complete = 0;
            
            /* 평균값 계산 */
            adc_value = Get_Average_ADC();
            
            /* 500ms마다 또는 큰 변화 시 출력 */
            if (HAL_GetTick() - last_print_time >= 500 ||
                (adc_value > prev_adc_value + 100) ||
                (prev_adc_value > adc_value + 100)) {
                
                printf("\r\n[Light Sensor Reading]\r\n");
                printf("  ADC Value: %4d / %d\r\n", adc_value, ADC_RESOLUTION - 1);
                printf("  Voltage  : %.2f V\r\n", (float)adc_value * 3.3f / ADC_RESOLUTION);
                printf("  Level    : %s\r\n", Get_Light_Level_String(adc_value));
                printf("  ");
                Print_Light_Bar(adc_value);
                printf("\r\n");
                
                last_print_time = HAL_GetTick();
                prev_adc_value = adc_value;
            }
            
            /* LED 밝기에 따른 제어 */
            if (adc_value > LIGHT_DIM) {
                /* 어두우면 LED ON */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            } else {
                /* 밝으면 LED OFF */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            }
            
            /* 다음 DMA 변환 시작 */
            HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLES_TO_AVG);
        }
        
        HAL_Delay(10);
    }
}

/**
 * @brief Calculate average ADC value from DMA buffer
 */
uint16_t Get_Average_ADC(void) {
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES_TO_AVG; i++) {
        sum += adc_buffer[i];
    }
    return (uint16_t)(sum / SAMPLES_TO_AVG);
}

/**
 * @brief Convert ADC value to light level string
 */
const char* Get_Light_Level_String(uint16_t adc_value) {
    if (adc_value < LIGHT_VERY_BRIGHT) return "Very Bright ☀☀";
    if (adc_value < LIGHT_BRIGHT)      return "Bright ☀";
    if (adc_value < LIGHT_NORMAL)      return "Normal 🌤";
    if (adc_value < LIGHT_DIM)         return "Dim 🌥";
    if (adc_value < LIGHT_DARK)        return "Dark 🌙";
    return "Very Dark 🌑";
}

/**
 * @brief Print graphical light bar
 */
void Print_Light_Bar(uint16_t adc_value) {
    /* ADC 값을 0-20 범위로 변환 (밝으면 바가 김) */
    int bar_length = 20 - (adc_value * 20 / ADC_RESOLUTION);
    if (bar_length < 0) bar_length = 0;
    if (bar_length > 20) bar_length = 20;
    
    printf("[");
    for (int i = 0; i < 20; i++) {
        if (i < bar_length) {
            printf("█");
        } else {
            printf("░");
        }
    }
    printf("] %d%%", bar_length * 5);
}

/**
 * @brief System Clock Configuration (72MHz)
 */
void SystemClock_Config(void) {
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
    
    /* ADC Clock: 72MHz / 6 = 12MHz (max 14MHz) */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

/**
 * @brief GPIO Initialization
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* PA5: 보드 내장 LED */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief DMA Initialization
 */
static void MX_DMA_Init(void) {
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_adc1);
    
    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);
    
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

/**
 * @brief ADC1 Initialization (PA0 - Channel 0)
 */
static void MX_ADC1_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    /* PA0: ADC1_IN0 (Analog Input) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);
    
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;  /* 최대 샘플링 시간 */
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/**
 * @brief USART2 Configuration
 */
static void MX_USART2_UART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_USART2_CLK_ENABLE();
    
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
    
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief DMA1 Channel1 Interrupt Handler
 */
void DMA1_Channel1_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_adc1);
}

/**
 * @brief Error Handler
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
