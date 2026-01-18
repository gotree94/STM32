/**
 ******************************************************************************
 * @file    main.c
 * @author  Namu
 * @brief   Dual Color LED Module Test for NUCLEO-F103RB
 * @note    2색 LED (Red/Green)를 이용한 신호등 및 상태 표시 구현
 *          PWM을 이용한 색상 혼합으로 황색(Orange/Yellow) 생성 가능
 ******************************************************************************
 * @hardware
 *   - Board: NUCLEO-F103RB
 *   - Dual Color LED Module (KY-011 또는 유사 모듈, 5mm 타입)
 *   - Connections:
 *     - R (Red)   -> PA6 (TIM3_CH1)
 *     - G (Green) -> PA7 (TIM3_CH2)
 *     - GND       -> GND (공통 캐소드)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines */
#define RED_PIN         GPIO_PIN_6
#define GREEN_PIN       GPIO_PIN_7
#define LED_PORT        GPIOA
#define PWM_PERIOD      999

/* LED Colors */
typedef enum {
    COLOR_OFF = 0,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,    // Red + Green
    COLOR_ORANGE     // Red 많이 + Green 조금
} LED_Color_t;

/* Private variables */
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
void DualLED_SetColor(LED_Color_t color);
void DualLED_SetRGB(uint8_t red, uint8_t green);
void DualLED_TrafficLight(void);
void DualLED_StatusIndicator(void);
void DualLED_ColorMix(void);
void DualLED_Alternating(void);

/* UART printf 리다이렉션 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

int main(void)
{
    /* MCU 초기화 */
    HAL_Init();
    SystemClock_Config();
    
    /* 주변장치 초기화 */
    MX_GPIO_Init();
    MX_TIM3_Init();
    MX_USART2_UART_Init();
    
    /* PWM 시작 */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  // Red
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);  // Green
    
    printf("\r\n============================================\r\n");
    printf("  Dual Color LED Module Test - NUCLEO-F103RB\r\n");
    printf("============================================\r\n\n");
    
    while (1)
    {
        /* Test 1: 기본 색상 테스트 */
        printf("[Test 1] Basic Colors\r\n");
        
        printf("  OFF...\r\n");
        DualLED_SetColor(COLOR_OFF);
        HAL_Delay(1000);
        
        printf("  RED...\r\n");
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(1000);
        
        printf("  GREEN...\r\n");
        DualLED_SetColor(COLOR_GREEN);
        HAL_Delay(1000);
        
        printf("  YELLOW (R+G)...\r\n");
        DualLED_SetColor(COLOR_YELLOW);
        HAL_Delay(1000);
        
        printf("  ORANGE (R+g)...\r\n");
        DualLED_SetColor(COLOR_ORANGE);
        HAL_Delay(1000);
        
        DualLED_SetColor(COLOR_OFF);
        HAL_Delay(500);
        
        /* Test 2: 신호등 시뮬레이션 */
        printf("\r\n[Test 2] Traffic Light Simulation\r\n");
        DualLED_TrafficLight();
        HAL_Delay(500);
        
        /* Test 3: 상태 표시기 */
        printf("\r\n[Test 3] Status Indicator\r\n");
        DualLED_StatusIndicator();
        HAL_Delay(500);
        
        /* Test 4: 색상 그라데이션 */
        printf("\r\n[Test 4] Color Gradient (Red -> Yellow -> Green)\r\n");
        DualLED_ColorMix();
        HAL_Delay(500);
        
        /* Test 5: 교대 점멸 */
        printf("\r\n[Test 5] Alternating Blink\r\n");
        DualLED_Alternating();
        
        DualLED_SetColor(COLOR_OFF);
        
        printf("\r\n--- Cycle Complete ---\r\n\n");
        HAL_Delay(2000);
    }
}

/**
 * @brief 사전 정의 색상 설정
 */
void DualLED_SetColor(LED_Color_t color)
{
    switch (color) {
        case COLOR_OFF:
            DualLED_SetRGB(0, 0);
            break;
        case COLOR_RED:
            DualLED_SetRGB(255, 0);
            break;
        case COLOR_GREEN:
            DualLED_SetRGB(0, 255);
            break;
        case COLOR_YELLOW:
            DualLED_SetRGB(255, 255);
            break;
        case COLOR_ORANGE:
            DualLED_SetRGB(255, 80);
            break;
    }
}

/**
 * @brief Red/Green 개별 밝기 설정 (0~255)
 */
void DualLED_SetRGB(uint8_t red, uint8_t green)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (red * PWM_PERIOD) / 255);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (green * PWM_PERIOD) / 255);
}

/**
 * @brief 신호등 시뮬레이션
 *        Green(3초) -> Yellow(1초) -> Red(3초) -> 반복
 */
void DualLED_TrafficLight(void)
{
    for (int cycle = 0; cycle < 2; cycle++) {
        printf("  GREEN (Go) - 3 sec\r\n");
        DualLED_SetColor(COLOR_GREEN);
        HAL_Delay(3000);
        
        printf("  YELLOW (Caution) - 1 sec\r\n");
        DualLED_SetColor(COLOR_YELLOW);
        HAL_Delay(1000);
        
        printf("  RED (Stop) - 3 sec\r\n");
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(3000);
    }
}

/**
 * @brief 상태 표시기 데모
 *        배터리 레벨, 시스템 상태 등 시각화
 */
void DualLED_StatusIndicator(void)
{
    printf("  Simulating battery level:\r\n");
    
    /* 배터리 레벨 시뮬레이션 */
    printf("    100%% - Green\r\n");
    DualLED_SetColor(COLOR_GREEN);
    HAL_Delay(1000);
    
    printf("    75%% - Green (dim)\r\n");
    DualLED_SetRGB(0, 180);
    HAL_Delay(1000);
    
    printf("    50%% - Yellow\r\n");
    DualLED_SetColor(COLOR_YELLOW);
    HAL_Delay(1000);
    
    printf("    25%% - Orange\r\n");
    DualLED_SetColor(COLOR_ORANGE);
    HAL_Delay(1000);
    
    printf("    10%% - Red (blinking)\r\n");
    for (int i = 0; i < 6; i++) {
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(250);
        DualLED_SetColor(COLOR_OFF);
        HAL_Delay(250);
    }
    
    printf("    0%% - Red (fast blink)\r\n");
    for (int i = 0; i < 12; i++) {
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(100);
        DualLED_SetColor(COLOR_OFF);
        HAL_Delay(100);
    }
}

/**
 * @brief 색상 그라데이션 효과
 *        Red -> Orange -> Yellow -> Green 부드러운 전환
 */
void DualLED_ColorMix(void)
{
    /* Red에서 시작 */
    DualLED_SetRGB(255, 0);
    HAL_Delay(500);
    
    /* Red -> Yellow (Green 증가) */
    printf("  Red -> Yellow...\r\n");
    for (int g = 0; g <= 255; g += 3) {
        DualLED_SetRGB(255, g);
        HAL_Delay(10);
    }
    HAL_Delay(300);
    
    /* Yellow -> Green (Red 감소) */
    printf("  Yellow -> Green...\r\n");
    for (int r = 255; r >= 0; r -= 3) {
        DualLED_SetRGB(r, 255);
        HAL_Delay(10);
    }
    HAL_Delay(300);
    
    /* Green -> Yellow (Red 증가) */
    printf("  Green -> Yellow...\r\n");
    for (int r = 0; r <= 255; r += 3) {
        DualLED_SetRGB(r, 255);
        HAL_Delay(10);
    }
    HAL_Delay(300);
    
    /* Yellow -> Red (Green 감소) */
    printf("  Yellow -> Red...\r\n");
    for (int g = 255; g >= 0; g -= 3) {
        DualLED_SetRGB(255, g);
        HAL_Delay(10);
    }
    HAL_Delay(300);
    
    DualLED_SetColor(COLOR_OFF);
}

/**
 * @brief 교대 점멸 효과
 */
void DualLED_Alternating(void)
{
    printf("  Slow alternating...\r\n");
    for (int i = 0; i < 5; i++) {
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(500);
        DualLED_SetColor(COLOR_GREEN);
        HAL_Delay(500);
    }
    
    printf("  Fast alternating...\r\n");
    for (int i = 0; i < 20; i++) {
        DualLED_SetColor(COLOR_RED);
        HAL_Delay(100);
        DualLED_SetColor(COLOR_GREEN);
        HAL_Delay(100);
    }
    
    printf("  Cross-fade...\r\n");
    for (int i = 0; i < 3; i++) {
        /* Red -> Green fade */
        for (int j = 0; j <= 255; j += 5) {
            DualLED_SetRGB(255 - j, j);
            HAL_Delay(8);
        }
        /* Green -> Red fade */
        for (int j = 255; j >= 0; j -= 5) {
            DualLED_SetRGB(255 - j, j);
            HAL_Delay(8);
        }
    }
}

/**
 * @brief System Clock Configuration (72MHz)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
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
}

/**
 * @brief GPIO Initialization
 */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

/**
 * @brief TIM3 Initialization (PWM for PA6, PA7)
 */
static void MX_TIM3_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    __HAL_RCC_TIM3_CLK_ENABLE();
    
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 71;              // 72MHz / 72 = 1MHz
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = PWM_PERIOD;         // 1MHz / 1000 = 1kHz PWM
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);  // PA6
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);  // PA7
    
    /* GPIO 설정 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief USART2 Initialization
 */
static void MX_USART2_UART_Init(void)
{
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
    
    /* USART2 GPIO 설정 (PA2: TX, PA3: RX) */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
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
 * @brief SysTick Handler
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
