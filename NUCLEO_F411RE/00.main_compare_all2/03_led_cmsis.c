/**
  ******************************************************************************
  * @file    03_led_cmsis.c
  * @brief   [SET1-③] LD2 LED 제어 - CMSIS 구조체 방식
  * @board   NUCLEO-F411RE  |  LED: LD2 = PA5
  * @sysclk  84MHz
  *
  * [CMSIS 방식 특징]
  *   ARM 표준 레지스터 구조체(GPIO_TypeDef)를 통한 직접 접근
  *   stm32f411xe.h 정의:
  *     typedef struct { __IO uint32_t MODER; ... __IO uint32_t BSRR; ... } GPIO_TypeDef;
  *     #define GPIOA ((GPIO_TypeDef *) 0x40020000UL)
  *   컴파일러가 구조체 오프셋을 자동 계산 → BSRR(offset 0x18)에 STR
  *   HAL/LL 내부에서 실제로 사용하는 방식
  *
  * [BSRR 레지스터]
  *   bit[ 0..15] = Set  비트: 1로 쓰면 해당 핀 High
  *   bit[16..31] = Reset비트: 1로 쓰면 해당 핀 Low
  *   SET:   GPIOA->BSRR = GPIO_PIN_5          → bit5  = 1
  *   RESET: GPIOA->BSRR = GPIO_PIN_5 << 16U   → bit21 = 1
  *
  * [사용법] Core/Src/main.c 로 복사 후 빌드
  ******************************************************************************
  */
#include "main.h"

UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    while (1)
    {
        /* ③ CMSIS: GPIOA->BSRR 구조체 멤버 직접 접근
         *   GPIOA = (GPIO_TypeDef *)0x40020000
         *   BSRR  오프셋 = 0x18  →  실제 주소 = 0x40020018
         *   컴파일러 자동 계산: LDR r0,=GPIOA_BASE / STR r1,[r0,#0x18]
         */
        GPIOA->BSRR = GPIO_PIN_5;                    /* ON  : bit5  = 1 */
        HAL_Delay(500);
        GPIOA->BSRR = (uint32_t)GPIO_PIN_5 << 16U;  /* OFF : bit21 = 1 */
        HAL_Delay(500);
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 16;
    RCC_OscInitStruct.PLL.PLLN            = 336;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ            = 4;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                     |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2; huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B; huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE; huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE; huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}
void Error_Handler(void) { __disable_irq(); while (1) {} }
