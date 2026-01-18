/**
 ******************************************************************************
 * @file    main.c
 * @author  Namu
 * @brief   SMD LED Module Test for NUCLEO-F103RB
 * @note    SMD LED는 표면실장형 LED로 일반 LED보다 밝고 소형
 *          간단한 GPIO ON/OFF 및 PWM 밝기 조절 테스트
 ******************************************************************************
 * @hardware
 *   - Board: NUCLEO-F103RB
 *   - SMD LED Module (KY-011 또는 유사 모듈)
 *   - Connections:
 *     - S (Signal) -> PA5 (TIM2_CH1 또는 GPIO)
 *     - VCC        -> 3.3V
 *     - GND        -> GND
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines */
#define LED_PIN         GPIO_PIN_5
#define LED_PORT        GPIOA
#define PWM_PERIOD      999

/* Operation Mode */
typedef enum {
    MODE_GPIO = 0,
    MODE_PWM
} LED_Mode_t;

/* Private variables */
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;
LED_Mode_t currentMode = MODE_GPIO;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
void LED_GPIO_Mode_Init(void);
void LED_PWM_Mode_Init(void);
void LED_SetBrightness(uint8_t brightness);
void LED_Blink(uint32_t on_time, uint32_t off_time, uint8_t count);
void LED_Breathe(uint16_t cycle_ms);
void LED_SOS(void);

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
    
    printf("\r\n========================================\r\n");
    printf("  SMD LED Module Test - NUCLEO-F103RB\r\n");
    printf("========================================\r\n\n");
    
    while (1)
    {
        /* Test 1: GPIO 모드 - 기본 점멸 */
        printf("[Test 1] GPIO Mode - Basic Blink\r\n");
        LED_GPIO_Mode_Init();
        currentMode = MODE_GPIO;
        
        printf("  Slow blink (500ms)...\r\n");
        LED_Blink(500, 500, 5);
        
        printf("  Fast blink (100ms)...\r\n");
        LED_Blink(100, 100, 10);
        
        printf("  Asymmetric blink (200/800ms)...\r\n");
        LED_Blink(200, 800, 5);
        
        HAL_Delay(500);
        
        /* Test 2: PWM 모드 - 밝기 조절 */
        printf("\r\n[Test 2] PWM Mode - Brightness Control\r\n");
        LED_PWM_Mode_Init();
        currentMode = MODE_PWM;
        
        printf("  Brightness levels: ");
        for (int level = 0; level <= 100; level += 25) {
            printf("%d%% ", level);
            LED_SetBrightness((level * 255) / 100);
            HAL_Delay(800);
        }
        printf("\r\n");
        
        HAL_Delay(500);
        
        /* Test 3: 호흡 효과 */
        printf("\r\n[Test 3] Breathing Effect\r\n");
        printf("  2 second cycle...\r\n");
        for (int i = 0; i < 3; i++) {
            LED_Breathe(2000);
        }
        
        printf("  1 second cycle...\r\n");
        for (int i = 0; i < 5; i++) {
            LED_Breathe(1000);
        }
        
        HAL_Delay(500);
        
        /* Test 4: SOS 신호 */
        printf("\r\n[Test 4] SOS Signal\r\n");
        LED_GPIO_Mode_Init();
        currentMode = MODE_GPIO;
        LED_SOS();
        
        HAL_Delay(500);
        
        /* Test 5: 심박 효과 */
        printf("\r\n[Test 5] Heartbeat Effect\r\n");
        LED_PWM_Mode_Init();
        currentMode = MODE_PWM;
        
        for (int beat = 0; beat < 10; beat++) {
            /* 첫 번째 박동 */
            for (int i = 0; i <= 255; i += 15) {
                LED_SetBrightness(i);
                HAL_Delay(5);
            }
            for (int i = 255; i >= 0; i -= 15) {
                LED_SetBrightness(i);
                HAL_Delay(5);
            }
            HAL_Delay(100);
            
            /* 두 번째 박동 (약간 약하게) */
            for (int i = 0; i <= 180; i += 15) {
                LED_SetBrightness(i);
                HAL_Delay(5);
            }
            for (int i = 180; i >= 0; i -= 15) {
                LED_SetBrightness(i);
                HAL_Delay(5);
            }
            HAL_Delay(400);
        }
        
        LED_SetBrightness(0);
        
        printf("\r\n--- Cycle Complete ---\r\n\n");
        HAL_Delay(2000);
    }
}

/**
 * @brief GPIO 모드 초기화
 */
void LED_GPIO_Mode_Init(void)
{
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

/**
 * @brief PWM 모드 초기화
 */
void LED_PWM_Mode_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

/**
 * @brief LED 밝기 설정 (PWM 모드)
 * @param brightness: 0~255
 */
void LED_SetBrightness(uint8_t brightness)
{
    if (currentMode == MODE_PWM) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (brightness * PWM_PERIOD) / 255);
    }
}

/**
 * @brief LED 점멸 (GPIO 모드)
 */
void LED_Blink(uint32_t on_time, uint32_t off_time, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_Delay(on_time);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(off_time);
    }
}

/**
 * @brief LED 호흡 효과 (PWM 모드)
 * @param cycle_ms: 한 호흡 주기(ms)
 */
void LED_Breathe(uint16_t cycle_ms)
{
    uint16_t step_delay = cycle_ms / 512;  // 256 up + 256 down
    
    /* 밝아지기 */
    for (int i = 0; i <= 255; i++) {
        LED_SetBrightness(i);
        HAL_Delay(step_delay);
    }
    
    /* 어두워지기 */
    for (int i = 255; i >= 0; i--) {
        LED_SetBrightness(i);
        HAL_Delay(step_delay);
    }
}

/**
 * @brief SOS 모스부호 신호
 *        S: ···  O: ---  S: ···
 */
void LED_SOS(void)
{
    const uint16_t dot = 150;    // 점
    const uint16_t dash = 450;   // 선 (점의 3배)
    const uint16_t gap = 150;    // 요소 간격
    const uint16_t letter_gap = 450;  // 문자 간격
    
    for (int repeat = 0; repeat < 3; repeat++) {
        /* S: ··· */
        for (int i = 0; i < 3; i++) {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
            HAL_Delay(dot);
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
            HAL_Delay(gap);
        }
        HAL_Delay(letter_gap);
        
        /* O: --- */
        for (int i = 0; i < 3; i++) {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
            HAL_Delay(dash);
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
            HAL_Delay(gap);
        }
        HAL_Delay(letter_gap);
        
        /* S: ··· */
        for (int i = 0; i < 3; i++) {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
            HAL_Delay(dot);
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
            HAL_Delay(gap);
        }
        
        HAL_Delay(1000);  // SOS 반복 간격
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
