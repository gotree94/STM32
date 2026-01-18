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

---

# STM32F103 NUCLEO 센서 테스트 프로젝트

STM32F103 NUCLEO 보드를 이용한 다양한 센서 모듈 테스트 예제 모음입니다.

## 프로젝트 구조

```
stm32_sensors/
├── README.md                    # 이 파일
├── 01_Relay/                    # 릴레이 모듈
│   ├── main.c
│   └── README.md
├── 02_Sound_Sensor_High/        # 고감도 소리센서 모듈
│   ├── main.c
│   └── README.md
├── 03_Sound_Sensor_Small/       # 소형 소리센서 모듈
│   ├── main.c
│   └── README.md
├── 04_Tracking_Module/          # 라인 트래킹 모듈
│   ├── main.c
│   └── README.md
└── 05_Obstacle_Sensor/          # 장애물 감지센서 모듈
    ├── main.c
    └── README.md
```

## 센서 모듈 요약

| # | 센서 | 출력 타입 | 주요 핀 | 용도 |
|---|------|----------|---------|------|
| 06 | 릴레이 | Digital Out | PA5 | AC/DC 부하 제어 |
| 07 | 고감도 소리센서 | Analog + Digital | PA0(AO), PA1(DO) | 소리 크기 측정 |
| 08 | 소형 소리센서 | Digital | PA0 (EXTI) | 소리 감지 |
| 09 | 트래킹 모듈 | Digital x3 | PA0, PA1, PA4 | 라인 추적 |
| 10 | 장애물 감지센서 | Digital x2 | PA0, PA1 | 장애물 감지 |

## 공통 하드웨어 요구사항

### 필수 장비
- STM32F103 NUCLEO 보드 (NUCLEO-F103RB)
- USB 케이블 (Mini-B 또는 보드에 맞는 타입)
- 점퍼 와이어 (M-F 타입)
- 브레드보드 (옵션)

### 개발 환경
- STM32CubeIDE (권장)
- STM32CubeMX (핀 설정용)
- 시리얼 터미널 (PuTTY, Tera Term 등)

## 공통 핀 배치

```
NUCLEO-F103RB 핀 맵 (주요 사용 핀)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

        CN7                          CN10
    ┌─────────┐                  ┌─────────┐
    │ PC10  1 │                  │ D10   1 │
    │ PC12  2 │                  │ D2    2 │
    │  VDD  3 │                  │ D3    3 │
    │ BOOT0 4 │                  │ D4    4 │
    │   NC  5 │                  │ D5    5 │
    │   NC  6 │                  │ D6    6 │
    │ PA13  7 │                  │ D7    7 │
    │ PA14  8 │                  │ D8    8 │
    │ PA15  9 │                  │ D9    9 │
    │  GND 10 │                  │ D10  10 │
    │ PB7  11 │                  │ PA5* 11 │ ← LED / 릴레이
    │ PC13 12 │ ← User Button   │ PA6  12 │
    │ PC14 13 │                  │ PA7  13 │
    │ PC15 14 │                  │ PB6  14 │
    │ PH0  15 │                  │ PC7  15 │
    │ PH1  16 │                  │ PA9  16 │
    │ VBAT 17 │                  │ PA8  17 │
    │ PC2  18 │                  │ PB10 18 │
    │ PC3  19 │                  │ PB4  19 │
    │  ... .. │                  │ PB5  20 │
    └─────────┘                  └─────────┘

        CN8                          CN9
    ┌─────────┐                  ┌─────────┐
    │ PA0* 28 │ ← ADC/센서1      │ PA1* 30 │ ← 센서2
    │ PA4* 32 │ ← 센서3          │ PB0  34 │
    │  ... .. │                  │  ... .. │
    └─────────┘                  └─────────┘

* 주로 사용하는 핀
```

## UART 설정 (공통)

모든 프로젝트는 USART2를 통해 디버그 메시지를 출력합니다.

| 설정 | 값 |
|------|-----|
| Baud Rate | 115200 |
| Data Bits | 8 |
| Stop Bits | 1 |
| Parity | None |
| Flow Control | None |

NUCLEO 보드는 ST-LINK의 Virtual COM Port를 통해 PC와 통신합니다.

## 빠른 시작 가이드

### 1. 프로젝트 생성

```
STM32CubeIDE:
File → New → STM32 Project
Board Selector → NUCLEO-F103RB → Next
프로젝트 이름 입력 → Finish
```

### 2. 핀 설정 (CubeMX)

각 센서별 README.md의 CubeMX 설정 참조

### 3. 코드 복사

각 폴더의 `main.c` 내용을 프로젝트의 `main.c`에 복사

### 4. 빌드 및 업로드

```
Project → Build All (Ctrl+B)
Run → Debug (F11) 또는 Run (Ctrl+F11)
```

### 5. 시리얼 모니터 연결

```
PuTTY / Tera Term 설정:
- Port: COMx (장치관리자에서 확인)
- Speed: 115200
```

## 센서별 Quick Reference

### 01. 릴레이 모듈
```c
// ON/OFF 제어
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);   // ON
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // OFF
```

### 02. 고감도 소리센서
```c
// ADC 읽기
uint16_t sound_level = HAL_ADC_GetValue(&hadc1);  // 0-4095

// 디지털 출력 읽기
uint8_t detected = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);  // 0=감지
```

### 03. 소형 소리센서
```c
// 인터럽트 콜백
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        // 소리 감지됨!
    }
}
```

### 04. 트래킹 모듈
```c
// 3채널 읽기
uint8_t left   = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);  // 0=라인
uint8_t center = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);
uint8_t right  = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);
```

### 05. 장애물 감지센서
```c
// 장애물 감지 확인
uint8_t obstacle = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);  // 0=장애물
```

## 트러블슈팅 공통 사항

| 증상 | 원인 | 해결방법 |
|------|------|----------|
| 시리얼 출력 없음 | UART 미초기화 | CubeMX에서 USART2 활성화 |
| 센서 무반응 | 전원 문제 | VCC/GND 연결 확인 |
| 불안정한 값 | 노이즈 | 디커플링 캐패시터 추가 |
| 업로드 실패 | ST-LINK 드라이버 | ST-LINK 드라이버 재설치 |

## 확장 아이디어

1. **스마트 홈**: 릴레이 + 소리센서 → 박수로 조명 제어
2. **라인트레이서**: 트래킹 모듈 + 모터 드라이버
3. **장애물 회피 로봇**: 장애물 센서 + 서보모터
4. **소음 모니터**: 소리센서 + OLED 디스플레이

## 참고 자료

- [STM32F103 Reference Manual (RM0008)](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [STM32CubeIDE User Guide](https://www.st.com/resource/en/user_manual/um2609-stm32cubeide-user-guide-stmicroelectronics.pdf)

copies of the Software.
