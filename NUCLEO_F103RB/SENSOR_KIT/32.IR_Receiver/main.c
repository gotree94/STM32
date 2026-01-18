/**
 ******************************************************************************
 * @file    main.c
 * @brief   IR Receiver Module Test for STM32F103 NUCLEO
 * @author  Embedded Systems Lab
 * @version V1.0.0
 * @date    2025-01-18
 ******************************************************************************
 * @details
 * 이 프로젝트는 IR 수신 모듈을 테스트합니다.
 * IR 신호를 감지하면 LED를 토글하고 시리얼로 상태를 출력합니다.
 * 
 * Hardware Setup:
 * - IR Receiver (TSOP1838/VS1838B): PA0 (Digital Input)
 * - LED: PA5 (On-board LED on NUCLEO)
 * - UART2: PA2(TX), PA3(RX) - USB Virtual COM Port
 * 
 * IR 신호가 감지되면 LED가 깜빡이고 시리얼로 출력됩니다.
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define IR_RECV_Pin         GPIO_PIN_0
#define IR_RECV_GPIO_Port   GPIOA
#define LED_Pin             GPIO_PIN_5
#define LED_GPIO_Port       GPIOA

/* IR Signal detection parameters */
#define SIGNAL_TIMEOUT_MS   100     // Timeout for signal detection
#define NOISE_FILTER_US     100     // Minimum pulse width to filter noise

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;

volatile uint32_t signal_start_time = 0;
volatile uint8_t ir_signal_detected = 0;
volatile uint32_t pulse_count = 0;
volatile uint32_t last_pulse_time = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
uint32_t GetMicros(void);

/* Printf redirect to UART ---------------------------------------------------*/
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  Main function
 */
int main(void) {
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_USART2_UART_Init();
    
    /* Start microsecond timer */
    HAL_TIM_Base_Start(&htim2);
    
    printf("\r\n========================================\r\n");
    printf("   IR Receiver Module Test - STM32F103\r\n");
    printf("========================================\r\n");
    printf("IR Receiver Pin: PA0\r\n");
    printf("Status LED: PA5 (On-board)\r\n");
    printf("----------------------------------------\r\n");
    printf("Waiting for IR signals...\r\n\n");
    
    uint8_t last_ir_state = 1;  // IR receiver is normally HIGH
    uint32_t signal_count = 0;
    uint32_t last_signal_time = 0;
    uint8_t signal_active = 0;
    uint32_t pulse_start = 0;
    uint32_t total_pulses = 0;
    uint32_t min_pulse_width = 0xFFFFFFFF;
    uint32_t max_pulse_width = 0;
    
    while (1) {
        uint8_t ir_state = HAL_GPIO_ReadPin(IR_RECV_GPIO_Port, IR_RECV_Pin);
        uint32_t current_time = HAL_GetTick();
        uint32_t current_us = GetMicros();
        
        /* Detect falling edge (IR signal start) */
        if (last_ir_state == 1 && ir_state == 0) {
            if (!signal_active) {
                /* New signal burst started */
                signal_active = 1;
                signal_count++;
                total_pulses = 0;
                min_pulse_width = 0xFFFFFFFF;
                max_pulse_width = 0;
                
                printf("[%lu] IR Signal Detected!\r\n", signal_count);
                
                /* Toggle LED */
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            }
            pulse_start = current_us;
            total_pulses++;
            last_signal_time = current_time;
        }
        
        /* Detect rising edge (IR pulse end) */
        if (last_ir_state == 0 && ir_state == 1) {
            uint32_t pulse_width = current_us - pulse_start;
            
            /* Filter noise */
            if (pulse_width > NOISE_FILTER_US) {
                if (pulse_width < min_pulse_width) min_pulse_width = pulse_width;
                if (pulse_width > max_pulse_width) max_pulse_width = pulse_width;
            }
            last_signal_time = current_time;
        }
        
        /* Check for signal timeout (end of IR burst) */
        if (signal_active && (current_time - last_signal_time > SIGNAL_TIMEOUT_MS)) {
            signal_active = 0;
            
            printf("       Total Pulses: %lu\r\n", total_pulses);
            if (min_pulse_width != 0xFFFFFFFF) {
                printf("       Pulse Width: %lu ~ %lu us\r\n", min_pulse_width, max_pulse_width);
            }
            printf("       Signal burst ended\r\n\n");
        }
        
        last_ir_state = ir_state;
    }
}

/**
 * @brief  Get microseconds from TIM2
 * @retval Current microsecond count
 */
uint32_t GetMicros(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
}

/**
 * @brief System Clock Configuration
 *        72MHz from HSE (8MHz) with PLL
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
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
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* IR Receiver Input - PA0 */
    GPIO_InitStruct.Pin = IR_RECV_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(IR_RECV_GPIO_Port, &GPIO_InitStruct);

    /* LED Output - PA5 */
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief TIM2 Initialization - Microsecond timer
 */
static void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 71;          // 72MHz / 72 = 1MHz (1us tick)
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;     // 32-bit counter max
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);
}

/**
 * @brief USART2 Initialization - 115200 baud
 */
static void MX_USART2_UART_Init(void) {
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* PA2 - TX */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PA3 - RX */
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

/**
 * @brief  Error Handler
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    printf("Assert failed: %s line %lu\r\n", file, line);
}
#endif
