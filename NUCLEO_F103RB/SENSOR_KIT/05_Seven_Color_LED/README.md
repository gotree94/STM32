# 7-Color LED Module Test - NUCLEO-F103RB

7색 자동 변환 LED 모듈을 STM32F103 NUCLEO 보드에서 제어하는 프로젝트입니다.

## 📌 개요

* 7색 LED 모듈(KY-034)은 내장된 IC가 자동으로 Red, Orange, Yellow, Green, Cyan, Blue, Purple 색상을 순환합니다. 
* 외부에서는 **GPIO ON/OFF 제어만 가능**하며, 색상 변화 속도와 패턴은 내장 IC가 자동으로 처리합니다.

### 모듈 특성

- **제어 방식**: GPIO ON/OFF (PWM 불필요)
- **색상 제어**: 불가능 (내장 IC 자동 순환)
- **밝기 제어**: 불가능 (내장 IC 고정)
- **패턴 제어**: 불가능 (내장 IC 고정)

## 🛠 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 비고 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 탑재 |
| 7색 LED 모듈 | 1 | KY-034 |
| 점퍼 와이어 | 2 | Female-Female |

### KY-034 핀 배치

```
┌─────────────────────────┐
│      KY-034 Module      │
│                         │
│  [Pin 1]  [Pin 2]  [Pin 3]
│   GND      N.C    Signal
└─────────────────────────┘
```

| 핀 번호 | 명칭 | 설명 |
|--------|------|------|
| 1 | GND | 그라운드 |
| 2 | N.C | 미사용 (No Connection) |
| 3 | Signal | 전원 입력 (VCC/GPIO) |

### 핀 연결

```
KY-034 Module           NUCLEO-F103RB
┌─────────────┐        ┌─────────────┐
│  GND (Pin1) ┼────────┤ GND         │
│  N.C (Pin2) ┼────────┤ (연결 안함)  │
│  S   (Pin3) ┼────────┤ PB0 (GPIO)  │
└─────────────┘        └─────────────┘
```

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/b1069af5-5147-49d5-99d3-212665834cd7" />




### 회로도

```
                    STM32F103RB
                   ┌───────────┐
                   │           │
    ┌──────────────┤ PB0       │
    │              │           │
    │              │           │
    ▼              │           │
┌───────────┐      │           │
│  KY-034   │      │           │
│ ┌───────┐ │      │           │
│ │Auto IC│ │      │           │
│ └───┬───┘ │      │           │
│ ┌───┴───┐ │      │           │
│ │RGB LED│ │      │           │
│ └───┬───┘ │      │           │
│     │     │      │           │
│    GND────┼──────┤ GND       │
└───────────┘      └───────────┘
```

## 💻 소프트웨어

### 제어 원리

KY-034는 Signal 핀에 전원(HIGH)이 공급되면 내장 IC가 자동으로 동작합니다:

1. **GPIO HIGH** → LED 동작 (자동 색상 변환 시작)
2. **GPIO LOW** → LED 정지

**PWM은 불필요합니다.** 단순 GPIO ON/OFF로 제어합니다.

### 색상 순환 (자동)

| 순서 | 색상 | 영문 |
|------|------|------|
| 1 | 빨강 | Red |
| 2 | 주황 | Orange |
| 3 | 노랑 | Yellow |
| 4 | 초록 | Green |
| 5 | 청록 | Cyan |
| 6 | 파랑 | Blue |
| 7 | 보라 | Purple |

### 주요 함수

```c
// 기본 제어 (GPIO 방식)
void SevenColorLED_On(void);    // LED 켜기 (자동 색상 변환 시작)
void SevenColorLED_Off(void);   // LED 끄기
void SevenColorLED_Toggle(void); // ON/OFF 토글

// 효과 데모 (ON/OFF 타이밍 제어)
void SevenColorLED_AutoCycleDemo(void);   // 자동 색상 순환 관찰
void SevenColorLED_StrobeDemo(void);      // 스트로브 효과 (ON/OFF 반복)
void SevenColorLED_PatternDemo(void);     // 다양한 점멸 패턴
void SevenColorLED_PartyMode(void);       // 파티 모드
void SevenColorLED_NotificationDemo(void); // 알림 패턴
```

### GPIO 설정

```c
GPIO Pin: PB0
Mode: GPIO_MODE_OUTPUT_PP
Speed: GPIO_SPEED_FREQ_LOW
```

### 코드 예시

```c
// GPIO 초기화
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

// LED 제어 함수
void SevenColorLED_On(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}

void SevenColorLED_Off(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
}

void SevenColorLED_Toggle(void)
{
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
}
```

## 📂 프로젝트 구조

```
05_Seven_Color_LED/
├── main.c          # 메인 소스 코드
└── README.md       # 프로젝트 설명서
```

## 🔧 빌드 및 실행

### STM32CubeIDE 사용 시
1. 새 STM32 프로젝트 생성 (NUCLEO-F103RB 선택)
2. PB0를 GPIO_Output으로 설정
3. `main.c` 내용을 프로젝트에 복사
4. 빌드 후 보드에 플래시

```c
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
#define LED_PIN         GPIO_PIN_0
#define LED_PORT        GPIOB
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
/* UART printf 리다이렉션 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN 0 */
/**
 * @brief LED ON (내장 IC 자동 색상 변환 시작)
 */
void SevenColorLED_On(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

/**
 * @brief LED OFF
 */
void SevenColorLED_Off(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

/**
 * @brief LED 토글
 */
void SevenColorLED_Toggle(void)
{
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}

/**
 * @brief 자동 색상 변환 관찰
 *        내장 IC가 자동으로 색상을 변환하므로 전원만 공급하면 됨
 */
void SevenColorLED_AutoCycleDemo(void)
{
    printf("  Observing auto color cycle for 10 seconds...\r\n");
    printf("  Colors: Red -> Orange -> Yellow -> Green -> Cyan -> Blue -> Purple\r\n");

    SevenColorLED_On();

    /* 색상 변화 관찰 */
    for (int i = 0; i < 10; i++) {
        printf("  [%d sec]\r\n", i + 1);
        HAL_Delay(1000);
    }

    SevenColorLED_Off();
}

/**
 * @brief 스트로브 효과 (ON/OFF 반복)
 */
void SevenColorLED_StrobeDemo(void)
{
    /* 느린 스트로브 */
    printf("  Slow strobe (500ms)...\r\n");
    for (int i = 0; i < 6; i++) {
        SevenColorLED_On();
        HAL_Delay(500);
        SevenColorLED_Off();
        HAL_Delay(500);
    }

    HAL_Delay(300);

    /* 빠른 스트로브 */
    printf("  Fast strobe (100ms)...\r\n");
    for (int i = 0; i < 20; i++) {
        SevenColorLED_On();
        HAL_Delay(100);
        SevenColorLED_Off();
        HAL_Delay(100);
    }

    HAL_Delay(300);

    /* 점점 빨라지는 스트로브 */
    printf("  Accelerating strobe...\r\n");
    for (int delay = 500; delay >= 50; delay -= 50) {
        SevenColorLED_On();
        HAL_Delay(delay);
        SevenColorLED_Off();
        HAL_Delay(delay);
    }

    SevenColorLED_Off();
}

/**
 * @brief 다양한 패턴 데모 (ON/OFF 타이밍 기반)
 */
void SevenColorLED_PatternDemo(void)
{
    /* 패턴 1: 심박 효과 (두 번 빠르게, 긴 휴식) */
    printf("  Heartbeat pattern...\r\n");
    for (int beat = 0; beat < 5; beat++) {
        /* 첫 번째 박동 */
        SevenColorLED_On();
        HAL_Delay(100);
        SevenColorLED_Off();
        HAL_Delay(100);

        /* 두 번째 박동 */
        SevenColorLED_On();
        HAL_Delay(100);
        SevenColorLED_Off();
        HAL_Delay(500);
    }

    HAL_Delay(500);

    /* 패턴 2: SOS 패턴 */
    printf("  SOS pattern...\r\n");
    for (int sos = 0; sos < 2; sos++) {
        /* S: 짧게 3번 */
        for (int i = 0; i < 3; i++) {
            SevenColorLED_On();
            HAL_Delay(200);
            SevenColorLED_Off();
            HAL_Delay(200);
        }
        HAL_Delay(300);

        /* O: 길게 3번 */
        for (int i = 0; i < 3; i++) {
            SevenColorLED_On();
            HAL_Delay(500);
            SevenColorLED_Off();
            HAL_Delay(200);
        }
        HAL_Delay(300);

        /* S: 짧게 3번 */
        for (int i = 0; i < 3; i++) {
            SevenColorLED_On();
            HAL_Delay(200);
            SevenColorLED_Off();
            HAL_Delay(200);
        }
        HAL_Delay(1000);
    }

    HAL_Delay(500);

    /* 패턴 3: 카운트다운 */
    printf("  Countdown pattern (5 to 1)...\r\n");
    for (int count = 5; count >= 1; count--) {
        printf("    %d\r\n", count);
        for (int i = 0; i < count; i++) {
            SevenColorLED_On();
            HAL_Delay(150);
            SevenColorLED_Off();
            HAL_Delay(150);
        }
        HAL_Delay(500);
    }

    /* 완료 표시: 길게 점등 */
    SevenColorLED_On();
    HAL_Delay(1000);
    SevenColorLED_Off();
}

/**
 * @brief 알림 패턴 데모
 */
void SevenColorLED_NotificationDemo(void)
{
    /* 알림 1: 새 메시지 (부드러운 펄스 3회) */
    printf("  New message notification...\r\n");
    for (int i = 0; i < 3; i++) {
        SevenColorLED_On();
        HAL_Delay(300);
        SevenColorLED_Off();
        HAL_Delay(300);
    }

    HAL_Delay(500);

    /* 알림 2: 경고 (빠른 점멸 6회) */
    printf("  Warning notification...\r\n");
    for (int i = 0; i < 6; i++) {
        SevenColorLED_On();
        HAL_Delay(100);
        SevenColorLED_Off();
        HAL_Delay(100);
    }

    HAL_Delay(500);

    /* 알림 3: 대기 중 (느린 점멸) */
    printf("  Standby notification...\r\n");
    for (int i = 0; i < 3; i++) {
        SevenColorLED_On();
        HAL_Delay(1000);
        SevenColorLED_Off();
        HAL_Delay(1000);
    }

    HAL_Delay(500);

    /* 알림 4: 완료 (길게 점등) */
    printf("  Completion notification...\r\n");
    SevenColorLED_On();
    HAL_Delay(300);
    SevenColorLED_Off();
    HAL_Delay(200);
    SevenColorLED_On();
    HAL_Delay(300);
    SevenColorLED_Off();
    HAL_Delay(200);
    SevenColorLED_On();
    HAL_Delay(1500);

    SevenColorLED_Off();
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  printf("\r\n=============================================\r\n");
  printf("  7-Color LED Module Test - NUCLEO-F103RB\r\n");
  printf("=============================================\r\n");
  printf("  Module: KY-034 (Auto Color Cycling)\r\n");
  printf("  Control: GPIO ON/OFF (No PWM)\r\n");
  printf("  Pin: PB0\r\n");
  printf("\r\n");

  /* 초기 테스트 */
  printf("[Init] Quick LED test...\r\n");
  SevenColorLED_On();
  HAL_Delay(1000);
  SevenColorLED_Off();
  HAL_Delay(500);
  printf("[Init] LED test complete.\r\n\r\n");
  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* Test 1: 기본 ON/OFF */
      printf("[Test 1] Basic ON/OFF Control\r\n");

      printf("  LED ON (5 seconds - watch color changes)...\r\n");
      SevenColorLED_On();
      HAL_Delay(5000);

      printf("  LED OFF...\r\n");
      SevenColorLED_Off();
      HAL_Delay(1000);

      /* Test 2: 자동 색상 변환 관찰 */
      printf("\r\n[Test 2] Auto Color Cycle Observation\r\n");
      SevenColorLED_AutoCycleDemo();
      HAL_Delay(500);

      /* Test 3: 스트로브 효과 */
      printf("\r\n[Test 3] Strobe Effect\r\n");
      SevenColorLED_StrobeDemo();
      HAL_Delay(500);

      /* Test 4: 패턴 데모 */
      printf("\r\n[Test 4] Pattern Demo\r\n");
      SevenColorLED_PatternDemo();
      HAL_Delay(500);

      /* Test 5: 알림 데모 */
      printf("\r\n[Test 5] Notification Patterns\r\n");
      SevenColorLED_NotificationDemo();

      SevenColorLED_Off();

      printf("\r\n--- Cycle Complete ---\r\n\r\n");
      HAL_Delay(2000);
    /* USER CODE END WHILE */
```

### STM32CubeMX 설정

```
Pinout:
  - PB0: GPIO_Output

GPIO:
  - PB0: 
    - Mode: Output Push Pull
    - Pull: No pull-up and no pull-down
    - Speed: Low
```

## 📊 시리얼 출력 예시

```
=============================================
  7-Color LED Module Test - NUCLEO-F103RB
=============================================
  Module: KY-034 (Auto Color Cycling)
  Control: GPIO ON/OFF (No PWM)
  Pin: PB0

[Init] Quick LED test...
[Init] LED test complete.

[Test 1] Basic ON/OFF Control
  LED ON (5 seconds - watch color changes)...
  LED OFF...

[Test 2] Auto Color Cycle Observation
  Observing auto color cycle for 10 seconds...
  Colors: Red -> Orange -> Yellow -> Green -> Cyan -> Blue -> Purple
  [1 sec]
  [2 sec]
  [3 sec]
  [4 sec]
  [5 sec]
  [6 sec]
  [7 sec]
  [8 sec]
  [9 sec]
  [10 sec]

[Test 3] Strobe Effect
  Slow strobe (500ms)...
  Fast strobe (100ms)...
  Accelerating strobe...

[Test 4] Pattern Demo
  Heartbeat pattern...
  SOS pattern...
  Countdown pattern (5 to 1)...
    5
    4
    3
    2
    1

[Test 5] Notification Patterns
  New message notification...
  Warning notification...
  Standby notification...
  Completion notification...

--- Cycle Complete ---
```

## 📝 효과 패턴 상세

### 스트로브 효과

```c
// 느린 스트로브
for (int i = 0; i < 10; i++) {
    SevenColorLED_On();
    HAL_Delay(500);
    SevenColorLED_Off();
    HAL_Delay(500);
}

// 빠른 스트로브
for (int i = 0; i < 20; i++) {
    SevenColorLED_On();
    HAL_Delay(100);
    SevenColorLED_Off();
    HAL_Delay(100);
}
```

### 심박 효과 (Heartbeat)

```c
// 두 번 빠르게 점멸 후 긴 휴식
SevenColorLED_On();
HAL_Delay(100);
SevenColorLED_Off();
HAL_Delay(100);
SevenColorLED_On();
HAL_Delay(100);
SevenColorLED_Off();
HAL_Delay(500);
```

### SOS 패턴

```c
// S: 짧게 3번
for (int i = 0; i < 3; i++) {
    SevenColorLED_On();
    HAL_Delay(200);
    SevenColorLED_Off();
    HAL_Delay(200);
}
HAL_Delay(300);

// O: 길게 3번
for (int i = 0; i < 3; i++) {
    SevenColorLED_On();
    HAL_Delay(500);
    SevenColorLED_Off();
    HAL_Delay(200);
}
HAL_Delay(300);

// S: 짧게 3번
for (int i = 0; i < 3; i++) {
    SevenColorLED_On();
    HAL_Delay(200);
    SevenColorLED_Off();
    HAL_Delay(200);
}
```

### 알림 패턴

| 패턴 | 설명 | 코드 |
|------|------|------|
| 알림 | 짧은 점멸 3회 | 200ms ON, 200ms OFF × 3 |
| 경고 | 빠른 점멸 6회 | 100ms ON, 100ms OFF × 6 |
| 완료 | 긴 점등 | 1000ms ON |

## 🔍 트러블슈팅

| 증상 | 원인 | 해결 방법 |
|------|------|----------|
| LED가 켜지지 않음 | 배선 오류 | GND/Signal 핀 확인 |
| LED가 계속 켜져있음 | Signal에 VCC 직결 | GPIO 핀에 연결 |
| 색상이 안 바뀜 | 모듈 불량 | Signal에 5V 직접 연결하여 테스트 |
| GPIO 제어 안됨 | GPIO 설정 오류 | Output 모드 확인 |

### 모듈 테스트 방법

MCU 없이 모듈 자체 테스트:

```
1. Signal 핀 → 5V 연결
2. GND 핀 → GND 연결
3. LED가 자동으로 색상 변환되면 정상
```

## 💡 응용 예제

### 간단한 상태 표시기

```c
void ShowStatus(uint8_t status) {
    switch (status) {
        case STATUS_IDLE:
            SevenColorLED_Off();
            break;
            
        case STATUS_RUNNING:
            SevenColorLED_On();  // 자동 색상 변환
            break;
            
        case STATUS_ERROR:
            // 빠른 점멸
            for (int i = 0; i < 10; i++) {
                SevenColorLED_Toggle();
                HAL_Delay(100);
            }
            break;
    }
}
```

### 타이머 알림

```c
void TimerAlert(void) {
    // 10초 동안 점멸
    for (int i = 0; i < 20; i++) {
        SevenColorLED_On();
        HAL_Delay(250);
        SevenColorLED_Off();
        HAL_Delay(250);
    }
}
```

### 버튼 반응

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == B1_Pin) {
        SevenColorLED_Toggle();
    }
}
```

## ⚠️ 주의사항

1. **색상 제어 불가**: 내장 IC가 자동으로 색상을 변환하므로 특정 색상 선택 불가
2. **밝기 제어 불가**: PWM으로 밝기 조절 시 내장 IC 동작에 영향을 줄 수 있음
3. **변환 속도 고정**: 색상 변환 속도는 내장 IC에 의해 고정됨
4. **핀 배치 주의**: PCB에 따라 핀 순서가 다를 수 있으므로 데이터시트 확인 필요

## 🔄 대안 모듈

개별 색상 제어가 필요한 경우:

| 모듈 | 핀 수 | 특징 |
|------|-------|------|
| KY-016 | 4핀 (R,G,B,GND) | 개별 색상 PWM 제어 가능 |
| WS2812B | 1핀 (Data) | 단선 통신, 개별 LED 제어 |
| Common Cathode RGB | 4핀 | 기본 RGB LED |

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [KY-034 7-Color Flash LED Module](https://arduinomodules.info/ky-034-automatic-flashing-color-led/)
- [Joy-IT KY-034 Datasheet](https://sensorkit.joy-it.net/en/sensors/ky-034)
