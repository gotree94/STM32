# 각도 스위치 모듈 (Tilt Switch Module) - STM32F103

<img width="200" height="200" alt="SW0105" src="https://github.com/user-attachments/assets/ba78d66d-c490-467d-b138-47ee7a0ebaa6" />
<img width="200" height="200" alt="SW0105-1" src="https://github.com/user-attachments/assets/5e6f37d1-6a7b-446b-983b-a26ee511a581" />
<img width="200" height="200" alt="SW0105-2" src="https://github.com/user-attachments/assets/e25011a1-d00e-4b1f-af94-09aeab7c401b" />
<br>

## 📋 개요

* 각도 스위치 모듈(KY-020)은 내부의 금속 볼이 기울기에 따라 두 접점을 연결하거나 분리하는 디지털 센서입니다. 
* 특정 임계 각도 이상 기울어지면 스위치가 ON 상태가 됩니다.

## 🔧 하드웨어 구성

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/8b4d4802-87c9-4cb4-bd2b-a39b72d026eb" />

### 센서 모듈 사양

| 항목 | 사양 |
|------|------|
| 동작 전압 | 3.3V ~ 5V |
| 출력 타입 | 디지털 (HIGH/LOW) |
| 감지 각도 | 약 15° ~ 45° (제품마다 상이) |
| 응답 시간 | 2ms 이내 |
| 접점 수명 | 100,000회 이상 |

### 핀 연결

```
Tilt Switch Module          NUCLEO-F103RB
==================          ===============
    S (Signal)  --------->  PA0 (EXTI0)
    + (VCC)     --------->  3.3V
    - (GND)     --------->  GND
```

### 회로도

```
      +3.3V
        │
        ├──────[10kΩ]──────┐
        │                   │
    ┌───┴───┐               │
    │ Tilt  │               │
    │Switch │───────────────┼──────> PA0 (EXTI0)
    │  ○    │               │
    │  ↓    │               │
    │ ○○○   │               │
    └───┬───┘               │
        │                   │
       GND                 GND

    ※ 모듈 내장 풀업 저항 사용 또는 MCU 내부 풀업 활성화
```

## 💻 소프트웨어 구성

### 주요 기능

1. **외부 인터럽트 감지**: 상승/하강 엣지 모두 감지
2. **디바운싱 처리**: 50ms 소프트웨어 디바운스
3. **이벤트 카운팅**: 기울기 변화 횟수 기록
4. **LED 피드백**: 기울기 상태에 따른 보드 LED 제어

### 핵심 코드 설명

```c
/* 외부 인터럽트 콜백 함수 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == TILT_PIN) {
        uint32_t current_time = HAL_GetTick();
        
        /* 디바운싱: 50ms 이내 재트리거 무시 */
        if ((current_time - last_interrupt_time) > DEBOUNCE_DELAY_MS) {
            tilt_state = HAL_GPIO_ReadPin(TILT_PORT, TILT_PIN);
            tilt_changed = 1;
            last_interrupt_time = current_time;
        }
    }
}
```

### 인터럽트 설정

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| 핀 | PA0 | EXTI Line 0 |
| 모드 | IT_RISING_FALLING | 양방향 엣지 감지 |
| 풀업 | Internal Pull-up | 10kΩ 등가 |
| 우선순위 | 2 | 중간 우선순위 |

## 📁 프로젝트 구조

```
02_angle_switch/
├── main.c              # 메인 애플리케이션
├── README.md           # 프로젝트 문서
└── STM32CubeIDE/       # (선택) IDE 프로젝트 파일
```

## 🚀 빌드 및 실행

### STM32CubeIDE 사용

1. 새 STM32 프로젝트 생성 (NUCLEO-F103RB 선택)
2. `main.c` 내용을 프로젝트에 복사
3. 빌드 및 다운로드

```c
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
#define TILT_PIN            GPIO_PIN_0
#define TILT_PORT           GPIOA
#define DEBOUNCE_DELAY_MS   50
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
volatile uint8_t tilt_changed = 0;
volatile uint8_t tilt_state = 0;
volatile uint32_t last_interrupt_time = 0;
uint32_t tilt_count = 0;
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN 0 */
/* Printf redirect -----------------------------------------------------------*/
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* Interrupt callback --------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == TILT_PIN) {
        uint32_t current_time = HAL_GetTick();

        /* 디바운싱: 50ms 이내 재트리거 무시 */
        if ((current_time - last_interrupt_time) > DEBOUNCE_DELAY_MS) {
            tilt_state = HAL_GPIO_ReadPin(TILT_PORT, TILT_PIN);
            tilt_changed = 1;
            last_interrupt_time = current_time;
        }
    }
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  printf("\r\n========================================\r\n");
  printf("  Tilt/Angle Switch Module Test\r\n");
  printf("  Board: NUCLEO-F103RB\r\n");
  printf("========================================\r\n\n");
  printf("Tilt the module to detect angle changes.\r\n");
  printf("Using external interrupt with debouncing.\r\n\n");

  /* 초기 상태 읽기 */
  tilt_state = HAL_GPIO_ReadPin(TILT_PORT, TILT_PIN);
  printf("Initial state: %s\r\n\n", tilt_state ? "TILTED" : "LEVEL");
  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* 인터럽트에서 상태 변화 감지 시 처리 */
      if (tilt_changed) {
          tilt_changed = 0;
          tilt_count++;

          printf("[%5lu] ", tilt_count);

          if (tilt_state == GPIO_PIN_SET) {
              printf(">>> TILTED - Module is angled\r\n");
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  /* LED ON */
          } else {
              printf("=== LEVEL - Module is horizontal\r\n");
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  /* LED OFF */
          }
      }

      /* 메인 루프에서 다른 작업 수행 가능 */
      HAL_Delay(10);
    /* USER CODE END WHILE */
```

### 터미널 확인

```bash
screen /dev/ttyACM0 115200
```

### 예상 출력

```
========================================
  Tilt/Angle Switch Module Test
  Board: NUCLEO-F103RB
========================================

Tilt the module to detect angle changes.
Using external interrupt with debouncing.

Initial state: TILTED

[    1] === LEVEL - Module is horizontal
[    2] === LEVEL - Module is horizontal
[    3] >>> TILTED - Module is angled
[    4] >>> TILTED - Module is angled
[    5] === LEVEL - Module is horizontal
[    6] >>> TILTED - Module is angled
[    7] >>> TILTED - Module is angled
[    8] === LEVEL - Module is horizontal
[    9] >>> TILTED - Module is angled
[   10] === LEVEL - Module is horizontal
[   11] >>> TILTED - Module is angled
[   12] === LEVEL - Module is horizontal
[   13] >>> TILTED - Module is angled
[   14] === LEVEL - Module is horizontal
[   15] >>> TILTED - Module is angled
[   16] >>> TILTED - Module is angled
[   17] >>> TILTED - Module is angled
...
```

## 🎯 동작 원리

```
    [수평 상태]              [기울인 상태]
    
    ┌─────────┐              ┌─────────┐
    │   ○     │              │         │
    │  Metal  │  ────────>   │     ○   │
    │   Ball  │              │   Metal │
    │ ═══════ │              │ ═══════ │
    │ Contact │              │ Contact │
    └─────────┘              └─────────┘
         │                        │
      Switch                   Switch
       OPEN                    CLOSED
         │                        │
    Output: HIGH            Output: LOW
```

### 상태 전이도

```
         기울임
  LEVEL ────────> TILTED
    │               │
    │<──────────────│
         수평 복귀
         
  State  │ Output │  LED
  ───────┼────────┼──────
  LEVEL  │  HIGH  │  OFF
  TILTED │  LOW   │  ON
```

## 📌 응용 아이디어

1. **도난 방지 센서**: 물체 이동 감지 알람
2. **자세 모니터링**: 의자 기울기 감지
3. **자동 절전**: 장치 사용 여부 감지
4. **장난감**: 움직임 반응 장난감
5. **스마트 홈**: 창문/문 개폐 감지

## 🔍 디바운싱 상세

### 바운싱 현상

```
              실제 접촉점
     _______      ___________
     |     |      |
     |     |______|
     
     기계적 바운싱
     _____   _   ___________
     |   | | | |
     |   |_| |_|
     
     0   5  10 15 20        (ms)
```

### 소프트웨어 디바운싱

```c
#define DEBOUNCE_DELAY_MS   50

/* 인터럽트 내에서 시간 체크 */
if ((current_time - last_interrupt_time) > DEBOUNCE_DELAY_MS) {
    /* 유효한 상태 변화로 처리 */
}
```

## ⚠️ 주의사항

- 강한 진동 환경에서는 오동작 가능
- 수평 감지가 필요하면 가속도 센서 고려
- 정밀한 각도 측정에는 부적합 (ON/OFF만 감지)
- 접점 산화 방지를 위해 밀봉된 모듈 권장

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [KY-020 Tilt Switch Datasheet](https://arduinomodules.info/ky-020-tilt-switch-module/)


