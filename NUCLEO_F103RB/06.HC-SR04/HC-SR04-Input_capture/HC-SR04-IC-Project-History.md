# HC-SR04 초음파 거리 센서 — STM32F103 Input Capture 프로젝트 기록

> **프로젝트**: HC-SR04-IC  
> **MCU**: STM32F103C8 (Blue Pill)  
> **IDE**: STM32CubeIDE 1.18.1  
> **위치**: `C:\Users\Administrator\STM32CubeIDE\workspace_1.18.1\STM32F103\HC-SR04-IC\`  
> **CubeMX .ioc**: `HC-SR04-IC.ioc`  
> **날짜**: 2026-06-23 기준

---

## 1. 하드웨어 구성

| 핀 | 포트 | 기능 | AF |
|----|------|------|----|
| **ECHO** | **PB0** | TIM3_CH3 (Input Capture) | AF2 (TIM3) |
| **TRIG** | **PC7** | GPIO Output Push-Pull | — |
| USART2 TX | PA2 | printf 출력 | AF1 (USART2) |
| USART2 RX | PA3 | — | AF1 (USART2) |
| LD2 | PA5 | GPIO Output | — |
| B1 (USER) | PC13 | EXTI (버튼) | — |

### HC-SR04 연결

```
STM32F103                    HC-SR04
  PC7  ───────────────────── TRIG
  PB0  ───────────────────── ECHO
  VCC  ───────────────────── VCC (5V)
  GND  ───────────────────── GND
```

> ⚠ PB0는 5V 내성이 있으므로 HC-SR04의 5V ECHO 출력을 직접 연결 가능.

---

## 2. 시스템 클럭 구성

```
HSI 8MHz → PLL x16 → SYSCLK 64MHz
  ├── AHB (HCLK) = 64MHz (Div1)
  │    ├── APB1 = 32MHz (Div2) → TIM3 clk = 64MHz (×2, prescaler>1 doubling 규칙)
  │    └── APB2 = 64MHz (Div1)
  └── Cortex-M3 DWT CYCCNT = 64MHz
```

### TIM3 설정 (CubeMX)

| 파라미터 | 값 | 비고 |
|----------|----|------|
| Prescaler | `64-1` | 64MHz ÷ 64 = 1MHz |
| Counter Mode | UP | — |
| Period (ARR) | `65535` | max 65.535ms (1 tick = 1µs) |
| AutoReloadPreload | Disable | — |
| ClockDivision | No Division | — |
| CH3 | Input Capture direct TI3, RISING | 에코 시작 |
| CH4 | Input Capture indirect TI3, FALLING | 에코 끝 |
| Update IT | `__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE)` (CODE) | 오버플로우 타임아웃 감지, `USER CODE`에 수동 추가 |

> ⚠ **NVIC 활성화도 별도 필요**: CubeMX **NVIC Settings** 탭에서 `TIM3_IRQn=true`로 설정해야 Update IT가 실제로 CPU에 전달됨.
> `Update IT`는 **어떤 이벤트**를 인터럽트로 받을지 선택하는 TIM 내부 마스크이고,
> `NVIC TIM3_IRQn`은 **인터럽트 자체를 CPU로 라우팅**하는 게이트 역할.
> CubeMX에서는 NVIC Settings 탭에서만 설정 가능하며, `USER CODE`에서는 불가능.

---

## 3. 공식: 거리 계산

```
음속 = 343 m/s = 0.0343 cm/µs
편도 거리 = (펄스폭 × 음속) / 2
         = 펄스폭(µs) / 58

distance_cm = (float)pulse_width / SOUND_SPEED_FACTOR;   // factor = 58
```

SPL 공식: 편도 μs를 cm로 환산하는 factor. HC-SR04 표준.

---

## 4. 개발 연혁 (변경 이력)

### 4.1. v1 — TIM2 폴링 방식 (HC-SR04 프로젝트)

별도 프로젝트: `workspace_1.18.1\STM32F103\HC-SR04\Core\Src\main.c`

- TIM2를 1MHz timebase로 사용
- ECHO 핀을 `while` 루프로 직접 polling (`__HAL_TIM_GET_COUNTER`)
- `trig()` → `echo()` 순차 호출
- **문제점**: CPU 100% 점유, 타이머 읽기 타이밍 불안정, 분해능 한계

<details>
<summary>v1 핵심 코드 구조</summary>

```c
void timer_start(void) { HAL_TIM_Base_Start(&htim2); }
void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while(__HAL_TIM_GET_COUNTER(&htim2) < us);
}
void trig(void) {
    while(HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == HIGH);
    HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, HIGH); delay_us(10);
    HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, LOW);  delay_us(20);
}
long unsigned int echo(void) {
    while(HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == LOW);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while(HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == HIGH) {
        if(__HAL_TIM_GET_COUNTER(&htim2) > 30000) return 0;
    }
    return __HAL_TIM_GET_COUNTER(&htim2);
}
// dist(mm) = echo_time * 1715 / 10000
```
</details>

---

### 4.2. v2 — TIM3 단일채널 Input Capture (초기)

최초 IC 버전. TIM3_CH3 하나로 RISING → FALLING 폴라리티 전환.

```c
// IC 상태 변수
static volatile uint32_t ic_rising_tick = 0;
static volatile uint32_t ic_falling_tick = 0;
static volatile uint8_t  ic_measurement_done = 0;
static volatile uint8_t  ic_edge = 0;          // 0=wait rising, 1=wait falling
static volatile uint8_t  ic_overflow = 0;
```

#### 발견된 문제 #1: `delay_us()`가 TIM3에 의존

```c
// ❌ BAD: delay_us가 TIM3 카운터를 사용
void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    while(__HAL_TIM_GET_COUNTER(&htim3) < us);  // TIM3 Stop_IT() 시 무한루프
}
```

`HAL_TIM_IC_Stop_IT()`이 내부적으로 TIM3를 disable → `__HAL_TIM_GET_COUNTER`가 0만 반환 → 무한루프 → Watchdog 리셋 → "Timeout" 반복 출력.

#### 발견된 문제 #2: `ic_overflow` 미사용

메인 루프에서 `ic_overflow`를 전혀 확인하지 않음. 오버플로우 시 거리 계산이 틀어짐.

#### 발견된 문제 #3: 폴라리티 전환 스퓨리어스 캡처

```c
// ❌ BAD: polarity 변경 시 스퓨리어스 캡처 발생
__HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
```

채널이 활성화된 상태에서 polarity 레지스터를 변경하면 내부 글리치가 발생하여 즉시 잘못된 캡처 이벤트가 트리거됨 → TIM3_CCR3에 현재 카운터값이 즉시 기록됨 → `ic_falling_tick`이 `ic_rising_tick`과 거의 같은 값 → 거리 0 또는 절반 이하.

---

### 4.3. v3 — 단일채널 버그픽스

v2의 문제점들을 순차적으로 수정.

#### Fix #1: DWT Cycle Counter 기반 `delay_us()`

TIM3와 완전히 독립적인 μs 지연 구현. `HAL_TIM_IC_Stop_IT()`의 영향을 전혀 받지 않음.

```c
static void delay_us(uint32_t us) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000UL);
    while ((DWT->CYCCNT - start) < cycles);
}
```

#### Fix #2: `ic_overflow` 플래그 사용

pulse width 계산에 오버플로우 처리 추가:

```c
if (ic_overflow) {
    pulse_width = (IC_PERIOD - ic_rising_tick) + ic_falling_tick;
} else {
    pulse_width = ic_falling_tick - ic_rising_tick;
}
```

#### Fix #3: 채널 비활성화 후 polarity 변경

```c
TIM_CCxChannelCmd(TIM3, TIM_CHANNEL_3, TIM_CCx_DISABLE);
__HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
TIM_CCxChannelCmd(TIM3, TIM_CHANNEL_3, TIM_CCx_ENABLE);
```

스퓨리어스 캡처를 완전히 막지는 못했지만 발생 확률을 낮춤.

---

### 4.4. v4 — 듀얼채널 Input Capture (현재, 최종)

폴라리티 전환 자체를 제거하는 **근본적인 해결책**.

#### 원리

`TIM3_CH4`의 IC selection을 `INDIRECTTI`로 설정 → CH4가 TI3 입력(PB0)을 사용하지만 FALLING edge로 캡처:

```
PB0(TI3) ─┬─→ TIM3_CH3 (direct TI3, RISING)    → ic_rising_tick
           └─→ TIM3_CH4 (indirect TI3, FALLING)  → ic_falling_tick
```

두 채널이 동시에 활성화되어 각각 다른 극성을 캡처. `__HAL_TIM_SET_CAPTUREPOLARITY()`를 **절대 호출하지 않음**.

#### 변경사항 요약

| 항목 | v3 (Before) | v4 (After) |
|------|------------|-----------|
| CH3 | RISING → FALLING 전환 | RISING 고정 (전환 없음) |
| CH4 | 사용 안함 | FALLING 고정 (indirect TI3) |
| polarity 전환 | `__HAL_TIM_SET_CAPTUREPOLARITY()` 사용 | **없음** |
| 채널 활성화 | CH3만 start | CH3 + CH4 동시 start |
| 채널 정지 | CH3만 stop | CH3 + CH4 모두 stop |
| `ic_edge` 변수 | 사용 | **제거** |
| 스퓨리어스 캡처 | 발생 가능 | **근본 차단** |

#### MX_TIM3_Init() — CH4 설정 추가

```c
/* CH3: RISING, direct TI3 */
sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_3);

/* CH4: FALLING, indirect TI3 (same PB0 pin, no polarity toggle ever) */
sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_4);
```

#### HCSR04_Trigger() — 동시 시작

```c
static void HCSR04_Trigger(void) {
    ic_measurement_done = 0;
    ic_overflow = 0;

    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    delay_us(TRIG_PULSE_US);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

    __HAL_TIM_SET_COUNTER(&htim3, 0);

    /* Dual-channel simultaneous start */
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_3);  // RISING
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4);  // FALLING
}
```

#### Capture Callback — CH3/CH4 분기

```c
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM3) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
            // CH3: Rising edge — save, keep both channels running
            ic_rising_tick = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_3);
        }
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
            // CH4: Falling edge — measurement complete, stop both
            ic_falling_tick = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_4);
            HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_3);
            HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_4);
            ic_measurement_done = 1;
        }
    }
}
```

#### 타임아웃 — 양 채널 정지

```c
if ((HAL_GetTick() - tick_start) > TIMEOUT_MS) {
    HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_3);    // before: only CH3
    HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_4);    // added
    break;
}
```

---

## 5. 물리적 Crosstalk 문제

### 발견

사용자가 HC-SR04의 TX와 RX 사이에 **일정 높이의 슬릿(격벽)**을 추가하자 거리 측정이 정확해짐을 관찰.

### 원인 분석

```
TX ──))))) ──→ 공기 전달 ──→ RX  (직접 음향 결합 = crosstalk)
TX ──))))) ──→ 벽(1.5m) ──→ RX  (실제 에코, ~8.8ms)
```

- HC-SR04 모듈 자체에 TX/RX 사이 작은 격벽이 있지만, **공기 중 직접 전달되는 초음파**를 완전히 차단하지 못함
- 근거리(수 cm~수십 cm): 실제 에코 신호가 crosstalk보다 훨씬 강하므로 영향 없음
- 원거리(1.5m 천정): 실제 에코는 도달하는데 ~8.8ms (1.5m × 2 / 0.0343), crosstalk는 수백 μs 만에 도착 → **ECHO 핀이 crosstalk에서 먼저 트리거**되어 절반 이하 거리 측정

### 교훈

- Input Capture 방식은 ECHO 핀의 펄스를 정밀 측정할 뿐, ECHO 핀에 들어오는 **신호 자체의 품질은 보장하지 않음**
- HC-SR04의 물리적 설치(격벽, 차폐)는 소프트웨어보다 우선하는 근본적인 해결책
- 필터링 알고리즘은 보조 수단일 뿐

---

## 6. 소프트웨어 필터링 옵션 (적용 가능)

현재 코드는 필터링 없음. 아래 4가지 옵션 중 선택 가능.

### 6.1. 중간값 필터 (Median, N=5)

```c
#define MEDIAN_WINDOW 5
static float dist_buffer[MEDIAN_WINDOW] = {0};
static uint8_t dist_idx = 0;

// 사용 위치: distance_cm 계산 직후
dist_buffer[dist_idx] = distance_cm;
dist_idx = (dist_idx + 1) % MEDIAN_WINDOW;

float sorted[MEDIAN_WINDOW];
for (int i = 0; i < MEDIAN_WINDOW; i++) sorted[i] = dist_buffer[i];
// Bubble sort (N=5라 무시할 수준)
for (int i = 0; i < MEDIAN_WINDOW - 1; i++)
    for (int j = i + 1; j < MEDIAN_WINDOW; j++)
        if (sorted[i] > sorted[j]) { float t = sorted[i]; sorted[i] = sorted[j]; sorted[j] = t; }
distance_cm = sorted[MEDIAN_WINDOW / 2];
```

### 6.2. 글리치 제거 (Outlier Rejection)

```c
#define GLITCH_THRESHOLD_CM  50.0f
#define GLITCH_MAX_RETRY      3
static float last_valid_dist = 0.0f;
static uint8_t glitch_count = 0;

// 사용 위치: pulse_width → distance_cm 계산 직후
if (glitch_count == 0 && last_valid_dist != 0.0f) {
    float delta = (distance_cm > last_valid_dist) ?
                  (distance_cm - last_valid_dist) : (last_valid_dist - distance_cm);
    if (delta > GLITCH_THRESHOLD_CM) {
        glitch_count++;
        if (glitch_count <= GLITCH_MAX_RETRY) {
            distance_cm = last_valid_dist;  // 이전값 유지
        }
    } else {
        glitch_count = 0;
        last_valid_dist = distance_cm;
    }
} else {
    last_valid_dist = distance_cm;
}
// distance_cm 출력 후, glitch_count <= GLITCH_MAX_RETRY면 continue로 재측정
```

### 6.3. 이동 평균 (Moving Average, N=8)

```c
#define MA_WINDOW 8
static float ma_buffer[MA_WINDOW] = {0};
static uint8_t ma_idx = 0;
static uint8_t ma_filled = 0;

ma_buffer[ma_idx] = distance_cm;
ma_idx = (ma_idx + 1) % MA_WINDOW;
if (ma_filled < MA_WINDOW) ma_filled++;

float sum = 0;
uint8_t count = (ma_filled < MA_WINDOW) ? ma_filled : MA_WINDOW;
for (int i = 0; i < count; i++) sum += ma_buffer[i];
distance_cm = sum / (float)count;
```

### 6.4. IIR 저역 통과 (Exponential MA)

```c
#define EMA_ALPHA  0.25f   // 0.0~1.0, 작을수록 부드러움
static float filtered_dist = 0.0f;
static uint8_t ema_first = 1;

if (ema_first) { filtered_dist = distance_cm; ema_first = 0; }
else           { filtered_dist = EMA_ALPHA * distance_cm + (1.0f - EMA_ALPHA) * filtered_dist; }
distance_cm = filtered_dist;
```

### 필터 비교

| 필터 | 저장 변수 | 계산량 | 응답 지연 | 튀는 값 제거 | 부드러움 |
|------|----------|--------|----------|-------------|---------|
| Median (N=5) | 5개 float | Bubble sort | 5회 | ★★★★★ | ★★★ |
| Glitch Reject | 2개 변수 | 최소 | 0 | ★★★★ (튀는값만) | 원본유지 |
| Moving Avg (N=8) | 8개 float | 덧셈 8회 | N/2회 | ★★★ | ★★★★ |
| EMA (α=0.25) | 1개 float | 곱셈 2회 | 약간 | ★★ | ★★★★★ |

---

## 7. 전체 파일 구조

```
HC-SR04-IC/
├── HC-SR04-IC.ioc              ← CubeMX 설정 파일
├── Core/
│   ├── Inc/
│   │   ├── main.h              ← 핀 정의, 상수, 타이밍 매크로
│   │   ├── stm32f1xx_it.h
│   │   └── stm32f1xx_hal_conf.h
│   └── Src/
│       ├── main.c              ← 메인 로직 (485 lines)
│       ├── stm32f1xx_it.c      ← TIM3_IRQHandler, EXTI 등
│       ├── stm32f1xx_hal_msp.c
│       └── system_stm32f1xx.c
├── Drivers/
│   └── STM32F1xx_HAL_Driver/
│       ├── Inc/stm32f1xx_hal_tim.h     ← TIM 매크로 (TIM_CCxChannelCmd 등)
│       └── Src/stm32f1xx_hal_tim.c     ← HAL_TIM_IC_Start_IT, Stop_IT 구현
└── startup_stm32f1xx.s         ← 벡터 테이블
```

---

## 8. 인터럽트 우선순위

| 인터럽트 | Priority | Precedence | 비고 |
|---------|----------|------------|------|
| TIM3_IRQn | 0, 0 | 최우선 | IC 캡처 + 오버플로우 |
| EXTI15_10_IRQn | 0, 0 | 최우선 | 버튼 (B1) |
| SysTick | 15, 0 | 최하위 | `HAL_GetTick()`용 |

---

## 9. 의존성: 중요 HAL 매크로 / 함수

| 심볼 | 파일 | 용도 |
|------|------|------|
| `TIM_CCxChannelCmd()` | `stm32f1xx_hal_tim.h` | 채널 CCx 활성/비활성 (v3 패치에서 사용, v4에서는 불필요) |
| `__HAL_TIM_SET_CAPTUREPOLARITY()` | `stm32f1xx_hal_tim.h` | IC 극성 변경 (v4에서는 사용 안 함) |
| `TIM_SET_CAPTUREPOLARITY()` | `stm32f1xx_hal_tim.h` | 레지스터 수준 매크로 |
| `HAL_TIM_IC_Start_IT()` | `stm32f1xx_hal_tim.c:L2200` | IC 시작 + CC IT enable |
| `HAL_TIM_IC_Stop_IT()` | `stm32f1xx_hal_tim.c:L2292` | IC 정지 + CC IT disable |
| `HAL_TIM_IRQHandler()` | `stm32f1xx_hal_tim.c` | 공통 IRQ 디스패처, `htim->Channel` 세팅 후 callback 호출 |

---

## 10. 타임라인

```
v1 ─── TIM2 polling (HC-SR04 project)
  │
  ├── 문제: CPU 100%, 분해능 한계
  │
v2 ─── TIM3 single-channel IC (HC-SR04-IC initial)
  │
  ├── 문제: delay_us TIM3 의존 → 무한루프
  ├── 문제: ic_overflow 미사용
  └── 문제: polarity 전환 스퓨리어스 캡처 → 거리 절반
      │
v3 ─── DWT delay, overflow fix, CCxChannelCmd 패치
  │
  ├── delay_us 독립 → 안정화
  ├── overflow 계산 정상화
  └── 스퓨리어스 캡처 감소 (근본 해결 아님)
      │
v4 ─── Dual-channel IC (CH3 RISING + CH4 FALLING indirect)
  │
  ├── polarity 전환 제거 → 스퓨리어스 캡처 완전 차단
  ├── ic_edge 변수 제거
  └── CH3+CH4 동시 시작/정지
      │
      └── ⚡ 현재 상태
```

---

## 11. 향후 과제

- [ ] 슬릿 없이도 안정적인 측정을 위한 소프트웨어 필터링 적용 검토
- [ ] 연속 측정 모드 (TRIG → 대기 → TRIG 반복) 속도 최적화
- [ ] 온도 보정: 계절/환경에 따른 음속 변화 반영
- [ ] TIM3 오버플로우 시 CH4 falling edge까지의 확장된 시간 처리 검증 (65ms 초과 거리)
