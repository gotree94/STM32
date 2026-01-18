# Dual Color LED Module Test - NUCLEO-F103RB

2색(Red/Green) LED 모듈을 STM32F103 NUCLEO 보드에서 PWM으로 제어하는 프로젝트입니다.

## 📌 개요

2색 LED 모듈은 하나의 패키지에 Red와 Green LED가 함께 들어있어 3가지 색상(Red, Green, Yellow/Orange)을 표현할 수 있습니다. 신호등, 상태 표시등, 배터리 레벨 인디케이터 등에 활용됩니다.

## 🛠 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 비고 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 탑재 |
| 2색 LED 모듈 | 1 | KY-011 또는 5mm 2색 LED |
| 점퍼 와이어 | 3 | Female-Female |

### 핀 연결

```
Dual Color LED          NUCLEO-F103RB
┌─────────────┐        ┌─────────────┐
│     R  ─────┼────────┤ PA6 (TIM3_CH1)
│     G  ─────┼────────┤ PA7 (TIM3_CH2)
│   GND  ─────┼────────┤ GND
└─────────────┘        └─────────────┘
```

### 회로도

```
                    ┌─────────────────┐
                    │   Dual Color    │
                    │      LED        │
                    │   ┌───┬───┐     │
         PA6 ───────┤ R │ R │ G │ G ──┼──── PA7
                    │   └───┴───┘     │
                    │       │         │
                    └───────┼─────────┘
                           GND
                            │
                           ───
```

## 💻 소프트웨어

### 표현 가능한 색상

| 색상 | Red | Green | 용도 |
|------|-----|-------|------|
| OFF | 0 | 0 | 꺼짐 |
| RED | 255 | 0 | 정지, 위험, 오류 |
| GREEN | 0 | 255 | 진행, 정상, 완료 |
| YELLOW | 255 | 255 | 주의, 대기 |
| ORANGE | 255 | 80 | 경고, 저전력 |

### 색상 열거형

```c
typedef enum {
    COLOR_OFF = 0,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,    // Red + Green 동일 밝기
    COLOR_ORANGE     // Red 많이 + Green 조금
} LED_Color_t;
```

### 주요 함수

```c
// 사전 정의 색상 설정
void DualLED_SetColor(LED_Color_t color);

// Red/Green 개별 밝기 설정 (0~255)
void DualLED_SetRGB(uint8_t red, uint8_t green);

// 데모 함수들
void DualLED_TrafficLight(void);     // 신호등 시뮬레이션
void DualLED_StatusIndicator(void);  // 상태 표시기
void DualLED_ColorMix(void);         // 색상 그라데이션
void DualLED_Alternating(void);      // 교대 점멸
```

### PWM 설정

```c
Timer: TIM3
Prescaler: 71 (72MHz / 72 = 1MHz)
Period: 999 (1MHz / 1000 = 1kHz PWM)
Channels: CH1(PA6)=Red, CH2(PA7)=Green
```

## 📂 프로젝트 구조

```
03_Dual_Color_LED/
├── main.c          # 메인 소스 코드
└── README.md       # 프로젝트 설명서
```

## 🔧 빌드 및 실행

### STM32CubeIDE 사용 시
1. 새 STM32 프로젝트 생성 (NUCLEO-F103RB 선택)
2. `main.c` 내용을 프로젝트에 복사
3. 빌드 후 보드에 플래시

## 📊 시리얼 출력 예시

```
============================================
  Dual Color LED Module Test - NUCLEO-F103RB
============================================

[Test 1] Basic Colors
  OFF...
  RED...
  GREEN...
  YELLOW (R+G)...
  ORANGE (R+g)...

[Test 2] Traffic Light Simulation
  GREEN (Go) - 3 sec
  YELLOW (Caution) - 1 sec
  RED (Stop) - 3 sec

[Test 3] Status Indicator
  Simulating battery level:
    100% - Green
    75% - Green (dim)
    50% - Yellow
    25% - Orange
    10% - Red (blinking)
    0% - Red (fast blink)

[Test 4] Color Gradient (Red -> Yellow -> Green)
  Red -> Yellow...
  Yellow -> Green...
  Green -> Yellow...
  Yellow -> Red...

[Test 5] Alternating Blink
  Slow alternating...
  Fast alternating...
  Cross-fade...

--- Cycle Complete ---
```

## 📝 데모 패턴 상세

### 신호등 시뮬레이션
```
GREEN  (3초) → YELLOW (1초) → RED (3초) → 반복
```

### 배터리 레벨 표시
```
100% → Green (밝게)
 75% → Green (어둡게)
 50% → Yellow
 25% → Orange
 10% → Red (느린 점멸)
  0% → Red (빠른 점멸)
```

### 색상 그라데이션
```
Red(255,0) → Yellow(255,255) → Green(0,255) → Yellow → Red
PWM을 이용해 부드럽게 전환
```

## 🔍 트러블슈팅

| 증상 | 원인 | 해결 방법 |
|------|------|----------|
| 한 색상만 동작 | 배선 오류 | 핀 연결 확인 |
| Yellow가 안 나옴 | PWM 불균형 | 각 채널 밝기 조정 |
| 색상이 반대 | 핀 매핑 오류 | PA6/PA7 확인 |
| 깜빡임 현상 | PWM 주파수 낮음 | Period 값 감소 |

## 💡 응용 예제

### 시스템 상태 표시기
```c
void ShowSystemStatus(uint8_t status) {
    switch (status) {
        case 0: DualLED_SetColor(COLOR_GREEN);  break; // 정상
        case 1: DualLED_SetColor(COLOR_YELLOW); break; // 경고
        case 2: DualLED_SetColor(COLOR_ORANGE); break; // 주의
        case 3: DualLED_SetColor(COLOR_RED);    break; // 오류
    }
}
```

### 프로그레스 표시
```c
void ShowProgress(uint8_t percent) {
    // 0% = Red, 50% = Yellow, 100% = Green
    uint8_t red = (percent < 50) ? 255 : 255 - ((percent - 50) * 255 / 50);
    uint8_t green = (percent > 50) ? 255 : (percent * 255 / 50);
    DualLED_SetRGB(red, green);
}
```

### 온도 표시
```c
void ShowTemperature(int temp) {
    if (temp < 20) {
        DualLED_SetColor(COLOR_GREEN);       // 적정
    } else if (temp < 30) {
        DualLED_SetColor(COLOR_YELLOW);      // 주의
    } else if (temp < 40) {
        DualLED_SetColor(COLOR_ORANGE);      // 경고
    } else {
        DualLED_SetColor(COLOR_RED);         // 위험
    }
}
```

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [Dual Color LED Basics](https://www.electronics-tutorials.ws/diode/diode_8.html)

## 📜 라이선스

MIT License
