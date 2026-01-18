/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : 매직 라이트컴 모듈 테스트 (Magic Light Cup Module)
 * @board          : NUCLEO-F103RB
 * @sensor         : KY-027 Magic Light Cup Module
 ******************************************************************************
 * @description
 * 매직 라이트컴 모듈은 수은 스위치와 LED가 결합된 모듈입니다.
 * 기울임에 따라 수은이 이동하여 스위치가 ON/OFF 되고,
 * 이를 감지하여 LED 밝기를 PWM으로 제어합니다.
 * 
 * @pinout
 * - Module S (Signal) -> PA0 (GPIO Input, 기울기 감지)
 * - Module L (LED)    -> PA6 (TIM3_CH1 PWM 출력)
 * - Module +          -> 3.3V
 * - Module -          -> GND
 * 
 * @note
 * 두 개의 모듈을 사용하여 좌우 기울임을 감지하면 더 흥미로운 효과 구현 가능
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);

/* Printf redirect -----------------------------------------------------------*/
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* Main program --------------------------------------------------------------*/
int main(void) {
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_TIM3_Init();
    MX_USART2_UART_Init();
    
    /* Start PWM */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    
    printf("\r\n========================================\r\n");
    printf("  Magic Light Cup Module Test\r\n");
    printf("  Board: NUCLEO-F103RB\r\n");
    printf("========================================\r\n\n");
    
    uint16_t pwm_value = 0;
    uint8_t tilt_state = 0;
    uint8_t prev_tilt_state = 0;
    uint16_t brightness = 0;
    int16_t step = 10;
    
    while (1) {
        /* 기울기 센서 상태 읽기 (PA0) */
        tilt_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        
        /* 상태 변화 감지 시 출력 */
        if (tilt_state != prev_tilt_state) {
            if (tilt_state == GPIO_PIN_SET) {
                printf("Tilt Detected! - LED Fading Up\r\n");
                step = 10;  /* 밝아지는 방향 */
            } else {
                printf("Level Position - LED Fading Down\r\n");
                step = -10;  /* 어두워지는 방향 */
            }
            prev_tilt_state = tilt_state;
        }
        
        /* LED 밝기 부드럽게 변화 (Fade 효과) */
        if (tilt_state == GPIO_PIN_SET) {
            /* 기울어짐: 밝기 증가 */
            if (brightness < 1000) {
                brightness += 10;
            }
        } else {
            /* 수평: 밝기 감소 */
            if (brightness > 0) {
                brightness -= 10;
            }
        }
        
        /* PWM 듀티사이클 설정 (0-1000) */
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, brightness);
        
        /* 보드 LED도 함께 제어 (PA5) */
        if (brightness > 500) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        }
        
        HAL_Delay(20);  /* 50Hz 업데이트 */
    }
}

/**
 * @brief System Clock Configuration (72MHz)
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* HSE 8MHz -> PLL 72MHz */
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
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* PA0: 기울기 센서 입력 (Pull-up) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PA5: 보드 내장 LED */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PC13: User Button (optional) */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
 * @brief TIM3 PWM Configuration (PA6 - CH1)
 */
static void MX_TIM3_Init(void) {
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_TIM3_CLK_ENABLE();
    
    /* PWM 주파수: 72MHz / 72 / 1000 = 1kHz */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 72 - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1000 - 1;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim3);
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    
    /* PA6: TIM3_CH1 (AF Push-Pull) */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief USART2 Configuration (115200 8N1)
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
    
    /* PA2: USART2_TX, PA3: USART2_RX */
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
 * @brief Error Handler
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
