# Line Tracking Module - STM32F103 NUCLEO

라인 트래킹(추적) 모듈(TCRT5000 기반)을 STM32F103 NUCLEO 보드로 테스트하는 예제입니다.

![1-PCS-Tracking-Sensor-IR-font-b-TCRT5000-b-font-Infrared-Obstacle-Avoidance-sensor-for-Arduino jpg_220x220](https://github.com/user-attachments/assets/c51873be-8cad-44ed-9694-152831172743)
<img width="400" height="150" alt="TCRT5000-1024x429" src="https://github.com/user-attachments/assets/80092496-d9a3-4c4b-97f5-1be8c25851fa" />


## 하드웨어 구성

### 부품 목록
| 부품 | 수량 | 비고 |
|------|------|------|
| STM32F103 NUCLEO | 1 | NUCLEO-F103RB |
| 라인 트래킹 모듈 | 1 | 3핀 단일채널 (TCRT5000) |
| 점퍼 와이어 | 3 | M-F 타입 |
| 테스트 라인 | 1 | 검은색 테이프 또는 인쇄물 |

### 센서 모듈 사양
- 센서 타입: TCRT5000 적외선 반사 센서
- 동작 전압: 3.3V ~ 5V
- 출력: 디지털 (Active Low)
- 감지 거리: 1mm ~ 8mm (최적: 2~3mm)
- 감지 대상: 검은색 = LOW, 흰색 = HIGH
- 핀 구성: VCC, GND, OUT (3핀)

### 핀 연결
```
트래킹 모듈 (3핀)      STM32F103 NUCLEO
-----------------------------------------
VCC      ────────►  3.3V (CN7-16)
GND      ────────►  GND (CN7-20)
OUT      ────────►  PA0 (CN7-28)
```

### 회로도
```
                     ┌─────────────────┐
                     │  트래킹 모듈    │
                     │   (3핀 타입)    │
                     │                 │
                     │  [IR]    [PR]   │
                     │   ◯       ◯     │
                     │                 │
    PA0 ◄────────────┤ OUT             │
    3.3V ──────────► │ VCC             │
    GND  ──────────► │ GND             │
                     └─────────────────┘
                     
    PA5 ──────────► LED (라인 감지 시 ON)
```

## 센서 동작 원리
```
   IR LED ─►  ◯ ─►  [표면]  ─►  ◯  ◄─ IR Receiver
            발광              수광
            
   흰색 표면: 반사 많음 ─► HIGH 출력 (라인 없음)
   검은색 표면: 반사 적음 ─► LOW 출력 (라인 감지)
```

## 기능 설명

1. **단일 채널 라인 감지**
   - 흑색 라인 감지 (Active Low)
   - 백색 표면 감지 (High)

2. **상태 표시**
   - `BLACK`: 검은색 라인 감지됨
   - `WHITE`: 흰색 표면 (라인 없음)

3. **통계 정보**
   - 상태 전환 시간 측정
   - 5초마다 감지 비율 출력

4. **시각적 표시**
   - LED: 라인 감지 시 ON
   - 시리얼: 바 그래프 형태 출력

## 빌드 및 업로드

### STM32CubeIDE 사용 시
1. 새 프로젝트 생성 (STM32F103RB 선택)
2. PA0를 GPIO_Input으로 설정
3. PA5를 GPIO_Output으로 설정
4. `main.c` 내용을 프로젝트에 복사
5. 빌드 후 업로드

```c
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```

```c
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SENSOR_PIN          GPIO_PIN_0
#define SENSOR_PORT         GPIOA
#define LED_PIN             GPIO_PIN_5
#define LED_PORT            GPIOA

/* Sensor state macros */
#define LINE_DETECTED       0   // Active Low (흑색 라인 감지)
#define LINE_NOT_DETECTED   1   // 백색 바닥
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
/* Statistics */
uint32_t detect_count = 0;
uint32_t total_samples = 0;
uint32_t last_transition_time = 0;
uint8_t prev_state = LINE_NOT_DETECTED;
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
uint8_t Read_Sensor(void);
void Print_Sensor_Bar(uint8_t state);

/* Printf redirect */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
/**
 * @brief  Read Tracking Sensor
 * @retval LINE_DETECTED (0) or LINE_NOT_DETECTED (1)
 */
uint8_t Read_Sensor(void)
{
    return HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_PIN);
}

/**
 * @brief  Print visual sensor bar
 */
void Print_Sensor_Bar(uint8_t state)
{
    if (state == LINE_DETECTED)
    {
        printf("[##########]  <- LINE DETECTED");
    }
    else
    {
        printf("[----------]  <- NO LINE");
    }
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  printf("\r\n============================================\r\n");
  printf("  Line Tracking Sensor Test (Single Channel)\r\n");
  printf("  STM32F103 NUCLEO\r\n");
  printf("============================================\r\n");
  printf("Module: 3-Pin (VCC, GND, OUT)\r\n");
  printf("PA0: Sensor Output\r\n");
  printf("(0=Black Line, 1=White Surface)\r\n\r\n");

  uint8_t sensor_state;
  uint32_t last_print_time = 0;
  uint32_t stats_time = 0;
  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* 센서 읽기 */
      sensor_state = Read_Sensor();
      total_samples++;

      /* 라인 감지 시 LED ON */
      if (sensor_state == LINE_DETECTED)
      {
          HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
          detect_count++;
      }
      else
      {
          HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
      }

      /* 상태 전환 감지 */
      if (sensor_state != prev_state)
      {
          uint32_t current_time = HAL_GetTick();
          uint32_t duration = current_time - last_transition_time;

          printf("[TRANSITION] %s -> %s (after %lu ms)\r\n",
                 prev_state == LINE_DETECTED ? "BLACK" : "WHITE",
                 sensor_state == LINE_DETECTED ? "BLACK" : "WHITE",
                 duration);

          last_transition_time = current_time;
          prev_state = sensor_state;
      }

      /* 100ms마다 상태 출력 */
      if (HAL_GetTick() - last_print_time >= 100)
      {
          last_print_time = HAL_GetTick();

          printf("Sensor: [%s] ",
                 sensor_state == LINE_DETECTED ? "BLACK ###" : "WHITE ---");
          Print_Sensor_Bar(sensor_state);
          printf("\r\n");
      }

      /* 5초마다 통계 출력 */
      if (HAL_GetTick() - stats_time >= 5000)
      {
          stats_time = HAL_GetTick();
          uint32_t detect_percent = (detect_count * 100) / total_samples;

          printf("\r\n--- Statistics (5s) ---\r\n");
          printf("Total samples: %lu\r\n", total_samples);
          printf("Line detected: %lu (%lu%%)\r\n", detect_count, detect_percent);
          printf("------------------------\r\n\r\n");

          detect_count = 0;
          total_samples = 0;
      }

      HAL_Delay(10);
    /* USER CODE END WHILE */
```

### CubeMX 설정
```
Pinout:
- PA0: GPIO_Input (Pull-up)
- PA5: GPIO_Output
- PA2: USART2_TX
- PA3: USART2_RX
```

## 시리얼 출력 예시
```
============================================
  Line Tracking Sensor Test (Single Channel)
  STM32F103 NUCLEO
============================================
Module: 3-Pin (VCC, GND, OUT)
PA0: Sensor Output
(0=Black Line, 1=White Surface)

Sensor: [WHITE ---] [----------]  <- NO LINE
Sensor: [WHITE ---] [----------]  <- NO LINE
[TRANSITION] WHITE -> BLACK (after 2340 ms)
Sensor: [BLACK ###] [##########]  <- LINE DETECTED
Sensor: [BLACK ###] [##########]  <- LINE DETECTED
[TRANSITION] BLACK -> WHITE (after 520 ms)
Sensor: [WHITE ---] [----------]  <- NO LINE

--- Statistics (5s) ---
Total samples: 500
Line detected: 52 (10%)
------------------------
```

## 센서 상태 해석

| OUT 핀 | 상태 | 의미 |
|--------|------|------|
| LOW (0) | BLACK | 검은색 라인 감지 |
| HIGH (1) | WHITE | 흰색 표면 (라인 없음) |

## 응용 예제

### 물체 감지 카운터
```c
uint32_t object_count = 0;
uint8_t prev_state = LINE_NOT_DETECTED;

void Count_Objects(void)
{
    uint8_t current = Read_Sensor();
    
    // 하강 에지 감지 (WHITE -> BLACK)
    if (prev_state == LINE_NOT_DETECTED && current == LINE_DETECTED)
    {
        object_count++;
        printf("Object detected! Count: %lu\r\n", object_count);
    }
    
    prev_state = current;
}
```

### 디바운싱 적용
```c
#define DEBOUNCE_TIME_MS    50

uint8_t Read_Sensor_Debounced(void)
{
    static uint8_t last_state = LINE_NOT_DETECTED;
    static uint32_t last_change_time = 0;
    
    uint8_t current = Read_Sensor();
    uint32_t now = HAL_GetTick();
    
    if (current != last_state)
    {
        if (now - last_change_time >= DEBOUNCE_TIME_MS)
        {
            last_state = current;
            last_change_time = now;
        }
    }
    
    return last_state;
}
```

### 다중 센서 확장 (3채널)
```c
/* 3채널로 확장 시 핀 정의 */
#define SENSOR_LEFT_PIN     GPIO_PIN_0
#define SENSOR_CENTER_PIN   GPIO_PIN_1
#define SENSOR_RIGHT_PIN    GPIO_PIN_4
#define SENSOR_PORT         GPIOA

typedef enum {
    POS_LEFT,
    POS_CENTER,
    POS_RIGHT,
    POS_LOST
} LinePosition;

LinePosition Get_Line_Position_3CH(void)
{
    uint8_t left = HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_LEFT_PIN);
    uint8_t center = HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_CENTER_PIN);
    uint8_t right = HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_RIGHT_PIN);
    
    if (center == LINE_DETECTED) return POS_CENTER;
    if (left == LINE_DETECTED) return POS_LEFT;
    if (right == LINE_DETECTED) return POS_RIGHT;
    
    return POS_LOST;
}
```

### 인터럽트 기반 감지
```c
/* GPIO 인터럽트 설정 (CubeMX에서 PA0을 EXTI로 설정) */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == SENSOR_PIN)
    {
        if (HAL_GPIO_ReadPin(SENSOR_PORT, SENSOR_PIN) == LINE_DETECTED)
        {
            // 라인 감지됨
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        }
        else
        {
            // 라인 이탈
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        }
    }
}
```

## 트러블슈팅

| 증상 | 원인 | 해결방법 |
|------|------|----------|
| 감지 안됨 | 거리 너무 멂 | 센서를 표면에 2-3mm로 조절 |
| 항상 감지 | 거리 너무 가까움 | 센서 높이 증가 |
| 불안정 | 조명 간섭 | 센서에 차광 튜브 추가 |
| 흰색도 감지 | 감도 과다 | 모듈 가변저항 조절 |
| 출력 반전 | 모듈 타입 다름 | 코드에서 로직 반전 |

## 테스트 환경 만들기

### 라인 트랙 제작
1. 흰색 바탕에 검은색 테이프 (폭 2-3cm)
2. A4 용지에 검은색 선 인쇄
3. 센서-표면 거리: 2-3mm 유지

### 조명 조건
- 직사광선 피하기
- 일정한 조명 환경 권장
- 필요시 센서 주변에 차광판 설치

## 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [TCRT5000 Datasheet](https://www.vishay.com/docs/83760/tcrt5000.pdf)

