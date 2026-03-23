/**
  ******************************************************************************
  * @file    05_led_asm.c
  * @brief   [SET1-⑤] LD2 LED 제어 - 인라인 어셈블러(ASM) 방식
  * @board   NUCLEO-F411RE  |  LED: LD2 = PA5
  * @sysclk  84MHz
  *
  * [ASM 방식 특징]
  *   GCC __asm volatile로 ARM Cortex-M4 명령어 직접 작성
  *   이론적 최소 명령어: LDR(주소) + MOV or LDR(값) + STR(쓰기) = 3 명령어
  *
  * [MOV vs LDR 선택]
  *   PA5_SET   = 0x00000020  (32)        ≤ 255 → MOV #imm8 사용 가능
  *   PA5_RESET = 0x00200000  (2,097,152) > 255 → LDR =imm32 필요
  *
  * [GCC 인라인 어셈블러 문법]
  *   __asm volatile (
  *       "명령어 \n\t"
  *       : 출력 피연산자    (없으면 생략)
  *       : 입력 피연산자    "i"=즉시값, "r"=레지스터
  *       : 클로버 리스트    "r0"=변경된 레지스터, "memory"=메모리 변경
  *   );
  *
  * [사용법] Core/Src/main.c 로 복사 후 빌드
  ******************************************************************************
  */
#include "main.h"

/* ── 레지스터 주소 / 비트 상수 ─────────────────────────────────────────── */
#define GPIOA_BSRR_ADDR     0x40020018UL   /* GPIOA BSRR 절대 주소 */
#define PA5_SET             (1UL <<  5)    /* 0x00000020 ≤ 255     */
#define PA5_RESET           (1UL << 21)    /* 0x00200000 > 255     */

/* ── Delay 카운트 (SYSCLK=84MHz, SUBS+BNE ≈ 3사이클/루프) ─────────────── */
#define DELAY_500MS_CNT     14000000UL     /* 84,000,000/3 × 0.5   */

UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init_ASM(void);
static void MX_USART2_UART_Init(void);
static void LED_ON_ASM(void);
static void LED_OFF_ASM(void);
static void Delay_ASM(uint32_t count);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init_ASM();     /* RCC + MODER 도 ASM으로 초기화 */
    MX_USART2_UART_Init();

    while (1)
    {
        LED_ON_ASM();
        Delay_ASM(DELAY_500MS_CNT);
        LED_OFF_ASM();
        Delay_ASM(DELAY_500MS_CNT);
    }
}

/* =========================================================================
 * MX_GPIO_Init_ASM
 *   Step1: RCC AHB1ENR bit0(GPIOAEN) = 1  → GPIOA 클럭 ON
 *   Step2: GPIOA MODER bit[11:10] = 01    → PA5 출력 모드
 *
 *   명령어:
 *     ORR rd,rn,rm : rd = rn | rm  (비트 OR: 비트 세트)
 *     BIC rd,rn,rm : rd = rn & ~rm (비트 클리어)
 * =========================================================================*/
static void MX_GPIO_Init_ASM(void)
{
    /* Step1: RCC->AHB1ENR |= (1<<0)  GPIOA 클럭 활성화 */
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &RCC_AHB1ENR (0x40023830) */
        "LDR  r1, [r0]         \n\t"   /* r1 = 현재값 읽기               */
        "ORR  r1, r1, #1       \n\t"   /* r1 |= 1  (GPIOAEN bit0)        */
        "STR  r1, [r0]         \n\t"   /* 업데이트                       */
        :: "i"(0x40023830UL) : "r0","r1","memory"
    );

    /* Step2: GPIOA->MODER  PA5: bit[11:10] = 01 (출력) */
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &GPIOA_MODER (0x40020000) */
        "LDR  r1, [r0]         \n\t"   /* r1 = 현재값 읽기               */
        "BIC  r1, r1, %1       \n\t"   /* bit[11:10] 클리어 (3<<10)      */
        "ORR  r1, r1, %2       \n\t"   /* bit[10] = 1 (출력 모드 01)     */
        "STR  r1, [r0]         \n\t"   /* 업데이트                       */
        :: "i"(0x40020000UL), "i"(3UL<<10), "i"(1UL<<10)
        : "r0","r1","memory"
    );
}

/* =========================================================================
 * LED_ON_ASM  : BSRR[5]=1 → PA5 High
 *   PA5_SET = 0x20 (32) ≤ 255 → MOV #imm8 사용 (1 명령어)
 * =========================================================================*/
static void LED_ON_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = GPIOA_BSRR 주소   */
        "MOV  r1, %1           \n\t"   /* r1 = 0x20 (PA5_SET)    */
        "STR  r1, [r0]         \n\t"   /* BSRR = 0x20            */
        :: "i"(GPIOA_BSRR_ADDR), "i"(PA5_SET)
        : "r0","r1","memory"
    );
}

/* =========================================================================
 * LED_OFF_ASM : BSRR[21]=1 → PA5 Low
 *   PA5_RESET = 0x00200000 > 255 → LDR =imm32 필요 (리터럴 풀)
 * =========================================================================*/
static void LED_OFF_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = GPIOA_BSRR 주소    */
        "LDR  r1, =%1          \n\t"   /* r1 = 0x00200000(PA5_RST)*/
        "STR  r1, [r0]         \n\t"   /* BSRR = (1<<21)          */
        :: "i"(GPIOA_BSRR_ADDR), "i"(PA5_RESET)
        : "r0","r1","memory"
    );
}

/* =========================================================================
 * Delay_ASM : SUBS/BNE 루프
 *   "+r"(count): 읽기+쓰기 피연산자 (루프에서 count 값 변경)
 *   "cc"       : SUBS가 CPSR 조건 코드 변경
 *   "1b"       : backward label 1 참조
 * =========================================================================*/
static void Delay_ASM(uint32_t count)
{
    __asm volatile (
        "1:                    \n\t"
        "SUBS %0, %0, #1       \n\t"   /* count--, Zero 플래그 갱신 */
        "BNE  1b               \n\t"   /* count≠0 → 레이블1 분기   */
        : "+r"(count) :: "cc"
    );
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
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2; huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B; huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE; huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE; huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}
void Error_Handler(void) { __disable_irq(); while (1) {} }
