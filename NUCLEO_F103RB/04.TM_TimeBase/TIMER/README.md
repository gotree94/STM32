# STM32F103 Timer - 사양 정리 및 활용 가이드

---

## 1. STM32F103 타이머 개요

| 타이머 | 종류 | 채널 | 비고 |
|--------|------|------|------|
| **TIM1** | Advanced | 4ch + CH1N~CH3N | 16bit, Dead-time, Break input, 보완출력 |
| **TIM2** | General | 4ch | 32bit 카운터 가능(F103 일부) |
| **TIM3** | General | 4ch | 16bit |
| **TIM4** | General | 4ch | 16bit |
| **TIM6** | Basic | 없음 | DAC 트리거 전용, 인터럽트만 |
| **TIM7** | Basic | 없음 | 인터럽트만 |

Nucleo-F103RB의 경우 **TIM1(Advanced), TIM2~4(General), TIM6~7(Basic)** 까지 사용 가능.

---

## 2. 조직화된 기능 맵 (STM32CubeMX 기준)

### 2.1 Slave Mode (슬레이브 모드)

| 모드 | 동작 | 실제 활용 빈도 |
|------|------|:---:|
| **Disabled** | 슬레이브 동작 안함 | ⭐⭐⭐⭐⭐ |
| **Reset Mode** | TRGI 이벤트시 CNT 리셋 후 재시작 | ⭐⭐⭐☆☆ |
| **Gated Mode** | TRGI High일때만 카운트, Low면 정지 | ⭐⭐⭐☆☆ |
| **Trigger Mode** | TRGI 에지에서 카운트 시작 | ⭐⭐☆☆☆ |
| **External Clock Mode 1** | TRGI 에지마다 CNT+1 (외부 클럭) | ⭐⭐☆☆☆ |

> **★ = 실무 사용 빈도.** 저주파 신호 계측(Gated), ADC 타이밍(Trigger), 타이머 체이닝(Reset) 위주.

### 2.2 Trigger Source (TRGI 선택)

```
ITR0 ──┐
ITR1 ──┤  ← 내부 타이머간 연결 (예: TIM1_TRGO → TIM2_ITR0)
ITR2 ──┤
ITR3 ──┘
TI1F_ED ──→ 모든 에지 검출
TI1FP1 ───→ CH1 필터링된 신호
TI2FP2 ───→ CH2 필터링된 신호
ETRF ─────→ 외부 핀(ETR) 필터링
```

**타이머간 ITR 연결 맵 (F103):**

| 슬레이브 | ITR0 | ITR1 | ITR2 | ITR3 |
|----------|------|------|------|------|
| TIM1 | TIM2 | - | - | - |
| TIM2 | TIM1 | TIM8 | TIM3 | TIM4 |
| TIM3 | TIM1 | TIM2 | TIM5 | TIM4 |
| TIM4 | TIM1 | TIM2 | TIM3 | TIM8 |

### 2.3 Clock Source

| 소스 | 경로 | 설명 |
|------|------|------|
| **Internal Clock (CK_INT)** | APB 타이머 클럭 | 기본 타이밍, PWM, IC/OC |
| **ETR2 (External Clock Mode 2)** | ETR 핀 → 카운터 | 외부 펄스 직접 카운트 |

### 2.4 Channel Mode (실무 사용 빈도 순)

| 채널 모드 | 용도 | 빈도 |
|-----------|------|:---:|
| **PWM Generation CHx** | 서보, LED 밝기, DC모터 속도 | ⭐⭐⭐⭐⭐ |
| **Input Capture direct** | 주파수/펄스폭 측정 | ⭐⭐⭐⭐☆ |
| **Output Compare (toggle)** | 가변 주파수 생성, 소프트웨어 타이밍 | ⭐⭐⭐☆☆ |
| **Output Compare (force)** | 포트 강제 세트/리셋 | ⭐☆☆☆☆ |
| **PWM CHx/CHxN** (TIM1 only) | BLDC, 하프브릿지 데드타임 필요 | ⭐☆☆☆ |
| **Input Capture indirect** | 두 신호 위상차 측정 | ⭐☆☆☆ |
| **Input Capture TRC** | 특수 목적 | ⭐☆☆☆☆ |

> **DEPRECATED → input Capture**: CubeMX에서 이전 방식 폐지 안내. 무시하고 Input Capture direct 쓰면 됨.

### 2.5 Combined Channels (복합 모드)

| 모드 | 원리 | 실무 활용 |
|------|------|:---:|
| **Encoder Mode (TI1+TI2)** | A/B상 엣지 → CNT 증감 | ⭐⭐⭐⭐☆ 로터리 엔코더 |
| **PWM Input on CH1** | CH1=Period, CH2=Duty | ⭐⭐⭐☆☆ RC 수신기, 센서 |
| **PWM Input on CH2** | 반대 매핑 | ⭐☆☆☆☆ |
| **XOR / Hall Sensor** | CH1⊕CH2⊕CH3 = TI1 | ⭐⭐☆☆☆ BLDC 홀센서 |

---

## 3. 실전 활용 예제 및 소스 코드

아래 코드는 **HAL 라이브러리** 기준. STM32CubeIDE + CubeMX로 핀 할당/클럭 설정했다고 가정.

---

### 3.1 PWM 출력 (LED 밝기 조절 / 서보 모터)

**CubeMX 설정:**
- TIM2, Channel 1 → PWM Generation CH1
- PA0 (TIM2_CH1)
- Prescaler = 72-1, Period = 1000-1 → 72MHz/72/1000 = 1kHz
- Pulse = 500 (50% duty)

**코드:**

```c
// main.c
TIM_OC_InitTypeDef sConfigOC = {0};

htim2.Instance = TIM2;
htim2.Init.Prescaler = 72-1;          // 72MHz / 72 = 1MHz
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
htim2.Init.Period = 1000-1;           // 1MHz / 1000 = 1kHz PWM
htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
HAL_TIM_PWM_Init(&htim2);

sConfigOC.OCMode = TIM_OCMODE_PWM1;
sConfigOC.Pulse = 500;                // 50% duty
sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);

HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

// 런타임 duty 변경
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 750);  // 75%
```

**서보모터 (50Hz, 1~2ms 펄스):**

```c
// 72MHz / 144 = 500kHz → Period = 10000 → 50Hz
htim2.Init.Prescaler = 144-1;
htim2.Init.Period = 10000-1;
// 0°   → Pulse = 500  (1.0ms)
// 90°  → Pulse = 750  (1.5ms)
// 180° → Pulse = 1000 (2.0ms)
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 750);  // 90°
```

---

### 3.2 Input Capture (주파수 측정)

**CubeMX 설정:**
- TIM3, Channel 1 → Input Capture direct mode
- PA6 (TIM3_CH1)
- Prescaler = 72-1 (1µs resolution)
- Slave Mode = Reset Mode, Trigger = TI1FP1 (매 캡처마다 카운터 리셋)

```c
// main.c
TIM_IC_InitTypeDef sConfigIC = {0};

htim3.Instance = TIM3;
htim3.Init.Prescaler = 72-1;          // 1 tick = 1µs (72MHz/72)
htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
htim3.Init.Period = 65535;
htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
HAL_TIM_IC_Init(&htim3);

sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
sConfigIC.ICFilter = 0;               // 필요시 0xF로 노이즈 필터
sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);

HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
// HAL_TIM_IC_Start_DMA() 도 가능

// stm32f1xx_it.c
void TIM3_IRQHandler(void)
{
    HAL_TIM_IC_CaptureCallback(&htim3);
}

// main.c
uint32_t gCapturedPeriod = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        gCapturedPeriod = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1);
        // 주파수 = 1,000,000 / gCapturedPeriod [Hz] (1µs 해상도 기준)
        // 예: gCapturedPeriod = 1000 → 1kHz
    }
}
```

**2채널 사용 (Period + Duty 함께 측정):**

```c
// TIM3 CH1: Rising edge → Period 측정 (Reset Mode)
// TIM3 CH2: Falling edge → Duty 측정

sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;     // CH1 → IC1
HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);

sConfigIC.ICPolarity = TIM_ICPOLARITY_FALLING;
sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;    // CH1 → IC2 (indirect)
HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2);

HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            period = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1);
        } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
            duty = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_2);
        }
        // duty_cycle = (float)duty / period * 100;
    }
}
```

---

### 3.3 엔코더 모드 (로터리 엔코더 위치/속도 측정)

**CubeMX 설정:**
- TIM4, Combined Channels → Encoder Mode TI1+TI2
- PB6 (TIM4_CH1), PB7 (TIM4_CH2)
- Prescaler = 0, Period = 65535
- Counter Mode = Center-aligned (Encoder Mode 전용)

```c
TIM_Encoder_InitTypeDef sEncoderConfig = {0};

htim4.Instance = TIM4;
htim4.Init.Prescaler = 0;
htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
htim4.Init.Period = 65535;
htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
HAL_TIM_Encoder_Init(&htim4, &sEncoderConfig);

sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;  // A상+B상 모두 카운트
sEncoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
sEncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
sEncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
sEncoderConfig.IC1Filter = 0;
sEncoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
sEncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
sEncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
sEncoderConfig.IC2Filter = 0;
HAL_TIM_Encoder_Init(&htim4, &sEncoderConfig);

HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

// 메인루프에서 읽기
int16_t position = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);

// 32bit 확장 (오버플로우 처리)
static int32_t gExtendedPos = 0;
static int16_t gLastCount = 0;

void ReadEncoder(void)
{
    int16_t current = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
    int16_t delta = current - gLastCount;
    gExtendedPos += delta;
    gLastCount = current;
}
```

---

### 3.4 Gated Mode (외부 게이트 신호로 특정 구간만 카운트)

**CubeMX 설정:**
- TIM2, Slave Mode = Gated Mode, Trigger Source = TI1FP1
- PA0 (TIM2_CH1) = 게이트 입력
- Prescaler = 72-1 (1µs resolution)

```c
TIM_SlaveConfigTypeDef sSlaveConfig = {0};

// 슬레이브 설정: Gated Mode + TI1FP1
sSlaveConfig.SlaveMode = TIM_SLAVEMODE_GATED;
sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
sSlaveConfig.TriggerFilter = 0;
HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig);

// 기본 Base 설정
htim2.Init.Prescaler = 72-1;    // 1 tick = 1µs
htim2.Init.Period = 65535;
HAL_TIM_Base_Init(&htim2);

HAL_TIM_Base_Start(&htim2);

// 게이트가 LOW → CNT 정지, HIGH → CNT 진행
// 게이트 폭이 끝난 후 CNT 읽으면 = 게이트 HIGH였던 시간 (µs)
```

**활용:**
- **RC 신호 펄스폭 측정**: 1~2ms 범위를 1µs 분해능으로 측정
- **외부 이벤트 지속시간 측정**: 센서 ON 시간

---

### 3.5 Trigger Mode (마스터 타이머 이벤트로 슬레이브 시작)

**CubeMX 설정:**
- TIM2 Master: PWM 출력, TRGO = Update Event
- TIM3 Slave: Trigger Mode, Trigger Source = ITR0 (TIM2 TRGO)

```c
// TIM2 (Master): 1kHz PWM
htim2.Init.Prescaler = 72-1;
htim2.Init.Period = 1000-1;
HAL_TIM_PWM_Init(&htim2);
// ... PWM config ...

TIM_MasterConfigTypeDef sMasterConfig = {0};
sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;  // UEV → TRGO
sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

// TIM3 (Slave): TIM2 업데이트마다 카운트 시작
htim3.Instance = TIM3;
htim3.Init.Prescaler = 0;    // TIM2와 동기화되므로 프리스케일러 0
htim3.Init.Period = 500;
HAL_TIM_Base_Init(&htim3);

TIM_SlaveConfigTypeDef sSlaveConfig = {0};
sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
sSlaveConfig.InputTrigger = TIM_TS_ITR0;    // TIM2 TRGO
HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig);

HAL_TIM_Base_Start(&htim3);  // Trigger 대기 모드

// TIM2가 UEV 발생할 때마다 TIM3가 0→500 카운트
```

**활용:**
- **타이머 체이닝**: 32bit 타이머 구현 (TIM2 16bit + TIM3 16bit)
- **ADC 타이밍**: 특정 PWM 위상에서 ADC 트리거
- **3상 모터 제어**: 위상 시프트된 PWM 생성

---

### 3.6 One Pulse Mode (단일 펄스 발생)

`[] One Pulse Mode`는 **TIMx_CR1의 OPM(One Pulse Mode) 비트**를 의미.

- OPM = 0: 일반 연속 모드 (카운터가 ARR 도달 후 다시 0부터 반복)
- OPM = 1: **단발 모드** — 카운터가 UEV(Update Event) 발생시 즉시 정지
- 트리거 입력과 결합하면 **"외부 신호 받아서 정확히 1개의 PWM 펄스만 출력"** 가능

**CubeMX 옵션 정리:**
| 위치 | 설정 | 설명 |
|------|------|------|
| TIMx_CR1.OPM | `[] One Pulse Mode` | 체크 = 단발, 해제 = 연속 |
| Slave Mode | Trigger Mode | 트리거 에지에서 카운트 시작 |
| Trigger Source | TI1FP1 / TI2FP2 | 어느 핀으로 트리거 받을지 |
| CH1 | PWM Generation | 출력 파형 설정 |
| CH1 Pulse | CCR 값 | High 구간 길이 |
| Period | ARR 값 | 전체 펄스 주기 (=delay + pulse) |

**타이밍 다이어그램:**

```
TRIGGER IN  ──╂───────┐
                   │
                   │ (rising edge)
CNT         0 ──────▸▸▸▸▸▸▸▸▸▸▸▸▸▸  ▏정지
                   │         │
CH1 OUT  ──────────┘████████┘──────
                   ←pulse→
                   ←──period──→
```

**코드 (HC-SR04 초음파 센서 트리거 예):**

```c
// CubeMX:
//   TIM1: Slave Mode=Trigger, Trigger Source=TI1FP1
//         Combined Channels = One Pulse Mode
//   CH1: PWM Generation CH1, Pulse=10 (10µs)
//   Prescaler=72-1 (1 tick=1µs), Period=100-1
//   PA8(TIM1_CH1) → HC-SR04 Trig 핀

htim1.Instance = TIM1;
htim1.Init.Prescaler = 72-1;    // 1µs/tick
htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
htim1.Init.Period = 100-1;      // 100µs 전체 구간
HAL_TIM_OnePulse_Init(&htim1, TIM_OPMODE_SINGLE);

// CH1: PWM1 output, Pulse=10 → 10µs High
sOnePulseConfig.OCMode = TIM_OCMODE_PWM1;
sOnePulseConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
sOnePulseConfig.Pulse = 10;
sOnePulseConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
sOnePulseConfig.OCIdleState = TIM_OCIDLESTATE_RESET;
sOnePulseConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
HAL_TIM_OnePulse_ConfigChannel(&htim1, &sOnePulseConfig,
                               TIM_CHANNEL_1, TIM_CHANNEL_2);

// 슬레이브: TI1FP1 라이징 에지에서 트리거
TIM_SlaveConfigTypeDef sSlaveConfig = {0};
sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
sSlaveConfig.TriggerFilter = 0;
HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig);

// 첫 트리거 대기 (TIM1 시작)
HAL_TIM_OnePulse_Start(&htim1, TIM_CHANNEL_1);

// 이후 소프트웨어 트리거:
// TIM1->EGR |= TIM_EGR_TG;  // SW trigger
// 또는 외부 핀(PA8)에 신호 입력시 자동 트리거
```

**활용 분야:**
| 응용 | Pulse 폭 | 트리거 소스 |
|------|---------|-----------|
| HC-SR04 초음파 | 10µs | SW 또는 외부 입력 |
| IR 리모컨 캐리어 | 38kHz x N cycle | USART or GPIO |
| 릴레이 구동 | 50ms | 버튼/센서 |
| 레이저 모듈 | 100µs | 타이머 체이닝 |

---

### 3.7 PWM Input Mode (RC 수신기 / PWM 신호 계측)

**CubeMX 설정 (TIM2 예):**
- Slave Mode = Reset Mode, Trigger Source = TI1FP1
- Combined Channels → PWM Input on CH1
- Prescaler = 72-1, Period = 65535
- CH1: IC direct, Rising edge (Period)
- CH2: IC indirect, Falling edge (Duty)

> ⚠ **PWM Input on CH1**을 선택하면 CubeMX가 CH1은 Period, CH2는 Duty로 자동 할당.
> CH2는 별도 설정할 필요 없음.

```c
// CubeMX 생성 코드 + 수동 보강

// ─── Slave: Reset Mode (매 rising edge마다 CNT=0) ───
TIM_SlaveConfigTypeDef sSlaveConfig = {0};
sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
sSlaveConfig.TriggerFilter = 0;
HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig);

// ─── CH1: IC1 direct → Period (Rising) ───
TIM_IC_InitTypeDef sConfigIC1 = {0};
sConfigIC1.ICPolarity = TIM_ICPOLARITY_RISING;
sConfigIC1.ICSelection = TIM_ICSELECTION_DIRECTTI;
sConfigIC1.ICPrescaler = TIM_ICPSC_DIV1;
sConfigIC1.ICFilter = 0;
HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC1, TIM_CHANNEL_1);

// ─── CH2: IC2 indirect → Duty (Falling, 같은 TI1 신호) ───
TIM_IC_InitTypeDef sConfigIC2 = {0};
sConfigIC2.ICPolarity = TIM_ICPOLARITY_FALLING;
sConfigIC2.ICSelection = TIM_ICSELECTION_INDIRECTTI;
sConfigIC2.ICPrescaler = TIM_ICPSC_DIV1;
sConfigIC2.ICFilter = 0;
HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC2, TIM_CHANNEL_2);

// ─── Start ───
HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);

// ─── Interrupt Callback ───
volatile uint32_t gPeriod2 = 0;
volatile uint32_t gDuty2 = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
            gPeriod2 = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
            gDuty2 = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2);
    }
}

// 메인루프에서 활용
// duty_cycle = (float)gDuty2 / gPeriod2 * 100.0f;
// frequency = 1000000 / gPeriod2;  // [Hz] at 1µs resolution
```

**PWM Input on CH2 선택시**: 반대 매핑 — CH2가 Period, CH1이 Duty.
```
PWM Input on CH1:  TI1 → CH1(Rising, Period) + CH2(Falling, Duty)
PWM Input on CH2:  TI2 → CH2(Rising, Period) + CH1(Falling, Duty)
```

**하나의 타이머로 RC 4채널 수신:**

```c
// TIM4: CH1~CH4 각각 PWM Input on CHx
// PA6(TIM3_CH1) → CH1
// PA7(TIM3_CH2) → CH2
// PB0(TIM3_CH3) → CH3
// PB1(TIM3_CH4) → CH4

// 각 채널 독립적으로 PWM Input 모드 설정
for ch in 1..4:
    SlaveMode=Reset, Trigger=TI{x}FP1
    ICx: direct rising  (Period)
    ICy: indirect falling (Duty)

// → 인터럽트 또는 DMA로 4채널 Period+Duty 동시 수신
```

---

### 3.8 Hall Sensor (BLDC 모터 홀센서 + XOR activation)

**`[] XOR activation`** 는 **TIMx_CR2의 TI1S(TI1 Selection) 비트**를 의미.

| TI1S | 입력 동작 |
|:----:|-----------|
| 0 | CH1 핀 단독 → TI1 |
| 1 | **CH1 ⊕ CH2 ⊕ CH3** (XOR) → TI1 |

3상 브러시리스 DC 모터의 **홀센서(Hall sensor)** 출력 3개(H1, H2, H3)를 각각
CH1, CH2, CH3에 연결하고 TI1S=1로 설정하면, XOR 결과가 TI1 입력이 됨.
→ **모든 에지에서 CNT 캡처 → 회전 속도 측정** 가능.

**타이밍 다이어그램:**

```
H1  ──████████████████████████████████████────
H2  ────────████████████████████████████████────
H3  ────────────████████████████████████████████

XOR ──████──████──████──████──████──████──████──
         ↑캡처  ↑캡처  ↑캡처  ↑캡처  ↑캡처
         (6 edges per electrical cycle)
```

**CubeMX 설정:**
- TIM3, Channel 1 → Input Capture direct mode
- Combined Channels → XOR / Hall Sensor (선택사항, 직접 TI1S 설정)
- Slave Mode = Reset Mode, Trigger Source = TI1F_ED
- PA6(TIM3_CH1), PA7(TIM3_CH2), PB0(TIM3_CH3)

```c
// XOR activation: TI1S = 1
// HAL에는 직접 함수가 없으므로 레지스터 설정
TIM3->CR2 |= TIM_CR2_TI1S;   // CH1⊕CH2⊕CH3 → TI1

// TI1F_ED: 모든 에지에서 트리거 (Reset Mode ≡ Clear CNT)
TIM_SlaveConfigTypeDef sSlaveConfig = {0};
sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
sSlaveConfig.InputTrigger = TIM_TS_TI1F_ED;  // 모든 에지
HAL_TIM_SlaveConfigSynchro(&tim3, &sSlaveConfig);

// CH1 IC: Rising edge, CNT 읽기
sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
sConfigIC.ICFilter = 0;
HAL_TIM_IC_ConfigChannel(&tim3, &sConfigIC, TIM_CHANNEL_1);

HAL_TIM_IC_Start_IT(&tim3, TIM_CHANNEL_1);

// 콜백에서 속도 계산
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        uint32_t capture = HAL_TIM_ReadCapturedValue(&tim3, TIM_CHANNEL_1);
        // capture = 마지막 에지 이후 경과 시간 (1µs resolution 기준)
        // 1회전 = 6 에지 (3상 홀센서)
        // rpm = 60,000,000 / (capture * 6)  [at 1µs/tick]
    }
}
```

---

### 3.9 Break Input (Active-Break-Input) — TIM1 전용

**`[] Active-Break-Input`** 는 **TIM1의 BRK(Break) 기능** — TIM1_BKIN 핀(PB12) 또는
시스템 브레이크( Cortex 시스템 오류 등) 입력시 출력을 강제로 안전한 상태로 전환.

**CubeMX 설정:**
- TIM1 → Break and DeadTime settings
  - `[] Active-Break-Input` 체크
  - Break Polarity: High (BKIN 핀 High=브레이크) / Low
  - Break Filter: 0~15 (클럭 샘플링 횟수)
  - Automatic Output: Enable (브레이크 해제시 자동 복구) / Disable (SW 재설정 필요)

**내부 동작:**

| 브레이크 발생시 | 복구 |
|----------------|------|
| • 모든 OC 출력 즉시 Idle 상태로 강제 | • Automatic Output ON → 브레이크 사라진 후 **7 idle clocks 후** 자동 복구 |
| • OCPolarity 무시하고 IdleState 출력 | • Automatic Output OFF → SW가 `TIM_BDTR_AOE` 재설정 필요 |
| • TIMx_SR.BIF 플래그 세트 | |
| • 인터럽트 가능 (BIE=1) | |

```c
// TIM1 Break + DeadTime 설정
TIM_BreakDeadTimeConfigTypeDef sBreakConfig = {0};

sBreakConfig.BreakState = TIM_BREAK_ENABLE;            // [] Active-Break-Input
sBreakConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;   // BKIN = High → Break
sBreakConfig.BreakFilter = 0;                          // 필터 OFF
sBreakConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;  // 자동 복구

// DeadTime 설정 (CHx와 CHxN 사이)
sBreakConfig.DeadTime = 72;   // 72 tick × (1/72MHz) = 1µs deadtime
// sBreakConfig.Break2State = TIM_BREAK2_DISABLE;   // break2 없음

HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakConfig);

// 브레이크 인터럽트 활성화
__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_BREAK);

// NVIC에서 TIM1_BRK_IRQn 활성화
HAL_NVIC_SetPriority(TIM1_BRK_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(TIM1_BRK_IRQn);

// ISR
void TIM1_BRK_IRQHandler(void)
{
    HAL_TIMEx_BreakCallback(&htim1);
}

void HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim)
{
    // 브레이크 발생시 처리 (로깅, 모터 정지 확인 등)
    // 자동 복구(AOE=1)면 별도 조치 없이 복구됨
}
```

**활용 분야:**
| 응용 | 브레이크 소스 | 기능 |
|------|-------------|------|
| **BLDC 모터 드라이버** | 과전류 비교기 출력 → BKIN | 쇼트/과부하시 즉시 PWM OFF |
| **DC-DC 컨버터** | 출력 과전압 검출 → BKIN | 스위칭 FET 보호 |
| **전동공구** | 물리적 비상정지 버튼 | 커터 즉시 정지 |

**핀 할당 (STM32F103):**
| TIM | BKIN 핀 | 리맵 |
|:---:|:--------:|:----:|
| TIM1 | PB12 | PA6 (AFIO 리맵) |
| TIM8 | PA6 | - |

---

### 3.10 Use ETR as Clearing Source

**`[] Use ETR as Clearing Source`** 는 **OCxCE(Output Compare Clear Enable)** 기능.

- **OCxCE = 1**: ETRF 입력이 High가 될 때마다 OCx 출력을 강제로 Low로 Clear
- PWM 생성 중 **ETR 핀으로 강제 리셋** 가능 → 외부 사건에 동기화된 파형 생성에 활용

**CubeMX 설정 위치:**
- TIMx → "Input Trigger" 설정 섹션
- Trigger Source = ETRF 선택
- Slave Mode 설정 하단 `[] Use ETR as Clearing Source` 체크

**동작 방식:**

```
CNT    ▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸▸
       │         │     │         │
CH1    ████████████     ████████████
(PWM)  ████████████     ████████████
              ↓ ETR clear    ↓ ETR clear
       ─────────┘     ─────────┘
ETR    ───────████──────████─────────
```

**코드 (레지스터 설정):**

```c
// OC4CE 비트 설정 (CCMR2 레지스터)
// CH4의 OC4CE = 1 → CH4 출력이 ETRF에 의해 클리어됨
TIM1->CCMR2 |= TIM_CCMR2_OC4CE;

// ETRF 설정 (필터/프리스케일러)
TIM1->ETR |= TIM_ETR_ETR_PSC_DIV1    // ETR 프리스케일러
          | TIM_ETR_ETR_FILTER_0      // 필터 OFF
          | TIM_ETR_ETP;              // polarity: non-inverted

// ETR 핀: TIM1의 경우 PA12 (TIM1_ETR)
```

**활용 예:**

| 응용 | ETR 연결 | 효과 |
|------|---------|------|
| **Zerocross 검출** | AC 라인 제로크로싱 → ETR | 매 제로크로싱에서 PWM 리셋 → 위상 동기 |
| **PFC 컨트롤러** | 인덕터 전류 제로검출 | 스위칭 타이밍 리셋 |
| **모터 속도 동기** | 홀센서 → ETR | 홀에지마다 PWM 싱크 |

> ⚠ OCxCE는 **TIM1의 CH4**에서 주로 사용. 다른 채널도 가능하나 CubeMX는 CH4에 매핑.

---

## 4. CubeMX 체크옵션 요약표

| 옵션 | 레지스터 | 위치 | 설명 |
|------|---------|------|------|
| `[] One Pulse Mode` | TIMx_CR1.OPM | Timer config → OPM | 카운터 1사이클 후 정지 |
| `[] Active-Break-Input` | TIMx_BDTR.BKE | Break & DeadTime | BKIN 핀으로 출력 강제 정지 |
| `[] Use ETR as Clearing Source` | TIMx_CCMRx.OCxCE | Input Trigger | ETR High시 OC 출력 Clear |
| `[] XOR activation` | TIMx_CR2.TI1S | Input Trigger | CH1⊕CH2⊕CH3 → TI1 |

---

## 5. 실무 활용 매트릭스

| 하고싶은 것 | 타이머 | 모드 | 채널 |
|------------|--------|------|------|
| LED 밝기 제어 | TIM2~4 | PWM | 1ch |
| 서보모터 4개 | TIM2+3+4+5 | PWM | 각 1ch |
| DC모터 속도 (PID) | TIM2 | PWM | CH1 |
| 엔코더 위치 읽기 | TIM4 | Encoder Mode | CH1+CH2 |
| 주파수 측정 | TIM3 | IC + Reset | CH1 |
| RC PWM 수신 4채널 | TIM3 | PWM Input | CH1~CH4 |
| 초음파 거리측정 | TIM1 | One Pulse + IC | CH1, CH2 |
| 3상 BLDC (6-step) | TIM1 | PWM CHx/CHxN | CH1~CH3 |
| BLDC 속도 피드백 | TIM3 | Hall Sensor(XOR) | CH1~CH3 |
| 모터 비상정지 | TIM1 | Break Input | BKIN |
| 32bit 카운터 | TIM2+3 | Master+Slave Trigger | - |
| 신호 지속시간 측정 | TIM3 | Gated Mode | CH1 |
| ADC 동기화 | TIM2 | TRGO=Update | → ADC |
| 제로크로싱 동기 PWM | TIM1 | ETR Clear Source | ETR |
| 1ms 시스템 틱 | TIM6 | Base | - (SW 전용) |

---

## 6. 결론

STM32F103 타이머는 방대한 기능을 갖추고 있지만, **실무에서는 4~5개 패턴만 집중 사용**함:

| 순위 | 패턴 | 사용처 |
|:---:|------|--------|
| ⭐⭐⭐⭐⭐ | **PWM Generation** | LED/모터/서보 |
| ⭐⭐⭐⭐ | **Input Capture** | 주파수/펄스폭 측정 |
| ⭐⭐⭐⭐ | **Encoder Mode** | 로터리 엔코더 |
| ⭐⭐⭐ | **Slave Mode (Reset/Gated/Trigger)** | 타이머 동기화, 체이닝 |
| ⭐⭐⭐ | **Timer Interrupt** | 주기적 SW 처리 |

위 5개로 90% 이상 커버 가능. 나머지 기능들은 **특수 도메인 전용**:
- **One Pulse** → 초음파, 레이저, 단발 액추에이터
- **PWM Input** → RC 수신기, PWM 통신 센서
- **Hall Sensor (XOR)** → BLDC 모터 속도
- **Break Input** → 모터 드라이버 비상정지, 전력변환기 보호
- **ETR Clear** → AC 라인 동기, PFC

필요할 때 이 문서를 레퍼런스로 찾아쓰는 전략이 효율적.
