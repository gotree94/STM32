# 매직 라이트컴 모듈 (Magic Light Cup Module) - STM32F103

## 📋 개요

매직 라이트컴 모듈(KY-027)은 수은 기울기 스위치와 LED가 결합된 센서 모듈입니다. 기울임에 따라 수은이 이동하여 회로를 연결/차단하며, 이 신호를 이용해 LED를 제어합니다.

## 🔧 하드웨어 구성

### 센서 모듈 사양

| 항목 | 사양 |
|------|------|
| 동작 전압 | 3.3V ~ 5V |
| 출력 타입 | 디지털 (HIGH/LOW) |
| LED | 내장 (PWM 제어 가능) |
| 감지 방식 | 수은 볼 기울기 스위치 |

### 핀 연결

```
Magic Light Module          NUCLEO-F103RB
================          ===============
    S (Signal)  --------->  PA0 (GPIO Input)
    L (LED)     --------->  PA6 (TIM3_CH1 PWM)
    + (VCC)     --------->  3.3V
    - (GND)     --------->  GND
```

### 회로도

```
                    +3.3V
                      │
                      │
    ┌─────────────────┴───────────────────┐
    │         Magic Light Module          │
    │                                     │
    │  [Mercury Switch]    [LED]          │
    │       │                │            │
    └───────┼────────────────┼────────────┘
            │                │
            │                │
           PA0              PA6
         (Input)       (PWM Output)
```

## 💻 소프트웨어 구성

### 주요 기능

1. **기울기 감지**: PA0에서 수은 스위치 상태 읽기
2. **PWM LED 제어**: 기울임에 따른 LED 밝기 페이드 효과
3. **UART 디버그**: 상태 변화 시리얼 출력

### 핵심 코드 설명

```c
/* 기울기 센서 상태 읽기 */
tilt_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

/* 기울어짐: 밝기 증가 / 수평: 밝기 감소 */
if (tilt_state == GPIO_PIN_SET) {
    if (brightness < 1000) brightness += 10;
} else {
    if (brightness > 0) brightness -= 10;
}

/* PWM 듀티사이클 적용 */
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, brightness);
```

### PWM 설정

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| Timer | TIM3 | 범용 타이머 |
| Channel | CH1 | PA6 핀 |
| Prescaler | 71 | 72MHz / 72 = 1MHz |
| Period | 999 | 1MHz / 1000 = 1kHz |
| Resolution | 0-999 | 10bit 정밀도 |

## 📁 프로젝트 구조

```
01_magic_light/
├── main.c              # 메인 애플리케이션
├── README.md           # 프로젝트 문서
└── STM32CubeIDE/       # (선택) IDE 프로젝트 파일
```

## 🚀 빌드 및 실행

### STM32CubeIDE 사용

1. 새 STM32 프로젝트 생성 (NUCLEO-F103RB 선택)
2. `main.c` 내용을 생성된 `main.c`에 복사
3. 빌드 후 보드에 다운로드

### 터미널 출력 확인

```bash
# Windows (PuTTY, Tera Term 등)
# Linux/Mac
screen /dev/ttyACM0 115200
```

### 예상 출력

```
========================================
  Magic Light Cup Module Test
  Board: NUCLEO-F103RB
========================================

Tilt Detected! - LED Fading Up
Level Position - LED Fading Down
Tilt Detected! - LED Fading Up
...
```

## 🎯 동작 원리

```
                    [수평 상태]                    [기울인 상태]
                    
                    ┌─────────┐                   ┌─────────┐
                    │ ○○○○○○  │                   │      ○○○│
                    │         │   ─────────>      │○○○      │
                    │ Mercury │                   │ Mercury │
                    └────┬────┘                   └────┬────┘
                         │                             │
                      Switch                        Switch
                       OFF                            ON
                         │                             │
                    LED Dim ◐                    LED Bright ●
```

## 📌 응용 아이디어

1. **양방향 기울기 감지**: 두 개의 모듈을 반대로 배치하여 좌/우 기울임 감지
2. **레벨 게이지**: 수평계 구현
3. **인터랙티브 조명**: 움직임에 반응하는 무드등
4. **게임 컨트롤러**: 기울기 기반 입력 장치

## ⚠️ 주의사항

- 수은 스위치는 진동에 민감하여 디바운싱 처리 권장
- 수은은 유해물질이므로 모듈 파손 시 주의 필요
- 3.3V 전원 사용 시 LED 밝기가 다소 낮을 수 있음

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [KY-027 Module Datasheet](https://arduinomodules.info/ky-027-magic-light-cup-module/)

## 📝 라이선스

MIT License - 자유롭게 사용, 수정, 배포 가능
