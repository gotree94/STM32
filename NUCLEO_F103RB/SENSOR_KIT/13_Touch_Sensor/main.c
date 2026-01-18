/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Touch Sensor Module Test for STM32F103 NUCLEO Board
 * @author         : Embedded Systems Lab
 * @date           : 2025
 ******************************************************************************
 * @attention
 *
 * Touch Sensor Module (TTP223 based) Interface:
 * - SIG (Signal Output) -> PA0 (Digital Input)
 * - VCC -> 3.3V
 * - GND -> GND
 *
 * The TTP223 touch sensor provides a digital output when touched.
 * Default mode: Momentary (HIGH while touched)
 * Can be configured for toggle mode via onboard jumper
 *
 * UART2 is used for debug output (connected to ST-Link Virtual COM Port)
 * - TX: PA2, RX: PA3, Baudrate: 115200
 *
 ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Private variables */
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;

/* Touch sensor state tracking */
typedef struct {
    uint8_t current_state;
    uint8_t previous_state;
    uint32_t touch_count;
    uint32_t touch_start_time;
    uint32_t touch_duration;
    uint32_t last_touch_time;
    uint8_t long_press_detected;
} TouchState_t;

TouchState_t touch = {0};

/* Configuration */
#define DEBOUNCE_TIME_MS        50      // Debounce time
#define LONG_PRESS_TIME_MS      1000    // Long press threshold
#define DOUBLE_TAP_TIME_MS      300     // Double tap window

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);

/* Printf redirect to UART */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  Read touch sensor state with debouncing
 * @retval 1 if touched, 0 if not touched
 */
uint8_t Touch_Read(void) {
    static uint8_t stable_state = 0;
    static uint32_t last_change_time = 0;
    
    uint8_t raw_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    uint32_t current_time = HAL_GetTick();
    
    if (raw_state != stable_state) {
        if (current_time - last_change_time >= DEBOUNCE_TIME_MS) {
            stable_state = raw_state;
            last_change_time = current_time;
        }
    } else {
        last_change_time = current_time;
    }
    
    return stable_state;
}

/**
 * @brief  Update touch state and detect gestures
 * @retval None
 */
void Touch_Update(void) {
    touch.previous_state = touch.current_state;
    touch.current_state = Touch_Read();
    
    uint32_t current_time = HAL_GetTick();
    
    // Touch started (rising edge)
    if (touch.current_state && !touch.previous_state) {
        touch.touch_start_time = current_time;
        touch.long_press_detected = 0;
        
        // Check for double tap
        if (current_time - touch.last_touch_time < DOUBLE_TAP_TIME_MS) {
            printf(">>> DOUBLE TAP detected! <<<\r\n");
        }
    }
    
    // Touch ongoing
    if (touch.current_state) {
        touch.touch_duration = current_time - touch.touch_start_time;
        
        // Long press detection
        if (!touch.long_press_detected && touch.touch_duration >= LONG_PRESS_TIME_MS) {
            touch.long_press_detected = 1;
            printf(">>> LONG PRESS detected! <<<\r\n");
        }
    }
    
    // Touch ended (falling edge)
    if (!touch.current_state && touch.previous_state) {
        touch.touch_count++;
        touch.last_touch_time = current_time;
        
        if (!touch.long_press_detected) {
            printf(">>> TAP detected! (duration: %lu ms) <<<\r\n", touch.touch_duration);
        }
        
        touch.touch_duration = 0;
    }
}

/**
 * @brief  Check if touch just started (rising edge)
 * @retval 1 if touch just started, 0 otherwise
 */
uint8_t Touch_JustPressed(void) {
    return (touch.current_state && !touch.previous_state);
}

/**
 * @brief  Check if touch just ended (falling edge)
 * @retval 1 if touch just ended, 0 otherwise
 */
uint8_t Touch_JustReleased(void) {
    return (!touch.current_state && touch.previous_state);
}

/**
 * @brief  Get current touch state
 * @retval 1 if touched, 0 if not touched
 */
uint8_t Touch_IsPressed(void) {
    return touch.current_state;
}

/**
 * @brief  Print progress bar for touch duration
 * @param  duration: Touch duration in ms
 * @retval None
 */
void Print_ProgressBar(uint32_t duration) {
    printf("[");
    int bars = (duration > 2000) ? 20 : (duration / 100);
    for (int i = 0; i < 20; i++) {
        if (i < bars) {
            printf("█");
        } else {
            printf("░");
        }
    }
    printf("] %4lu ms", duration);
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_TIM2_Init();

    printf("\r\n========================================\r\n");
    printf("   Touch Sensor Module Test Program\r\n");
    printf("   NUCLEO-F103RB Board\r\n");
    printf("========================================\r\n\n");
    printf("SIG Pin: PA0 (Digital Input)\r\n\n");
    printf("Gesture Recognition:\r\n");
    printf("  - TAP: Quick touch and release\r\n");
    printf("  - DOUBLE TAP: Two taps within %d ms\r\n", DOUBLE_TAP_TIME_MS);
    printf("  - LONG PRESS: Hold for %d ms\r\n\n", LONG_PRESS_TIME_MS);

    uint32_t last_print_time = 0;
    uint8_t led_state = 0;

    while (1) {
        Touch_Update();
        
        uint32_t current_time = HAL_GetTick();
        
        // LED control based on touch
        if (Touch_IsPressed()) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        }
        
        // Periodic status update
        if (current_time - last_print_time >= 200) {
            last_print_time = current_time;
            
            printf("\rState: %s | Count: %3lu | ", 
                   Touch_IsPressed() ? "TOUCHED " : "RELEASED",
                   touch.touch_count);
            
            if (Touch_IsPressed()) {
                Print_ProgressBar(touch.touch_duration);
            } else {
                printf("[░░░░░░░░░░░░░░░░░░░░]    0 ms");
            }
            
            printf("    ");  // Clear any leftover characters
            fflush(stdout);
        }
        
        HAL_Delay(10);  // Small delay for stability
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
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
 * @brief GPIO Initialization Function
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PA0 - Touch sensor SIG (Digital Input)
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // Pull-down for clean signal
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA5 - Onboard LED (Output)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief TIM2 Initialization Function (for future PWM use)
 * @retval None
 */
static void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 72 - 1;  // 1MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1000 - 1;   // 1kHz
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);
}

/**
 * @brief USART2 Initialization Function
 * @retval None
 */
static void MX_USART2_UART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART2_CLK_ENABLE();

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

void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
