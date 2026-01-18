/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : 아날로그 홀센서 모듈 테스트 (Analog Hall Sensor Module)
 * @board          : NUCLEO-F103RB
 * @sensor         : KY-035 Analog Hall Effect Sensor (49E)
 ******************************************************************************
 * @description
 * 아날로그 홀센서 모듈은 자기장의 세기를 아날로그 전압으로 출력합니다.
 * 자석의 N극이 가까워지면 전압이 낮아지고, S극이 가까워지면 전압이 높아집니다.
 * 자석이 없는 상태에서는 Vcc/2 (약 1.65V) 부근의 전압을 출력합니다.
 * 
 * @pinout
 * - Module S (Signal) -> PA0 (ADC1_CH0)
 * - Module +          -> 3.3V
 * - Module -          -> GND
 * 
 * @note
 * 자기장 극성과 강도를 모두 감지할 수 있는 선형 홀센서
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Private defines -----------------------------------------------------------*/
#define ADC_RESOLUTION      4096
#define ADC_MIDPOINT        2048    /* Vcc/2 기준점 */
#define SAMPLES_TO_AVG      32      /* 평균 샘플 수 */

/* 자기장 감지 임계값 */
#define THRESHOLD_NORTH     1800    /* N극 감지 임계값 (중심점보다 낮음) */
#define THRESHOLD_SOUTH     2300    /* S극 감지 임계값 (중심점보다 높음) */
#define NOISE_MARGIN        50      /* 노이즈 마진 */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim3;

volatile uint16_t adc_buffer[SAMPLES_TO_AVG];
volatile uint8_t adc_complete = 0;

/* Calibration values */
uint16_t zero_point = ADC_MIDPOINT;  /* 자석 없을 때 기준값 */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);

uint16_t Get_Average_ADC(void);
void Calibrate_Zero_Point(void);
void Print_Magnetic_Bar(int16_t magnetic_value);
const char* Get_Polarity_String(int16_t magnetic_value);

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
    MX_TIM3_Init();
    
    printf("\r\n========================================\r\n");
    printf("  Analog Hall Sensor Module Test\r\n");
    printf("  Board: NUCLEO-F103RB\r\n");
    printf("  Sensor: 49E Linear Hall Effect\r\n");
    printf("========================================\r\n\n");
    
    /* ADC Calibration */
    HAL_ADCEx_Calibration_Start(&hadc1);
    
    /* Zero-point Calibration */
    printf("Calibrating... Keep magnets away!\r\n");
    HAL_Delay(1000);
    Calibrate_Zero_Point();
    printf("Zero point calibrated: %d (%.2fV)\r\n\n", 
           zero_point, (float)zero_point * 3.3f / ADC_RESOLUTION);
    
    /* Start PWM for buzzer/indicator (optional) */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    
    /* Start DMA-based ADC conversion */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLES_TO_AVG);
    
    uint16_t adc_value = 0;
    int16_t magnetic_value = 0;
    int16_t prev_magnetic_value = 0;
    uint32_t last_print_time = 0;
    uint8_t magnet_detected = 0;
    
    printf("Bring a magnet close to the sensor...\r\n\n");
    
    while (1) {
        if (adc_complete) {
            adc_complete = 0;
            
            /* 평균값 계산 */
            adc_value = Get_Average_ADC();
            
            /* 자기장 값 계산 (기준점 대비 차이) */
            magnetic_value = (int16_t)adc_value - (int16_t)zero_point;
            
            /* 자석 감지 상태 확인 */
            if (abs(magnetic_value) > NOISE_MARGIN) {
                if (!magnet_detected) {
                    magnet_detected = 1;
                    printf(">>> Magnet Detected! <<<\r\n");
                }
                
                /* LED PWM으로 자기장 세기 표시 */
                uint16_t led_brightness = abs(magnetic_value) * 1000 / ADC_MIDPOINT;
                if (led_brightness > 999) led_brightness = 999;
                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, led_brightness);
                
            } else {
                if (magnet_detected) {
                    magnet_detected = 0;
                    printf("<<< Magnet Removed >>>\r\n\n");
                }
                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
            }
            
            /* 300ms마다 또는 큰 변화 시 출력 */
            if (HAL_GetTick() - last_print_time >= 300 ||
                abs(magnetic_value - prev_magnetic_value) > 100) {
                
                printf("[Hall Sensor Reading]\r\n");
                printf("  ADC Raw    : %4d\r\n", adc_value);
                printf("  Voltage    : %.3f V\r\n", (float)adc_value * 3.3f / ADC_RESOLUTION);
                printf("  Magnetic   : %+5d (relative to zero)\r\n", magnetic_value);
                printf("  Polarity   : %s\r\n", Get_Polarity_String(magnetic_value));
                printf("  Strength   : ");
                Print_Magnetic_Bar(magnetic_value);
                printf("\r\n\n");
                
                last_print_time = HAL_GetTick();
                prev_magnetic_value = magnetic_value;
            }
            
            /* 보드 LED로 극성 표시 */
            if (magnetic_value < -NOISE_MARGIN) {
                /* N극: LED 빠른 깜빡임 */
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            } else if (magnetic_value > NOISE_MARGIN) {
                /* S극: LED 켜짐 */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            } else {
                /* 무자기장: LED 꺼짐 */
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
 * @brief Calibrate zero point (no magnetic field)
 */
void Calibrate_Zero_Point(void) {
    uint32_t sum = 0;
    const int cal_samples = 100;
    
    for (int i = 0; i < cal_samples; i++) {
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLES_TO_AVG);
        while (!adc_complete);
        adc_complete = 0;
        sum += Get_Average_ADC();
        HAL_Delay(10);
    }
    
    zero_point = sum / cal_samples;
}

/**
 * @brief Get polarity string based on magnetic value
 */
const char* Get_Polarity_String(int16_t magnetic_value) {
    if (magnetic_value < -200)      return "Strong NORTH (N) ↓";
    if (magnetic_value < -NOISE_MARGIN) return "Weak NORTH (N)";
    if (magnetic_value > 200)       return "Strong SOUTH (S) ↑";
    if (magnetic_value > NOISE_MARGIN)  return "Weak SOUTH (S)";
    return "No magnetic field";
}

/**
 * @brief Print graphical magnetic field bar (bipolar)
 */
void Print_Magnetic_Bar(int16_t magnetic_value) {
    /* -2048 ~ +2048 범위를 -10 ~ +10 바로 변환 */
    int bar_position = magnetic_value * 10 / ADC_MIDPOINT;
    if (bar_position < -10) bar_position = -10;
    if (bar_position > 10) bar_position = 10;
    
    printf("N [");
    for (int i = -10; i <= 10; i++) {
        if (i == 0) {
            printf("|");  /* 중심점 */
        } else if ((bar_position < 0 && i >= bar_position && i < 0) ||
                   (bar_position > 0 && i > 0 && i <= bar_position)) {
            printf("█");
        } else {
            printf("░");
        }
    }
    printf("] S");
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
 * @brief ADC1 Initialization
 */
static void MX_ADC1_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_ADC1_CLK_ENABLE();
    
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
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/**
 * @brief TIM3 PWM Configuration (PA6 - CH1)
 */
static void MX_TIM3_Init(void) {
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_TIM3_CLK_ENABLE();
    
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 72 - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1000 - 1;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim3);
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
