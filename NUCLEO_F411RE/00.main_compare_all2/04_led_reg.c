/**
  ******************************************************************************
  * @file    04_led_reg.c
  * @brief   [SET1-④] LD2 LED 제어 - 절대주소 직접 접근(REG) 방식
  * @board   NUCLEO-F411RE  |  LED: LD2 = PA5
  * @sysclk  84MHz
  *
  * [REG 방식 특징]
  *   volatile 포인터로 레지스터 절대 주소 직접 접근
  *   RM0383 레지스터 맵에서 주소 직접 확인 후 사용
  *   CMSIS와 생성 어셈블리 동일, 이식성만 다름
  *
  * [GPIOA 레지스터 주소 - RM0383 p.281]
  *   베이스: 0x40020000
  *   +0x00  MODER  : 핀 모드 (00=입력 01=출력 10=AF 11=아날로그)
  *   +0x14  ODR    : 출력 데이터
  *   +0x18  BSRR   : 비트 Set/Reset (쓰기 전용)
  *     bit[ 0..15] Set  비트 → 1: 해당 핀 High
  *     bit[16..31] Reset비트 → 1: 해당 핀 Low
  *
  * [사용법] Core/Src/main.c 로 복사 후 빌드
  ******************************************************************************
  */
#include "main.h"

/* ── 레지스터 주소 직접 정의 (RM0383 기준) ─────────────────────────────── */
#define REG32(addr)     (*(volatile uint32_t *)(addr))

#define GPIOA_MODER     REG32(0x40020000UL)   /* 모드 레지스터   */
#define GPIOA_BSRR      REG32(0x40020018UL)   /* Set/Reset 레지스터 */

#define PA5_SET         (1UL <<  5)            /* BSRR[5]  → High */
#define PA5_RESET       (1UL << 21)            /* BSRR[21] → Low  */

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
        /* ④ REG: volatile 포인터 역참조로 BSRR 직접 쓰기
         *   GPIOA_BSRR = PA5_SET   → *(volatile uint32_t *)0x40020018 = 0x20
         *   컴파일 결과: LDR r0,=0x40020018 / MOV r1,#0x20 / STR r1,[r0]
         *   → CMSIS(GPIOA->BSRR = GPIO_PIN_5)와 동일한 어셈블리 생성
         */
        GPIOA_BSRR = PA5_SET;    /* ON  */
        HAL_Delay(500);
        GPIOA_BSRR = PA5_RESET;  /* OFF */
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
