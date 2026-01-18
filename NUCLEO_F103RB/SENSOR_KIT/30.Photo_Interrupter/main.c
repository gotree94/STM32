/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Photo Interrupter Module Test for NUCLEO-F103RB
 * @author         : 나무
 * @date           : 2025
 ******************************************************************************
 * @description
 * 포토 인터럽터(광차단) 센서 모듈 테스트 프로그램
 * - 센서: KY-010 Photo Interrupter Module
 * - 빛의 차단/통과 감지 (슬롯형 광센서)
 * - 펄스 카운팅 및 RPM 측정
 *
 * @pinout
 * - PA0: Photo Interrupter Output (GPIO Input with EXTI)
 * - PA5: On-board LED (LD2)
 * - PA2: USART2 TX
 * - PA3: USART2 RX
 *
 * @connection
 * Photo Module   NUCLEO-F103RB
 * -----------    -------------
 * VCC        ->  3.3V
 * GND        ->  GND
 * DO/OUT     ->  PA0 (A0)
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* RPM Calculation Parameters */
#define SLOTS_PER_REVOLUTION    1       /* 회전당 슬롯 수 (엔코더 디스크) */
#define RPM_UPDATE_INTERVAL_MS  1000    /* RPM 업데이트 주기 (ms) */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;

/* Photo interrupter variables */
volatile uint32_t pulse_count = 0;
volatile uint32_t total_pulses = 0;
volatile uint32_t last_pulse_time = 0;
volatile uint8_t object_detected = 0;
volatile uint8_t state_changed = 0;

/* RPM calculation */
float current_rpm = 0.0f;
uint32_t last_rpm_update = 0;
uint32_t pulses_for_rpm = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void GPIO_Init(void);
static void USART2_Init(void);
static void TIM2_Init(void);
void Calculate_RPM(void);
void Display_Status(void);

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
        
        /* Simple debounce (minimum 2ms between pulses) */
        if ((current_time - last_pulse_time) > 2)
        {
            pulse_count++;
            total_pulses++;
            pulses_for_rpm++;
            last_pulse_time = current_time;
            
            /* Check current state */
            object_detected = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) ? 1 : 0;
            state_changed = 1;
            
            /* LED feedback */
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
    TIM2_Init();
    
    /* Welcome message */
    printf("\r\n========================================\r\n");
    printf("  Photo Interrupter Test Program\r\n");
    printf("  NUCLEO-F103RB\r\n");
    printf("========================================\r\n");
    printf("Place objects in the slot to detect!\r\n");
    printf("Slots per revolution: %d\r\n\n", SLOTS_PER_REVOLUTION);
    
    /* Initial state */
    object_detected = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) ? 1 : 0;
    printf("Initial state: %s\r\n\n", object_detected ? "BLOCKED" : "CLEAR");
    
    /* Start timer for RPM calculation */
    HAL_TIM_Base_Start(&htim2);
    last_rpm_update = HAL_GetTick();
    
    while (1)
    {
        /* Process state change */
        if (state_changed)
        {
            state_changed = 0;
            
            if (object_detected)
            {
                printf("[%lu] BLOCKED - Object detected (Pulse #%lu)\r\n", 
                       HAL_GetTick(), total_pulses);
            }
            else
            {
                printf("[%lu] CLEAR   - Object removed\r\n", HAL_GetTick());
            }
        }
        
        /* Calculate and display RPM periodically */
        uint32_t current_time = HAL_GetTick();
        if ((current_time - last_rpm_update) >= RPM_UPDATE_INTERVAL_MS)
        {
            Calculate_RPM();
            Display_Status();
            last_rpm_update = current_time;
        }
        
        HAL_Delay(10);
    }
}

/**
 * @brief  Calculate RPM from pulse count
 */
void Calculate_RPM(void)
{
    /* RPM = (Pulses per second / Slots per revolution) * 60 */
    float pulses_per_second = (float)pulses_for_rpm * (1000.0f / RPM_UPDATE_INTERVAL_MS);
    current_rpm = (pulses_per_second / SLOTS_PER_REVOLUTION) * 60.0f;
    
    /* Reset pulse count for next calculation */
    pulses_for_rpm = 0;
}

/**
 * @brief  Display current status
 */
void Display_Status(void)
{
    static uint32_t prev_total = 0;
    
    /* Only display if there was activity */
    if (total_pulses > prev_total || current_rpm > 0)
    {
        printf("\r\n--- Status ---\r\n");
        printf("Total Pulses: %lu\r\n", total_pulses);
        printf("RPM: %.1f\r\n", current_rpm);
        
        /* Calculate frequency */
        float frequency = current_rpm * SLOTS_PER_REVOLUTION / 60.0f;
        printf("Frequency: %.2f Hz\r\n", frequency);
        printf("--------------\r\n\n");
        
        prev_total = total_pulses;
    }
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
 *        - PA0: Input with EXTI (Photo Interrupter)
 *        - PA5: Output (LED)
 */
static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    
    /* Configure PA0 as EXTI Input (Photo Interrupter) */
    /* Both edges to detect block and clear */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure PA5 as Output (LED) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    
    /* Enable EXTI0 interrupt */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
 * @brief TIM2 Initialization (for timing measurements)
 */
static void TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 64000 - 1;   /* 64MHz / 64000 = 1kHz */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFF;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);
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
