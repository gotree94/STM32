/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : 볼 스위치 모듈 테스트 (Ball Switch Module)
 * @board          : NUCLEO-F103RB
 * @sensor         : KY-002 Vibration/Shock Switch Module
 ******************************************************************************
 * @description
 * 볼 스위치 모듈은 내부의 금속 볼이 진동이나 충격에 의해 움직여
 * 순간적으로 접점을 연결하는 센서입니다.
 * 진동, 충격, 노크 감지 등에 활용됩니다.
 * 
 * @pinout
 * - Module S (Signal) -> PA0 (GPIO Input with EXTI)
 * - Module +          -> 3.3V
 * - Module -          -> GND
 * 
 * @note
 * 순간적인 펄스를 감지하므로 인터럽트 사용 권장
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
#define BALL_SWITCH_PIN     GPIO_PIN_0
#define BALL_SWITCH_PORT    GPIOA
#define SENSITIVITY_MS      100    /* 감도 조절: 연속 트리거 방지 시간 */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;

volatile uint32_t shock_count = 0;
volatile uint8_t shock_detected = 0;
volatile uint32_t last_shock_time = 0;
uint32_t led_off_time = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);

/* Printf redirect -----------------------------------------------------------*/
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* Interrupt callback --------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == BALL_SWITCH_PIN) {
        uint32_t current_time = HAL_GetTick();
        
        /* 감도 조절: 일정 시간 내 재트리거 무시 */
        if ((current_time - last_shock_time) > SENSITIVITY_MS) {
            shock_count++;
            shock_detected = 1;
            last_shock_time = current_time;
        }
    }
}

/* Main program --------------------------------------------------------------*/
int main(void) {
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_TIM2_Init();
    
    printf("\r\n========================================\r\n");
    printf("  Ball/Vibration Switch Module Test\r\n");
    printf("  Board: NUCLEO-F103RB\r\n");
    printf("========================================\r\n\n");
    printf("Tap or shake the sensor to detect vibration.\r\n");
    printf("Sensitivity: %dms between detections\r\n\n", SENSITIVITY_MS);
    
    uint32_t display_count = 0;
    uint8_t intensity = 0;
    
    while (1) {
        /* 충격 감지 시 처리 */
        if (shock_detected) {
            shock_detected = 0;
            
            /* 충격 강도 추정 (짧은 시간 내 연속 감지 횟수) */
            intensity++;
            
            /* 시리얼 출력 */
            printf("[%5lu] SHOCK! ", shock_count);
            
            /* 진동 바 시각화 */
            for (int i = 0; i < intensity && i < 20; i++) {
                printf("█");
            }
            printf("\r\n");
            
            /* LED 켜기 */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            led_off_time = HAL_GetTick() + 100;  /* 100ms 후 끄기 */
        }
        
        /* LED 자동 끄기 */
        if (led_off_time > 0 && HAL_GetTick() >= led_off_time) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            led_off_time = 0;
        }
        
        /* 1초마다 통계 출력 */
        if (HAL_GetTick() - display_count >= 1000) {
            display_count = HAL_GetTick();
            
            if (intensity > 0) {
                printf("  └─ Intensity this second: %d events\r\n", intensity);
                intensity = 0;
            }
        }
        
        HAL_Delay(1);
    }
}

/**
 * @brief System Clock Configuration (72MHz)
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
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
}

/**
 * @brief GPIO Initialization
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    
    /* PA0: 볼 스위치 입력 (External Interrupt, Falling Edge) */
    GPIO_InitStruct.Pin = BALL_SWITCH_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  /* 스위치 ON = LOW */
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BALL_SWITCH_PORT, &GPIO_InitStruct);
    
    /* PA5: 보드 내장 LED (출력) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    
    /* EXTI interrupt enable */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
 * @brief TIM2 Configuration (Timestamp용)
 */
static void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 72 - 1;  /* 1MHz */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);
    HAL_TIM_Base_Start(&htim2);
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
 * @brief EXTI0 Interrupt Handler
 */
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(BALL_SWITCH_PIN);
}

/**
 * @brief Error Handler
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
