# Shock/Vibration Sensor Module - STM32F103 Test

## 📌 개요

충격/진동 센서 모듈을 NUCLEO-F103RB 보드에 연결하여 물리적 충격과 진동을 감지하는 프로젝트입니다. 디지털 및 아날로그 출력을 모두 활용하여 충격 강도를 측정하고 통계를 제공합니다.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| Shock Sensor Module | 1 | KY-002/SW-18010P 또는 호환 모듈 |
| Jumper Wires | 4 | 연결용 점퍼선 |

### 핀 연결

```
┌─────────────────┐         ┌──────────────────┐
│  Shock Sensor   │         │  NUCLEO-F103RB   │
├─────────────────┤         ├──────────────────┤
│     VCC     ────┼─────────┼──── 3.3V         │
│     GND     ────┼─────────┼──── GND          │
│     DO      ────┼─────────┼──── PA0 (Digital)│
│     AO      ────┼─────────┼──── PA1 (ADC)    │
└─────────────────┘         │                  │
                            │     PA5 ─────────│──── Built-in LED
                            └──────────────────┘
```

### 회로도

```
       +3.3V
         │
    ┌────┴────┐
    │  Shock  │
    │ Sensor  │
    │         │
    │   DO────┼───────────── PA0 (Digital Input)
    │   AO────┼───────────── PA1 (ADC Input)
    │         │
    │   GND───┼───────────── GND
    └─────────┘

    센서 구조:
    ┌─────────────────┐
    │    Spring       │ ← 스프링이 진동에 반응
    │      │          │
    │    ┌─┴─┐        │
    │    │   │ Contact│
    │    └───┘        │
    └─────────────────┘
```

## 📂 프로젝트 구조

```
shock_sensor/
├── main.c          # 메인 소스 코드
├── README.md       # 프로젝트 문서
└── Makefile        # 빌드 설정 (선택)
```

## 💻 소프트웨어 동작

### 감지 모드

| 모드 | 핀 | 설명 |
|------|-----|------|
| 디지털 | PA0 (DO) | ON/OFF 감지 (임계값 기반) |
| 아날로그 | PA1 (AO) | 충격 강도 측정 (0-4095) |

### 충격 강도 분류

| 레벨 | ADC 범위 | 설명 |
|------|----------|------|
| Light | 0-499 | 가벼운 터치/진동 |
| Medium | 500-999 | 중간 충격 |
| Strong | 1000-1999 | 강한 충격 |
| Very Strong | 2000-2999 | 매우 강한 충격 |
| EXTREME | 3000+ | 극심한 충격 |

### 파라미터 설정

```c
#define SHOCK_THRESHOLD     100     /* ADC 감지 임계값 */
#define COOLDOWN_MS         200     /* 이벤트 간 최소 간격 */
#define HISTORY_SIZE        10      /* 통계용 기록 크기 */
```

### 동작 흐름

```
┌──────────────┐
│  센서 대기   │
└──────┬───────┘
       │ 충격 감지
       ▼
┌──────────────┐     ┌──────────────┐
│ 디지털 트리거 │────►│ 아날로그 읽기 │
│   (PA0)      │     │   (PA1)      │
└──────────────┘     └──────┬───────┘
                            │
       ┌────────────────────┼────────────────────┐
       │                    │                    │
       ▼                    ▼                    ▼
┌────────────┐      ┌────────────┐      ┌────────────┐
│ LED 점등   │      │ 강도 계산   │      │ 통계 업데이트│
└────────────┘      └────────────┘      └────────────┘
       │                    │                    │
       └────────────────────┼────────────────────┘
                            ▼
                    ┌──────────────┐
                    │  UART 출력   │
                    └──────────────┘
```

## 🚀 빌드 및 실행

### STM32CubeIDE 사용

1. **새 프로젝트 생성**
   - File → New → STM32 Project
   - Board Selector에서 NUCLEO-F103RB 선택

2. **핀 설정 (CubeMX)**
   - PA0: GPIO_Input (Pull-up)
   - PA1: ADC1_IN1
   - PA5: GPIO_Output (LED)
   - USART2: Asynchronous, 115200 baud
   - ADC1: Enable, Single conversion mode

3. **코드 추가**
   - `main.c` 내용을 생성된 프로젝트에 적용

4. **빌드 및 다운로드**
   - Project → Build All
   - Run → Debug

### 시리얼 모니터 설정
- Baud Rate: 115200
- Data Bits: 8
- Stop Bits: 1
- Parity: None

## 📊 출력 예시

```
========================================
  Shock/Vibration Sensor Module Test
  NUCLEO-F103RB
========================================
Tap or shake the sensor to detect impacts.
Intensity levels: Light, Medium, Strong, Very Strong, Extreme

[READY] Waiting for shock events...

[SHOCK #1] Intensity: Light (ADC: 245)
         |█|

[SHOCK #2] Intensity: Medium (ADC: 756)
         |███|

[SHOCK #3] Intensity: Strong (ADC: 1523)
         |███████|

[SHOCK #4] Intensity: Very Strong (ADC: 2834)
         |██████████████|

─────────── Statistics ───────────
  Total shocks: 4
  Max intensity: 2834 (ADC)
  Avg intensity: 1339 (last 4 events)
──────────────────────────────────
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결 방법 |
|------|------|----------|
| 감지가 안됨 | 센서 방향 | 센서를 수직으로 설치 |
| 너무 민감함 | 임계값 낮음 | SHOCK_THRESHOLD 값 증가 |
| 연속 트리거 | 쿨다운 짧음 | COOLDOWN_MS 값 증가 |
| ADC 값이 0 | 배선 오류 | AO 핀 연결 확인 |
| 불규칙 감지 | 노이즈 | 전원 안정화, 디커플링 캐패시터 추가 |

### 민감도 조절

1. **하드웨어**: 모듈의 가변저항(있는 경우) 조절
2. **소프트웨어**: SHOCK_THRESHOLD 값 변경

```c
/* 고감도 설정 (작은 진동도 감지) */
#define SHOCK_THRESHOLD     50

/* 저감도 설정 (강한 충격만 감지) */
#define SHOCK_THRESHOLD     500
```

## 🔌 응용 분야

| 분야 | 적용 |
|------|------|
| 보안 | 침입/충격 감지 알람 |
| 물류 | 운송 중 충격 모니터링 |
| 스포츠 | 충격 측정 장비 |
| IoT | 스마트 센싱 노드 |
| 게임 | 터치/탭 인터페이스 |

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [SW-18010P Shock Sensor Datasheet](https://components101.com/sensors/sw-18010p-vibration-sensor)

## 📝 라이선스

MIT License

## ✍️ 작성자

Embedded Systems Lab - 2025
