# MH-SR602 PIR 모션 센서 가이드

- PIR(Passive Infrared, 수동 적외선) 모션 센서는 움직임을 감지할 수 있으며, <br>
  사람이 감지 범위 안으로 들어오거나 나가는 것을 거의 항상 감지하는 데 사용됩니다.
- 사람이 특정 영역에 들어오거나 나갔는지, 또는 접근했는지를 감지해야 하는 많은 DIY 프로젝트나 장치에 PIR 센서는 훌륭한 선택입니다.
- 요즘에는 매우 컴팩트하고 저렴하며 저전력이고 사용하기 쉬운 디지털 PIR 센서를 구입할 수 있습니다.
- 훌륭한 예가 **MH-SR602 PIR 모션 센서 모듈**입니다.
- 0 ~ 3.5미터 거리의 움직임 감지에 이상적입니다.
- 이 모듈은 높은 감도, 빠른 응답, 낮은 정적 에너지 소비 및 작은 크기로 설치가 쉽습니다.

![](MH-SR602x2.jpg)

## 주요 사양 (Quick Specifications)

| 항목 | 값 |
|:---:|:---:|
| 모델 번호 | SR602 |
| 감지 거리 | 최대 5미터 (0~3.5미터 권장) |
| 출력 | High Level (H=3.3V / L=0V) |
| 전원 공급 | 3.3V ~ 15V DC |
| 대기 전류 | 20μA |

### 핀 정보 (Pin Data)

| 핀 | 설명 |
|:---:|:---:|
| – | 0V / GND |
| + | 3.3V ~ 15V DC 입력 |
| OUT | 논리 H/L 출력 (3.3V / 0V) |

### 주요 포인트 (Key Points)

- 이 센서의 **High-Level(H) 출력 시간**은 2.5초에서 1시간까지 조정 가능하지만, **기본값은 2.5초**입니다.
- 출력 신호 지속 시간을 변경해야 하는 경우 칩 저항 하나를 교체할 수 있습니다 (아래 그림 참조).
- 기본 지연 시간을 수정하면 초기 전원 켜짐 후 센서 모듈이 High-Level 신호를 출력하는 시간도 그에 따라 증가합니다.
- 마찬가지로, 이 센서의 **감도**도 조정 가능하며, 칩 저항을 교체해야 합니다.
- 또한 **외부 포토센서(광센서)를 추가**할 수 있는 옵션이 있어, <br>
  포토센서를 설치하면 모션 센서 메커니즘이 낮에는 작동하지 않고 밤에만 작동합니다 <br>
  (포토센서의 감지 감도도 칩 저항 교체로 변경 가능).

![](MH-SR602-MODs-768x396.jpg)

## 주간/야간 감지 (Day/Night Detection)

- 데이터시트에 따르면, 주간/야간 감지를 위한 권장 포토센서는 **F3 포토다이오드**입니다.
- F3은 고속 및 고감도 PIN 포토다이오드로, 표준 3mm 플라스틱 패키지로 제공됩니다 <br>
  (아래는 판매자 페이지에서 가져온 간략한 사양입니다).

![](F3-Photodiode-Data.jpg)

## 핵심 부품 (Core Part)

- 이 모듈에 사용된 실제 PIR(수동 적외선) 모션 센서는 **6개의 핀**을 가지고 있지만, 부품 번호를 식별할 수 있는 라벨이 없습니다.
- 육안 검사 결과, 널리 사용되는 **BM612 스마트 디지털 모션 감지기**와 다소 유사해 보입니다.
- **BM612**는 모든 전자 회로가 감지기 하우징에 내장된 완전한 모션 감지기 솔루션을 제공합니다.
- 전체 모션 스위치를 만들기 위해 전원 공급 장치와 전원 스위칭 구성 요소만 추가하면 되며, 타이머가 포함되어 있습니다.
- 또한 주변 조도 레벨 및 감도 조정 옵션도 제공합니다.

### 테스트 설정 (Test Setup)

- 여기서 소개하는 것과 같은 디지털 PIR 모션 센서 모듈을 빠르게 테스트하는 데 반드시 고급 마이크로컨트롤러가 필요하지는 않습니다.
- **MH-SR602 모듈에는 보드에 3.3V LDO 선형 전압 레귤레이터 칩이 탑재**되어 있으므로, 아래 회로도를 따라 빠른 테스트를 시작할 수 있습니다.

![](MH-SR602-Basic-Test-Circuit.jpg)

- 이것은 PIR 센서가 트리거될 때 액티브 부저를 미리 결정된 최소 시간 동안 구동하기 위해 <br>
  논리 레벨 MOSFET과 결합된 매우 간단한 **펄스 스트레칭(pulse-stretching) 회로**입니다.

![](SR602-Quick-Test.jpg)

- 관심 있는 분은 원하는 대로 회로를 조정할 수 있습니다.
- 즉, 기본 회로의 기본 펄스 길이가 마음에 들지 않으면 저항 및/또는 커패시터를 조정하여 테스트 설정의 동작을 변경할 수 있습니다.

![](SR602-Quick-Test-2.jpg)

---

## MH-SR602 요약

| 항목 | 내용 |
|:---|:---|
| **모델** | SR602 |
| **감지 방식** | PIR (수동 적외선) |
| **감지 거리** | 최대 5m (0~3.5m 권장) |
| **출력 신호** | High (3.3V) / Low (0V) |
| **전원 범위** | 3.3V ~ 15V DC |
| **대기 전류** | 20μA |
| **출력 시간 (기본)** | 2.5초 (조정 가능, 최대 1시간) |
| **추가 기능** | 감도 조절, 주간/야간 감지 (외부 포토다이오드) |
| **온보드 레귤레이터** | 3.3V LDO |

---

# STM32F103 NUCLEO 샘플 코드

## 하드웨어 연결

![](Fig.NUCLEO-F103RB.png)

### 핀 연결도

```
MH-SR602 PIR 센서          NUCLEO-F103RB
┌────────────────┐        ┌────────────────┐
│  OUT (출력)    │───────▶│ PA0 (CN7-28)   │  Digital Input
│  +  (VCC)      │───────▶│ 5V  (CN7-18)   │
│  –  (GND)      │───────▶│ GND (CN7-20)   │
└────────────────┘        └────────────────┘
```

### 회로도

```
                    ┌─────────────────────┐
                    │   MH-SR602 PIR      │
                    │   Motion Sensor     │
                    │                     │
    5V  ───────────▶│ + (VCC)             │
                    │     ┌──────────┐    │
    GND ───────────▶│ –   │ PIR      │    │
                    │     │ Element  │    │
    PA0 ◀───────────│ OUT └──────────┘    │
                    │     (Fresnel Lens)  │
                    └─────────────────────┘
                              │
                              ▼
                        인체 감지 (적외선)
```

### NUCLEO-F103RB 보드 핀맵

| NUCLEO 핀 | 기능 | 연결 |
|:---------:|:----:|:----:|
| CN7-28 (PA0) | PIR 출력 (Digital IN) | PIR OUT |
| CN7-18 (5V)  | 전원 (5V) | PIR + |
| CN7-20 (GND) | 접지 | PIR – |
| PA5 (LD2)    | 온보드 LED | 모션 감지 표시 |
| PA2 (CN7-14) | USART2 TX | ST-Link VCP |
| PA3 (CN7-13) | USART2 RX | ST-Link VCP |

## 코드 구조 (main.c)

```
main.c
├── PIR_ReadRaw()             // PIR 센서 원시값 읽기
├── PIR_Read()                // 디바운싱된 상태 읽기
├── PIR_Init()                // PIR 상태 초기화
├── PIR_Update()              // 상태 업데이트 (에지 검출)
├── PIR_JustActivated()       // 방금 모션 감지 시작
├── PIR_JustDeactivated()     // 방금 모션 감지 종료
├── PIR_IsMotionDetected()    // 현재 모션 상태 확인
├── PIR_GetMotionCount()      // 총 모션 이벤트 횟수
├── FormatTime()              // 시간 포맷팅 (HH:MM:SS)
├── Print_Status()            // 상태 바 출력
├── Print_Statistics()        // 통계 리포트 출력
├── MX_GPIO_Init()            // GPIO 초기화
├── MX_USART2_UART_Init()     // UART 초기화
└── main()                    // 메인 루프
```

### 주요 기능

| 기능 | 설명 |
|:----|:------|
| **디바운싱** | 소프트웨어 디바운스 (30ms)로 안정적인 센서 읽기 |
| **에지 검출** | Rising edge (모션 시작) / Falling edge (모션 종료) 감지 |
| **쿨다운 타이머** | 센서 출력 펄스(2.5s) 동안 재트리거 방지 |
| **LED 표시** | 모션 감지 시 온보드 LED ON |
| **상태 표시줄** | 진행 바로 모션 지속 시간 시각화 |
| **통계** | 30초마다 모션 횟수/시간 통계 출력 |
| **사용자 버튼** | PC13 버튼으로 통계 리셋 |

### 동작 흐름

```
                    ┌─────────────┐
                    │  HAL_Init() │
                    │  Clock Cfg  │
                    │  GPIO Init  │
                    │  UART Init  │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │ PIR_Init()  │
                    │ 초기 상태 확인 │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
         ┌─────────┤   while(1)  │◄──────────────────┐
         │         └──────┬──────┘                    │
         │                │                           │
         │         ┌──────▼──────┐                    │
         │         │ PIR_Update()│                    │
         │         │ - 디바운스   │                    │
         │         │ - 에지 검출   │                    │
         │         │ - 쿨다운 체크 │                    │
         │         └──────┬──────┘                    │
         │                │                           │
         │         ┌──────▼──────┐                    │
         │         │ LED 제어     │                    │
         │         │  모션=ON     │                    │
         │         │  대기=OFF    │                    │
         │         └──────┬──────┘                    │
         │                │                           │
         │         ┌──────▼──────┐                    │
         │         │ 상태 출력    │                    │
         │         │ (250ms 주기) │                    │
         │         └──────┬──────┘                    │
         │                │                           │
         │         ┌──────▼──────┐                    │
         │         │ 통계 출력    │                    │
         │         │ (30초 주기)  │                    │
         │         └──────┬──────┘                    │
         │                │                           │
         │         ┌──────▼──────┐                    │
         └─────────┤ HAL_Delay   │                    │
                   │ (10ms)     │────────────────────┘
                   └─────────────┘
```

### PIR 감지 타이밍도

```
         모션 없음         모션 감지         모션 종료 (쿨다운)
         ══════════╗      ╔══≈2.5s══╗      ╔══════════════
                   ║      ║         ║      ║
    OUT  ──────────╨──────╨─────────╨──────╨──────────────
                   3.3V               0V
         ▲         ▲                  ▲
         │         │                  │
    PA0 읽기=0  PA0 읽기=1        PA0 읽기=0
    (IDLE)    (MOTION)           (쿨다운 중 무시)
                                       │
                                  cooldown_until
                                  동안 추가 트리거 차단
```

### 시리얼 출력 예시 (UART, 115200 baud)

```
============================================
   MH-SR602 PIR Motion Sensor Test Program
   NUCLEO-F103RB Board
============================================

OUT Pin: PA0 (Digital Input, Active HIGH)
Detection Range: 0~5 meters (0~3.5m recommended)
Output Duration: ~2.5s (default, adjustable)

Sensor Behavior:
  - HIGH signal when motion is detected
  - LOW signal when no motion (idle)
  - Cooldown period: 2500 ms to avoid retrigger

Wave your hand in front of the sensor to test.
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

PIR Sensor initialized. Initial state: IDLE

[░░░░░░░░░░░░░░░░░░░░] IDLE               | Events:   0 | Motion: 00:00:00 | Idle: 00:00:05

>>> [MOTION DETECTED] #1 <<<
    Previous idle duration: 5240 ms
[████████░░░░░░░░░░░░] MOTION! (530 ms)   | Events:   1 | Motion: 00:00:00 | Idle: 00:00:05

>>> [MOTION ENDED] Duration: 1230 ms <<<
[░░░░░░░░░░░░░░░░░░░░] IDLE               | Events:   1 | Motion: 00:00:01 | Idle: 00:00:05

>>> [MOTION DETECTED] #2 <<<
    Previous idle duration: 3120 ms
[████████████████░░░░] MOTION! (1050 ms)  | Events:   2 | Motion: 00:00:02 | Idle: 00:00:08

========== PIR Sensor Statistics ==========
  Total motion events : 2
  Total motion time   : 2350 ms (0%)
  Total idle time     : 8420 ms (78%)
  Total elapsed time  : 10770 ms
  Avg event duration  : 1175 ms
============================================
```

## CubeMX 설정 방법

### STM32CubeIDE 프로젝트 생성

1. **File > New > STM32 Project**
2. **Board Selector**에서 `NUCLEO-F103RB` 선택
3. 프로젝트 이름 입력 후 **Finish**

### Pinout 설정 (Pinout View)

| 핀 | 설정 | Configuration |
|:---:|:----:|:------------:|
| PA0 | **GPIO_Input** | Pull-up: **Pull-down**, Label: `PIR_OUT` |
| PA5 | **GPIO_Output** | (기본값 - 온보드 LED) |
| PA2 | **USART2_TX** | (기본값 - ST-Link VCP) |
| PA3 | **USART2_RX** | (기본값 - ST-Link VCP) |

### Clock Configuration

```
HSE (8MHz) → PLL (x9) → SYSCLK = 72MHz
APB1 = 36MHz, APB2 = 72MHz
```

### NVIC Settings

| Interrupt | Priority | Enable |
|:---------:|:--------:|:------:|
| USART2 | 0 | ✅ |
| EXTI15_10 (B1 Button) | 0 | ✅ |

### Project Generator 설정

1. **Project > Settings > Code Generator**
   - `Generate peripheral initialization as a pair of .c/.h files per peripheral` ✅
   - `Backup previously generated files when re-generating` ✅

2. **Generate Code** 클릭

3. 생성된 `main.c`를 위 샘플 코드로 교체 (또는 USER CODE 블록에 복사)

---

## EXTI 인터럽트 모드 (고급)

메인 루프 폴링 대신 EXTI 인터럽트를 사용하여 모션을 즉시 감지할 수 있습니다.

### CubeMX 설정 변경

1. PA0를 **GPIO_EXTI0**으로 변경
2. **NVIC > EXTI line0 interrup**t Enable
3. 우선순위 설정

### 코드 추가

```c
/* stm32f1xx_it.c 또는 main.c USER CODE 블록 */

/* 전역 플래그 */
volatile uint8_t pir_interrupt_flag = 0;
volatile uint32_t pir_interrupt_time = 0;

/* EXTI Callback */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
        /* 모션 감지 인터럽트 - 즉시 응답 필요 시 사용 */
        pir_interrupt_flag = 1;
        pir_interrupt_time = HAL_GetTick();

        /* 긴급 동작 (예: LED 즉시 ON, 부저 울리기 등) */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    }
}
```

### 폴링 vs 인터럽트 비교

| 방식 | 장점 | 단점 |
|:---:|:----|:----|
| **폴링** | 구현 간단, 코드 이해 쉬움 | CPU 지속 사용, 응답 지연 가능 |
| **인터럽트** | 즉각 응답, CPU 효율적 | 구현 복잡, 공유 변수 보호 필요 |

---

# 응용 프로젝트

## 1. 자동 조명 시스템 (Auto Lighting)

PIR 센서로 사람 감지 시 조명을 자동으로 켜고, 일정 시간 후 자동 소등합니다.

```
        ┌─────────────────────────────────────┐
        │         자동 조명 시스템              │
        │                                      │
        │  PIR ───┬──► STM32 ──► 릴레이 ──► 전등  │
        │         │                              │
        │    광센서 (옵션: 주간에는 미동작)          │
        └─────────────────────────────────────┘

    동작:
    ┌──────┐                    ┌──────┐
    │ 모션  │                    │ 모션  │
    │ 감지  │                    │ 없음  │
    └──┬───┘                    └──┬───┘
       ▼                           ▼
    ┌──────┐     ┌──────────┐   ┌──────┐
    │ 조명 ON│────│ 타이머   │───│ 조명  │
    │       │    │ 시작(30s)│   │ OFF  │
    └──────┘    └──────────┘   └──────┘
                    │
        모션 재감지 → 타이머 리셋 (계속 ON)
```

```c
/* 자동 조명 예제 코드 */
#define LIGHT_ON_TIME_MS    30000   /* 30초 후 소등 */
#define LIGHT_PORT          GPIOB
#define LIGHT_PIN           GPIO_PIN_0  /* 릴레이 모듈 연결 */

uint32_t light_timer = 0;
uint8_t light_state = 0;

void AutoLight_Update(void) {
    if (PIR_IsMotionDetected()) {
        /* 모션 감지 → 조명 ON + 타이머 리셋 */
        if (!light_state) {
            HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_SET);
            light_state = 1;
            printf("Light: ON (motion detected)\r\n");
        }
        light_timer = HAL_GetTick();
    } else {
        /* 모션 없음 → 타이머 체크 */
        if (light_state && (HAL_GetTick() - light_timer >= LIGHT_ON_TIME_MS)) {
            HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_RESET);
            light_state = 0;
            printf("Light: OFF (timeout)\r\n");
        }
    }
}
```

---

## 2. 침입 경보 시스템 (Security Alarm)

PIR 센서와 부저/LED를 결합하여 침입 감지 시 경보를 울립니다.

```
    ┌─────────────────────────────────────┐
    │         보안 경보 시스템              │
    │                                      │
    │  PIR ──► STM32 ──┬──► 부저 (알람)     │
    │                  ├──► LED (점멸)      │
    │                  ├──► LCD (상태 표시)  │
    │                  └──► UART (PC 로깅)  │
    │                                      │
    │  Keypad ──► 암호 해제                  │
    └─────────────────────────────────────┘

    상태 천이:
    ┌─────────┐    모션감지    ┌─────────┐
    │  해제    │──────────────▶│  경보   │
    │ (DISARM)│                │ (ALARM) │
    └────┬────┘                └────┬────┘
         ▲                          │
         │    암호 해제              │
         └──────────────────────────┘
                                │ 30초 또는
                                │ 수동 해제까지
                                ▼
                          부저 ON / LED 점멸
```

```c
/* 보안 경보 예제 코드 */
typedef enum { STATE_DISARMED, STATE_ARMED, STATE_ALARM } SecurityState;
SecurityState sec_state = STATE_DISARMED;

#define ALARM_SIREN_PORT    GPIOB
#define ALARM_SIREN_PIN     GPIO_PIN_1   /* 부저/사이렌 */
#define ALARM_DURATION_MS   30000        /* 30초 경보 */
#define ENTRY_DELAY_MS      10000        /* 진입 지연 10초 */

void Security_Update(void) {
    uint32_t now = HAL_GetTick();

    switch (sec_state) {
        case STATE_ARMED:
            if (PIR_IsMotionDetected()) {
                printf("ALERT! Motion detected while armed!\r\n");
                sec_state = STATE_ALARM;

                /* 사이렌 ON */
                HAL_GPIO_WritePin(ALARM_SIREN_PORT, ALARM_SIREN_PIN, GPIO_PIN_SET);

                /* 알람 시작 시간 기록 */
                pir.last_motion_time = now;
            }
            break;

        case STATE_ALARM:
            /* 30초 후 자동 해제 */
            if (now - pir.last_motion_time >= ALARM_DURATION_MS) {
                HAL_GPIO_WritePin(ALARM_SIREN_PORT, ALARM_SIREN_PIN, GPIO_PIN_RESET);
                sec_state = STATE_DISARMED;
                printf("Alarm auto-reset after %d seconds\r\n", ALARM_DURATION_MS / 1000);
            } else {
                /* LED 점멸 효과 */
                if ((now / 200) % 2) {
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                } else {
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                }
            }
            break;

        case STATE_DISARMED:
            /* 대기 상태 - LED OFF */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            break;
    }
}
```

---

## 3. 에너지 절약 시스템 (Energy Saving)

일정 시간 사람이 없으면 조명/에어컨 등을 자동으로 차단합니다.

```c
/* 에너지 절약 예제 */
#define ENERGY_SAVE_TIMEOUT_MS   300000    /* 5분 */

typedef struct {
    uint32_t last_motion_time;
    uint8_t occupied;           /* 1=사용중, 0=비어있음 */
    uint32_t empty_start_time;
} RoomOccupancy;

RoomOccupancy room = {0};

void EnergySave_Update(void) {
    uint32_t now = HAL_GetTick();

    if (PIR_IsMotionDetected()) {
        room.last_motion_time = now;

        if (!room.occupied) {
            /* 사람 들어옴 → 장비 ON */
            room.occupied = 1;
            room.empty_start_time = 0;
            printf("Room: OCCUPIED - Equipment ON\r\n");
            /* 조명 ON, 에어컨 ON 등 */
        }
    } else {
        if (room.occupied) {
            uint32_t empty_time = now - room.last_motion_time;

            if (empty_time >= ENERGY_SAVE_TIMEOUT_MS) {
                /* 일정 시간 비었음 → 장비 OFF */
                room.occupied = 0;
                room.empty_start_time = now;
                printf("Room: EMPTY - Equipment OFF (saved energy)\r\n");
                /* 조명 OFF, 에어컨 OFF 등 */
            } else if (empty_time > 60000) {
                /* 1분 이상 비었음 → 알림 */
                printf("Room empty for %lu seconds...\r\n", empty_time / 1000);
            }
        }
    }
}
```

---

## 4. 스마트 복도/계단 조명

```c
/* 복도 조명: 한쪽 끝에서 감지 → 반대쪽 도달 전까지 ON */
#define CORRIDOR_LIGHT_TIMEOUT  15000   /* 15초 */

void CorridorLight_Update(void) {
    static uint32_t light_off_time = 0;
    static uint8_t corridor_light = 0;

    if (PIR_JustActivated()) {
        /* 모션 감지 → 조명 ON (타이머 리셋) */
        HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_SET);
        corridor_light = 1;
        light_off_time = HAL_GetTick() + CORRIDOR_LIGHT_TIMEOUT;
        printf("Corridor light: ON\r\n");
    }

    if (corridor_light && HAL_GetTick() >= light_off_time) {
        /* 타임아웃 → 조명 OFF */
        HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_RESET);
        corridor_light = 0;
        printf("Corridor light: OFF\r\n");
    }
}
```

---

## 5. 다중 PIR 센서 배열

여러 PIR 센서로 넓은 공간을 커버하거나 방향 감지가 가능합니다.

```c
/* 3채널 PIR 센서 배열 */
#define NUM_PIR_SENSORS     3

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t state;
    uint32_t last_trigger;
    char name[16];
} PIRChannel;

PIRChannel pir_channels[NUM_PIR_SENSORS] = {
    {GPIOA, GPIO_PIN_0, 0, 0, "PIR-1 (Left)"},
    {GPIOA, GPIO_PIN_1, 0, 0, "PIR-2 (Center)"},
    {GPIOA, GPIO_PIN_4, 0, 0, "PIR-3 (Right)"},
};

void MultiPIR_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    for (int i = 0; i < NUM_PIR_SENSORS; i++) {
        GPIO_InitStruct.Pin = pir_channels[i].pin;
        HAL_GPIO_Init(pir_channels[i].port, &GPIO_InitStruct);
    }
}

void MultiPIR_Scan(void) {
    for (int i = 0; i < NUM_PIR_SENSORS; i++) {
        uint8_t raw = HAL_GPIO_ReadPin(pir_channels[i].port, pir_channels[i].pin);

        if (raw && !pir_channels[i].state) {
            /* Rising edge */
            pir_channels[i].state = 1;
            pir_channels[i].last_trigger = HAL_GetTick();
            printf(">>> %s triggered! <<<\r\n", pir_channels[i].name);
        }

        if (!raw && pir_channels[i].state) {
            /* Falling edge */
            pir_channels[i].state = 0;
        }
    }
}

/* 이동 방향 감지 */
typedef enum {
    DIR_NONE,
    DIR_LEFT_TO_RIGHT,
    DIR_RIGHT_TO_LEFT
} MovementDirection;

MovementDirection Detect_Direction(void) {
    static uint32_t last_left_time = 0;
    static uint32_t last_right_time = 0;

    if (pir_channels[0].state && !pir_channels[1].state) {
        /* PIR-1 (Left) triggered */
        last_left_time = HAL_GetTick();
    }
    if (pir_channels[2].state && !pir_channels[1].state) {
        /* PIR-3 (Right) triggered */
        last_right_time = HAL_GetTick();
    }

    uint32_t now = HAL_GetTick();
    if (last_left_time > last_right_time && (now - last_left_time) < 2000) {
        return DIR_LEFT_TO_RIGHT;
    }
    if (last_right_time > last_left_time && (now - last_right_time) < 2000) {
        return DIR_RIGHT_TO_LEFT;
    }
    return DIR_NONE;
}
```

**다중 PIR 센서 레이아웃:**

```
    ┌─────────────────────────────────────────┐
    │              감시 영역                    │
    │                                          │
    │  ┌──────┐    ┌──────┐    ┌──────┐      │
    │  │ PIR 1│    │ PIR 2│    │ PIR 3│      │
    │  │ LEFT │    │ CENTER│   │ RIGHT│      │
    │  └──┬───┘    └──┬───┘    └──┬───┘      │
    │     │           │           │          │
    │     └─────┬─────┘           │          │
    │           │                 │          │
    │     PA0   │  PA1      PA4  │          │
    └───────────┴─────────────────┴──────────┘

    방향 감지:
    PIR1 → PIR2 → PIR3 : ← Left to Right
    PIR3 → PIR2 → PIR1 : → Right to Left
```

---

## 6. 스마트 화장실/재실 감지

화장실이나 회의실의 재실 여부를 감지하여 관리합니다.

```c
/* 재실 감지 시스템 */
#define VACANT_TIMEOUT_MS   60000   /* 1분 후 빈방 처리 */

void RoomStatus_Update(void) {
    static uint32_t vacant_check_time = 0;
    static uint32_t motion_seen_time = 0;

    if (PIR_IsMotionDetected()) {
        motion_seen_time = HAL_GetTick();
    }

    /* 주기적 상태 확인 (10초마다) */
    if (HAL_GetTick() - vacant_check_time >= 10000) {
        vacant_check_time = HAL_GetTick();

        if (HAL_GetTick() - motion_seen_time > VACANT_TIMEOUT_MS) {
            printf("Room: VACANT\r\n");
        } else {
            printf("Room: OCCUPIED\r\n");
        }
    }
}
```

---

## 7. PIR + 광센서 결합 (주간/야간 모드)

MH-SR602의 외부 포토다이오드 옵션을 하드웨어 없이 소프트웨어로 구현합니다.

```c
/* 주간/야간 모드 (CDS 광센서 + PIR) */
/* 하드웨어: CDS 모듈 (KY-018) → PA1 (ADC1_IN1) */

#define LIGHT_THRESHOLD_DAY     2000   /* ADC 값 기준 (0-4095, 낮을수록 어두움) */

uint8_t IsDark(void) {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        uint16_t adc_val = HAL_ADC_GetValue(&hadc1);
        return (adc_val < LIGHT_THRESHOLD_DAY) ? 1 : 0;
    }
    return 0;
}

void SmartLight_Update(void) {
    if (IsDark() && PIR_IsMotionDetected()) {
        /* 어둡고 모션 감지 → 조명 ON */
        HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_SET);
        printf("Night mode: Light ON (motion + dark)\r\n");
    } else {
        /* 밝거나 모션 없음 → 조명 OFF */
        HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_RESET);
    }
}
```

---

## 8. PIR 센서 데이터 로깅 (SD 카드 + FATFS)

SD 카드에 모션 이벤트를 CSV 파일로 기록합니다.

```c
/* FATFS + SD 카드 로깅 (RTC 필요) */
#include "ff.h"
#include "rtc.h"

FATFS fs;
FIL fil;

void PIR_LogToSD(void) {
    static uint8_t last_log_state = 0;
    uint8_t current = PIR_IsMotionDetected();

    if (current != last_log_state) {
        last_log_state = current;

        /* SD 카드 마운트 */
        if (f_mount(&fs, "", 1) == FR_OK) {
            /* 로그 파일 열기 (append) */
            if (f_open(&fil, "pir_log.csv", FA_OPEN_APPEND | FA_WRITE) == FR_OK) {
                RTC_TimeTypeDef rtc_time;
                RTC_DateTypeDef rtc_date;
                HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
                HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

                /* CSV 기록: 날짜,시간,상태,이벤트번호 */
                char line[64];
                sprintf(line, "%02d-%02d-%02d,%02d:%02d:%02d,%s,%lu\r\n",
                        rtc_date.Year, rtc_date.Month, rtc_date.Date,
                        rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds,
                        current ? "MOTION" : "IDLE",
                        pir.motion_count);
                f_puts(line, &fil);
                f_close(&fil);
            }
            f_unmount("");
        }
    }
}
```

**CSV 로그 예시:**
```csv
26-06-07,14:23:15,MOTION,1
26-06-07,14:23:18,IDLE,1
26-06-07,14:25:30,MOTION,2
26-06-07,14:25:33,IDLE,2
```

---

## 9. IoT 연동 (ESP8266/ESP32를 통한 MQTT)

PIR 센서 데이터를 MQTT로 전송하여 스마트폰 앱과 연동합니다.

```c
/* UART3를 통해 ESP8266 AT 커맨드로 WiFi 전송 */
/* ESP8266: PB10 (TX), PB11 (RX), 115200 baud */

extern UART_HandleTypeDef huart3;

void ESP8266_Send(const char *cmd) {
    HAL_UART_Transmit(&huart3, (uint8_t *)cmd, strlen(cmd), 1000);
}

void PIR_MQTTPublish(void) {
    char topic[] = "home/sensor/pir";
    char payload[32];

    if (PIR_JustActivated()) {
        sprintf(payload, "{\"motion\":1,\"count\":%lu}", pir.motion_count);

        /* MQTT publish (AT+CIPSEND) */
        char cmd[128];
        sprintf(cmd, "AT+CIPSEND=%d\r\n", strlen(payload) + strlen(topic) + 5);
        ESP8266_Send(cmd);
        HAL_Delay(100);
        ESP8266_Send(topic);
        ESP8266_Send(" ");
        ESP8266_Send(payload);
        printf("MQTT published: %s\r\n", payload);
    }
}
```

---

## 10. 애완동물 구분 (듀얼 PIR + 높이 차이)

두 개의 PIR 센서를 수직으로 배치하여 사람과 애완동물을 구분합니다.

```
    ┌─────────────┐
    │  PIR Upper  │ ← 120cm (사람 감지)
    │  (PA0)      │
    ├─────────────┤
    │             │
    │   80cm      │ ← 애완동물은 하단만 감지
    │             │
    ├─────────────┤
    │  PIR Lower  │ ← 30cm  (애완동물 감지)
    │  (PA1)      │
    └─────────────┘

    Upper + Lower = 사람
    Lower only    = 애완동물
    Upper only    = 무시 (오류)
```

```c
/* 애완동물 구분 */
typedef enum {
    DETECT_NONE,
    DETECT_HUMAN,       /* Upper + Lower */
    DETECT_PET,         /* Lower only */
    DETECT_ERROR        /* Upper only */
} DetectionType;

DetectionType ClassifyDetection(uint8_t upper, uint8_t lower) {
    if (upper && lower)  return DETECT_HUMAN;
    if (!upper && lower) return DETECT_PET;
    if (upper && !lower) return DETECT_ERROR;
    return DETECT_NONE;
}

void PetSafe_Update(void) {
    static uint8_t last_upper = 0, last_lower = 0;
    uint8_t upper = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    uint8_t lower = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);

    /* 에지 검출 */
    if (upper || lower) {
        if (!last_upper && !last_lower) {
            /* 새 감지 이벤트 */
            DetectionType type = ClassifyDetection(upper, lower);
            switch (type) {
                case DETECT_HUMAN:
                    printf("Human detected (full body)\r\n");
                    /* 조명 ON, 알람 해제 등 */
                    break;
                case DETECT_PET:
                    printf("Pet detected (lower only) - IGNORED\r\n");
                    /* 아무 동작 안함 */
                    break;
                case DETECT_ERROR:
                    printf("Sensor error: upper only!\r\n");
                    break;
                default:
                    break;
            }
        }
    }

    last_upper = upper;
    last_lower = lower;
}
```

---

## 응용 아이디어 요약

| 프로젝트 | 난이도 | 핵심 기능 | 추가 하드웨어 |
|:---------|:------:|:----------|:------------:|
| 자동 조명 | ⭐ | 모션 감지 → 릴레이 제어 | 릴레이 모듈 |
| 침입 경보 | ⭐⭐ | 모션 감지 → 부저/LED | 부저, LED |
| 에너지 절약 | ⭐ | 부재 감지 → 장비 OFF | 릴레이 모듈 |
| 복도/계단 조명 | ⭐ | 모션 감지 → 타이머 ON | 릴레이 모듈 |
| 다중 PIR 배열 | ⭐⭐⭐ | 3채널 스캔, 방향 감지 | PIR 2~3개 |
| 재실 감지 | ⭐⭐ | 재실/부재 상태 관리 | 없음 |
| PIR + 광센서 | ⭐⭐ | 주간/야간 모드 전환 | CDS 모듈 |
| SD 카드 로깅 | ⭐⭐⭐ | FATFS + CSV 기록 | SD 카드 모듈 |
| IoT MQTT | ⭐⭐⭐⭐ | WiFi 통신, 앱 알림 | ESP8266 모듈 |
| 애완동물 구분 | ⭐⭐⭐⭐ | 2채널 수직 배열 | PIR 추가 1개 |

---

## 문제 해결 및 FAQ

### 센서가 전혀 반응하지 않아요
- 전원 연결 확인 (VCC = 5V, GND = GND)
- OUT 핀이 PA0에 올바르게 연결되었는지 확인
- PA0의 풀업/풀다운 저항 설정 확인 (PULLDOWN 권장)
- 움직임이 센서 정면에서 이루어지는지 확인
- 감지 거리 내에 있는지 확인 (최대 5m, 권장 0~3.5m)

### LED가 계속 켜져 있어요 (항상 모션 감지)
- PA0의 풀업/풀다운 설정 확인 (PULLDOWN 사용)
- 센서 앞에 움직이는 물체가 없는지 확인
- 센서의 감도가 너무 높게 설정되지 않았는지 확인
- 전원 노이즈 여부 확인 (5V 안정적인지)

### 모션 감지가 불규칙해요
- 디바운스 시간 조정 (DEBOUNCE_TIME_MS 증가)
- 센서의 감도 조절 (보드의 칩 저항 교체)
- 쿨다운 시간 확인 (COOLDOWN_TIME_MS = 2500ms 권장)
- 센서 렌즈가 깨끗한지 확인
- 직사광선 또는 히터 근처에 설치하지 않음

### 감지 범위가 너무 짧아요
- 센서의 감도 조절 필요 (칩 저항 교체)
- 전원 전압 확인 (5V 권장)
- 프레넬 렌즈가 막히지 않았는지 확인

### 사람이 지나가도 감지했다 안했다 해요
- 센서의 감지 패턴 이해 필요 (PIR은 교차 패턴에 민감)
- 움직임 방향이 센서에 수직일 때 가장 잘 감지
- 천천히 움직이면 감지 안 될 수 있음

### 주간에는 동작하지 않고 야간에만 동작하게 하려면?
- MH-SR602는 외부 F3 포토다이오드 추가로 하드웨어 구현 가능
- 또는 CDS 광센서 모듈을 추가하여 소프트웨어로 구현 (위 응용 예제 참조)

---

## 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo-64-boards-mb1136-stmicroelectronics.pdf)
- [PIR Sensor Technology Guide](https://www.mpja.com/download/31227sc.pdf)

*출처: https://arduinomodules.info/*


*출처: https://arduinomodules.info/*
