/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : 각도 스위치 모듈 테스트 (Tilt Switch Module)
 * @board          : NUCLEO-F103RB
 * @sensor         : KY-020 Tilt Switch Module
 ******************************************************************************
 * @description
 * 각도 스위치 모듈은 내부의 금속 볼이 기울기에 따라 접점을 연결하는 
 * 디지털 센서입니다. 특정 각도 이상 기울어지면 ON, 수평이면 OFF 상태가 됩니다.
 * 
 * @pinout
 * - Module S (Signal) -> PA0 (GPIO Input)
 * - Module +          -> 3.3V
 * - Module -          -> GND
 * 
 * @note
 * 외부 인터럽트를 사용하여 기울기 변화를 즉시 감지
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
#define TILT_PIN            GPIO_PIN_0
#define TILT_PORT           GPIOA
#define DEBOUNCE_DELAY_MS   50

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

volatile uint8_t tilt_changed = 0;
volatile uint8_t tilt_state = 0;
volatile uint32_t last_interrupt_time = 0;
uint32_t tilt_count = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* Printf redirect -----------------------------------------------------------*/
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* Interrupt callback --------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == TILT_PIN) {
        uint32_t current_time = HAL_GetTick();
        
        /* 디바운싱: 50ms 이내 재트리거 무시 */
        if ((current_time - last_interrupt_time) > DEBOUNCE_DELAY_MS) {
            tilt_state = HAL_GPIO_ReadPin(TILT_PORT, TILT_PIN);
            tilt_changed = 1;
            last_interrupt_time = current_time;
        }
    }
}

/* Main program --------------------------------------------------------------*/
int main(void) {
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    
    printf("\r\n========================================\r\n");
    printf("  Tilt/Angle Switch Module Test\r\n");
    printf("  Board: NUCLEO-F103RB\r\n");
    printf("========================================\r\n\n");
    printf("Tilt the module to detect angle changes.\r\n");
    printf("Using external interrupt with debouncing.\r\n\n");
    
    /* 초기 상태 읽기 */
    tilt_state = HAL_GPIO_ReadPin(TILT_PORT, TILT_PIN);
    printf("Initial state: %s\r\n\n", tilt_state ? "TILTED" : "LEVEL");
    
    while (1) {
        /* 인터럽트에서 상태 변화 감지 시 처리 */
        if (tilt_changed) {
            tilt_changed = 0;
            tilt_count++;
            
            printf("[%5lu] ", tilt_count);
            
            if (tilt_state == GPIO_PIN_SET) {
                printf(">>> TILTED - Module is angled\r\n");
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  /* LED ON */
            } else {
                printf("=== LEVEL - Module is horizontal\r\n");
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  /* LED OFF */
            }
        }
        
        /* 메인 루프에서 다른 작업 수행 가능 */
        HAL_Delay(10);
    }
}

/**
 * @brief System Clock Configuration (72MHz)
 */
void SystemClock_Config(void) {
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
 * @brief GPIO Initialization with External Interrupt
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    
    /* PA0: 각도 스위치 입력 (External Interrupt, Rising & Falling Edge) */
    GPIO_InitStruct.Pin = TILT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(TILT_PORT, &GPIO_InitStruct);
    
    /* PA5: 보드 내장 LED (출력) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    
    /* EXTI interrupt enable */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
 * @brief USART2 Configuration
 */
static void MX_USART2_UART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
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
 * @brief EXTI0 Interrupt Handler
 */
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(TILT_PIN);
}

/**
 * @brief Error Handler
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
