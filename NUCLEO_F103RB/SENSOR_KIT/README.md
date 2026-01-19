# STM32F103 LED Modules Test Collection

NUCLEO-F103RB 보드를 이용한 다양한 LED 모듈 테스트 프로젝트 모음입니다.

<img width="797" height="515" alt="016" src="https://github.com/user-attachments/assets/9171c78e-659c-459f-9610-1c25bbe0b4fc" />

* [X] 01_RGB_LED
* [X] 02_SMD_LED
* [X] 03_Dual_Color_LED
* [X] 04_Mini_Dual_Color_LED
* [ ] 05_Seven_Color_LED
* [X] 06_Relay
* [X] 07_Sound_Sensor_High
* [X] 08_Sound_Sensor_Small
* [X] 09_Tracking_Module
* [X] 10_Obstacle_Sensor
* [ ] 11_Flame_Sensor
* [ ] 12_Linear_Hall_Sensor
* [ ] 13_Touch_Sensor
* [ ] 14_Digital_Temperature_Sensor
* [ ] 15_Reed_Switch
* [ ] 16.mini_reed
* [ ] 17.heartbeat
* [X] 18.laser
* [X] 19.button_switch
* [ ] 20.shock_sensor
* [ ] 21.magic_light
* [ ] 22.angle_switch
* [ ] 23.ball_switch
* [X] 24.light_sensor
* [ ] 25.analog_hall
* [ ] 26.Hall_Magnetic
* [ ] 27.Temperature_Sensor
* [ ] 28.Analog_Light_Sensor
* [ ] 29.Knock_Sensor
* [ ] 30.Photo_Interrupter
* [ ] 31.IR_Transmitter
* [ ] 32.IR_Receiver
* [ ] 33.IR_Remote_Decoder



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

---
# STM32F103 센서 모듈 테스트 프로젝트

NUCLEO-F103RB 보드를 이용한 다양한 센서 모듈 테스트 프로젝트 모음

## 📋 프로젝트 개요

이 저장소는 STM32F103 마이크로컨트롤러를 사용하여 다양한 센서 모듈을 테스트하는 예제 코드를 포함합니다. 각 센서별로 독립적인 프로젝트로 구성되어 있으며, HAL 라이브러리를 기반으로 작성되었습니다.

## 🎯 대상 보드

- **MCU**: STM32F103RB
- **보드**: NUCLEO-F103RB
- **클럭**: 72MHz (HSE + PLL)
- **디버그 출력**: UART2 (115200bps, ST-Link Virtual COM)

## 📁 프로젝트 구조

```
stm32f103_sensors/
├── README.md                           # 이 파일
├── 01_Flame_Sensor/                    # 불꽃 감지 센서
│   ├── main.c
│   └── README.md
├── 02_Linear_Hall_Sensor/              # 리니어 홀 센서
│   ├── main.c
│   └── README.md
├── 03_Touch_Sensor/                    # 터치 센서
│   ├── main.c
│   └── README.md
├── 04_Digital_Temperature_Sensor/      # 디지털 온도 센서 (DS18B20)
│   ├── main.c
│   └── README.md
└── 05_Reed_Switch/                     # 리드 스위치
    ├── main.c
    └── README.md
```

## 📊 센서 모듈 요약

| # | 센서 | 인터페이스 | 주요 기능 | 응용 분야 |
|---|------|-----------|----------|----------|
| 11 | 불꽃 감지 | Digital + ADC | 화염 감지, 강도 측정 | 화재 경보, 안전 시스템 |
| 12 | 리니어 홀 | Digital + ADC | 자기장 세기/극성 측정 | 위치 감지, 회전 측정 |
| 13 | 터치 | Digital | 정전식 터치, 제스처 인식 | UI 입력, 버튼 대체 |
| 14 | DS18B20 | 1-Wire | 디지털 온도 측정 | 온도 모니터링, 환경 제어 |
| 15 | 리드 스위치 | Digital | 자기장 ON/OFF 감지 | 문/창문 센서, 보안 |

## 🔌 공통 핀 배치

모든 프로젝트는 아래의 공통 핀 배치를 사용합니다:

```
센서 모듈              NUCLEO-F103RB
┌─────────────┐       ┌─────────────────┐
│     VCC     │──────▶│      3.3V       │
│     GND     │──────▶│      GND        │
│     DO      │──────▶│      PA0        │ (Digital Input)
│     AO      │──────▶│      PA1        │ (ADC1_IN1)
└─────────────┘       └─────────────────┘

UART2 (디버그 출력):
  - TX: PA2
  - RX: PA3
  - Baudrate: 115200

온보드 LED: PA5 (상태 표시용)
```

## 🛠️ 개발 환경

### 필수 도구
- **IDE**: STM32CubeIDE 1.x 이상
- **HAL 라이브러리**: STM32Cube_FW_F1
- **시리얼 터미널**: PuTTY, Tera Term, 또는 Arduino Serial Monitor

### 빌드 방법

1. STM32CubeIDE에서 새 프로젝트 생성
   - Board Selector에서 "NUCLEO-F103RB" 선택
   - Project Name 입력 후 생성

2. 생성된 `main.c` 파일을 원하는 센서의 `main.c`로 교체

3. 빌드 및 다운로드
   ```
   Project → Build All (Ctrl+B)
   Run → Debug (F11) 또는 Run (Ctrl+F11)
   ```

4. 시리얼 터미널로 결과 확인
   - COM 포트: ST-Link Virtual COM Port
   - Baudrate: 115200
   - Data: 8-N-1

## 📝 사용 방법

### 1. 하드웨어 연결
```
1. 센서 모듈의 VCC를 NUCLEO 보드의 3.3V에 연결
2. 센서 모듈의 GND를 NUCLEO 보드의 GND에 연결
3. 센서 모듈의 DO를 PA0에 연결
4. (옵션) 센서 모듈의 AO를 PA1에 연결
```

### 2. 소프트웨어 설정
```
1. 해당 센서의 main.c 파일을 프로젝트에 복사
2. 빌드 및 다운로드
3. 시리얼 터미널로 출력 확인
```

### 3. 테스트
```
1. 센서에 적절한 자극 제공 (불꽃, 자석, 터치 등)
2. 시리얼 출력에서 센서 반응 확인
3. 온보드 LED 상태 확인
```

## 🔧 핀 변경 방법

기본 핀(PA0, PA1)을 변경하려면:

```c
// 1. GPIO 핀 정의 수정
#define SENSOR_PORT     GPIOB      // 원하는 포트
#define SENSOR_PIN      GPIO_PIN_5  // 원하는 핀

// 2. GPIO 초기화에서 클럭 활성화 수정
__HAL_RCC_GPIOB_CLK_ENABLE();      // 해당 포트 클럭

// 3. ADC 채널 수정 (아날로그 입력 사용 시)
sConfig.Channel = ADC_CHANNEL_X;   // 해당 ADC 채널
```

## 🎓 학습 목표

이 프로젝트를 통해 다음을 학습할 수 있습니다:

1. **GPIO 입력 처리**: 디지털 센서 인터페이싱
2. **ADC 사용**: 아날로그 센서 값 읽기
3. **디바운싱**: 스위치/버튼 노이즈 제거
4. **1-Wire 프로토콜**: DS18B20 통신
5. **UART 통신**: 디버그 출력
6. **타이머 활용**: 마이크로초 딜레이, 시간 측정
7. **상태 머신**: 센서 상태 추적

## 📚 참고 자료

### STM32 문서
- [STM32F103 Reference Manual (RM0008)](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [STM32F103 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)

### 센서 데이터시트
- 각 센서별 README.md 파일 참조

## ⚠️ 주의사항

1. **전압 레벨**: 모든 센서는 3.3V로 구동 (5V 사용 시 MCU 손상 가능)
2. **전류 제한**: GPIO 핀당 최대 25mA
3. **정전기 주의**: ESD에 민감한 부품 취급 주의
4. **단락 방지**: 연결 전 배선 확인

---

# STM32F103 Sensor Module Test Projects

## 📌 개요

NUCLEO-F103RB 보드를 이용한 다양한 센서 모듈 테스트 프로젝트 모음입니다. 각 센서별로 독립적인 테스트 코드와 상세한 문서를 제공합니다.

## 🎯 프로젝트 목록

| No | 센서 | 디렉토리 | 설명 |
|----|------|----------|------|
| 16 | 미니 리드 모듈 | `mini_reed/` | 자석 감지 (문/창문 열림 감지) |
| 17 | 심박 센서 모듈 | `heartbeat/` | 심박수 측정 (BPM 계산) |
| 18 | 레이저 모듈 | `laser/` | 레이저 제어 (PWM, SOS 등) |
| 19 | 버튼 스위치 모듈 | `button_switch/` | 다양한 버튼 이벤트 감지 |
| 20 | 충격 센서 모듈 | `shock_sensor/` | 충격/진동 감지 |

## 📂 프로젝트 구조

```
stm32_sensors/
├── README.md                 # 이 파일
├── mini_reed/
│   ├── main.c               # 미니 리드 모듈 소스
│   └── README.md
├── heartbeat/
│   ├── main.c               # 심박 센서 모듈 소스
│   └── README.md
├── laser/
│   ├── main.c               # 레이저 모듈 소스
│   └── README.md
├── button_switch/
│   ├── main.c               # 버튼 스위치 모듈 소스
│   └── README.md
└── shock_sensor/
    ├── main.c               # 충격 센서 모듈 소스
    └── README.md
```

## 🔧 공통 하드웨어

### 개발 보드
- **NUCLEO-F103RB** (STM32F103RBT6)
- 72MHz Cortex-M3
- 128KB Flash, 20KB SRAM

### 공통 핀 배치

| 기능 | 핀 | 설명 |
|------|-----|------|
| User LED | PA5 | 내장 LED |
| User Button | PC13 | 내장 버튼 |
| UART TX | PA2 | USART2 TX (ST-Link VCP) |
| UART RX | PA3 | USART2 RX (ST-Link VCP) |

### 센서별 핀 배치 요약

| 센서 | 신호 핀 | 추가 핀 |
|------|---------|---------|
| Mini Reed | PA0 (Digital) | - |
| Heartbeat | PA0 (ADC) | - |
| Laser | PA0 (PWM) | - |
| Button Switch | PA0 (Digital) | - |
| Shock Sensor | PA0 (Digital) | PA1 (ADC) |

## 🚀 빠른 시작

### 1. 개발 환경 설정

1. **STM32CubeIDE 설치**
   - [STM32CubeIDE 다운로드](https://www.st.com/en/development-tools/stm32cubeide.html)

2. **새 프로젝트 생성**
   ```
   File → New → STM32 Project
   Board Selector → NUCLEO-F103RB
   ```

3. **CubeMX 설정**
   - 각 센서 README.md의 핀 설정 참조

### 2. 코드 적용

1. 원하는 센서 디렉토리의 `main.c` 열기
2. 생성된 프로젝트의 `main.c`에 코드 복사
3. 빌드 및 다운로드

### 3. 시리얼 모니터 연결

- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None

## 📊 센서별 기능 요약

### 1. 미니 리드 모듈 (Mini Reed)
```
기능: 자석 감지
출력: Digital (HIGH/LOW)
응용: 도어 센서, 위치 감지
특징: 폴링 방식, 디바운싱 처리
```

### 2. 심박 센서 모듈 (Heartbeat)
```
기능: 심박수 측정
출력: Analog (ADC)
응용: 건강 모니터링
특징: BPM 계산, 피크 감지 알고리즘
```

### 3. 레이저 모듈 (Laser)
```
기능: 레이저 제어
출력: PWM (밝기 조절)
응용: 포인터, 거리 측정
특징: 다중 모드 (ON/BLINK/PWM/SOS)
```

### 4. 버튼 스위치 모듈 (Button Switch)
```
기능: 버튼 이벤트 감지
출력: Digital
응용: UI 입력
특징: 짧은/긴 누름, 더블 클릭 감지
```

### 5. 충격 센서 모듈 (Shock Sensor)
```
기능: 충격/진동 감지
출력: Digital + Analog
응용: 보안, 물류
특징: 강도 측정, 통계 제공
```

## 🛠️ 공통 트러블슈팅

| 문제 | 원인 | 해결 방법 |
|------|------|----------|
| 컴파일 오류 | HAL 라이브러리 누락 | CubeMX에서 HAL 생성 확인 |
| 다운로드 실패 | ST-Link 연결 | USB 연결 및 드라이버 확인 |
| UART 출력 없음 | Baud rate 불일치 | 115200 설정 확인 |
| 센서 무반응 | 전원 문제 | 3.3V/5V 확인 |

## 📚 참고 자료

### STM32 문서
- [STM32F103 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

### HAL 라이브러리
- [STM32F1 HAL Description](https://www.st.com/resource/en/user_manual/um1850-description-of-stm32f1-hal-and-lowlayer-drivers-stmicroelectronics.pdf)



---

# STM32F103 센서 모듈 테스트 프로젝트

NUCLEO-F103RB 보드를 사용한 다양한 센서 모듈 테스트 코드 모음입니다.

## 📋 프로젝트 개요

이 저장소는 STM32F103 마이크로컨트롤러를 사용하여 다양한 센서 모듈을 테스트하고 학습하기 위한 예제 코드를 제공합니다. 각 센서별로 독립적인 프로젝트로 구성되어 있어 필요한 부분만 선택적으로 활용할 수 있습니다.

## 🔧 개발 환경

| 항목 | 사양 |
|------|------|
| 보드 | NUCLEO-F103RB |
| MCU | STM32F103RBT6 (Cortex-M3, 72MHz) |
| IDE | STM32CubeIDE 1.13+ |
| Library | STM32 HAL Driver |
| Debugger | ST-Link V2-1 (온보드) |

## 📁 프로젝트 구조

```
stm32_sensors/
├── README.md                    # 이 파일
├── 01_magic_light/              # 매직 라이트컴 모듈
│   ├── main.c
│   └── README.md
├── 02_angle_switch/             # 각도 스위치 모듈
│   ├── main.c
│   └── README.md
├── 03_ball_switch/              # 볼 스위치 모듈
│   ├── main.c
│   └── README.md
├── 04_light_sensor/             # 조도 센서 모듈
│   ├── main.c
│   └── README.md
└── 05_analog_hall/              # 아날로그 홀센서 모듈
    ├── main.c
    └── README.md
```

## 📚 센서 모듈 요약

| # | 센서 | 모델 | 출력 타입 | 인터페이스 | 주요 기능 |
|---|------|------|----------|-----------|----------|
| 21 | 매직 라이트컴 | KY-027 | 디지털 | GPIO + PWM | 기울기 감지 + LED 페이드 |
| 22 | 각도 스위치 | KY-020 | 디지털 | EXTI | 기울기 ON/OFF 감지 |
| 23 | 볼 스위치 | KY-002 | 디지털 | EXTI | 진동/충격 감지 |
| 24 | 조도 센서 | KY-018 | 아날로그 | ADC + DMA | 밝기 측정 |
| 25 | 아날로그 홀센서 | KY-035 | 아날로그 | ADC + DMA | 자기장 극성/강도 측정 |

## 🔌 공통 핀 배치

```
NUCLEO-F103RB 핀 할당
═══════════════════════════════════════

센서 입력:
  PA0 ─────► 센서 Signal (ADC/GPIO)

통신:
  PA2 ─────► USART2_TX (ST-Link VCP)
  PA3 ─────► USART2_RX (ST-Link VCP)

출력:
  PA5 ─────► 보드 내장 LED (LD2)
  PA6 ─────► PWM 출력 (TIM3_CH1)

전원:
  3.3V ────► 센서 VCC
  GND ─────► 센서 GND
```

## 🚀 빠른 시작

### 1. 저장소 클론

```bash
git clone https://github.com/your-username/stm32_sensors.git
cd stm32_sensors
```

### 2. STM32CubeIDE에서 프로젝트 생성

1. **File → New → STM32 Project** 선택
2. **Board Selector** 탭에서 `NUCLEO-F103RB` 선택
3. 프로젝트 이름 입력 후 **Finish**
4. 생성된 `main.c`에 원하는 센서 폴더의 `main.c` 내용 복사

### 3. 빌드 및 다운로드

1. **Project → Build All** (Ctrl+B)
2. **Run → Debug** (F11) 또는 **Run → Run** (Ctrl+F11)

### 4. 시리얼 모니터 연결

```bash
# Linux/Mac
screen /dev/ttyACM0 115200

# Windows: PuTTY 또는 Tera Term 사용
# - COM 포트 확인 (장치 관리자)
# - 속도: 115200 bps
```

## 📊 센서별 특징

### 01. 매직 라이트컴 모듈 (KY-027)

수은 기울기 스위치와 LED가 결합된 모듈. 기울임에 따라 부드럽게 LED 밝기가 변화합니다.

```
특징: GPIO 입력 + PWM 출력
용도: 인터랙티브 조명, 레벨 표시기
```

### 02. 각도 스위치 모듈 (KY-020)

금속 볼 기반 기울기 스위치. 특정 각도 이상 기울면 ON 상태가 됩니다.

```
특징: 외부 인터럽트 + 디바운싱
용도: 자세 감지, 도난 방지
```

### 03. 볼 스위치 모듈 (KY-002)

진동/충격에 반응하는 순간 접촉 스위치. 짧은 펄스 신호를 생성합니다.

```
특징: 외부 인터럽트 + 강도 추정
용도: 노크 감지, 충격 모니터링
```

### 04. 조도 센서 모듈 (KY-018)

CdS 광저항(LDR)을 이용한 밝기 측정. 빛이 밝으면 저항이 낮아집니다.

```
특징: ADC + DMA + 평균 필터
용도: 자동 조명, 광량 모니터링
```

### 05. 아날로그 홀센서 모듈 (KY-035)

49E 선형 홀센서로 자기장의 세기와 극성을 측정합니다.

```
특징: ADC + DMA + 영점 보정 + 극성 감지
용도: 비접촉 위치 감지, RPM 측정
```

## 🛠️ 공통 기능 코드

### UART printf 리다이렉트

```c
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
```

### 시스템 클럭 설정 (72MHz)

```c
/* HSE 8MHz → PLL → 72MHz SYSCLK */
RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
```

### ADC DMA 설정

```c
/* ADC1 Channel 0 (PA0), 12-bit, DMA 연속 전송 */
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buffer, size);
```

## 📌 트러블슈팅

### 시리얼 출력이 안 보임

1. ST-Link 드라이버 설치 확인
2. COM 포트 번호 확인 (장치 관리자)
3. 보드 리셋 후 재시도
4. 다른 터미널 프로그램 시도

### ADC 값이 불안정함

1. 전원 노이즈 확인 (100nF 디커플링 커패시터 추가)
2. 샘플링 시간 증가 (`ADC_SAMPLETIME_239CYCLES_5`)
3. 평균 필터 샘플 수 증가

### 인터럽트가 동작하지 않음

1. NVIC 인터럽트 활성화 확인
2. AFIO 클럭 활성화 확인
3. 인터럽트 핸들러 함수명 확인

## 🤝 기여

이슈 리포트, 기능 제안, Pull Request를 환영합니다!

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingSensor`)
3. Commit your changes (`git commit -m 'Add some AmazingSensor'`)
4. Push to the branch (`git push origin feature/AmazingSensor`)
5. Open a Pull Request

---

# STM32F103 Sensor Module Test Collection

NUCLEO-F103RB 보드를 이용한 다양한 센서 모듈 테스트 프로젝트 모음

## 📌 프로젝트 개요

STM32F103 마이크로컨트롤러를 이용하여 다양한 센서 모듈을 테스트하는 예제 코드 모음입니다. 각 센서별로 독립적인 프로젝트로 구성되어 있으며, HAL 라이브러리 기반으로 작성되었습니다.

## 📂 프로젝트 구조

```
stm32_sensors/
├── README.md                    # 이 파일
├── 01_Hall_Magnetic/            # 홀 마그네틱 센서
│   ├── main.c
│   └── README.md
├── 02_Temperature_Sensor/       # 온도 센서 (NTC/LM35)
│   ├── main.c
│   └── README.md
├── 03_Analog_Light_Sensor/      # 아날로그 광센서 (LDR)
│   ├── main.c
│   └── README.md
├── 04_Knock_Sensor/             # 노크(진동) 센서
│   ├── main.c
│   └── README.md
└── 05_Photo_Interrupter/        # 포토 인터럽터
    ├── main.c
    └── README.md
```

## 🔧 공통 하드웨어

### 개발 보드
- **NUCLEO-F103RB**: STM32F103RBT6 탑재 (64KB Flash, 20KB RAM)
- 온보드 ST-Link/V2 디버거
- Virtual COM Port (USART2)

### 공통 핀 배치
| 기능 | 핀 | 설명 |
|------|-----|------|
| Sensor Input | PA0 | 센서 신호 입력 (ADC/GPIO) |
| LED | PA5 | 온보드 LED (LD2) |
| UART TX | PA2 | ST-Link Virtual COM |
| UART RX | PA3 | ST-Link Virtual COM |

## 📊 센서 모듈 요약

| 센서 | 입력 타입 | 주요 기능 |
|--------|----------|----------|
| [26.Hall_Magnetic](./26.Hall_Magnetic) | Digital | 자석 감지, 회전 감지 |
| [27.Temperature](./27.Temperature) | Analog (ADC) | 온도 측정, 알람 |
| [28.Light_Sensor](./28.Light_Sensor) | Analog (ADC) | 조도 측정, 자동 조명 |
| [29.Knock_Sensor](./29.Knock_Sensor) | Digital (EXTI) | 진동 감지, 패턴 인식 |
| [30.Photo_Interrupter](./30.Photo_Interrupter) | Digital (EXTI) | 물체 감지, RPM 측정 |

## 💻 개발 환경

### 필수 소프트웨어
- **STM32CubeIDE** 1.10.0 이상
- **STM32CubeMX** (선택, 프로젝트 설정용)
- **ST-Link Driver**

### 시리얼 터미널
- PuTTY, Tera Term, 또는 Arduino Serial Monitor
- 설정: 115200 baud, 8N1

## 🚀 빠른 시작

### 1. 프로젝트 생성

STM32CubeIDE에서 새 프로젝트를 생성합니다:
1. File → New → STM32 Project
2. Board Selector에서 "NUCLEO-F103RB" 선택
3. 프로젝트 이름 입력 후 Finish

### 2. 코드 복사

원하는 센서의 `main.c` 내용을 프로젝트의 `Core/Src/main.c`에 복사합니다.

### 3. 빌드 및 실행

```bash
1. Project → Build All (Ctrl+B)
2. Run → Run As → STM32 Application
3. 시리얼 터미널로 출력 확인
```

### 4. 하드웨어 연결

각 센서 폴더의 README.md에서 핀 연결 정보를 확인합니다.

## 📋 센서별 세부 정보

### 01. Hall Magnetic Sensor (홀 마그네틱)
- **모듈**: KY-003 또는 호환
- **원리**: 홀 효과를 이용한 자기장 감지
- **응용**: 도어 센서, RPM 측정, 위치 감지

### 02. Temperature Sensor (온도 센서)
- **모듈**: KY-013 (NTC) 또는 LM35
- **원리**: 서미스터 저항 변화 / 전압 출력
- **응용**: 환경 모니터링, 온도 알람

### 03. Analog Light Sensor (광센서)
- **모듈**: KY-018 (LDR/CdS)
- **원리**: 광저항의 저항 변화
- **응용**: 자동 조명, 주야간 감지

### 04. Knock Sensor (노크 센서)
- **모듈**: KY-031 (압전 소자)
- **원리**: 진동에 의한 전압 발생
- **응용**: 진동 감지, 비밀 노크 잠금

### 05. Photo Interrupter (포토 인터럽터)
- **모듈**: KY-010 (슬롯형)
- **원리**: 적외선 빛의 차단/통과 감지
- **응용**: 엔코더, 카운터, 속도 측정

## ⚙️ 공통 코드 설명

### 시스템 클럭 설정 (64MHz)
```c
void SystemClock_Config(void)
{
    // HSI 8MHz -> PLL x16 -> 64MHz SYSCLK
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    // APB1: 32MHz (최대), APB2: 64MHz
}
```

### UART printf 리다이렉션
```c
#ifdef __GNUC__
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif
```

### ADC 캘리브레이션
```c
HAL_ADCEx_Calibration_Start(&hadc1);  // 정확도 향상
```

## 🔌 커넥터 핀맵 (NUCLEO-F103RB)

### CN8 (Arduino Analog)
```
Pin  Function    Sensor Use
---  --------    ----------
1    PA0 (A0)    Sensor Input
2    PA1 (A1)    -
3    PA4 (A2)    -
4    PB0 (A3)    -
5    PC1 (A4)    -
6    PC0 (A5)    -
```

### CN9 (Arduino Digital)
```
Pin  Function    Use
---  --------    ---
1    PC7 (D9)    -
2    PA9 (D8)    -
...
```

## 📚 참고 자료

### 공식 문서
- [STM32F103 Reference Manual (RM0008)](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F103 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

### HAL 라이브러리
- [STM32F1 HAL Driver Documentation](https://www.st.com/resource/en/user_manual/um1850-description-of-stm32f1-hal-and-lowlayer-drivers-stmicroelectronics.pdf)


---

# STM32F103 IR Sensor Projects

NUCLEO-F103RB 보드를 이용한 IR(적외선) 센서 테스트 프로젝트 모음

## 프로젝트 목록

| 프로젝트 | 설명 | 난이도 |
|----------|------|--------|
| [31.IR_Transmitter](./31.IR_Transmitter) | IR LED를 이용한 38kHz 변조 신호 송신 | ⭐⭐ |
| [32.IR_Receiver](./32.IR_Receiver) | IR 수신 모듈 기본 테스트 (신호 감지) | ⭐ |
| [33.IR_Remote_Decoder](./33.IR_Remote_Decoder) | IR 리모컨 NEC 프로토콜 디코딩 | ⭐⭐⭐ |

## 하드웨어 요구사항

### 필수

- NUCLEO-F103RB 개발보드
- USB 케이블 (Mini-B)
- 브레드보드 및 점퍼 와이어

### IR 송신 모듈용

- IR LED (940nm 권장)
- 100Ω 저항
- (선택) 2N2222 트랜지스터 + 1kΩ 저항

### IR 수신 모듈용

- IR 수신 모듈 (TSOP1838, VS1838B 등)
- IR 리모컨 (NEC 프로토콜 지원)

## 전체 회로도

```
                    NUCLEO-F103RB
                   ┌─────────────┐
                   │             │
    [IR LED] ←──── │ PA8 (PWM)   │
                   │             │
    [IR RX]  ───→  │ PA0         │
                   │             │
    [IR RX]  ───→  │ PA6 (TIM3)  │ ← Input Capture 사용 시
                   │             │
    [On-board] ←── │ PA5 (LED)   │
                   │             │
    [Button]  ───→ │ PC13        │ ← 내장 버튼
                   │             │
    [USB VCP] ←──→ │ PA2/PA3     │ ← UART2
                   │             │
                   │     GND     │───── GND
                   │    3.3V     │───── VCC
                   └─────────────┘
```

## 개발 환경

### 소프트웨어

- STM32CubeIDE (권장)
- 또는 STM32CubeMX + Keil/IAR/GCC
- 시리얼 터미널 프로그램 (PuTTY, Tera Term 등)

### 드라이버

- STM32 HAL Library
- CMSIS

## 빠른 시작

### 1. 저장소 클론

```bash
git clone https://github.com/your-username/stm32-ir-projects.git
cd stm32-ir-projects
```

### 2. STM32CubeIDE에서 열기

1. File → Import → General → Existing Projects
2. 원하는 프로젝트 폴더 선택
3. Finish

### 3. 빌드 및 플래시

```
Build: Ctrl+B
Flash: Ctrl+F11
```

### 4. 시리얼 모니터

```
Baud: 115200 | Data: 8 | Stop: 1 | Parity: None
```

## NEC 프로토콜 요약

```
프레임: Leader(9ms) + Space(4.5ms) + 32-bit Data + Stop
비트 0: 560us pulse + 560us space
비트 1: 560us pulse + 1690us space
리피트: 9ms pulse + 2.25ms space + Stop
```
## 참고 자료

- [NEC Protocol](https://www.sbprojects.net/knowledge/ir/nec.php)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008.pdf)












