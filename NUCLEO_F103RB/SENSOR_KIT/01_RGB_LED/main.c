/**
 ******************************************************************************
 * @file    main.c
 * @author  Namu
 * @brief   RGB LED Module Test for NUCLEO-F103RB
 * @note    RGB LED는 공통 캐소드(Common Cathode) 타입 사용
 *          PWM을 이용한 색상 혼합 구현
 ******************************************************************************
 * @hardware
 *   - Board: NUCLEO-F103RB
 *   - RGB LED Module (KY-016 또는 유사 모듈)
 *   - Connections:
 *     - R (Red)   -> PA0 (TIM2_CH1)
 *     - G (Green) -> PA1 (TIM2_CH2)  
 *     - B (Blue)  -> PA2 (TIM2_CH3)
 *     - GND       -> GND (공통 캐소드) 또는 3.3V (공통 애노드)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines */
#define PWM_PERIOD      999     // PWM 주기 (0~999 = 1000단계)

/* Private variables */
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
void RGB_SetColor(uint8_t red, uint8_t green, uint8_t blue);
void RGB_Demo_Fade(void);
void RGB_Demo_Rainbow(void);

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
    MX_TIM2_Init();
    MX_USART2_UART_Init();
    
    /* PWM 시작 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  // Red
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  // Green
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);  // Blue
    
    printf("\r\n========================================\r\n");
    printf("  RGB LED Module Test - NUCLEO-F103RB\r\n");
    printf("========================================\r\n\n");
    
    while (1)
    {
        /* 기본 색상 테스트 */
        printf("[Test 1] Basic Colors\r\n");
        
        printf("  Red...\r\n");
        RGB_SetColor(255, 0, 0);
        HAL_Delay(1000);
        
        printf("  Green...\r\n");
        RGB_SetColor(0, 255, 0);
        HAL_Delay(1000);
        
        printf("  Blue...\r\n");
        RGB_SetColor(0, 0, 255);
        HAL_Delay(1000);
        
        printf("  Yellow (R+G)...\r\n");
        RGB_SetColor(255, 255, 0);
        HAL_Delay(1000);
        
        printf("  Cyan (G+B)...\r\n");
        RGB_SetColor(0, 255, 255);
        HAL_Delay(1000);
        
        printf("  Magenta (R+B)...\r\n");
        RGB_SetColor(255, 0, 255);
        HAL_Delay(1000);
        
        printf("  White (R+G+B)...\r\n");
        RGB_SetColor(255, 255, 255);
        HAL_Delay(1000);
        
        printf("  OFF...\r\n\n");
        RGB_SetColor(0, 0, 0);
        HAL_Delay(500);
        
        /* 페이드 효과 */
        printf("[Test 2] Fade Effect\r\n");
        RGB_Demo_Fade();
        HAL_Delay(500);
        
        /* 레인보우 효과 */
        printf("[Test 3] Rainbow Effect\r\n");
        RGB_Demo_Rainbow();
        HAL_Delay(500);
        
        printf("\r\n--- Cycle Complete ---\r\n\n");
    }
}

/**
 * @brief RGB LED 색상 설정 (0~255)
 */
void RGB_SetColor(uint8_t red, uint8_t green, uint8_t blue)
{
    /* 0~255를 0~PWM_PERIOD로 변환 */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (red * PWM_PERIOD) / 255);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (green * PWM_PERIOD) / 255);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (blue * PWM_PERIOD) / 255);
}

/**
 * @brief 페이드 효과 데모
 */
void RGB_Demo_Fade(void)
{
    /* Red 페이드 인/아웃 */
    for (int i = 0; i <= 255; i += 5) {
        RGB_SetColor(i, 0, 0);
        HAL_Delay(10);
    }
    for (int i = 255; i >= 0; i -= 5) {
        RGB_SetColor(i, 0, 0);
        HAL_Delay(10);
    }
    
    /* Green 페이드 인/아웃 */
    for (int i = 0; i <= 255; i += 5) {
        RGB_SetColor(0, i, 0);
        HAL_Delay(10);
    }
    for (int i = 255; i >= 0; i -= 5) {
        RGB_SetColor(0, i, 0);
        HAL_Delay(10);
    }
    
    /* Blue 페이드 인/아웃 */
    for (int i = 0; i <= 255; i += 5) {
        RGB_SetColor(0, 0, i);
        HAL_Delay(10);
    }
    for (int i = 255; i >= 0; i -= 5) {
        RGB_SetColor(0, 0, i);
        HAL_Delay(10);
    }
}

/**
 * @brief 레인보우 효과 데모 (색상환 순환)
 */
void RGB_Demo_Rainbow(void)
{
    uint8_t r, g, b;
    
    for (int i = 0; i < 360; i += 2) {
        /* HSV to RGB 변환 (S=1, V=1 고정) */
        int region = i / 60;
        int remainder = (i - (region * 60)) * 255 / 60;
        
        switch (region) {
            case 0:  r = 255; g = remainder; b = 0; break;
            case 1:  r = 255 - remainder; g = 255; b = 0; break;
            case 2:  r = 0; g = 255; b = remainder; break;
            case 3:  r = 0; g = 255 - remainder; b = 255; break;
            case 4:  r = remainder; g = 0; b = 255; break;
            default: r = 255; g = 0; b = 255 - remainder; break;
        }
        
        RGB_SetColor(r, g, b);
        HAL_Delay(20);
    }
    
    RGB_SetColor(0, 0, 0);
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
 * @brief TIM2 Initialization (PWM)
 */
static void MX_TIM2_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 71;              // 72MHz / 72 = 1MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = PWM_PERIOD;         // 1MHz / 1000 = 1kHz PWM
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim2);
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3);
    
    /* GPIO 설정 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
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
