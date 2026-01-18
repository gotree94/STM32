/**
 ******************************************************************************
 * @file    main.c
 * @author  Namu
 * @brief   Mini Dual Color LED Module Test for NUCLEO-F103RB
 * @note    소형(3mm) 2색 LED 모듈 - 콤팩트한 상태 표시용
 *          Red/Green 독립 제어 및 PWM 색상 혼합
 ******************************************************************************
 * @hardware
 *   - Board: NUCLEO-F103RB
 *   - Mini Dual Color LED Module (KY-029 또는 3mm 2색 LED)
 *   - Connections:
 *     - R (Red)    -> PB0 (TIM3_CH3)
 *     - GND/COM    -> GND (공통 캐소드) 또는 3.3V (공통 애노드)
 *     - G (Green)  -> PB1 (TIM3_CH4)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines */
#define RED_PIN         GPIO_PIN_0
#define GREEN_PIN       GPIO_PIN_1
#define LED_PORT        GPIOB
#define PWM_PERIOD      999

/* Common Type - 공통 캐소드/애노드 설정 */
#define COMMON_CATHODE  1   // 1: 공통 캐소드, 0: 공통 애노드

/* LED States */
typedef enum {
    LED_OFF = 0,
    LED_RED,
    LED_GREEN,
    LED_YELLOW,
    LED_ORANGE,
    LED_LIME
} LED_State_t;

/* System Status */
typedef enum {
    STATUS_OK = 0,
    STATUS_WARNING,
    STATUS_ERROR,
    STATUS_BUSY,
    STATUS_STANDBY
} SystemStatus_t;

/* Private variables */
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
void MiniLED_SetState(LED_State_t state);
void MiniLED_SetRGB(uint8_t red, uint8_t green);
void MiniLED_Pulse(LED_State_t color, uint8_t count);
void MiniLED_StatusDemo(void);
void MiniLED_BootSequence(void);
void MiniLED_DataTransfer(void);
void MiniLED_BatteryCharging(void);

/* UART printf 리다이렉션 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    MX_GPIO_Init();
    MX_TIM3_Init();
    MX_USART2_UART_Init();
    
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    
    printf("\r\n================================================\r\n");
    printf("  Mini Dual Color LED Module Test - NUCLEO-F103RB\r\n");
    printf("================================================\r\n\n");
    
    printf("[Boot] Starting...\r\n");
    MiniLED_BootSequence();
    HAL_Delay(500);
    
    while (1)
    {
        printf("\r\n[Test 1] Basic Colors\r\n");
        
        printf("  OFF\r\n");
        MiniLED_SetState(LED_OFF);
        HAL_Delay(800);
        
        printf("  RED\r\n");
        MiniLED_SetState(LED_RED);
        HAL_Delay(800);
        
        printf("  GREEN\r\n");
        MiniLED_SetState(LED_GREEN);
        HAL_Delay(800);
        
        printf("  YELLOW\r\n");
        MiniLED_SetState(LED_YELLOW);
        HAL_Delay(800);
        
        printf("  ORANGE\r\n");
        MiniLED_SetState(LED_ORANGE);
        HAL_Delay(800);
        
        printf("  LIME\r\n");
        MiniLED_SetState(LED_LIME);
        HAL_Delay(800);
        
        MiniLED_SetState(LED_OFF);
        HAL_Delay(500);
        
        printf("\r\n[Test 2] Pulse Patterns\r\n");
        
        printf("  Red pulse x3\r\n");
        MiniLED_Pulse(LED_RED, 3);
        HAL_Delay(500);
        
        printf("  Green pulse x3\r\n");
        MiniLED_Pulse(LED_GREEN, 3);
        HAL_Delay(500);
        
        printf("  Yellow pulse x2\r\n");
        MiniLED_Pulse(LED_YELLOW, 2);
        HAL_Delay(500);
        
        printf("\r\n[Test 3] System Status Indicators\r\n");
        MiniLED_StatusDemo();
        HAL_Delay(500);
        
        printf("\r\n[Test 4] Data Transfer Simulation\r\n");
        MiniLED_DataTransfer();
        HAL_Delay(500);
        
        printf("\r\n[Test 5] Battery Charging Simulation\r\n");
        MiniLED_BatteryCharging();
        
        MiniLED_SetState(LED_OFF);
        
        printf("\r\n--- Cycle Complete ---\r\n");
        HAL_Delay(2000);
    }
}

void MiniLED_SetState(LED_State_t state)
{
    switch (state) {
        case LED_OFF:    MiniLED_SetRGB(0, 0);       break;
        case LED_RED:    MiniLED_SetRGB(255, 0);     break;
        case LED_GREEN:  MiniLED_SetRGB(0, 255);     break;
        case LED_YELLOW: MiniLED_SetRGB(255, 255);   break;
        case LED_ORANGE: MiniLED_SetRGB(255, 100);   break;
        case LED_LIME:   MiniLED_SetRGB(100, 255);   break;
    }
}

void MiniLED_SetRGB(uint8_t red, uint8_t green)
{
#if COMMON_CATHODE
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (red * PWM_PERIOD) / 255);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, (green * PWM_PERIOD) / 255);
#else
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, PWM_PERIOD - (red * PWM_PERIOD) / 255);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, PWM_PERIOD - (green * PWM_PERIOD) / 255);
#endif
}

void MiniLED_Pulse(LED_State_t color, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        for (int b = 0; b <= 255; b += 15) {
            switch (color) {
                case LED_RED:    MiniLED_SetRGB(b, 0); break;
                case LED_GREEN:  MiniLED_SetRGB(0, b); break;
                case LED_YELLOW: MiniLED_SetRGB(b, b); break;
                default: break;
            }
            HAL_Delay(5);
        }
        for (int b = 255; b >= 0; b -= 15) {
            switch (color) {
                case LED_RED:    MiniLED_SetRGB(b, 0); break;
                case LED_GREEN:  MiniLED_SetRGB(0, b); break;
                case LED_YELLOW: MiniLED_SetRGB(b, b); break;
                default: break;
            }
            HAL_Delay(5);
        }
        HAL_Delay(150);
    }
}

void MiniLED_BootSequence(void)
{
    for (int i = 0; i < 6; i++) {
        MiniLED_SetState(LED_RED);
        HAL_Delay(80);
        MiniLED_SetState(LED_GREEN);
        HAL_Delay(80);
    }
    
    for (int b = 0; b <= 255; b += 5) {
        MiniLED_SetRGB(b, b);
        HAL_Delay(8);
    }
    HAL_Delay(200);
    
    for (int r = 255; r >= 0; r -= 5) {
        MiniLED_SetRGB(r, 255);
        HAL_Delay(8);
    }
    
    for (int i = 0; i < 2; i++) {
        MiniLED_SetState(LED_OFF);
        HAL_Delay(150);
        MiniLED_SetState(LED_GREEN);
        HAL_Delay(150);
    }
    
    MiniLED_SetState(LED_OFF);
    printf("  Boot complete!\r\n");
}

void MiniLED_StatusDemo(void)
{
    printf("  Status: OK\r\n");
    MiniLED_SetState(LED_GREEN);
    HAL_Delay(2000);
    
    printf("  Status: BUSY\r\n");
    for (int j = 0; j < 8; j++) {
        MiniLED_SetState(LED_YELLOW);
        HAL_Delay(150);
        MiniLED_SetState(LED_OFF);
        HAL_Delay(150);
    }
    
    printf("  Status: WARNING\r\n");
    for (int j = 0; j < 4; j++) {
        MiniLED_SetState(LED_ORANGE);
        HAL_Delay(300);
        MiniLED_SetState(LED_OFF);
        HAL_Delay(300);
    }
    
    printf("  Status: ERROR\r\n");
    for (int j = 0; j < 10; j++) {
        MiniLED_SetState(LED_RED);
        HAL_Delay(100);
        MiniLED_SetState(LED_OFF);
        HAL_Delay(100);
    }
    
    printf("  Status: STANDBY\r\n");
    for (int cycle = 0; cycle < 2; cycle++) {
        for (int b = 0; b <= 200; b += 5) {
            MiniLED_SetRGB(0, b);
            HAL_Delay(10);
        }
        for (int b = 200; b >= 0; b -= 5) {
            MiniLED_SetRGB(0, b);
            HAL_Delay(10);
        }
    }
    
    MiniLED_SetState(LED_OFF);
}

void MiniLED_DataTransfer(void)
{
    printf("  Connecting...\r\n");
    for (int i = 0; i < 6; i++) {
        MiniLED_SetState(LED_YELLOW);
        HAL_Delay(100);
        MiniLED_SetState(LED_OFF);
        HAL_Delay(100);
    }
    
    printf("  Transferring data...\r\n");
    for (int i = 0; i < 30; i++) {
        MiniLED_SetState(LED_GREEN);
        HAL_Delay(30 + (i % 5) * 20);
        MiniLED_SetState(LED_OFF);
        HAL_Delay(20 + (i % 3) * 15);
    }
    
    printf("  Transfer complete!\r\n");
    MiniLED_SetState(LED_GREEN);
    HAL_Delay(500);
    
    for (int b = 255; b >= 0; b -= 5) {
        MiniLED_SetRGB(0, b);
        HAL_Delay(15);
    }
}

void MiniLED_BatteryCharging(void)
{
    printf("  Charging: ");
    
    for (int level = 0; level <= 100; level += 20) {
        printf("%d%% ", level);
        
        if (level < 20) {
            for (int i = 0; i < 2; i++) {
                MiniLED_SetState(LED_RED);
                HAL_Delay(100);
                MiniLED_SetState(LED_OFF);
                HAL_Delay(100);
            }
        } else if (level < 50) {
            for (int b = 100; b <= 255; b += 20) {
                MiniLED_SetRGB(b, b * 40 / 100);
                HAL_Delay(15);
            }
            for (int b = 255; b >= 100; b -= 20) {
                MiniLED_SetRGB(b, b * 40 / 100);
                HAL_Delay(15);
            }
        } else if (level < 80) {
            for (int b = 150; b <= 255; b += 15) {
                MiniLED_SetRGB(b, b);
                HAL_Delay(12);
            }
            for (int b = 255; b >= 150; b -= 15) {
                MiniLED_SetRGB(b, b);
                HAL_Delay(12);
            }
        } else {
            for (int b = 200; b <= 255; b += 10) {
                MiniLED_SetRGB(0, b);
                HAL_Delay(15);
            }
            for (int b = 255; b >= 200; b -= 10) {
                MiniLED_SetRGB(0, b);
                HAL_Delay(15);
            }
        }
    }
    
    printf("\r\n  Fully charged!\r\n");
    MiniLED_SetState(LED_GREEN);
    HAL_Delay(1500);
}

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

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

static void MX_TIM3_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    __HAL_RCC_TIM3_CLK_ENABLE();
    
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 71;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = PWM_PERIOD;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3);
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

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

void SysTick_Handler(void)
{
    HAL_IncTick();
}
