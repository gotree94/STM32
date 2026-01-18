# 리드 스위치 모듈 (Reed Switch Module)

STM32F103 NUCLEO 보드를 이용한 리드 스위치 테스트 프로젝트

## 📋 개요

리드 스위치는 자기장에 반응하여 회로를 개폐하는 전자기 스위치입니다. 유리 튜브 안에 두 개의 강자성 금속 리드가 있으며, 자석이 접근하면 리드가 접촉하여 회로가 닫힙니다. 문/창문 보안 센서, 위치 감지, 회전 계수 등에 널리 사용됩니다.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| 리드 스위치 모듈 | 1 | 자기 감응 스위치 |
| 네오디뮴 자석 | 1 | 테스트용 (모듈에 포함된 경우 있음) |
| 점퍼 와이어 | 3~4 | 연결용 |

### 핀 연결

```
리드 스위치 모듈        NUCLEO-F103RB
┌─────────────┐        ┌─────────────┐
│     VCC     │───────▶│    3.3V     │
│     GND     │───────▶│    GND      │
│     DO      │───────▶│    PA0      │ (Digital Input)
│     AO      │───────▶│    PA1      │ (Optional, ADC)
└─────────────┘        └─────────────┘
```

### 회로도

```
                    ┌─────────────────────┐
                    │   Reed Switch       │
                    │   Module            │
    3.3V ──────────▶│ VCC                 │
                    │    ┌──────────┐     │
    GND  ──────────▶│ GND│  Reed    │     │
                    │    │  Switch  │     │
    PA0  ◀──────────│ DO └──────────┘     │
                    │    (Magnetic)       │
    PA1  ◀──────────│ AO  (Optional)      │
                    └─────────────────────┘
                              ↑
                          [Magnet]
                          N     S

    리드 스위치 내부 구조:
    ┌────────────────────────────────────┐
    │         Glass Tube                 │
    │  ══════╗          ╔══════          │
    │        ╚══════════╝                │
    │     Ferromagnetic Reeds            │
    └────────────────────────────────────┘
           자석 접근 → 리드 접촉 → 회로 닫힘
```

## 💻 소프트웨어 설명

### 주요 기능

1. **상태 감지**: 자석 접근/이탈 감지
2. **디바운싱**: 안정적인 스위치 읽기
3. **통계 추적**: 개폐 횟수 및 지속 시간 기록
4. **알람 시뮬레이션**: 열림 상태에서 LED 점멸

### 동작 원리

```
    자석 없음 (OPEN)              자석 있음 (CLOSED)
    ┌─────────────────┐          ┌─────────────────┐
    │  ══════   ══════│          │  ══════════════ │
    │   OFF    OFF    │          │       ON        │
    │  DO = HIGH      │          │  DO = LOW       │
    └─────────────────┘          └─────────────────┘
```

- **작동 거리**: 일반적으로 10~20mm
- **히스테리시스**: 닫힘 거리 < 열림 거리 (채터링 방지)
- **출력**: 대부분의 모듈은 Active LOW (자석 감지 시 LOW)

### 코드 구조

```
main.c
├── Reed_Read()           // 디바운싱된 상태 읽기
├── Reed_ReadAnalog()     // 아날로그 값 읽기 (옵션)
├── Reed_Update()         // 상태 업데이트 및 통계
├── Reed_JustClosed()     // 방금 닫힘 감지
├── Reed_JustOpened()     // 방금 열림 감지
├── Reed_IsClosed()       // 현재 상태 확인
├── FormatTime()          // 시간 포맷팅
├── Print_Status()        // 상태 출력
├── MX_GPIO_Init()        // GPIO 초기화
├── MX_ADC1_Init()        // ADC 초기화
└── MX_USART2_UART_Init() // UART 초기화
```

## 📊 출력 예시

```
========================================
    Reed Switch Module Test Program
    NUCLEO-F103RB Board
========================================

DO Pin: PA0 (Digital Input)
AO Pin: PA1 (ADC1_IN1, optional)

Application: Door/Window Security Sensor

Initial state: OPEN
Bring a magnet close to the reed switch to test.
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

[○○○○○  OPEN  ○○○○○] | Opens:   0 | Closes:   0 | Open: 00:00:05 | Closed: 00:00:00

>>> CLOSED (Magnet detected) - Open duration: 5230 ms <<<
[■■■■■ CLOSED ■■■■■] | Opens:   0 | Closes:   1 | Open: 00:00:05 | Closed: 00:00:02

>>> OPEN (Magnet removed) - Closed duration: 2150 ms <<<
[○○○○○  OPEN  ○○○○○] | Opens:   1 | Closes:   1 | Open: 00:00:07 | Closed: 00:00:02

[Analog: 4012]
```

## ⚙️ 빌드 및 실행

### STM32CubeIDE 사용 시

1. 새 프로젝트 생성 (STM32F103RB 선택)
2. `main.c` 파일 내용으로 교체
3. 빌드 및 다운로드

### 설정 변경

```c
// main.c 상단의 설정값 수정
#define DEBOUNCE_TIME_MS    20      // 디바운스 시간 (채터링 방지)
```

## 🏠 응용 예제

### 1. 문/창문 보안 센서
```
    문틀                         문
┌──────────┐                ┌──────────┐
│  Reed    │                │  Magnet  │
│  Switch  │◄───── Gap ────►│   N S    │
└──────────┘                └──────────┘

문 닫힘: Gap 작음 → CLOSED
문 열림: Gap 큼  → OPEN → 알람!
```

### 2. 회전 계수기 (RPM 측정)
```c
// 회전체에 자석 부착, 고정부에 리드 스위치
volatile uint32_t pulse_count = 0;

void EXTI0_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0)) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
        pulse_count++;
    }
}

// 1초당 펄스 수 = RPM
```

### 3. 액체 레벨 센서
```
    ┌───────────────────┐
    │     Float         │
    │   ┌───────┐       │
    │   │Magnet │ ↑↓    │
    │   └───────┘       │
    │ ═══Reed═══        │ ← 고정 위치
    │                   │
    └───────────────────┘
    
Float 상승 → 자석이 리드 스위치에 접근 → CLOSED
```

### 4. 사이클 컴퓨터
```c
// 바퀴 자석으로 속도 계산
float wheel_circumference = 2.1f;  // 미터
uint32_t last_time = 0;
float speed_kmh = 0;

if (Reed_JustClosed()) {
    uint32_t current_time = HAL_GetTick();
    uint32_t period_ms = current_time - last_time;
    
    // 속도 = 거리 / 시간
    speed_kmh = (wheel_circumference / period_ms) * 3600.0f;
    last_time = current_time;
}
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결방법 |
|------|------|----------|
| 감지 안됨 | 자석 거리 멂 | 자석을 더 가까이 |
| 항상 닫힘 | 자석이 붙어있음 | 자석 거리 확인 |
| 불안정한 감지 | 채터링 | 디바운스 시간 증가 |
| 반대 동작 | 극성 문제 | 모듈 회로 확인, 코드 수정 |

## ⚠️ 주의사항

1. **자석 극성**: N극/S극 방향에 따라 감지 거리가 다를 수 있음
2. **작동 횟수**: 기계식이라 수명이 있음 (수백만~수천만 회)
3. **고속 스위칭**: 너무 빠른 스위칭은 채터링 발생 가능
4. **강한 자기장**: 영구 자기화 가능 (가급적 피할 것)
5. **충격**: 유리 튜브라 물리적 충격에 취약

### 리드 스위치 vs 홀 센서

| 특성 | 리드 스위치 | 홀 센서 |
|------|------------|--------|
| 출력 | 디지털 (ON/OFF) | 아날로그/디지털 |
| 반응 속도 | 느림 (~1ms) | 빠름 (~µs) |
| 수명 | 제한적 | 무제한 |
| 전력 소모 | 0 (기계식) | 약간 |
| 감지 거리 | 길음 (10-20mm) | 짧음 (1-5mm) |
| 가격 | 저렴 | 상대적 고가 |

## 📚 참고 자료

- [Reed Switch Operating Principles](https://standexelectronics.com/reed-switch-technology/reed-switch-operating-principles/)
- [Magnetic Reed Switch Application Guide](https://www.littelfuse.com/products/magnetic-sensors-and-reed-switches/reed-switches)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

## 📜 라이선스

MIT License
