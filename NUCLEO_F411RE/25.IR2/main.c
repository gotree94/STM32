/* ============================================================
 * LG TV IR Remote Control - STM32F411 Nucleo
 * ============================================================
 * 프로토콜  : NEC IR Protocol (38kHz 반송파)
 * LG TV Power Toggle 코드: 0x20DF10EF
 *
 * 하드웨어 연결:
 *   IR LED (+) ─── 100Ω ─── PA8 (TIM1_CH1)
 *   IR LED (-) ─── GND
 *   버튼      ─── PC13 (Nucleo User Button, Active LOW)
 *
 * TIM 구성:
 *   TIM1 CH1 PWM  : 38kHz 반송파 생성 (IR LED 변조)
 *   TIM2          : NEC 타이밍 기준 (1μs 단위 딜레이)
 *
 * 클럭 구성 (STM32F411RE Nucleo 기본값):
 *   HSI(16MHz) → PLL → SYSCLK = 100MHz
 *   APB1 = 50MHz  → TIM2 클럭 = 100MHz (×2 배수 규칙)
 *   APB2 = 100MHz → TIM1 클럭 = 100MHz (×1)
 * ============================================================ */

#include "main.h"

/* ─── LG TV NEC 코드 ──────────────────────────────────────
 *   Address    : 0x04  (8-bit)
 *   ~Address   : 0xFB  (반전)
 *   Command    : 0x08  (Power Toggle)
 *   ~Command   : 0xF7  (반전)
 *   32비트     : 0x20DF10EF  (LSB-first NEC 포맷)
 * ─────────────────────────────────────────────────────── */
#define LG_TV_POWER      0x20DF10EF   /* Power On/Off 토글 (일반 사용) */
#define LG_TV_POWER_ON   0x20DFB34C  /* Power On  전용                */
#define LG_TV_POWER_OFF  0x20DFA35C  /* Power Off 전용                */

/* ─── NEC 프로토콜 타이밍 (단위: μs) ─────────────────── */
#define NEC_LEADER_MARK   9000   /* 리더 마크       9.0ms */
#define NEC_LEADER_SPACE  4500   /* 리더 스페이스   4.5ms */
#define NEC_BIT_MARK       560   /* 비트 마크       560μs */
#define NEC_ONE_SPACE     1690   /* '1' 스페이스   1.69ms */
#define NEC_ZERO_SPACE     560   /* '0' 스페이스    560μs */
#define NEC_REPEAT_SPACE  2250   /* 반복 프레임 스페이스  */

/* ─── TIM1 PWM 설정 (38kHz 반송파) ────────────────────
 *  SYSCLK = 100MHz, APB2 = 100MHz
 *  APB2 프리스케일러 = 1  →  TIM1 클럭 = 100MHz
 *
 *  PSC  = 0    → TIM1 동작 클럭 = 100,000,000 Hz
 *  ARR  = 2631 → 주파수 = 100,000,000 / 2632 ≈ 38,004 Hz ≈ 38kHz
 *  CCR1 = 877  → 듀티비 = 877 / 2632 ≈ 33%  (IR LED 표준)
 * ─────────────────────────────────────────────────── */
#define IR_TIM_PSC    0
#define IR_TIM_ARR    2631
#define IR_TIM_CCR    877

/* ─── TIM2 딜레이 설정 (1μs 기준) ─────────────────────
 *  APB1 = 50MHz, APB1 프리스케일러 = 2
 *  → TIM2 클럭 = 50MHz × 2 = 100MHz  (타이머 배수 규칙)
 *
 *  PSC  = 99   → 100,000,000 / 100 = 1,000,000 Hz
 *  ARR  = 0xFFFFFFFF (32비트 최대)
 *  → 1틱 = 1μs
 * ─────────────────────────────────────────────────── */
#define DELAY_TIM_PSC   99

/* ─── 주변장치 핸들 ───────────────────────────────────── */
TIM_HandleTypeDef htim1;   /* PWM 출력 (PA8) */
TIM_HandleTypeDef htim2;   /* μs 딜레이 타이머 */

/* ─── 함수 선언 ───────────────────────────────────────── */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);

static void IR_CarrierOn(void);
static void IR_CarrierOff(void);
static void IR_DelayUs(uint32_t us);
static void IR_SendMark(uint32_t duration_us);
static void IR_SendSpace(uint32_t duration_us);
static void NEC_SendFrame(uint32_t code);
static void NEC_SendRepeat(void);


/* ============================================================
 * main()
 * ============================================================ */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();

    /* TIM2 시작 (μs 카운터) */
    HAL_TIM_Base_Start(&htim2);

    /* IR LED 초기 OFF */
    IR_CarrierOff();

    uint8_t btn_prev = GPIO_PIN_SET;   /* PC13: Active LOW */

    while (1)
    {
        uint8_t btn_now = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

        /* 버튼 눌림 감지 (falling edge) */
        if ((btn_prev == GPIO_PIN_SET) && (btn_now == GPIO_PIN_RESET))
        {
            HAL_Delay(20);   /* 디바운싱 20ms */
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
            {
                /* ★ LG TV 전원 토글 신호 전송 ★ */
                NEC_SendFrame(LG_TV_POWER);

                /* 시각적 피드백: LD2 (PA5) 토글 */
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            }
        }
        btn_prev = btn_now;

        HAL_Delay(10);
    }
}


/* ============================================================
 * NEC 프로토콜 구현
 * ============================================================ */

/**
 * @brief  38kHz 반송파 ON (IR LED 점등)
 *         TIM1 CH1 PWM 출력 활성화
 */
static void IR_CarrierOn(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, IR_TIM_CCR);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

/**
 * @brief  38kHz 반송파 OFF (IR LED 소등)
 *         TIM1 CH1 PWM 출력 정지
 */
static void IR_CarrierOff(void)
{
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

/**
 * @brief  TIM2 기반 마이크로초 딜레이
 * @param  us: 대기 시간 (μs)
 *
 *  TIM2 PSC=99, 클럭=100MHz → 1틱=1μs
 *  카운터를 0으로 초기화 후 목표값 도달까지 폴링
 */
static void IR_DelayUs(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

/**
 * @brief  반송파 ON 구간 (마크) 생성
 * @param  duration_us: 마크 길이 (μs)
 */
static void IR_SendMark(uint32_t duration_us)
{
    IR_CarrierOn();
    IR_DelayUs(duration_us);
}

/**
 * @brief  반송파 OFF 구간 (스페이스) 생성
 * @param  duration_us: 스페이스 길이 (μs)
 */
static void IR_SendSpace(uint32_t duration_us)
{
    IR_CarrierOff();
    IR_DelayUs(duration_us);
}

/**
 * @brief  NEC 32비트 프레임 전송
 * @param  code: 32비트 NEC 코드 (예: 0x20DF10EF)
 *
 *  NEC 프레임 구조:
 *    [리더 마크 9ms] [리더 스페이스 4.5ms]
 *    [32비트 데이터 LSB first]
 *    [종료 마크 560μs]
 *
 *  비트 인코딩:
 *    '0' = 마크 560μs + 스페이스  560μs
 *    '1' = 마크 560μs + 스페이스 1690μs
 */
static void NEC_SendFrame(uint32_t code)
{
    /* 1. 리더 코드 */
    IR_SendMark(NEC_LEADER_MARK);
    IR_SendSpace(NEC_LEADER_SPACE);

    /* 2. 32비트 데이터 (LSB first) */
    for (int i = 0; i < 32; i++)
    {
        IR_SendMark(NEC_BIT_MARK);
        if (code & (1UL << i))
            IR_SendSpace(NEC_ONE_SPACE);   /* 비트 '1' */
        else
            IR_SendSpace(NEC_ZERO_SPACE);  /* 비트 '0' */
    }

    /* 3. 종료 마크 (Stop bit) */
    IR_SendMark(NEC_BIT_MARK);
    IR_CarrierOff();

    /* 4. 프레임 후 최소 40ms 간격 */
    HAL_Delay(40);
}

/**
 * @brief  NEC 반복 프레임 전송 (버튼 길게 누를 때 사용)
 *         리더 마크 9ms + 스페이스 2.25ms + 종료 마크 560μs
 */
static void NEC_SendRepeat(void)
{
    IR_SendMark(NEC_LEADER_MARK);
    IR_SendSpace(NEC_REPEAT_SPACE);
    IR_SendMark(NEC_BIT_MARK);
    IR_CarrierOff();
    HAL_Delay(40);
}


/* ============================================================
 * STM32 주변장치 초기화
 * ============================================================ */

/**
 * @brief  시스템 클럭: HSI → PLL → SYSCLK 100MHz
 *
 *  HSI = 16MHz
 *  PLL_M=8, PLL_N=200, PLL_P=4
 *  → VCO = 16 / 8 × 200 = 400MHz
 *  → SYSCLK = 400 / 4 = 100MHz
 *
 *  AHB  프리스케일러 = 1 → HCLK  = 100MHz
 *  APB1 프리스케일러 = 2 → PCLK1 =  50MHz  → TIM2 클럭 = 100MHz
 *  APB2 프리스케일러 = 1 → PCLK2 = 100MHz  → TIM1 클럭 = 100MHz
 *
 *  ★ TIM 클럭 배수 규칙 (RM0383):
 *    APBx PSC = 1 → TIMx 클럭 = APBx 클럭
 *    APBx PSC > 1 → TIMx 클럭 = APBx 클럭 × 2
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 8;
    RCC_OscInitStruct.PLL.PLLN            = 200;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ            = 4;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    /* FLASH_LATENCY_3: 100MHz, 2.7~3.6V 조건 */
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
}

/**
 * @brief  GPIO 초기화
 *   PA5  : LD2 User LED (Output Push-Pull)
 *   PA8  : TIM1_CH1 IR LED PWM 출력 (AF1)
 *   PC13 : User Button (Input Pull-Up, Active LOW)
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* PA5 - LD2 User LED */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA8 - TIM1_CH1 (AF1) */
    GPIO_InitStruct.Pin       = GPIO_PIN_8;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PC13 - User Button (Active LOW) */
    GPIO_InitStruct.Pin  = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
 * @brief  TIM1 초기화 - 38kHz PWM 반송파 생성 (PA8)
 *
 *  TIM1 클럭 = 100MHz (APB2 = 100MHz, PSC=1)
 *  PSC  = 0    → TIM1 동작 클럭 = 100MHz
 *  ARR  = 2631 → 100,000,000 / 2632 ≈ 38,004 Hz ≈ 38kHz
 *  CCR1 = 877  → 877 / 2632 ≈ 33% 듀티비
 *
 *  ※ TIM1은 Advanced Timer → BDTR 레지스터의
 *    MOE(Main Output Enable) 비트 활성화 필수
 */
static void MX_TIM1_Init(void)
{
    TIM_ClockConfigTypeDef           sClockSourceConfig  = {0};
    TIM_OC_InitTypeDef               sConfigOC           = {0};
    TIM_BreakDeadTimeConfigTypeDef   sBreakDeadTimeConfig = {0};

    __HAL_RCC_TIM1_CLK_ENABLE();

    htim1.Instance               = TIM1;
    htim1.Init.Prescaler         = IR_TIM_PSC;
    htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim1.Init.Period            = IR_TIM_ARR;
    htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim1);

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);
    HAL_TIM_PWM_Init(&htim1);

    /* BDTR: MOE 활성화 (TIM1 필수) */
    sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime         = 0;
    sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_ENABLE;
    HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);

    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = IR_TIM_CCR;
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
}

/**
 * @brief  TIM2 초기화 - 1μs 단위 딜레이 타이머
 *
 *  APB1 = 50MHz, APB1 프리스케일러 = 2
 *  → TIM2 클럭 = 50MHz × 2 = 100MHz  (타이머 배수 규칙)
 *
 *  PSC  = 99  → 100,000,000 / 100 = 1,000,000 Hz
 *  ARR  = 0xFFFFFFFF (32비트 최대)
 *  → 1틱 = 1μs
 */
static void MX_TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = DELAY_TIM_PSC;
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 0xFFFFFFFF;
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);
}

/* ─── HAL MSP ─────────────────────────────────────────── */
void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

/* ─── 에러 핸들러 ─────────────────────────────────────── */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
