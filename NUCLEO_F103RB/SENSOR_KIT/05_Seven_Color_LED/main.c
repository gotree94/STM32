/**
 ******************************************************************************
 * @file    main.c
 * @author  Namu
 * @brief   7-Color LED Module Test for NUCLEO-F103RB
 * @note    7색 자동 변환 LED 모듈 (KY-034 또는 유사)
 *          내장 IC가 자동으로 색상 변환, 외부에서는 ON/OFF 제어만 가능
 *          추가로 외부 PWM으로 밝기 조절 및 수동 색상 제어 옵션 제공
 ******************************************************************************
 * @hardware
 *   - Board: NUCLEO-F103RB
 *   - 7-Color LED Module (KY-034, KY-009 또는 유사)
 *   - Connections:
 *     - S (Signal) -> PC8 (GPIO 또는 TIM3_CH3 PWM)
 *     - VCC        -> 3.3V
 *     - GND        -> GND
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines */
#define LED_PIN         GPIO_PIN_8
#define LED_PORT        GPIOC
#define PWM_PERIOD      999

/* LED 모듈 타입 선택 */
#define LED_TYPE_AUTO_CYCLE     0   // KY-034: 자동 색상 변환 (내장 IC)
#define LED_TYPE_FLASH          1   // 깜빡이는 타입
#define LED_TYPE_MANUAL_RGB     2   // 수동 RGB 제어 (3핀 RGB 모듈)

/* 현재 모듈 타입 설정 */
#define CURRENT_LED_TYPE    LED_TYPE_AUTO_CYCLE

/* Private variables */
TIM_HandleTypeDef htim8;
UART_HandleTypeDef huart2;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM8_Init(void);
static void MX_USART2_UART_Init(void);

/* 기본 제어 함수 */
void SevenColorLED_On(void);
void SevenColorLED_Off(void);
void SevenColorLED_SetBrightness(uint8_t brightness);
void SevenColorLED_Toggle(void);

/* 데모 함수 */
void SevenColorLED_AutoCycleDemo(void);
void SevenColorLED_StrobeDemo(void);
void SevenColorLED_PatternDemo(void);
void SevenColorLED_BrightnessDemo(void);
void SevenColorLED_PartyMode(void);
void SevenColorLED_NotificationDemo(void);

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
    MX_TIM8_Init();
    MX_USART2_UART_Init();
    
    /* PWM 시작 */
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
    
    printf("\r\n=============================================\r\n");
    printf("  7-Color LED Module Test - NUCLEO-F103RB\r\n");
    printf("=============================================\r\n");
    printf("  Module Type: ");
    
#if CURRENT_LED_TYPE == LED_TYPE_AUTO_CYCLE
    printf("Auto Color Cycling (KY-034)\r\n");
#elif CURRENT_LED_TYPE == LED_TYPE_FLASH
    printf("Flash Type\r\n");
#else
    printf("Manual RGB Control\r\n");
#endif
    printf("\r\n");
    
    while (1)
    {
        /* Test 1: 기본 ON/OFF */
        printf("[Test 1] Basic ON/OFF Control\r\n");
        
        printf("  LED ON (5 seconds - watch color changes)...\r\n");
        SevenColorLED_On();
        HAL_Delay(5000);
        
        printf("  LED OFF...\r\n");
        SevenColorLED_Off();
        HAL_Delay(1000);
        
        /* Test 2: 밝기 조절 */
        printf("\r\n[Test 2] Brightness Control (PWM)\r\n");
        SevenColorLED_BrightnessDemo();
        HAL_Delay(500);
        
        /* Test 3: 자동 색상 변환 관찰 */
        printf("\r\n[Test 3] Auto Color Cycle Observation\r\n");
        SevenColorLED_AutoCycleDemo();
        HAL_Delay(500);
        
        /* Test 4: 스트로브 효과 */
        printf("\r\n[Test 4] Strobe Effect\r\n");
        SevenColorLED_StrobeDemo();
        HAL_Delay(500);
        
        /* Test 5: 패턴 데모 */
        printf("\r\n[Test 5] Pattern Demo\r\n");
        SevenColorLED_PatternDemo();
        HAL_Delay(500);
        
        /* Test 6: 파티 모드 */
        printf("\r\n[Test 6] Party Mode\r\n");
        SevenColorLED_PartyMode();
        HAL_Delay(500);
        
        /* Test 7: 알림 데모 */
        printf("\r\n[Test 7] Notification Patterns\r\n");
        SevenColorLED_NotificationDemo();
        
        SevenColorLED_Off();
        
        printf("\r\n--- Cycle Complete ---\r\n\n");
        HAL_Delay(2000);
    }
}

/**
 * @brief LED ON (최대 밝기)
 */
void SevenColorLED_On(void)
{
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, PWM_PERIOD);
}

/**
 * @brief LED OFF
 */
void SevenColorLED_Off(void)
{
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
}

/**
 * @brief LED 밝기 설정 (0~255)
 */
void SevenColorLED_SetBrightness(uint8_t brightness)
{
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (brightness * PWM_PERIOD) / 255);
}

/**
 * @brief LED 토글
 */
void SevenColorLED_Toggle(void)
{
    static uint8_t state = 0;
    state = !state;
    if (state) {
        SevenColorLED_On();
    } else {
        SevenColorLED_Off();
    }
}

/**
 * @brief 밝기 조절 데모
 */
void SevenColorLED_BrightnessDemo(void)
{
    printf("  Brightness levels: ");
    
    /* 단계별 밝기 */
    uint8_t levels[] = {25, 50, 75, 100};
    for (int i = 0; i < 4; i++) {
        printf("%d%% ", levels[i]);
        SevenColorLED_SetBrightness((levels[i] * 255) / 100);
        HAL_Delay(1500);
    }
    printf("\r\n");
    
    /* 페이드 인/아웃 */
    printf("  Fade in/out...\r\n");
    for (int cycle = 0; cycle < 2; cycle++) {
        for (int b = 0; b <= 255; b += 3) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(8);
        }
        for (int b = 255; b >= 0; b -= 3) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(8);
        }
    }
    
    SevenColorLED_Off();
}

/**
 * @brief 자동 색상 변환 관찰
 *        내장 IC가 자동으로 색상을 변환하므로 전원만 공급하면 됨
 */
void SevenColorLED_AutoCycleDemo(void)
{
    printf("  Observing auto color cycle for 10 seconds...\r\n");
    printf("  Colors: Red -> Orange -> Yellow -> Green -> Cyan -> Blue -> Purple\r\n");
    
    SevenColorLED_On();
    
    /* 색상 변화 관찰 */
    for (int i = 0; i < 10; i++) {
        printf("  [%d sec]\r\n", i + 1);
        HAL_Delay(1000);
    }
    
    SevenColorLED_Off();
}

/**
 * @brief 스트로브 효과
 */
void SevenColorLED_StrobeDemo(void)
{
    /* 느린 스트로브 */
    printf("  Slow strobe...\r\n");
    for (int i = 0; i < 10; i++) {
        SevenColorLED_On();
        HAL_Delay(200);
        SevenColorLED_Off();
        HAL_Delay(200);
    }
    
    HAL_Delay(300);
    
    /* 빠른 스트로브 */
    printf("  Fast strobe...\r\n");
    for (int i = 0; i < 30; i++) {
        SevenColorLED_On();
        HAL_Delay(50);
        SevenColorLED_Off();
        HAL_Delay(50);
    }
    
    HAL_Delay(300);
    
    /* 점점 빨라지는 스트로브 */
    printf("  Accelerating strobe...\r\n");
    for (int delay = 300; delay >= 30; delay -= 20) {
        SevenColorLED_On();
        HAL_Delay(delay);
        SevenColorLED_Off();
        HAL_Delay(delay);
    }
    
    SevenColorLED_Off();
}

/**
 * @brief 다양한 패턴 데모
 */
void SevenColorLED_PatternDemo(void)
{
    /* 패턴 1: 심박 효과 */
    printf("  Heartbeat pattern...\r\n");
    for (int beat = 0; beat < 5; beat++) {
        /* 첫 번째 박동 */
        for (int b = 0; b <= 255; b += 20) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(5);
        }
        for (int b = 255; b >= 0; b -= 20) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(5);
        }
        HAL_Delay(80);
        
        /* 두 번째 박동 */
        for (int b = 0; b <= 180; b += 20) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(5);
        }
        for (int b = 180; b >= 0; b -= 20) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(5);
        }
        HAL_Delay(350);
    }
    
    HAL_Delay(500);
    
    /* 패턴 2: 캔들 효과 (불규칙한 깜빡임) */
    printf("  Candle flicker effect...\r\n");
    for (int i = 0; i < 50; i++) {
        uint8_t brightness = 150 + (HAL_GetTick() % 100);
        SevenColorLED_SetBrightness(brightness);
        HAL_Delay(50 + (HAL_GetTick() % 80));
    }
    
    HAL_Delay(500);
    
    /* 패턴 3: 펄스 효과 */
    printf("  Pulse effect...\r\n");
    for (int pulse = 0; pulse < 8; pulse++) {
        for (int b = 50; b <= 255; b += 10) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(10);
        }
        for (int b = 255; b >= 50; b -= 10) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(10);
        }
    }
    
    SevenColorLED_Off();
}

/**
 * @brief 파티 모드 (다양한 효과 조합)
 */
void SevenColorLED_PartyMode(void)
{
    printf("  Party mode active for 10 seconds...\r\n");
    
    uint32_t startTime = HAL_GetTick();
    uint8_t pattern = 0;
    
    while ((HAL_GetTick() - startTime) < 10000) {
        switch (pattern % 4) {
            case 0: /* 빠른 점멸 */
                for (int i = 0; i < 10; i++) {
                    SevenColorLED_On();
                    HAL_Delay(30);
                    SevenColorLED_Off();
                    HAL_Delay(30);
                }
                break;
                
            case 1: /* 호흡 */
                for (int b = 0; b <= 255; b += 10) {
                    SevenColorLED_SetBrightness(b);
                    HAL_Delay(5);
                }
                for (int b = 255; b >= 0; b -= 10) {
                    SevenColorLED_SetBrightness(b);
                    HAL_Delay(5);
                }
                break;
                
            case 2: /* 스트로브 */
                for (int i = 0; i < 20; i++) {
                    SevenColorLED_On();
                    HAL_Delay(20);
                    SevenColorLED_Off();
                    HAL_Delay(20);
                }
                break;
                
            case 3: /* 랜덤 밝기 */
                for (int i = 0; i < 15; i++) {
                    SevenColorLED_SetBrightness(50 + (HAL_GetTick() % 200));
                    HAL_Delay(80);
                }
                break;
        }
        pattern++;
    }
    
    SevenColorLED_Off();
}

/**
 * @brief 알림 패턴 데모
 */
void SevenColorLED_NotificationDemo(void)
{
    /* 알림 1: 새 메시지 */
    printf("  New message notification...\r\n");
    for (int i = 0; i < 3; i++) {
        for (int b = 0; b <= 255; b += 15) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(8);
        }
        for (int b = 255; b >= 0; b -= 15) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(8);
        }
        HAL_Delay(200);
    }
    
    HAL_Delay(500);
    
    /* 알림 2: 경고 */
    printf("  Warning notification...\r\n");
    for (int i = 0; i < 6; i++) {
        SevenColorLED_On();
        HAL_Delay(150);
        SevenColorLED_Off();
        HAL_Delay(150);
    }
    
    HAL_Delay(500);
    
    /* 알림 3: 충전 중 */
    printf("  Charging notification...\r\n");
    for (int cycle = 0; cycle < 3; cycle++) {
        for (int b = 50; b <= 200; b += 3) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(15);
        }
        for (int b = 200; b >= 50; b -= 3) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(15);
        }
    }
    
    HAL_Delay(500);
    
    /* 알림 4: 완료 */
    printf("  Completion notification...\r\n");
    SevenColorLED_On();
    HAL_Delay(300);
    SevenColorLED_Off();
    HAL_Delay(200);
    SevenColorLED_On();
    HAL_Delay(300);
    SevenColorLED_Off();
    HAL_Delay(200);
    SevenColorLED_On();
    HAL_Delay(1000);
    
    SevenColorLED_Off();
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

static void MX_TIM8_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    __HAL_RCC_TIM8_CLK_ENABLE();
    
    htim8.Instance = TIM8;
    htim8.Init.Prescaler = 71;
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = PWM_PERIOD;
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(&htim8);
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3);
    
    /* TIM8_CH3 -> PC8 (Remap 필요) */
    __HAL_RCC_AFIO_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /* TIM8 Main Output Enable */
    __HAL_TIM_MOE_ENABLE(&htim8);
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
