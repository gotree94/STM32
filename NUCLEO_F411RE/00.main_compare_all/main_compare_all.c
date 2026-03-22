/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file     : main_compare_all.c
  * @brief    : STM32 GPIO 제어 5가지 방식 총망라 비교
  * @board    : NUCLEO-F411RE
  * @led      : LD2 = PA5
  * @uart     : USART2 PA2(TX) 115200bps → ST-Link Virtual COM
  * @sysclk   : 84MHz (HSI 16MHz → PLL M=16, N=336, P=4)
  *
  * ╔══════════════════════════════════════════════════════════════╗
  * ║  STM32 GPIO 제어 5가지 방식 계층 구조                       ║
  * ║                                                              ║
  * ║  [높음] HAL   → HAL_GPIO_WritePin()     ST 고수준 API       ║
  * ║          LL    → LL_GPIO_SetOutputPin()  ST 저수준 API       ║
  * ║          CMSIS → GPIOA->BSRR = pin      ARM 표준 구조체     ║
  * ║          REG   → *(volatile*)0x40020018  절대주소 직접접근   ║
  * ║  [낮음] ASM   → __asm volatile(STR)     ARM 명령어 직접     ║
  * ╚══════════════════════════════════════════════════════════════╝
  *
  * [DWT 측정 항목]
  *   각 방식으로 PA5를 SET(ON) / RESET(OFF) 하는 사이클 수 측정
  *   10회 측정 후 최솟값 사용 (캐시/파이프라인 안정화)
  *
  * [UART 출력 예시]
  *   ╔══════════════════════════════════════════╗
  *   ║  STM32F411 GPIO Control Method Compare  ║
  *   ║  NUCLEO-F411RE | SYSCLK: 84MHz          ║
  *   ╚══════════════════════════════════════════╝
  *   [DWT Cycle Measurement - GPIO PA5 SET]
  *   ① HAL    HAL_GPIO_WritePin(SET)  : 13 cycles  ( 0.15 us)
  *   ② LL     LL_GPIO_SetOutputPin()  :  8 cycles  ( 0.10 us)
  *   ③ CMSIS  GPIOA->BSRR = pin      :  5 cycles  ( 0.06 us)
  *   ④ REG    *(volatile*)addr = val  :  4 cycles  ( 0.05 us)
  *   ⑤ ASM    __asm volatile(STR)    :  3 cycles  ( 0.04 us)
  *   [LED Blink: 5가지 방식 순서대로 점멸]
  *
  * [LL 사용 설정]
  *   CubeIDE: Project Manager → Advanced Settings
  *   GPIO → Driver: LL 선택
  *   또는 #include "stm32f4xx_ll_gpio.h" 직접 포함
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "stm32f4xx_ll_gpio.h"      /* LL GPIO API */
#include "stm32f4xx_ll_bus.h"       /* LL RCC/BUS  */
#include <stdio.h>
#include <string.h>

/* =========================================================================
 * 레지스터 직접 접근용 주소 상수 (방법 4: REG)
 *   STM32F411 Reference Manual (RM0383) 기준
 * =========================================================================*/
#define GPIOA_BASE_ADDR     0x40020000UL
#define GPIOA_MODER_REG     (*(volatile uint32_t *)(GPIOA_BASE_ADDR + 0x00UL))
#define GPIOA_BSRR_REG      (*(volatile uint32_t *)(GPIOA_BASE_ADDR + 0x18UL))

#define PA5_SET_VAL         (1UL <<  5)   /* BSRR[5]  → PA5 High */
#define PA5_RESET_VAL       (1UL << 21)   /* BSRR[21] → PA5 Low  */

/* 인라인 어셈블러용 즉시값 상수 (방법 5: ASM) */
#define GPIOA_BSRR_ADDR     (GPIOA_BASE_ADDR + 0x18UL)   /* 0x40020018 */

/* =========================================================================
 * DWT Cycle Counter 레지스터
 * =========================================================================*/
#define DWT_CYCCNT          (*(volatile uint32_t *)0xE0001004UL)
#define DWT_CTRL            (*(volatile uint32_t *)0xE0001000UL)

/* =========================================================================
 * 측정 설정
 * =========================================================================*/
#define MEASURE_ITER        10          /* 측정 반복 횟수 (최솟값 채택)   */
#define BLINK_DELAY_MS      300         /* LED 점멸 간격                   */

/* ─── 함수 선언 ──────────────────────────────────────────────────────────── */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void DWT_Init(void);

/* 측정 함수 */
static uint32_t Measure_MIN(uint32_t *arr, int n);
static void     Measure_All(uint32_t results[5][2]);

/* LED 제어 - 5가지 방식 SET/RESET */
static void LED_SET_HAL(void);
static void LED_RST_HAL(void);
static void LED_SET_LL(void);
static void LED_RST_LL(void);
static void LED_SET_CMSIS(void);
static void LED_RST_CMSIS(void);
static void LED_SET_REG(void);
static void LED_RST_REG(void);
static void LED_SET_ASM(void);
static void LED_RST_ASM(void);

/* UART 출력 */
static void UART_Print(const char *str);
static void UART_PrintResult(int idx, const char *method, const char *func,
                              uint32_t cyc_set, uint32_t cyc_rst, uint32_t mhz);
static void UART_u32(uint32_t v, char *buf, int *len);

/* HAL 핸들 */
UART_HandleTypeDef huart2;

/* =========================================================================
 * main
 * =========================================================================*/
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    DWT_Init();

    HAL_Delay(200);

    /* ── 헤더 출력 ──────────────────────────────────────────────────────*/
    UART_Print("\r\n");
    UART_Print("╔══════════════════════════════════════════════════╗\r\n");
    UART_Print("║   STM32F411 GPIO Control Method Comparison      ║\r\n");
    UART_Print("║   NUCLEO-F411RE  |  SYSCLK: 84MHz  |  PA5(LD2) ║\r\n");
    UART_Print("╠══════════════════════════════════════════════════╣\r\n");
    UART_Print("║  ① HAL   HAL_GPIO_WritePin()    ST 고수준 API  ║\r\n");
    UART_Print("║  ② LL    LL_GPIO_SetOutputPin() ST 저수준 API  ║\r\n");
    UART_Print("║  ③ CMSIS GPIOA->BSRR = pin      ARM 구조체     ║\r\n");
    UART_Print("║  ④ REG   *(volatile uint32_t*)  절대주소 직접  ║\r\n");
    UART_Print("║  ⑤ ASM   __asm volatile(STR)    ARM 명령어     ║\r\n");
    UART_Print("╚══════════════════════════════════════════════════╝\r\n\r\n");

    /* ====================================================================
     * STEP 1: DWT 사이클 측정
     *   10회 반복 → 최솟값 채택 (캐시/분기예측 안정화 이후 순수 실행)
     * ==================================================================== */
    uint32_t results[5][2];   /* [방식][0=SET사이클, 1=RST사이클] */
    Measure_All(results);

    /* ── 결과 출력 ──────────────────────────────────────────────────────*/
    UART_Print("[STEP 1] DWT Cycle Measurement (10회 최솟값)\r\n");
    UART_Print("──────────────────────────────────────────────────────────\r\n");
    UART_Print(" #  방식    함수/코드                   SET      RESET\r\n");
    UART_Print("──────────────────────────────────────────────────────────\r\n");

    UART_PrintResult(1, "HAL  ", "HAL_GPIO_WritePin()    ",
                     results[0][0], results[0][1], 84);
    UART_PrintResult(2, "LL   ", "LL_GPIO_SetOutputPin() ",
                     results[1][0], results[1][1], 84);
    UART_PrintResult(3, "CMSIS", "GPIOA->BSRR = pin      ",
                     results[2][0], results[2][1], 84);
    UART_PrintResult(4, "REG  ", "*(volatile uint32_t*)  ",
                     results[3][0], results[3][1], 84);
    UART_PrintResult(5, "ASM  ", "__asm volatile(STR)    ",
                     results[4][0], results[4][1], 84);

    UART_Print("──────────────────────────────────────────────────────────\r\n");
    UART_Print(" 기준: ASM SET = 3 cycles = 0.036us @ 84MHz\r\n\r\n");

    /* ── 계층 설명 ──────────────────────────────────────────────────────*/
    UART_Print("[방식별 특성 요약]\r\n");
    UART_Print(" HAL  : 파라미터 검증+상태기계+에러처리 → 최다 오버헤드\r\n");
    UART_Print(" LL   : 인라인 함수로 레지스터 직접 → HAL 절반 수준\r\n");
    UART_Print(" CMSIS: 구조체 포인터 → LL과 동급, ARM 표준\r\n");
    UART_Print(" REG  : volatile 포인터 역참조 → 최소 C 코드\r\n");
    UART_Print(" ASM  : LDR+MOV/LDR+STR 3명령어 → 이론적 최솟값\r\n\r\n");

    UART_Print("[STEP 2] LED 순서대로 점멸 (각 방식 3회 × BLINK_DELAY_MS)\r\n");
    UART_Print("──────────────────────────────────────────────────────────\r\n\r\n");

    /* ====================================================================
     * STEP 2: 각 방식으로 LED 순서대로 점멸
     *   → LED 동작은 모두 동일 (기능 동등)
     *   → 차이는 DWT 사이클 수에 있음
     * ==================================================================== */
    uint32_t loop = 0;

    while (1)
    {
        loop++;

        /* ① HAL ─────────────────────────────────────────────────────────*/
        UART_Print("[① HAL  ] 3x blink\r\n");
        for (int i = 0; i < 3; i++) {
            LED_SET_HAL();  HAL_Delay(BLINK_DELAY_MS);
            LED_RST_HAL();  HAL_Delay(BLINK_DELAY_MS);
        }

        /* ② LL ──────────────────────────────────────────────────────────*/
        UART_Print("[② LL   ] 3x blink\r\n");
        for (int i = 0; i < 3; i++) {
            LED_SET_LL();   HAL_Delay(BLINK_DELAY_MS);
            LED_RST_LL();   HAL_Delay(BLINK_DELAY_MS);
        }

        /* ③ CMSIS ───────────────────────────────────────────────────────*/
        UART_Print("[③ CMSIS] 3x blink\r\n");
        for (int i = 0; i < 3; i++) {
            LED_SET_CMSIS(); HAL_Delay(BLINK_DELAY_MS);
            LED_RST_CMSIS(); HAL_Delay(BLINK_DELAY_MS);
        }

        /* ④ REG ─────────────────────────────────────────────────────────*/
        UART_Print("[④ REG  ] 3x blink\r\n");
        for (int i = 0; i < 3; i++) {
            LED_SET_REG();  HAL_Delay(BLINK_DELAY_MS);
            LED_RST_REG();  HAL_Delay(BLINK_DELAY_MS);
        }

        /* ⑤ ASM ─────────────────────────────────────────────────────────*/
        UART_Print("[⑤ ASM  ] 3x blink\r\n");
        for (int i = 0; i < 3; i++) {
            LED_SET_ASM();  HAL_Delay(BLINK_DELAY_MS);
            LED_RST_ASM();  HAL_Delay(BLINK_DELAY_MS);
        }

        UART_Print("[NOTE] LED looks identical for all methods\r\n");
        UART_Print("[NOTE] Difference exists only in CPU cycles\r\n\r\n");

        HAL_Delay(1000);

        /* 5루프마다 측정 결과 재출력 */
        if (loop % 5 == 0) {
            UART_Print("══════════ Re-measurement ══════════\r\n");
            Measure_All(results);
            UART_PrintResult(1,"HAL  ","HAL_GPIO_WritePin()    ",results[0][0],results[0][1],84);
            UART_PrintResult(2,"LL   ","LL_GPIO_SetOutputPin() ",results[1][0],results[1][1],84);
            UART_PrintResult(3,"CMSIS","GPIOA->BSRR = pin      ",results[2][0],results[2][1],84);
            UART_PrintResult(4,"REG  ","*(volatile uint32_t*)  ",results[3][0],results[3][1],84);
            UART_PrintResult(5,"ASM  ","__asm volatile(STR)    ",results[4][0],results[4][1],84);
            UART_Print("════════════════════════════════════\r\n\r\n");
        }
    }
}

/* =========================================================================
 * DWT_Init
 * =========================================================================*/
static void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT_CYCCNT = 0;
    DWT_CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

/* =========================================================================
 * Measure_MIN : 배열에서 최솟값 반환
 * =========================================================================*/
static uint32_t Measure_MIN(uint32_t *arr, int n)
{
    uint32_t m = arr[0];
    for (int i = 1; i < n; i++) if (arr[i] < m) m = arr[i];
    return m;
}

/* =========================================================================
 * Measure_All : 5가지 방식 모두 10회 측정 → 최솟값 저장
 * =========================================================================*/
static void Measure_All(uint32_t results[5][2])
{
    uint32_t t_start, t_end;
    uint32_t buf[MEASURE_ITER];

    /* ── ① HAL ─────────────────────────────────────────────────────────*/
    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[0][0] = Measure_MIN(buf, MEASURE_ITER);

    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[0][1] = Measure_MIN(buf, MEASURE_ITER);

    /* ── ② LL ──────────────────────────────────────────────────────────*/
    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[1][0] = Measure_MIN(buf, MEASURE_ITER);

    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[1][1] = Measure_MIN(buf, MEASURE_ITER);

    /* ── ③ CMSIS ────────────────────────────────────────────────────────*/
    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        GPIOA->BSRR = GPIO_PIN_5;
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[2][0] = Measure_MIN(buf, MEASURE_ITER);

    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        GPIOA->BSRR = (uint32_t)GPIO_PIN_5 << 16U;
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[2][1] = Measure_MIN(buf, MEASURE_ITER);

    /* ── ④ REG (절대주소 직접 접근) ─────────────────────────────────────*/
    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        GPIOA_BSRR_REG = PA5_SET_VAL;
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[3][0] = Measure_MIN(buf, MEASURE_ITER);

    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        GPIOA_BSRR_REG = PA5_RESET_VAL;
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[3][1] = Measure_MIN(buf, MEASURE_ITER);

    /* ── ⑤ ASM (인라인 어셈블러) ────────────────────────────────────────*/
    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        __asm volatile (
            "LDR r0, =%0 \n\t"
            "MOV r1, %1  \n\t"
            "STR r1, [r0]\n\t"
            :: "i"(GPIOA_BSRR_ADDR), "i"(PA5_SET_VAL)
            : "r0","r1","memory"
        );
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[4][0] = Measure_MIN(buf, MEASURE_ITER);

    for (int i = 0; i < MEASURE_ITER; i++) {
        DWT_CYCCNT = 0; t_start = DWT_CYCCNT;
        __asm volatile (
            "LDR r0, =%0 \n\t"
            "LDR r1, =%1 \n\t"
            "STR r1, [r0]\n\t"
            :: "i"(GPIOA_BSRR_ADDR), "i"(PA5_RESET_VAL)
            : "r0","r1","memory"
        );
        t_end = DWT_CYCCNT; buf[i] = t_end - t_start;
    }
    results[4][1] = Measure_MIN(buf, MEASURE_ITER);
}

/* =========================================================================
 * LED 제어 함수 - 5가지 방식
 * =========================================================================*/

/* ① HAL ─────────────────────────────────────────────────────────────────
 * HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState)
 *   내부: 파라미터 assert → PinState 분기 → GPIOx->BSRR 쓰기
 *   오버헤드: 파라미터 검증 + 함수 호출 스택 + 분기
 */
static void LED_SET_HAL(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}
static void LED_RST_HAL(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/* ② LL ──────────────────────────────────────────────────────────────────
 * LL_GPIO_SetOutputPin(GPIOx, PinMask)
 *   내부 (stm32f4xx_ll_gpio.h 인라인):
 *     SET_BIT(GPIOx->BSRR, PinMask)
 *     → GPIOx->BSRR |= PinMask  (단 1~2 명령어)
 *   HAL보다 빠름, 에러 처리/타임아웃 없음
 */
static void LED_SET_LL(void) {
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
}
static void LED_RST_LL(void) {
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
}

/* ③ CMSIS ───────────────────────────────────────────────────────────────
 * GPIOA->BSRR = GPIO_PIN_5
 *   CMSIS: ARM 표준 레지스터 구조체 (stm32f411xe.h 정의)
 *   typedef struct { __IO uint32_t MODER; ... __IO uint32_t BSRR; } GPIO_TypeDef;
 *   #define GPIOA ((GPIO_TypeDef *) GPIOA_BASE)
 *   컴파일러가 구조체 오프셋을 자동 계산 → BSRR에 직접 STR
 *
 *   SET:   BSRR = (1<<5)         → bit5=1  PA5 High
 *   RESET: BSRR = (1<<5)<<16     → bit21=1 PA5 Low
 */
static void LED_SET_CMSIS(void) {
    GPIOA->BSRR = GPIO_PIN_5;
}
static void LED_RST_CMSIS(void) {
    GPIOA->BSRR = (uint32_t)GPIO_PIN_5 << 16U;
}

/* ④ REG (절대주소 직접 접근) ────────────────────────────────────────────
 * GPIOA_BSRR_REG = PA5_SET_VAL
 *   #define GPIOA_BSRR_REG (*(volatile uint32_t *)0x40020018UL)
 *   C 레벨 최저: volatile 포인터 역참조로 레지스터 직접 쓰기
 *   CMSIS와 생성 코드 동일, 이식성만 다름
 */
static void LED_SET_REG(void) {
    GPIOA_BSRR_REG = PA5_SET_VAL;
}
static void LED_RST_REG(void) {
    GPIOA_BSRR_REG = PA5_RESET_VAL;
}

/* ⑤ ASM (인라인 어셈블러) ───────────────────────────────────────────────
 * __asm volatile("LDR r0,=%0 \n\t MOV r1,%1 \n\t STR r1,[r0]")
 *   SET:   PA5_SET=0x20(32) ≤255 → MOV #imm8 가능 → 3 명령어
 *   RESET: PA5_RESET=0x200000 >255 → LDR =imm32 필요 → 3 명령어
 *   이론적 최소: LDR(주소) + MOV or LDR(값) + STR(쓰기)
 */
static void LED_SET_ASM(void) {
    __asm volatile (
        "LDR  r0, =%0  \n\t"   /* r0 = GPIOA_BSRR 주소 (32비트)         */
        "MOV  r1, %1   \n\t"   /* r1 = 0x20 (PA5_SET, 8비트 → MOV 가능) */
        "STR  r1, [r0] \n\t"   /* BSRR = 0x20 → PA5 High                */
        :: "i"(GPIOA_BSRR_ADDR), "i"(PA5_SET_VAL)
        : "r0", "r1", "memory"
    );
}
static void LED_RST_ASM(void) {
    __asm volatile (
        "LDR  r0, =%0  \n\t"   /* r0 = GPIOA_BSRR 주소                  */
        "LDR  r1, =%1  \n\t"   /* r1 = 0x00200000 (8비트 초과 → LDR)    */
        "STR  r1, [r0] \n\t"   /* BSRR = (1<<21) → PA5 Low              */
        :: "i"(GPIOA_BSRR_ADDR), "i"(PA5_RESET_VAL)
        : "r0", "r1", "memory"
    );
}

/* =========================================================================
 * UART_Print / UART_PrintResult / UART_u32
 * =========================================================================*/
static void UART_Print(const char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/* uint32_t → 문자열 (sprintf 미사용) */
static void UART_u32(uint32_t v, char *buf, int *len)
{
    *len = 0;
    if (v == 0) { buf[(*len)++] = '0'; buf[*len] = '\0'; return; }
    char tmp[12]; int t = 0;
    while (v > 0) { tmp[t++] = '0' + (char)(v % 10); v /= 10; }
    for (int i = t - 1; i >= 0; i--) buf[(*len)++] = tmp[i];
    buf[*len] = '\0';
}

/*
 * 출력 형식:
 *  idx method func : SET_cyc cycles (SET_us us)  |  RST_cyc cycles (RST_us us)
 */
static void UART_PrintResult(int idx, const char *method, const char *func,
                              uint32_t cyc_set, uint32_t cyc_rst, uint32_t mhz)
{
    char line[120];
    char s_cyc[12], r_cyc[12], s_us[12], r_us[12], s_uf[4], r_uf[4];
    int lsc, lrc, lsu, lru;

    UART_u32(cyc_set,           s_cyc, &lsc);
    UART_u32(cyc_rst,           r_cyc, &lrc);
    UART_u32(cyc_set / mhz,     s_us,  &lsu);
    UART_u32(cyc_rst / mhz,     r_us,  &lru);

    uint32_t sf = (cyc_set % mhz) * 100 / mhz;
    uint32_t rf = (cyc_rst % mhz) * 100 / mhz;
    s_uf[0] = '0' + (char)(sf / 10); s_uf[1] = '0' + (char)(sf % 10); s_uf[2] = '\0';
    r_uf[0] = '0' + (char)(rf / 10); r_uf[1] = '0' + (char)(rf % 10); r_uf[2] = '\0';

    /* 조립 */
    int p = 0;
    char idxc = '0' + (char)idx;
    line[p++] = ' '; line[p++] = idxc; line[p++] = ' ';
    for (int i = 0; method[i]; i++) line[p++] = method[i];
    line[p++] = ' ';
    for (int i = 0; func[i]; i++)   line[p++] = func[i];
    line[p++] = ':'; line[p++] = ' ';

    /* SET */
    /* 우측정렬 5자리 */
    int pad = 5 - lsc; while (pad-- > 0) line[p++] = ' ';
    for (int i = 0; s_cyc[i]; i++) line[p++] = s_cyc[i];
    const char *u1 = " cyc (";
    for (int i = 0; u1[i]; i++) line[p++] = u1[i];
    for (int i = 0; s_us[i]; i++)  line[p++] = s_us[i];
    line[p++] = '.'; line[p++] = s_uf[0]; line[p++] = s_uf[1];
    const char *u2 = "us)  | RST: ";
    for (int i = 0; u2[i]; i++) line[p++] = u2[i];

    /* RESET */
    pad = 5 - lrc; while (pad-- > 0) line[p++] = ' ';
    for (int i = 0; r_cyc[i]; i++) line[p++] = r_cyc[i];
    const char *u3 = " cyc (";
    for (int i = 0; u3[i]; i++) line[p++] = u3[i];
    for (int i = 0; r_us[i]; i++)  line[p++] = r_us[i];
    line[p++] = '.'; line[p++] = r_uf[0]; line[p++] = r_uf[1];
    const char *u4 = "us)\r\n";
    for (int i = 0; u4[i]; i++) line[p++] = u4[i];
    line[p] = '\0';

    UART_Print(line);
}

/* =========================================================================
 * 주변장치 초기화
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

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin  = GPIO_PIN_13;   /* B1 */
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = GPIO_PIN_5;   /* LD2 */
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance          = USART2;
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
