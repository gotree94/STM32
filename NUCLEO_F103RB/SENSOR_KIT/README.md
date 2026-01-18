# STM32F103 LED Modules Test Collection

NUCLEO-F103RB 보드를 이용한 다양한 LED 모듈 테스트 프로젝트 모음입니다.

## 📋 프로젝트 목록

| No. | 모듈명 | 설명 | 핀 연결 |
|-----|--------|------|---------|
| 01 | [RGB LED](./01_RGB_LED/) | PWM 색상 혼합, 레인보우 효과 | PA0, PA1, PA2 |
| 02 | [SMD LED](./02_SMD_LED/) | GPIO/PWM 제어, SOS 신호 | PA5 |
| 03 | [Dual Color LED](./03_Dual_Color_LED/) | 2색(R/G) 신호등 시뮬레이션 | PA6, PA7 |
| 04 | [Mini Dual Color LED](./04_Mini_Dual_Color_LED/) | 소형 2색 상태 표시 | PB0, PB1 |
| 05 | [7-Color LED](./05_Seven_Color_LED/) | 자동 색상 순환 | PC8 |

## 🔧 하드웨어 요구사항

### 보드
- **NUCLEO-F103RB** (STM32F103RB 탑재)
- ST-Link 내장 (USB로 직접 프로그래밍)

### 공통 연결
```
NUCLEO Board
┌─────────────────────────────────────┐
│                                     │
│  PA0 ─── RGB LED (Red)              │
│  PA1 ─── RGB LED (Green)            │
│  PA2 ─── RGB LED (Blue) / USART2_TX │
│  PA3 ─── USART2_RX                  │
│  PA5 ─── SMD LED                    │
│  PA6 ─── Dual Color (Red)           │
│  PA7 ─── Dual Color (Green)         │
│  PB0 ─── Mini Dual Color (Red)      │
│  PB1 ─── Mini Dual Color (Green)    │
│  PC8 ─── 7-Color LED                │
│                                     │
│  3.3V ─── VCC (각 모듈)             │
│  GND  ─── GND (각 모듈)             │
└─────────────────────────────────────┘
```

## 📁 프로젝트 구조

```
led_modules/
├── README.md                    # 이 파일
├── 01_RGB_LED/
│   ├── main.c
│   └── README.md
├── 02_SMD_LED/
│   ├── main.c
│   └── README.md
├── 03_Dual_Color_LED/
│   ├── main.c
│   └── README.md
├── 04_Mini_Dual_Color_LED/
│   ├── main.c
│   └── README.md
└── 05_Seven_Color_LED/
    ├── main.c
    └── README.md
```

## 🚀 빌드 및 실행 방법

### STM32CubeIDE 사용

1. **새 프로젝트 생성**
   - File → New → STM32 Project
   - Board Selector에서 "NUCLEO-F103RB" 선택
   - 프로젝트 이름 입력 후 생성

2. **소스코드 복사**
   - 원하는 모듈의 `main.c` 내용을 프로젝트의 `Core/Src/main.c`에 복사
   - 기존 자동 생성된 코드는 삭제 또는 주석 처리

3. **빌드 및 플래시**
   - Project → Build Project (Ctrl+B)
   - Run → Debug 또는 Run

### 시리얼 모니터
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None

## 📊 모듈별 기능 비교

| 기능 | RGB LED | SMD LED | Dual Color | Mini Dual | 7-Color |
|------|---------|---------|------------|-----------|---------|
| 색상 수 | 무한대 | 1 | 3 | 3 | 7 (자동) |
| PWM 제어 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 색상 선택 | ✅ | ❌ | ✅ | ✅ | ❌ |
| 용도 | 분위기등 | 상태 표시 | 신호등 | 상태 표시 | 장식등 |

## 💡 LED 모듈 선택 가이드

| 용도 | 추천 모듈 | 이유 |
|------|----------|------|
| 무드등/분위기 조명 | RGB LED, 7-Color | 다양한 색상 표현 |
| 시스템 상태 표시 | Dual Color, Mini Dual | Red/Green으로 직관적 표시 |
| 단순 표시등 | SMD LED | 간단한 ON/OFF |
| 신호등 구현 | Dual Color | R/G/Y 3색 표현 |
| 파티/이벤트 | 7-Color, RGB LED | 화려한 효과 |

## 🛠 개발 환경

- **IDE**: STM32CubeIDE 1.x
- **HAL Library**: STM32F1xx HAL
- **컴파일러**: ARM GCC
- **Clock**: 72MHz (HSE + PLL)

## 📝 공통 코드 구조

모든 프로젝트는 동일한 구조를 따릅니다:

```c
// 1. 헤더 포함
#include "stm32f1xx_hal.h"
#include <stdio.h>

// 2. 핸들러 선언
TIM_HandleTypeDef htimX;
UART_HandleTypeDef huart2;

// 3. 초기화 함수
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIMx_Init(void);
static void MX_USART2_UART_Init(void);

// 4. LED 제어 함수
void LED_On(void);
void LED_Off(void);
void LED_SetBrightness(uint8_t brightness);

// 5. 메인 루프
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIMx_Init();
    MX_USART2_UART_Init();
    
    while (1) {
        // 테스트 코드
    }
}
```

## ⚠️ 주의사항

1. **전류 제한**: LED에 직접 연결 시 적절한 저항 사용 (모듈에는 대부분 내장)
2. **전압 레벨**: STM32F103은 3.3V 로직 사용
3. **공통 타입 확인**: 공통 캐소드/애노드에 따라 연결 방법 다름
4. **핀 충돌 주의**: PA2는 USART2_TX와 TIM2_CH3 동시 사용 불가

## 📚 참고 자료

- [STM32F103 Reference Manual (RM0008)](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [STM32CubeF1 HAL Documentation](https://www.st.com/resource/en/user_manual/um1850-description-of-stm32f1-hal-and-lowlayer-drivers-stmicroelectronics.pdf)

## 📜 라이선스

MIT License

Copyright (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.
