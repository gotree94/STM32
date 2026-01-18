/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Knock Sensor Module Test for NUCLEO-F103RB
 * @author         : 나무
 * @date           : 2025
 ******************************************************************************
 * @description
 * 노크(진동) 센서 모듈 테스트 프로그램
 * - 센서: KY-031 Knock Sensor Module (압전 소자)
 * - 진동/충격 감지 시 디지털 출력
 * - 노크 패턴 인식 및 카운팅
 *
 * @pinout
 * - PA0: Knock Sensor Digital Output (GPIO Input with EXTI)
 * - PA5: On-board LED (LD2)
 * - PA2: USART2 TX
 * - PA3: USART2 RX
 *
 * @connection
 * Knock Module   NUCLEO-F103RB
 * -----------    -------------
 * VCC        ->  3.3V
 * GND        ->  GND
 * DO/OUT     ->  PA0 (A0)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Knock Detection Parameters */
#define DEBOUNCE_TIME_MS    50      /* 디바운스 시간 */
#define PATTERN_TIMEOUT_MS  1000    /* 패턴 인식 타임아웃 */
#define MAX_KNOCKS          10      /* 최대 노크 횟수 기록 */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Knock detection variables */
volatile uint32_t knock_count = 0;
volatile uint32_t last_knock_time = 0;
volatile uint8_t knock_detected = 0;

/* Pattern recognition */
uint32_t knock_times[MAX_KNOCKS];
uint8_t knock_index = 0;
uint32_t pattern_start_time = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void GPIO_Init(void);
static void USART2_Init(void);
void Process_Knock(void);
void Analyze_Pattern(void);

/* Redirect printf to UART ---------------------------------------------------*/
#ifdef __GNUC__
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

/**
 * @brief  EXTI Line0 Interrupt Handler
 */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/**
 * @brief  GPIO EXTI Callback
 * @param  GPIO_Pin: Pin that triggered the interrupt
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
        uint32_t current_time = HAL_GetTick();
        
        /* Debounce check */
        if ((current_time - last_knock_time) > DEBOUNCE_TIME_MS)
        {
            knock_detected = 1;
            knock_count++;
            last_knock_time = current_time;
            
            /* Record knock time for pattern recognition */
            if (knock_index == 0) {
                pattern_start_time = current_time;
            }
            
            if (knock_index < MAX_KNOCKS) {
                knock_times[knock_index] = current_time - pattern_start_time;
                knock_index++;
            }
            
            /* Toggle LED */
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
    }
}

/**
 * @brief  Main program
 * @retval int
 */
int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    GPIO_Init();
    USART2_Init();
    
    /* Welcome message */
    printf("\r\n========================================\r\n");
    printf("  Knock Sensor Test Program\r\n");
    printf("  NUCLEO-F103RB\r\n");
    printf("========================================\r\n");
    printf("Knock the sensor to detect vibration!\r\n\n");
    
    uint32_t last_pattern_check = 0;
    
    while (1)
    {
        /* Process knock event */
        if (knock_detected)
        {
            Process_Knock();
            knock_detected = 0;
        }
        
        /* Check for pattern timeout (analyze pattern after 1 second of no knocks) */
        uint32_t current_time = HAL_GetTick();
        if (knock_index > 0 && 
            (current_time - last_knock_time) > PATTERN_TIMEOUT_MS &&
            (current_time - last_pattern_check) > PATTERN_TIMEOUT_MS)
        {
            Analyze_Pattern();
            last_pattern_check = current_time;
        }
        
        HAL_Delay(10);
    }
}

/**
 * @brief  Process knock event
 */
void Process_Knock(void)
{
    printf("[Knock #%lu] Detected at %lu ms\r\n", 
           knock_count, last_knock_time);
    
    /* Brief LED flash */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief  Analyze knock pattern
 */
void Analyze_Pattern(void)
{
    if (knock_index < 2)
    {
        printf("\r\n>> Single knock detected\r\n\n");
        knock_index = 0;
        return;
    }
    
    printf("\r\n>> Pattern Analysis (%d knocks):\r\n", knock_index);
    printf("   Times: ");
    for (uint8_t i = 0; i < knock_index; i++)
    {
        printf("%lu ", knock_times[i]);
    }
    printf("ms\r\n");
    
    /* Calculate intervals */
    printf("   Intervals: ");
    uint32_t total_interval = 0;
    for (uint8_t i = 1; i < knock_index; i++)
    {
        uint32_t interval = knock_times[i] - knock_times[i-1];
        printf("%lu ", interval);
        total_interval += interval;
    }
    printf("ms\r\n");
    
    /* Calculate average interval and BPM */
    if (knock_index > 1)
    {
        float avg_interval = (float)total_interval / (knock_index - 1);
        float bpm = 60000.0f / avg_interval;
        printf("   Average interval: %.1f ms (%.1f BPM)\r\n", avg_interval, bpm);
    }
    
    /* Simple pattern matching */
    if (knock_index == 2)
    {
        printf("   Pattern: DOUBLE KNOCK\r\n");
    }
    else if (knock_index == 3)
    {
        printf("   Pattern: TRIPLE KNOCK\r\n");
    }
    else if (knock_index >= 4)
    {
        /* Check if rhythmic (consistent intervals) */
        uint8_t is_rhythmic = 1;
        uint32_t first_interval = knock_times[1] - knock_times[0];
        
        for (uint8_t i = 2; i < knock_index; i++)
        {
            uint32_t interval = knock_times[i] - knock_times[i-1];
            if (interval < first_interval * 0.7 || interval > first_interval * 1.3)
            {
                is_rhythmic = 0;
                break;
            }
        }
        
        if (is_rhythmic)
        {
            printf("   Pattern: RHYTHMIC KNOCK\r\n");
        }
        else
        {
            printf("   Pattern: IRREGULAR KNOCK\r\n");
        }
    }
    
    printf("\r\n");
    
    /* Reset for next pattern */
    knock_index = 0;
}

/**
 * @brief System Clock Configuration
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
 *        - PA0: Input with EXTI (Knock Sensor)
 *        - PA5: Output (LED)
 */
static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    
    /* Configure PA0 as EXTI Input (Knock Sensor) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  /* 떨어지는 엣지에서 감지 */
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure PA5 as Output (LED) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    
    /* Enable EXTI0 interrupt */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
 * @brief USART2 Initialization
 */
static void USART2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
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
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}
