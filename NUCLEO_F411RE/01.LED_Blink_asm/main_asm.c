/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main_asm.c
  * @brief          : LD2 GPIO 제어 - 인라인 어셈블러 전용
  * @board          : NUCLEO-F411RE
  * @led            : LD2 = PA5
  * @sysclk         : 84MHz (HSI 16MHz → PLL: M=16, N=336, P=4)
  *
  * [인라인 어셈블러 방식]
  *   - RCC AHB1ENR  : GPIOA 클럭 활성화 (레지스터 직접 쓰기)
  *   - GPIOA MODER  : PA5 출력 모드 설정 (BIC + ORR)
  *   - GPIOA BSRR   : PA5 SET / RESET (STR 단일 명령)
  *   - Delay        : SUBS/BNE 루프 (HAL_Delay 대신 사용)
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"

/* =========================================================================
 * 레지스터 주소 정의
 *   STM32F411 Reference Manual (RM0383) 기준
 * =========================================================================*/

/* RCC */
#define RCC_BASE            0x40023800UL
#define RCC_AHB1ENR         (RCC_BASE + 0x30UL)   /* AHB1 클럭 활성화      */

/* GPIOA */
#define GPIOA_BASE          0x40020000UL
#define GPIOA_MODER         (GPIOA_BASE + 0x00UL)  /* 핀 모드 레지스터      */
#define GPIOA_BSRR          (GPIOA_BASE + 0x18UL)  /* 비트 세트/리셋 레지스터*/

/* PA5 비트 상수 */
#define GPIOAEN_BIT         (1UL << 0)             /* AHB1ENR: GPIOA 클럭   */
#define PA5_MODER_CLR       (3UL << 10)            /* MODER[11:10] 클리어   */
#define PA5_MODER_OUT       (1UL << 10)            /* MODER[11:10] = 01     */
#define PA5_SET             (1UL << 5)             /* BSRR[5]  → PA5 High  */
#define PA5_RESET           (1UL << 21)            /* BSRR[21] → PA5 Low   */

/* =========================================================================
 * Delay 카운트 계산
 *   SYSCLK = 84MHz, SUBS+BNE = 약 3 사이클/루프
 *   1ms = 84,000,000 / 3 / 1000 = 28,000 count
 *   500ms = 28,000 * 500 = 14,000,000
 * =========================================================================*/
#define DELAY_500MS         14000000UL

/* ─── 함수 선언 ──────────────────────────────────────────────────────────── */
void SystemClock_Config(void);
static void GPIO_Init_ASM(void);
static void LED_ON_ASM(void);
static void LED_OFF_ASM(void);
static void Delay_ASM(uint32_t count);

/* =========================================================================
 * main
 * =========================================================================*/
int main(void)
{
    /* HAL_Init(): SysTick 1ms 설정만 사용 (Delay는 ASM 루프로 대체) */
    HAL_Init();

    SystemClock_Config();   /* 84MHz PLL 설정 (원본 유지)               */

    GPIO_Init_ASM();        /* RCC + MODER: 인라인 어셈블러로 초기화    */

    /* ── LED 초기 상태: OFF ──────────────────────────────────────────── */
    LED_OFF_ASM();

    while (1)
    {
        LED_ON_ASM();
        Delay_ASM(DELAY_500MS);   /* ~500ms */

        LED_OFF_ASM();
        Delay_ASM(DELAY_500MS);   /* ~500ms */
    }
}

/* =========================================================================
 * GPIO_Init_ASM
 *   Step1: RCC AHB1ENR → GPIOA 클럭 ON  (주소: 0x40023830)
 *   Step2: GPIOA MODER → PA5 출력 모드  (주소: 0x40020000)
 *
 *  ARM 명령어 설명:
 *    LDR rd, =imm  : 32비트 상수를 rd에 로드 (리터럴 풀 사용)
 *    LDR rd, [rn]  : 메모리[rn] 값을 rd에 로드
 *    STR rs, [rn]  : rs 값을 메모리[rn]에 저장
 *    ORR rd,rn,rm  : rd = rn | rm  (비트 OR)
 *    BIC rd,rn,rm  : rd = rn & ~rm (비트 클리어)
 * =========================================================================*/
static void GPIO_Init_ASM(void)
{
    /* ── Step 1: GPIOA 클럭 활성화 ─────────────────────────────────────
     *   RCC->AHB1ENR |= (1 << 0)  // GPIOAEN = bit0
     * ─────────────────────────────────────────────────────────────────*/
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &RCC_AHB1ENR              */
        "LDR  r1, [r0]         \n\t"   /* r1 = RCC_AHB1ENR 현재값        */
        "ORR  r1, r1, %1       \n\t"   /* r1 |= GPIOAEN (bit0=1)         */
        "STR  r1, [r0]         \n\t"   /* RCC_AHB1ENR 업데이트           */
        :                              /* 출력 피연산자: 없음             */
        : "i"(RCC_AHB1ENR),           /* %0 = 주소 상수 (immediate)      */
          "i"(GPIOAEN_BIT)            /* %1 = 비트 마스크               */
        : "r0", "r1", "memory"         /* 클로버: 변경된 레지스터 명시   */
    );

    /* ── Step 2: PA5 출력 모드 설정 ────────────────────────────────────
     *   GPIOA->MODER: bit[11:10] = 01 (General purpose output)
     *   BIC로 기존 비트 클리어 후 ORR로 새 값 세트
     * ─────────────────────────────────────────────────────────────────*/
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &GPIOA_MODER              */
        "LDR  r1, [r0]         \n\t"   /* r1 = GPIOA_MODER 현재값        */
        "LDR  r2, =%1          \n\t"   /* r2 = PA5_MODER_CLR (3<<10)     */
        "BIC  r1, r1, r2       \n\t"   /* r1 &= ~(3<<10)  → 비트 클리어  */
        "LDR  r2, =%2          \n\t"   /* r2 = PA5_MODER_OUT (1<<10)     */
        "ORR  r1, r1, r2       \n\t"   /* r1 |= (1<<10)   → 출력 모드    */
        "STR  r1, [r0]         \n\t"   /* GPIOA_MODER 업데이트           */
        :
        : "i"(GPIOA_MODER),
          "i"(PA5_MODER_CLR),
          "i"(PA5_MODER_OUT)
        : "r0", "r1", "r2", "memory"
    );
}

/* =========================================================================
 * LED_ON_ASM
 *   GPIOA->BSRR = (1 << 5)   → PA5 High (LED ON)
 *
 *   BSRR는 쓰기 전용 레지스터 → 읽기 없이 바로 STR
 *   bit[ 0..15]: Set  → 해당 핀 High
 *   bit[16..31]: Reset→ 해당 핀 Low
 * =========================================================================*/
static void LED_ON_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &GPIOA_BSRR               */
        "MOV  r1, %1           \n\t"   /* r1 = PA5_SET = (1<<5) = 0x20   */
        "STR  r1, [r0]         \n\t"   /* BSRR = 0x20 → PA5 High         */
        :
        : "i"(GPIOA_BSRR),
          "i"(PA5_SET)
        : "r0", "r1", "memory"
    );
}

/* =========================================================================
 * LED_OFF_ASM
 *   GPIOA->BSRR = (1 << 21)  → PA5 Low (LED OFF)
 *   PA5_RESET = 0x00200000  (> 8비트 → MOV 불가, LDR 사용)
 * =========================================================================*/
static void LED_OFF_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 = &GPIOA_BSRR               */
        "LDR  r1, =%1          \n\t"   /* r1 = PA5_RESET = (1<<21)        */
        "STR  r1, [r0]         \n\t"   /* BSRR = (1<<21) → PA5 Low       */
        :
        : "i"(GPIOA_BSRR),
          "i"(PA5_RESET)
        : "r0", "r1", "memory"
    );
}

/* =========================================================================
 * Delay_ASM
 *   SUBS/BNE 루프: count가 0이 될 때까지 1씩 감소
 *   SYSCLK=84MHz 기준: count=14,000,000 → 약 500ms
 *
 *   "+r"(count) : 읽기+쓰기 피연산자 (루프 내에서 count 값 수정)
 *   "cc"        : 클로버에 cc(condition code) 명시 → SUBS가 플래그 변경
 * =========================================================================*/
static void Delay_ASM(uint32_t count)
{
    __asm volatile (
        "1:                    \n\t"   /* 로컬 레이블 1:                  */
        "SUBS %0, %0, #1       \n\t"   /* count-- , Zero 플래그 업데이트 */
        "BNE  1b               \n\t"   /* Zero != 0 이면 레이블 1로 분기 */
        : "+r"(count)                  /* 출력+입력: count (읽고 씀)      */
        :                              /* 별도 입력 없음                  */
        : "cc"                         /* 조건 코드 레지스터 변경됨       */
    );
}

/* =========================================================================
 * SystemClock_Config  ← 원본 main.c 에서 그대로 유지
 *   HSI 16MHz → PLL → SYSCLK 84MHz
 *   PLLM=16, PLLN=336, PLLP=4  ⇒  16/16*336/4 = 84MHz
 * =========================================================================*/
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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
