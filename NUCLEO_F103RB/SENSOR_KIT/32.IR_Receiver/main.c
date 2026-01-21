/**
 ******************************************************************************
 * @file    main.c
 * @brief   IR Receiver Module Test for STM32F103 NUCLEO
 * @author  Embedded Systems Lab
 * @version V1.1.0
 * @date    2025-01-22
 ******************************************************************************
 * @details
 * 이 프로젝트는 IR 수신 모듈을 테스트합니다.
 * IR 신호를 감지하면 LED를 토글하고 시리얼로 상태를 출력합니다.
 * 
 * System Clock: 64MHz (HSI + PLL)
 * 
 * Hardware Setup:
 * - IR Receiver (TSOP1838/VS1838B): PA0 (Digital Input with Pull-up)
 * - LED: PA5 (On-board LED on NUCLEO)
 * - UART2: PA2(TX), PA3(RX) - USB Virtual COM Port @ 115200 baud
 * 
 * Clock Configuration:
 * - HSI: 8MHz (Internal RC)
 * - PLL: HSI/2 × 16 = 64MHz
 * - AHB: 64MHz
 * - APB1: 32MHz (Timer Clock: 64MHz)
 * - APB2: 64MHz
 * 
 * Timer Configuration:
 * - TIM2: 1MHz tick (PSC=63, 64MHz/64=1MHz)
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

/* System Clock defines */
#define SYSCLK_FREQ_HZ      64000000UL  // 64MHz
#define APB1_TIMER_FREQ_HZ  64000000UL  // APB1 Timer Clock (APB1×2)

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
void Error_Handler(void);

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
    HAL_Init();                     // SysTick, NVIC, Flash 초기화
    SystemClock_Config();           // 64MHz 클럭 설정
    
    /* Initialize peripherals */
    MX_GPIO_Init();                 // GPIO 초기화 (PA0: IR, PA5: LED)
    MX_TIM2_Init();                 // TIM2 마이크로초 타이머 초기화
    MX_USART2_UART_Init();          // USART2 초기화 (115200 baud)
    
    /* Start microsecond timer */
    HAL_TIM_Base_Start(&htim2);
    
    printf("\r\n========================================\r\n");
    printf("   IR Receiver Module Test - STM32F103\r\n");
    printf("========================================\r\n");
    printf("System Clock: %lu MHz\r\n", SYSCLK_FREQ_HZ / 1000000);
    printf("IR Receiver Pin: PA0 (Input, Pull-up)\r\n");
    printf("Status LED: PA5 (On-board)\r\n");
    printf("Measurement: GPIO Polling + TIM2 (1us tick)\r\n");
    printf("----------------------------------------\r\n");
    printf("Waiting for IR signals...\r\n\n");
    
    uint8_t last_ir_state = 1;  // IR receiver is normally HIGH (Active Low)
    uint32_t signal_count = 0;
    uint32_t last_signal_time = 0;
    uint8_t signal_active = 0;
    uint32_t pulse_start = 0;
    uint32_t total_pulses = 0;
    uint32_t min_pulse_width = 0xFFFFFFFF;
    uint32_t max_pulse_width = 0;
    
    while (1) {
        /* Read current IR pin state */
        uint8_t ir_state = HAL_GPIO_ReadPin(IR_RECV_GPIO_Port, IR_RECV_Pin);
        uint32_t current_time = HAL_GetTick();      // ms 단위 (SysTick)
        uint32_t current_us = GetMicros();          // us 단위 (TIM2)
        
        /* Detect falling edge (IR signal start - Active Low) */
        if (last_ir_state == 1 && ir_state == 0) {
            if (!signal_active) {
                /* New signal burst started */
                signal_active = 1;
                signal_count++;
                total_pulses = 0;
                min_pulse_width = 0xFFFFFFFF;
                max_pulse_width = 0;
                
                printf("[%lu] IR Signal Detected!\r\n", signal_count);
                
                /* Toggle LED to indicate signal reception */
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            }
            pulse_start = current_us;
            total_pulses++;
            last_signal_time = current_time;
        }
        
        /* Detect rising edge (IR pulse end) */
        if (last_ir_state == 0 && ir_state == 1) {
            uint32_t pulse_width = current_us - pulse_start;
            
            /* Filter noise - ignore pulses shorter than threshold */
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
 * @note   TIM2 is configured as a free-running 32-bit counter at 1MHz
 * @retval Current microsecond count (wraps after ~71.6 minutes)
 */
uint32_t GetMicros(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
}

/**
 * @brief System Clock Configuration - 64MHz from HSI with PLL
 * 
 * Clock Tree:
 *   HSI (8MHz) → /2 → PLL (×16) → SYSCLK (64MHz)
 *                                      ↓
 *                     ┌────────────────┼────────────────┐
 *                     ↓                ↓                ↓
 *                 AHB /1           APB1 /2          APB2 /1
 *                 64MHz            32MHz            64MHz
 *                                   ↓
 *                              Timer ×2
 *                               64MHz
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /*--------------------------------------------------------------------------
     * Oscillator Configuration - HSI + PLL
     * HSI: 8MHz internal RC oscillator
     * PLL Input: HSI/2 = 4MHz
     * PLL Output: 4MHz × 16 = 64MHz
     *------------------------------------------------------------------------*/
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;  // HSI/2 = 4MHz
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;              // 4MHz × 16 = 64MHz
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /*--------------------------------------------------------------------------
     * Bus Clock Configuration
     * SYSCLK = 64MHz (from PLL)
     * AHB    = 64MHz (SYSCLK/1) - Core, DMA, Memory
     * APB1   = 32MHz (AHB/2)    - TIM2~4, USART2~3, I2C, SPI2
     * APB2   = 64MHz (AHB/1)    - TIM1, USART1, SPI1, ADC, GPIO
     * 
     * Note: APB1 Timer Clock = APB1 × 2 = 64MHz (because APB1 prescaler ≠ 1)
     *------------------------------------------------------------------------*/
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;    // 64MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;     // 32MHz (max 36MHz)
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     // 64MHz
    
    /*--------------------------------------------------------------------------
     * Flash Latency Configuration
     * 0 WS: 0 < SYSCLK ≤ 24MHz
     * 1 WS: 24MHz < SYSCLK ≤ 48MHz
     * 2 WS: 48MHz < SYSCLK ≤ 72MHz
     * 
     * 64MHz requires 2 wait states
     *------------------------------------------------------------------------*/
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief GPIO Initialization
 * 
 * PA0: IR Receiver Input (Digital Input with Internal Pull-up)
 *      - Mode: Input
 *      - Pull: Pull-up (IR receiver is Active Low)
 * 
 * PA5: LED Output (On-board LED on NUCLEO)
 *      - Mode: Push-Pull Output
 *      - Speed: Low (2MHz, sufficient for LED)
 *      - Initial: Low (LED OFF)
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIOA clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*--------------------------------------------------------------------------
     * IR Receiver Input - PA0
     * 
     * IR 수신 모듈은 Active Low 출력:
     * - 신호 없음: HIGH (풀업에 의해)
     * - 신호 있음: LOW (내부 트랜지스터 ON)
     * 
     * 내부 풀업 사용 (외부 풀업 저항 불필요)
     *------------------------------------------------------------------------*/
    GPIO_InitStruct.Pin = IR_RECV_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(IR_RECV_GPIO_Port, &GPIO_InitStruct);

    /*--------------------------------------------------------------------------
     * LED Output - PA5
     * 
     * NUCLEO 보드 내장 LED (LD2)
     * Active High: HIGH = ON, LOW = OFF
     *------------------------------------------------------------------------*/
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);  // Initial: OFF
    
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief TIM2 Initialization - Microsecond Timer (1MHz, 1µs tick)
 * 
 * TIM2 is a 32-bit timer on APB1 bus.
 * 
 * Clock Source:
 *   APB1 Timer Clock = 64MHz (APB1 × 2, because APB1 prescaler = /2)
 * 
 * Timer Configuration:
 *   Prescaler (PSC) = 63
 *   Timer Frequency = 64MHz / (63 + 1) = 1MHz
 *   Tick Period = 1 / 1MHz = 1µs
 * 
 *   Auto-Reload (ARR) = 0xFFFFFFFF (32-bit max)
 *   Overflow Period = 2^32 × 1µs ≈ 4295 seconds ≈ 71.6 minutes
 * 
 * Usage:
 *   uint32_t start = GetMicros();
 *   // ... do something ...
 *   uint32_t elapsed_us = GetMicros() - start;
 */
static void MX_TIM2_Init(void) {
    /* Enable TIM2 clock (APB1 peripheral) */
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    
    /*--------------------------------------------------------------------------
     * Prescaler: 64MHz → 1MHz
     * 
     * Timer Clock = APB1 Timer Clock / (PSC + 1)
     *             = 64MHz / (63 + 1)
     *             = 64MHz / 64
     *             = 1MHz (1µs per tick)
     *------------------------------------------------------------------------*/
    htim2.Init.Prescaler = 63;
    
    /* Counter Mode: Up counting (0 → ARR → 0 → ...) */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    
    /*--------------------------------------------------------------------------
     * Auto-Reload Register: Maximum value for 32-bit timer
     * 
     * TIM2 is a 32-bit timer (unlike TIM3, TIM4 which are 16-bit)
     * Maximum count: 0xFFFFFFFF = 4,294,967,295
     * At 1MHz: overflow after ~4295 seconds (~71.6 minutes)
     *------------------------------------------------------------------------*/
    htim2.Init.Period = 0xFFFFFFFF;
    
    /* Clock Division: No division (for input filter, not used here) */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    
    /* Auto-Reload Preload: Disabled (ARR updates immediately) */
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief USART2 Initialization - 115200 baud, 8N1
 * 
 * USART2 is connected to ST-LINK Virtual COM Port on NUCLEO board.
 * Pins: PA2 (TX), PA3 (RX)
 * 
 * USART2 Clock Source: APB1 = 32MHz
 * 
 * Baud Rate Calculation:
 *   USARTDIV = USART_CLK / (16 × BaudRate)  [OverSampling = 16]
 *            = 32,000,000 / (16 × 115,200)
 *            = 17.361...
 * 
 *   BRR = (Mantissa << 4) | Fraction
 *       = (17 << 4) | round(0.361 × 16)
 *       = 0x110 | 0x6
 *       = 0x116
 * 
 *   Actual Baud = 32,000,000 / (16 × 17.375) = 115,108 bps
 *   Error = |115,108 - 115,200| / 115,200 = 0.08% (acceptable)
 */
static void MX_USART2_UART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable clocks */
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*--------------------------------------------------------------------------
     * GPIO Configuration for USART2
     * PA2 - TX: Alternate Function Push-Pull
     * PA3 - RX: Input (or Alternate Function Input)
     *------------------------------------------------------------------------*/
    
    /* PA2 - USART2_TX */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PA3 - USART2_RX */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*--------------------------------------------------------------------------
     * USART2 Configuration
     *------------------------------------------------------------------------*/
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief  Error Handler
 * @note   Called when HAL functions return error
 *         Disables interrupts and enters infinite loop
 */
void Error_Handler(void) {
    __disable_irq();
    
    /* Blink LED rapidly to indicate error (if GPIO initialized) */
    while (1) {
        /* Could add LED blink code here for debugging */
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 */
void assert_failed(uint8_t *file, uint32_t line) {
    printf("Assert failed: %s line %lu\r\n", file, line);
}
#endif
