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
 *   TIM2          : NEC 타이밍 기준 (1μs 단위)
 * ============================================================ */

#include "main.h"

/* ─── 사용자 설정 ─────────────────────────────────────── */
/*
 * LG TV NEC 코드 (32비트):
 *   Address      : 0x04  (8-bit)
 *   ~Address     : 0xFB  (8-bit, 반전)
 *   Command      : 0x08  (Power Toggle)
 *   ~Command     : 0xF7  (반전)
 *   32비트 합산  : 0x20DF10EF  (LSB-first NEC 포맷)
 *
 * 참고: 일부 LG TV 모델은 아래 코드를 사용합니다.
 *   Power On  전용: 0x20DFB34C
 *   Power Off 전용: 0x20DFA35C
 *   Power Toggle  : 0x20DF10EF  ← 일반적으로 이 코드 사용
 */
#define LG_TV_POWER     0x20DF10EF   /* Power On/Off 토글 */
#define LG_TV_POWER_ON  0x20DFB34C  /* Power On 전용     */
#define LG_TV_POWER_OFF 0x20DFA35C  /* Power Off 전용    */

/* ─── NEC 프로토콜 타이밍 (단위: μs) ─────────────────── */
#define NEC_LEADER_MARK   9000   /* 리더 마크  9ms    */
#define NEC_LEADER_SPACE  4500   /* 리더 스페이스 4.5ms */
#define NEC_BIT_MARK       560   /* 비트 마크  560μs  */
#define NEC_ONE_SPACE     1690   /* '1' 스페이스 1.69ms */
#define NEC_ZERO_SPACE     560   /* '0' 스페이스 560μs  */
#define NEC_REPEAT_SPACE  2250   /* 반복 프레임 스페이스 */
#define NEC_FRAME_GAP    40000   /* 프레임 후 간격 40ms */

/* ─── TIM1 PWM 설정 (38kHz 반송파) ────────────────────
 *  HSI = 16MHz, APB2 = 16MHz
 *  PSC = 0, ARR = 421 → 16,000,000 / 422 ≈ 37,914 Hz ≈ 38kHz
 *  CCR1 = ARR/3 = 140  → 약 33% 듀티 (IR 표준)
 * ─────────────────────────────────────────────────── */
#define IR_TIM_PSC    0
#define IR_TIM_ARR    421
#define IR_TIM_CCR    140   /* ~33% 듀티비 */

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

    /* LED 초기 OFF */
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

                /* 시각적 피드백: 녹색 LED (LD2=PA5) 토글 */
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
 */
static void IR_CarrierOn(void)
{
    /* TIM1 CH1 PWM 출력 활성화 */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, IR_TIM_CCR);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

/**
 * @brief  38kHz 반송파 OFF (IR LED 소등)
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
 * TIM2: PSC=15, 16MHz → 1μs 틱
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
 * NEC 프레임 구조:
 *   [리더 마크 9ms] [리더 스페이스 4.5ms]
 *   [32비트 데이터 LSB first]
 *   [최종 마크 560μs]
 *
 * 비트 인코딩:
 *   '0' = 마크 560μs + 스페이스 560μs
 *   '1' = 마크 560μs + 스페이스 1690μs
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

    /* 3. 최종 마크 (Stop bit) */
    IR_SendMark(NEC_BIT_MARK);
    IR_CarrierOff();

    /* 4. 프레임 후 최소 40ms 간격 */
    HAL_Delay(40);
}

/**
 * @brief  NEC 반복 프레임 전송 (버튼 길게 누를 때 사용)
 *         리더 마크 9ms + 스페이스 2.25ms + 마크 560μs
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
 * @brief  시스템 클럭: HSI 16MHz (PLL 미사용)
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}

/**
 * @brief  GPIO 초기화
 *   PA5  : LD2 (User LED, Output Push-Pull)
 *   PA8  : TIM1_CH1 (IR LED PWM 출력, AF1)
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
 * @brief  TIM1 초기화 - 38kHz PWM 반송파 생성
 *
 *  SYSCLK=16MHz, APB2=16MHz
 *  PSC=0 → TIM 클럭 = 16MHz
 *  ARR=421 → 주파수 = 16,000,000 / 422 ≈ 37,914Hz ≈ 38kHz
 *  CCR1=140 → 듀티비 = 140/422 ≈ 33%
 */
static void MX_TIM1_Init(void)
{
    TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
    TIM_OC_InitTypeDef      sConfigOC          = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

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

    /* MOE(Main Output Enable) - TIM1은 반드시 필요 */
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
 *  SYSCLK=16MHz, APB1=16MHz
 *  PSC=15 → TIM 클럭 = 16MHz/(15+1) = 1MHz → 1틱 = 1μs
 *  ARR=0xFFFF (최대)
 */
static void MX_TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = 15;          /* 16MHz/16 = 1MHz */
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 0xFFFFFFFF;  /* TIM2는 32비트 */
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);
}

/* ─── HAL MSP (필요 최소 오버라이드) ─────────────────── */
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
