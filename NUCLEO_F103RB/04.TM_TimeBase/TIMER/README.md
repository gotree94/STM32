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
| **Disabled** | 슬레이브 동작 안함 | ★★★★★ |
| **Reset Mode** | TRGI 이벤트시 CNT 리셋 후 재시작 | ★★★☆☆ |
| **Gated Mode** | TRGI High일때만 카운트, Low면 정지 | ★★★☆☆ |
| **Trigger Mode** | TRGI 에지에서 카운트 시작 | ★★☆☆☆ |
| **External Clock Mode 1** | TRGI 에지마다 CNT+1 (외부 클럭) | ★★☆☆☆ |

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
| **PWM Generation CHx** | 서보, LED 밝기, DC모터 속도 | ★★★★★ |
| **Input Capture direct** | 주파수/펄스폭 측정 | ★★★★☆ |
| **Output Compare (toggle)** | 가변 주파수 생성, 소프트웨어 타이밍 | ★★★☆☆ |
| **Output Compare (force)** | 포트 강제 세트/리셋 | ★☆☆☆☆ |
| **PWM CHx/CHxN** (TIM1 only) | BLDC, 하프브릿지 데드타임 필요 | ★★☆☆☆ |
| **Input Capture indirect** | 두 신호 위상차 측정 | ★★☆☆☆ |
| **Input Capture TRC** | 특수 목적 | ★☆☆☆☆ |

> **DEPRECATED → input Capture**: CubeMX에서 이전 방식 폐지 안내. 무시하고 Input Capture direct 쓰면 됨.

### 2.5 Combined Channels (복합 모드)

| 모드 | 원리 | 실무 활용 |
|------|------|:---:|
| **Encoder Mode (TI1+TI2)** | A/B상 엣지 → CNT 증감 | ★★★★☆ 로터리 엔코더 |
| **PWM Input on CH1** | CH1=Period, CH2=Duty | ★★★☆☆ RC 수신기, 센서 |
| **PWM Input on CH2** | 반대 매핑 | ★☆☆☆☆ |
| **XOR / Hall Sensor** | CH1⊕CH2⊕CH3 = TI1 | ★★☆☆☆ BLDC 홀센서 |

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

**더 정확한 방법 (2채널 사용, Period + Duty):**

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
// Gated mode: 게이트 HIGH인 동안만 카운터 증가
// 게이트 폭 = 측정 시간

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

**CubeMX 설정:**
- TIM1, Combined Channels → One Pulse Mode
- Slave Mode = Trigger Mode, Trigger Source = TI1FP1

```c
// 외부 트리거 입력시 정확히 1개의 PWM 펄스만 출력

TIM_OnePulse_InitTypeDef sOnePulseConfig = {0};

htim1.Instance = TIM1;
htim1.Init.Prescaler = 72-1;    // 1 tick = 1µs
htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
htim1.Init.Period = 2000-1;     // 펄스 폭 + 딜레이 = 2000µs
HAL_TIM_OnePulse_Init(&htim1, TIM_OPMODE_SINGLE);

sOnePulseConfig.OCMode = TIM_OCMODE_PWM1;
sOnePulseConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
sOnePulseConfig.Pulse = 500;     // 500µs High (펄스 폭)
sOnePulseConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
sOnePulseConfig.OCIdleState = TIM_OCIDLESTATE_RESET;
sOnePulseConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
HAL_TIM_OnePulse_ConfigChannel(&htim1, &sOnePulseConfig, TIM_CHANNEL_1, TIM_CHANNEL_2);

// 외부 트리거 입력마다 500µs 펄스 1개 출력
```

**활용:**
- **초음파 센서 (HC-SR04)**: 트리거 펄스 정밀 생성
- **릴레이/솔레노이드 구동**: 정해진 시간만 ON
- **레이저 펄스**: 정밀 단발 펄스

---

### 3.7 Hall Sensor (BLDC 모터 홀센서)

**CubeMX 설정:**
- TIM3, Combined Channels → XOR / Hall Sensor
- PA6(TIM3_CH1), PA7(TIM3_CH2), PB0(TIM3_CH3) → 3상 홀센서 입력

```c
// XOR로 합쳐진 TI1 에지마다 캡처 → 모터 속도 계산

TIM_SlaveConfigTypeDef sSlaveConfig = {0};

// TI1F_ED: CH1(3개 XOR된 신호) 모든 에지에서 캡처
sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
sSlaveConfig.InputTrigger = TIM_TS_TI1F_ED;
HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig);

// CH1 IC 설정
sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
sConfigIC.ICFilter = 0;
HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);

// XOR TI1 활성화
TIM3->CR2 |= TIM_CR2_TI1S;  // TI1 Selection = XOR

HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);

// 각 에지마다 캡처 → 6-step commutation 타이밍 계산
```

**활용:**
- **BLDC 모터 6-step 제어**: 홀센서 에지마다 정류 패턴 전환
- **모터 속도계**: 홀센서 주파수 = RPM

---

### 3.8 Break Input (비상 정지 - TIM1 전용)

```c
// TIM1 Break 설정
TIM_BreakDeadTimeConfigTypeDef sBreakConfig = {0};

sBreakConfig.BreakState = TIM_BREAK_ENABLE;
sBreakConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;  // Break 핀 High = 정지
sBreakConfig.BreakFilter = 0;
sBreakConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;  // Break 해제시 자동 복구
HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakConfig);
```

**활용:**
- **모터 드라이버 과전류/과열 비상정지**
- **전원 컨버터 출력 셧다운**

---

## 4. 실무 활용 매트릭스

| 하고싶은 것 | 타이머 | 모드 | 채널 |
|------------|--------|------|------|
| LED 밝기 제어 | TIM2~4 | PWM | 1ch |
| 서보모터 4개 | TIM2+3+4+5 | PWM | 각 1ch |
| DC모터 속도 (PID) | TIM2 | PWM | CH1 |
| 엔코더 위치 읽기 | TIM4 | Encoder Mode | CH1+CH2 |
| 주파수 측정 | TIM3 | IC + Reset | CH1 |
| RC PWM 수신 4채널 | TIM2 | PWM Input | CH1~CH4 |
| 초음파 거리측정 | TIM1 | One Pulse + IC | CH1, CH2 |
| 3상 BLDC (6-step) | TIM1 | PWM CHx/CHxN | CH1~CH3 |
| 32bit 카운터 | TIM2+3 | Master+Slave Trigger | - |
| 신호 지속시간 측정 | TIM3 | Gated Mode | CH1 |
| ADC 동기화 | TIM2 | TRGO=Update | → ADC |
| 1ms 시스템 틱 | TIM6 | Base | - (SW 전용) |

---

## 5. 결론

STM32F103 타이머는 방대한 기능을 갖추고 있지만, **실무에서는 4~5개 패턴만 집중 사용**함:

1. **PWM** (LED/모터/서보) ← 가장 많이 씀
2. **Input Capture** (주파수/펄스폭 측정)
3. **Encoder Mode** (엔코더)
4. **Slave Mode (Reset/Gated/Trigger)** (타이머 동기화)
5. **Timer Interrupt** (주기적 SW 처리)

위 5개로 90% 이상 커버 가능. 나머지 기능(One Pulse, Hall, Break, Combined PWM Input 등)은 특수 도메인(모터제어, 전력변환, 초음파)에서만 필요하므로, 필요할 때 레퍼런스로 찾아쓰는 전략이 효율적임.
