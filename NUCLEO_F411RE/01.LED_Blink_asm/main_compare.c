/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main_compare.c
  * @brief          : LD2 제어 - HAL 방식 vs 인라인 어셈블러 방식 비교
  * @board          : NUCLEO-F411RE
  * @led            : LD2 = PA5
  * @sysclk         : 84MHz (HSI 16MHz → PLL: M=16, N=336, P=4)
  *
  * [동작 순서]
  *   ① HAL 방식으로  LD2를 3회 점멸  (200ms 간격, 빠름)
  *   ② 1초 정지 (구분용)
  *   ③ ASM 방식으로  LD2를 3회 점멸  (500ms 간격, 느림)
  *   ④ 1초 정지 후 처음부터 반복
  *
  * [비교 포인트]
  *   HAL_GPIO_WritePin() → 내부적으로 10+ 명령어 실행
  *   LED_ON_ASM()        → LDR + MOV + STR 단 3 명령어
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"

/* =========================================================================
 * 레지스터 주소 정의 (RM0383 기준)
 * =========================================================================*/
#define GPIOA_BSRR_ADDR     0x40020018UL   /* GPIOA BSRR 절대 주소        */
#define PA5_SET             (1UL << 5)     /* BSRR[5]  → PA5 High (ON)   */
#define PA5_RESET           (1UL << 21)    /* BSRR[21] → PA5 Low  (OFF)  */

/* =========================================================================
 * Delay 카운트 (SYSCLK=84MHz 기준)
 *   SUBS+BNE ≈ 3 사이클/루프
 *   1ms  ≈ 84,000,000 / 3 / 1000 = 28,000
 *   500ms = 14,000,000  /  200ms = 5,600,000
 * =========================================================================*/
#define DELAY_200MS_COUNT   5600000UL
#define DELAY_500MS_COUNT   14000000UL

/* ─── 함수 선언 ──────────────────────────────────────────────────────────── */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* ASM 제어 함수 */
static void LED_ON_ASM(void);
static void LED_OFF_ASM(void);
static void Delay_ASM(uint32_t count);

/* =========================================================================
 *  HAL 방식 인라인 래퍼 (원본 코드와 동일한 호출 방식)
 *    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state)
 *    LD2_GPIO_Port = GPIOA,  LD2_Pin = GPIO_PIN_5
 * =========================================================================*/
static inline void LED_ON_HAL(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

static inline void LED_OFF_HAL(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}

/* =========================================================================
 * main
 * =========================================================================*/
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    while (1)
    {
        /* ================================================================
         * [HAL 방식] LD2 3회 점멸 - 200ms 간격
         *   HAL_GPIO_WritePin() 내부 동작:
         *     1. 파라미터 확인 (assert)
         *     2. GPIO_PIN_SET 여부 분기
         *     3. GPIOx->BSRR = GPIO_PIN  또는  GPIOx->BSRR = (GPIO_PIN << 16)
         *   → 컴파일 후 약 10~15 ARM 명령어 실행
         * ================================================================*/
        for (int i = 0; i < 3; i++)
        {
            LED_ON_HAL();
            HAL_Delay(200);
            LED_OFF_HAL();
            HAL_Delay(200);
        }

        HAL_Delay(1000);   /* ── HAL / ASM 구분 정지 1초 ── */

        /* ================================================================
         * [ASM 방식] LD2 3회 점멸 - 500ms 간격
         *   LED_ON_ASM() 내부 동작:
         *     LDR r0, =0x40020018   ; BSRR 주소 로드
         *     MOV r1, #0x20         ; (1<<5) = 0x20
         *     STR r1, [r0]          ; PA5 Set → LED ON
         *   → 단 3 ARM 명령어 실행
         * ================================================================*/
        for (int i = 0; i < 3; i++)
        {
            LED_ON_ASM();
            Delay_ASM(DELAY_500MS_COUNT);
            LED_OFF_ASM();
            Delay_ASM(DELAY_500MS_COUNT);
        }

        HAL_Delay(1000);   /* ── 다음 사이클 전 정지 1초 ── */
    }
}

/* =========================================================================
 * LED_ON_ASM
 *   GPIOA->BSRR = (1<<5)  → PA5 High
 *
 *   BSRR(Bit Set Reset Register)는 쓰기 전용
 *   → 현재 핀 상태를 읽지 않고 원자적(atomic)으로 Set/Reset 가능
 *   → Read-Modify-Write 불필요 → 인터럽트 안전
 * =========================================================================*/
static void LED_ON_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 ← GPIOA_BSRR 주소           */
        "MOV  r1, %1           \n\t"   /* r1 ← 0x20  (PA5_SET, 8비트 이하→MOV 가능) */
        "STR  r1, [r0]         \n\t"   /* BSRR[5]=1 → PA5 High           */
        :
        : "i"(GPIOA_BSRR_ADDR),
          "i"(PA5_SET)
        : "r0", "r1", "memory"
    );
}

/* =========================================================================
 * LED_OFF_ASM
 *   GPIOA->BSRR = (1<<21) → PA5 Low
 *
 *   PA5_RESET = 0x00200000 (21비트) → 8비트 초과
 *   → MOV #imm8 불가 → LDR =imm32 (리터럴 풀) 사용
 * =========================================================================*/
static void LED_OFF_ASM(void)
{
    __asm volatile (
        "LDR  r0, =%0          \n\t"   /* r0 ← GPIOA_BSRR 주소           */
        "LDR  r1, =%1          \n\t"   /* r1 ← 0x00200000 (PA5_RESET)    */
        "STR  r1, [r0]         \n\t"   /* BSRR[21]=1 → PA5 Low           */
        :
        : "i"(GPIOA_BSRR_ADDR),
          "i"(PA5_RESET)
        : "r0", "r1", "memory"
    );
}

/* =========================================================================
 * Delay_ASM
 *   SUBS: count-- 후 Zero 플래그 업데이트
 *   BNE : Zero 플래그가 0(아직 남음)이면 루프 반복
 *   "1b": 뒤에서 앞쪽(backward)의 로컬 레이블 1을 참조
 *
 *   "+r"(count): 입력이자 출력 (루프에서 count를 수정하므로 +r 필수)
 *   "cc"       : SUBS 명령이 CPSR 조건 코드를 변경함을 GCC에 알림
 * =========================================================================*/
static void Delay_ASM(uint32_t count)
{
    __asm volatile (
        "1:                    \n\t"
        "SUBS %0, %0, #1       \n\t"   /* count = count - 1, 플래그 갱신 */
        "BNE  1b               \n\t"   /* count != 0 → 레이블 1로 점프   */
        : "+r"(count)
        :
        : "cc"
    );
}

/* =========================================================================
 * MX_GPIO_Init  ← 원본 main.c 와 동일
 * =========================================================================*/
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    /* B1 버튼: PC13, Falling Edge 인터럽트 */
    GPIO_InitStruct.Pin  = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    /* LD2: PA5, 출력 Push-Pull */
    GPIO_InitStruct.Pin   = LD2_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

/* =========================================================================
 * MX_USART2_UART_Init  ← 원본 main.c 와 동일
 * =========================================================================*/
UART_HandleTypeDef huart2;

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

/* =========================================================================
 * SystemClock_Config  ← 원본 main.c 와 동일
 *   HSI(16MHz) → PLL → 84MHz
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
