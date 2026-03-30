# 🐛 초음파 센서 인식 불량 — 원인 분석 및 수정 가이드

> **대상 파일**: `main.c`, `ultrasonic.c`, CubeMX TIM2/DMA 설정  
> **증상**: 시스템 전체가 느려지고, 초음파 거리 인식이 간헐적으로 실패함  
> **발생 조건**: 자동 주행 모드(`STATE_SCAN` 루프) 진입 후

---

## 📋 버그 목록 요약

| # | 심각도 | 위치 | 증상 영향 |
|---|--------|------|----------|
| 1 | 🔴 높음 | `main.c` 618번 줄 | 서보 진동 중 Trigger → 잘못된 거리값 |
| 2 | 🔴 높음 | `main.c` 632번 줄 | Echo 미수신 → 과거 거리값으로 주행 판단 |
| 3 | 🟡 중간 | `main.c` 93번 줄 | 잠재적 80ms 블로킹 함수 잔존 |
| 4 | 🟡 중간 | CubeMX NVIC 설정 | DMA vs IC 인터럽트 우선순위 충돌 |
| 5 | 🔵 낮음 | CubeMX TIM2 설정 | IC 노이즈 필터 미적용 |

> **Bug 1 + 2만 수정해도 증상의 80% 이상 해결됨**

---

## 🔴 Bug 1 — 서보 안정화 대기 시간 부족

### 원인

`STATE_SCAN`에서 `Servo_SetAngle()`을 호출한 뒤, `STATE_WAIT_ECHO`에서 **20ms만 기다리고** Trigger를 발사합니다.

SG90 서보의 실측 이동 시간:

| 이동 각도 | 소요 시간 |
|-----------|-----------|
| 10° | 약 60~80ms |
| 30° | 약 120~160ms |
| 60° | 약 200~250ms |

20ms 후에는 서보가 목표 각도에 도달하지 못하고 **진동하는 중**입니다.  
이 상태에서 초음파를 발사하면 빔 방향이 틀어져 엉뚱한 거리가 측정됩니다.

### 코드 위치

```c
// main.c — STATE_WAIT_ECHO (618번 줄)
case STATE_WAIT_ECHO:
{
    if (HAL_GetTick() - echo_start_time < 20)   // ← ⚠️ 20ms는 너무 짧음
        break;

    servo_moving = 0;
    Ultrasonic_Trigger();
    ...
}
```

### 수정 방법

```c
// 수정 후
case STATE_WAIT_ECHO:
{
    if (HAL_GetTick() - echo_start_time < 80)   // ✅ 80ms로 변경
        break;

    servo_moving = 0;
    Ultrasonic_Trigger();
    echo_start_time = HAL_GetTick();
    RobotState_Set(STATE_READ_ECHO);
    break;
}
```

> **참고**: 스캔 범위가 30°~150°이고 10° 단위로 이동하므로 80ms면 충분합니다.  
> 정밀도가 더 필요하면 100ms까지 늘려도 됩니다.

---

## 🔴 Bug 2 — Echo 수신 대기 타임아웃 10ms

### 원인

`STATE_READ_ECHO`에서 Echo 응답을 **10ms만 기다리고 포기**합니다.

HC-SR04의 Echo 신호 타이밍:

| 상황 | Echo HIGH 지속 시간 |
|------|---------------------|
| 2cm (최소 거리) | 약 116µs |
| 100cm | 약 5,800µs (5.8ms) |
| 400cm (최대 측정) | 약 23,200µs (23ms) |
| 측정 불가 (장애물 없음) | 약 38ms |

**10ms 타임아웃이면 약 85cm 이상의 거리에서 정상 Echo도 포기**하게 됩니다.  
포기 후에는 `last_valid_dist`(직전 유효값)를 그대로 사용하여 주행을 판단합니다.  
즉, **현재 상황이 아닌 과거 상황으로 장애물 회피를 결정**하는 구조입니다.

### 코드 위치

```c
// main.c — STATE_READ_ECHO (632번 줄)
case STATE_READ_ECHO:
{
    uint16_t dist = Ultrasonic_Read();

    if (dist == 999 && HAL_GetTick() - echo_start_time < 10)  // ← ⚠️ 10ms
        break;   // 아직 에코 안 들어옴 → 대기

    // 값 들어왔거나 타임아웃
    if (dist != 999)
    {
        last_valid_dist = dist;
        g_distance = dist;
    }
    // dist==999이면 last_valid_dist 재사용 ← 위험
    ...
}
```

### 수정 방법

```c
// 수정 후
case STATE_READ_ECHO:
{
    uint16_t dist = Ultrasonic_Read();

    if (dist == 999 && HAL_GetTick() - echo_start_time < 40)  // ✅ 40ms로 변경
        break;

    if (dist != 999)
    {
        last_valid_dist = dist;
        g_distance = dist;
    }
    else
    {
        // ✅ 타임아웃 시 명시적으로 "먼 거리"로 처리 (장애물 없음 가정)
        // 또는 별도 miss 카운터로 누적 관리
        g_distance = 999;
    }
    ...
}
```

### 추가 권장: miss 카운터 기반 안전 처리

연속으로 수신 실패가 쌓이면 보수적으로 정지하는 로직을 추가하면 안전합니다.

```c
static uint8_t echo_miss_count = 0;

if (dist != 999)
{
    last_valid_dist = dist;
    g_distance = dist;
    echo_miss_count = 0;              // 성공 시 카운터 초기화
}
else
{
    echo_miss_count++;
    if (echo_miss_count >= 3)         // 3회 연속 실패 시
    {
        Motor_Stop();                 // 안전을 위해 정지
        echo_miss_count = 0;
        RobotState_Set(STATE_ALERT);
    }
}
```

---

## 🟡 Bug 3 — 잠재적 블로킹 함수 `read_stable_distance()`

### 원인

`main.c` 93번 줄에 정의된 함수가 현재는 상태머신에서 **호출되지 않지만** 코드에 잔존합니다.  
이 함수는 내부에서 `while(40ms)` 루프를 2회 반복하므로 **최대 80ms 블로킹**이 발생합니다.

```c
// main.c — 93번 줄 (현재 미사용이지만 위험한 코드)
static uint16_t read_stable_distance(void)
{
    uint16_t d[2];
    int count = 0;

    for(int i = 0; i < 2; i++)
    {
        uint32_t t = HAL_GetTick();
        while(HAL_GetTick() - t < 40)    // ← 40ms 블로킹 × 2회 = 최대 80ms
        {
            uint16_t dist = Ultrasonic_Read();
            if(dist != 999)
            {
                d[count++] = dist;
                break;
            }
        }
    }
    ...
}
```

### 영향

만약 이 함수를 호출하면 80ms 동안 아래 모든 작업이 멈춥니다:
- `Anim_Update()` → LCD 눈 표정 갱신 지연
- `Buzzer_Update()` → 멜로디 끊김
- UART DMA 수신 버퍼 처리 지연 → 명령 누락 가능

### 수정 방법

```c
// 현재 사용하지 않으므로 제거 권장
// 필요 시 상태머신 기반 논블로킹 버전으로 재작성해야 함

// 제거 또는 아래 주석 처리
/*
static uint16_t read_stable_distance(void) { ... }
*/
```

---

## 🟡 Bug 4 — DMA vs TIM2 IC 인터럽트 우선순위 충돌

### 원인

UART DMA 채널의 NVIC 우선순위가 **0 (최고)**로 설정되어 있습니다.  
TIM2 IC 캡처 인터럽트와 동시에 발생하면 DMA가 먼저 처리되어  
Echo Rising/Falling 엣지 캡처 시각이 수십µs 늦어질 수 있습니다.

```c
// main.c — MX_DMA_Init() (1108번 줄)
HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);   // ← UART3 DMA: 최고 우선순위
HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);   // ← UART2 DMA: 최고 우선순위
```

### 거리 오차 계산

```
캡처 지연 50µs → 거리 오차: 50µs × 0.034cm/µs = 약 0.85cm
캡처 지연 200µs → 거리 오차: 약 3.4cm
```

가까운 거리에서는 이 오차가 장애물 판단에 영향을 줄 수 있습니다.

### 수정 방법

CubeMX에서 NVIC 우선순위를 조정하거나, `MX_DMA_Init()` 이후에 아래 코드를 추가합니다.

```c
// USER CODE BEGIN 2 에 추가 (또는 MX_DMA_Init 수정)

// TIM2 IC 캡처를 UART DMA보다 높은 우선순위로 설정
HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);            // IC 캡처: 최우선
HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 1, 0);   // UART3 DMA: 한 단계 낮춤
HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 1, 0);   // UART2 DMA: 한 단계 낮춤
```

> **주의**: CubeMX에서 재생성하면 덮어써지므로  
> `USER CODE BEGIN` 영역에 배치하거나 CubeMX NVIC 설정에서 직접 변경하세요.

---

## 🔵 Bug 5 — TIM2 IC 노이즈 필터 미적용

### 원인

TIM2 Input Capture의 `ICFilter`가 `0`으로 설정되어 있어  
ECHO 선에 유입되는 전기 노이즈도 유효한 엣지로 처리됩니다.

```c
// main.c — MX_TIM2_Init() (985번 줄)
sConfigIC.ICFilter = 0;   // ← 필터 없음: 노이즈 바로 캡처
```

### HC-SR04 ECHO 선 노이즈 발생 원인

- DC 모터와 ECHO 선이 같은 배선 묶음에 있을 때 (모터 스위칭 노이즈)
- 서보모터 PWM 신호의 고조파
- 점퍼선이 길거나 브레드보드 연결 시 임피던스 불일치

### 수정 방법

**CubeMX에서 변경** (재생성 시 유지되므로 권장):
```
TIM2 → Channel2 → Input Capture Channel 2
  ICFilter: 8     (N=8 샘플 연속 일치 시 엣지 인정)
```

또는 코드에서 직접 설정:
```c
// MX_TIM2_Init() 수정
sConfigIC.ICFilter = 8;   // ✅ 8 샘플 필터 적용 (~1µs @ 64MHz)
```

| ICFilter 값 | 샘플 수 | 노이즈 제거 효과 |
|------------|--------|----------------|
| 0 | 없음 | 필터 없음 |
| 4 | 4 | 약한 노이즈 제거 |
| 8 | 8 | 일반 환경 충분 ✅ |
| 15 | 15 | 고노이즈 환경 |

---

## 🛠️ 전체 수정 적용 체크리스트

```
[ ] Bug 1: STATE_WAIT_ECHO 대기 시간 20ms → 80ms 변경
[ ] Bug 2: STATE_READ_ECHO 타임아웃 10ms → 40ms 변경
[ ] Bug 2: echo_miss_count 기반 연속 실패 처리 추가
[ ] Bug 3: read_stable_distance() 함수 제거
[ ] Bug 4: TIM2_IRQn 우선순위 0, DMA 채널 우선순위 1 설정
[ ] Bug 5: CubeMX TIM2 ICFilter = 8 설정 후 재생성
```

---

## ✅ 수정 후 기대 효과

| 항목 | 수정 전 | 수정 후 |
|------|---------|---------|
| Echo 미수신율 | 높음 (10ms 포기) | 낮음 (40ms 대기) |
| 거리 정확도 | 서보 진동 영향 받음 | 안정화 후 측정 |
| 잘못된 과거값 사용 | 빈번 | miss 카운터로 방지 |
| 노이즈 오인식 | 발생 가능 | ICFilter=8로 억제 |
| 시스템 응답성 | 블로킹 위험 잠재 | 완전 논블로킹 구조 |

---

## 📐 참고: HC-SR04 타이밍 규격

```
TRIG 펄스: 최소 10µs HIGH
Echo 응답:
  ├─ TRIG 발사 후 약 450µs 뒤 Echo 시작
  ├─ Echo HIGH 폭 = 거리(cm) × 58µs
  └─ 최대 대기: 38ms (측정 불가 시)

권장 Trigger 주기: 최소 60ms (반드시 이전 Echo 종료 후)
```

```
[TRIG]  ______|‾‾‾‾‾‾‾‾‾‾|___________________________
               ↑ 10µs

[ECHO]  __________________|‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|__________
                          ↑                 ↑
                     Rising 캡처       Falling 캡처
                     (ic_val1)         (ic_val2)
                          |←── diff(µs) ──→|
                               ÷ 58 = cm
```

---

*광주인력개발원 임베디드시스템 과정 — STM32 Robot Car 디버깅 노트*
