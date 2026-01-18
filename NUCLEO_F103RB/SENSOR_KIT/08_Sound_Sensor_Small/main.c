/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Small Sound Sensor Module Test for STM32F103
 * @author         : 
 * @date           : 2025
 ******************************************************************************
 * @description
 * 소형 소리센서 모듈 테스트 (KY-038 또는 유사 모듈)
 * - 디지털 출력만 지원하는 간단한 소리 감지
 * - 가변저항으로 감도 조절
 * - 인터럽트 기반 소리 감지
 * 
 * @pinout
 * - PA0  : Digital Output (EXTI)
 * - PA5  : LED Indicator (Output)
 * - PA2  : USART2 TX (Debug)
 * - PA3  : USART2 RX (Debug)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private defines */
#define SOUND_DO_PIN        GPIO_PIN_0
#define SOUND_DO_PORT       GPIOA
#define LED_PIN             GPIO_PIN_5
#define LED_PORT            GPIOA

/* Private variables */
UART_HandleTypeDef huart2;
volatile uint32_t sound_event_count = 0;
volatile uint32_t last_sound_time = 0;
volatile uint8_t new_sound_detected = 0;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void LED_Blink(uint8_t count, uint32_t delay_ms);
void Process_Sound_Event(void);

/* Printf redirect */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  EXTI Callback (Interrupt Handler)
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == SOUND_DO_PIN)
    {
        uint32_t current_time = HAL_GetTick();
        
        /* Debounce: 50ms 이내 재발생 무시 */
        if (current_time - last_sound_time > 50)
        {
            sound_event_count++;
            last_sound_time = current_time;
            new_sound_detected = 1;
            
            /* LED 켜기 */
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        }
    }
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

    printf("\r\n============================================\r\n");
    printf("  Small Sound Sensor Module Test\r\n");
    printf("  STM32F103 NUCLEO\r\n");
    printf("============================================\r\n");
    printf("PA0: Digital Output (Interrupt)\r\n");
    printf("PA5: LED Indicator\r\n");
    printf("Waiting for sound...\r\n\r\n");

    uint32_t last_status_time = 0;
    uint32_t led_off_time = 0;
    uint8_t led_on = 0;

    while (1)
    {
        /* 소리 감지 이벤트 처리 */
        if (new_sound_detected)
        {
            new_sound_detected = 0;
            led_on = 1;
            led_off_time = HAL_GetTick() + 200;  // 200ms 동안 LED 유지
            
            printf("[%lu] Sound Detected! Total Count: %lu\r\n", 
                   HAL_GetTick(), sound_event_count);
            
            Process_Sound_Event();
        }

        /* LED 자동 끄기 */
        if (led_on && HAL_GetTick() >= led_off_time)
        {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
            led_on = 0;
        }

        /* 폴링 방식으로도 상태 확인 (백업) */
        if (HAL_GPIO_ReadPin(SOUND_DO_PORT, SOUND_DO_PIN) == GPIO_PIN_RESET)
        {
            /* Active Low: 소리 감지됨 */
            if (!led_on)
            {
                HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
            }
        }

        /* 5초마다 상태 출력 */
        if (HAL_GetTick() - last_status_time >= 5000)
        {
            last_status_time = HAL_GetTick();
            printf("--- Status: %lu events detected ---\r\n", sound_event_count);
        }

        HAL_Delay(10);
    }
}

/**
 * @brief  Process sound event (pattern detection)
 */
void Process_Sound_Event(void)
{
    static uint32_t event_times[5] = {0};
    static uint8_t event_index = 0;
    
    /* 이벤트 시간 기록 */
    event_times[event_index] = HAL_GetTick();
    event_index = (event_index + 1) % 5;
    
    /* 패턴 분석: 1초 내 3번 이상 감지 */
    uint8_t recent_count = 0;
    uint32_t current_time = HAL_GetTick();
    
    for (int i = 0; i < 5; i++)
    {
        if (current_time - event_times[i] < 1000)
        {
            recent_count++;
        }
    }
    
    if (recent_count >= 3)
    {
        printf("  >> Rapid sound pattern detected! (%d events/sec)\r\n", recent_count);
    }
}

/**
 * @brief  LED Blink
 */
void LED_Blink(uint8_t count, uint32_t delay_ms)
{
    for (uint8_t i = 0; i < count; i++)
    {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_Delay(delay_ms);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(delay_ms);
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
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* LED Pin - Output */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);

    /* Sound DO Pin - External Interrupt (Falling Edge) */
    GPIO_InitStruct.Pin = SOUND_DO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  // Active Low
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SOUND_DO_PORT, &GPIO_InitStruct);

    /* Enable EXTI interrupt */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
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

/**
 * @brief EXTI0 IRQ Handler
 */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(SOUND_DO_PIN);
}
