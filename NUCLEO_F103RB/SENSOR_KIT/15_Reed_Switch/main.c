/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Reed Switch Module Test for STM32F103 NUCLEO Board
 * @author         : Embedded Systems Lab
 * @date           : 2025
 ******************************************************************************
 * @attention
 *
 * Reed Switch Module Interface:
 * - DO (Digital Output) -> PA0 (Digital Input, Active LOW when magnet detected)
 * - AO (Analog Output)  -> PA1 (ADC1_IN1, optional)
 * - VCC -> 3.3V
 * - GND -> GND
 *
 * Reed switch closes when a magnetic field is present.
 * Commonly used for door/window security sensors.
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
ADC_HandleTypeDef hadc1;

/* Reed switch state tracking */
typedef struct {
    uint8_t current_state;
    uint8_t previous_state;
    uint32_t open_count;
    uint32_t close_count;
    uint32_t last_change_time;
    uint32_t total_open_time;
    uint32_t total_closed_time;
    uint32_t state_start_time;
} ReedState_t;

ReedState_t reed = {0};

/* Configuration */
#define DEBOUNCE_TIME_MS    20      // Debounce time for reed switch

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);

/* Printf redirect to UART */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  Read reed switch state with debouncing
 * @retval 1 if closed (magnet present), 0 if open
 */
uint8_t Reed_Read(void) {
    static uint8_t stable_state = 0;
    static uint32_t last_change_time = 0;
    
    // Reed switch output is active LOW on most modules
    uint8_t raw_state = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) ? 1 : 0;
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
 * @brief  Read analog value (if module supports AO)
 * @retval ADC value (0-4095)
 */
uint16_t Reed_ReadAnalog(void) {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        return HAL_ADC_GetValue(&hadc1);
    }
    return 0xFFFF;
}

/**
 * @brief  Update reed switch state and track statistics
 * @retval None
 */
void Reed_Update(void) {
    reed.previous_state = reed.current_state;
    reed.current_state = Reed_Read();
    
    uint32_t current_time = HAL_GetTick();
    
    // State changed
    if (reed.current_state != reed.previous_state) {
        uint32_t state_duration = current_time - reed.state_start_time;
        
        if (reed.current_state) {
            // Switched to CLOSED (magnet detected)
            reed.close_count++;
            reed.total_open_time += state_duration;
            printf("\r\n>>> CLOSED (Magnet detected) - Open duration: %lu ms <<<\r\n", state_duration);
        } else {
            // Switched to OPEN (magnet removed)
            reed.open_count++;
            reed.total_closed_time += state_duration;
            printf("\r\n>>> OPEN (Magnet removed) - Closed duration: %lu ms <<<\r\n", state_duration);
        }
        
        reed.last_change_time = current_time;
        reed.state_start_time = current_time;
    }
}

/**
 * @brief  Check if reed switch just closed (magnet detected)
 * @retval 1 if just closed, 0 otherwise
 */
uint8_t Reed_JustClosed(void) {
    return (reed.current_state && !reed.previous_state);
}

/**
 * @brief  Check if reed switch just opened (magnet removed)
 * @retval 1 if just opened, 0 otherwise
 */
uint8_t Reed_JustOpened(void) {
    return (!reed.current_state && reed.previous_state);
}

/**
 * @brief  Get current state
 * @retval 1 if closed (magnet present), 0 if open
 */
uint8_t Reed_IsClosed(void) {
    return reed.current_state;
}

/**
 * @brief  Format time as hours:minutes:seconds
 * @param  ms: Time in milliseconds
 * @param  buffer: Output buffer
 * @retval None
 */
void FormatTime(uint32_t ms, char *buffer) {
    uint32_t seconds = ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes % 60, seconds % 60);
}

/**
 * @brief  Print status visualization
 * @retval None
 */
void Print_Status(void) {
    char open_time_str[16], closed_time_str[16];
    uint32_t current_duration = HAL_GetTick() - reed.state_start_time;
    
    FormatTime(reed.total_open_time + (reed.current_state ? 0 : current_duration), open_time_str);
    FormatTime(reed.total_closed_time + (reed.current_state ? current_duration : 0), closed_time_str);
    
    printf("\r");
    
    // State indicator
    if (Reed_IsClosed()) {
        printf("[■■■■■ CLOSED ■■■■■]");
    } else {
        printf("[○○○○○  OPEN  ○○○○○]");
    }
    
    // Statistics
    printf(" | Opens: %3lu | Closes: %3lu | ", reed.open_count, reed.close_count);
    printf("Open: %s | Closed: %s", open_time_str, closed_time_str);
    printf("     ");  // Clear leftover characters
    fflush(stdout);
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
    MX_ADC1_Init();

    printf("\r\n========================================\r\n");
    printf("    Reed Switch Module Test Program\r\n");
    printf("    NUCLEO-F103RB Board\r\n");
    printf("========================================\r\n\n");
    printf("DO Pin: PA0 (Digital Input)\r\n");
    printf("AO Pin: PA1 (ADC1_IN1, optional)\r\n\n");
    printf("Application: Door/Window Security Sensor\r\n\n");

    // Initial state
    reed.current_state = Reed_Read();
    reed.previous_state = reed.current_state;
    reed.state_start_time = HAL_GetTick();

    printf("Initial state: %s\r\n\n", reed.current_state ? "CLOSED (Magnet detected)" : "OPEN");
    printf("Bring a magnet close to the reed switch to test.\r\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n\n");

    uint32_t last_print_time = 0;
    uint32_t last_analog_time = 0;

    while (1) {
        Reed_Update();
        
        uint32_t current_time = HAL_GetTick();
        
        // LED control
        if (Reed_IsClosed()) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        } else {
            // Blink when open (alarm simulation)
            if ((current_time / 250) % 2) {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            } else {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            }
        }
        
        // Status update
        if (current_time - last_print_time >= 500) {
            last_print_time = current_time;
            Print_Status();
        }
        
        // Periodic analog reading (if module has AO)
        if (current_time - last_analog_time >= 5000) {
            last_analog_time = current_time;
            uint16_t adc_value = Reed_ReadAnalog();
            if (adc_value != 0xFFFF) {
                printf("\r\n[Analog: %d]\r\n", adc_value);
            }
        }
        
        HAL_Delay(10);
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

/**
 * @brief GPIO Initialization Function
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PA0 - Reed switch DO (Digital Input)
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA5 - Onboard LED (Output)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief ADC1 Initialization Function
 * @retval None
 */
static void MX_ADC1_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);

    sConfig.Channel = ADC_CHANNEL_1;  // PA1
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADCEx_Calibration_Start(&hadc1);
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
