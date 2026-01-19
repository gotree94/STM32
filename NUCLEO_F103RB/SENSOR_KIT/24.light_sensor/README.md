# 조도 센서 모듈 (Light Sensor / Photoresistor Module) - STM32F103

## 📋 개요

조도 센서 모듈(KY-018)은 CdS(황화카드뮴) 광저항(LDR, Light Dependent Resistor)을 사용하여 주변 밝기를 아날로그 값으로 측정합니다. 빛이 밝으면 저항이 낮아지고, 어두우면 저항이 높아지는 특성을 이용합니다.

## 🔧 하드웨어 구성

### 센서 모듈 사양

| 항목 | 사양 |
|------|------|
| 동작 전압 | 3.3V ~ 5V |
| 출력 타입 | 아날로그 (전압 분배) |
| 광저항 범위 | 밝음: ~1kΩ, 어두움: ~10MΩ |
| 파장 감도 | 540nm (녹색광 최대) |
| 응답 시간 | 상승: 20ms, 하강: 30ms |

### 핀 연결

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/3ffe9cd0-13b4-44e3-9f31-411c800873a4" />



```
Light Sensor Module          NUCLEO-F103RB
===================          ===============
    S (Signal)  ---------->  PA0 (ADC1_CH0)
    + (VCC)     ---------->  3.3V
    - (GND)     ---------->  GND
```

### 회로 원리 (전압 분배)

```
      +3.3V
        │
        │
       ┌┴┐
       │ │ R1 (10kΩ 고정저항, 모듈 내장)
       │ │
       └┬┘
        │
        ├─────────────────> ADC Input (PA0)
        │
       ┌┴┐
       │ │ LDR (광저항)
       │◯│ 밝음: 낮은 저항 → 높은 전압
       └┬┘ 어두움: 높은 저항 → 낮은 전압
        │
       GND

    ※ 일부 모듈은 LDR과 R1 위치가 반대일 수 있음
```

## 💻 소프트웨어 구성

### 주요 기능

1. **12비트 ADC**: 0-4095 범위의 조도 측정
2. **DMA 전송**: CPU 부하 없는 연속 샘플링
3. **평균 필터**: 16개 샘플 평균으로 노이즈 제거
4. **조도 레벨 분류**: 6단계 밝기 분류
5. **자동 조명 제어**: 어두우면 LED 자동 점등

### 클럭 설정

```
HSI RC (8MHz) → /2 → PLL (×4) → SYSCLK (16MHz)
                                     ↓
                               AHB Prescaler (/1)
                                     ↓
                               HCLK (16MHz)
                                     ↓
                               APB2 Prescaler (/1)
                                     ↓
                               PCLK2 (16MHz)
                                     ↓
                               ADC Prescaler (/2)
                                     ↓
                               ADC Clock (8MHz)
```

### ADC 설정

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| Resolution | 12-bit | 0~4095 |
| System Clock | 16MHz | HSI(8MHz) / 2 × PLL(4) |
| HCLK | 16MHz | SYSCLK / 1 |
| PCLK2 | 16MHz | HCLK / 1 |
| ADC Prescaler | /2 | PCLK2 / 2 |
| ADC Clock | 8MHz | 최대 14MHz 이내 |
| Sampling Time | 239.5 cycles | 고임피던스 소스 대응 |
| Conversion Time | 252 cycles | 샘플링(239.5) + 변환(12.5) |
| Total Conversion | 31.5μs | 252 cycles / 8MHz |
| Conversion Mode | Continuous | 연속 변환 |
| DMA Mode | Normal | 버퍼 채움 후 인터럽트 |

> **참고**: 조도 센서(LDR) 응답 시간이 20~30ms이므로 31.5μs 변환 주기는 충분합니다.

### 조도 레벨 분류

| ADC 값 범위 | 레벨 | 대략적 환경 |
|-------------|------|-------------|
| 0 ~ 499 | Very Bright ☀☀ | 직사광선 |
| 500 ~ 1499 | Bright ☀ | 밝은 실내 |
| 1500 ~ 2499 | Normal 🌤 | 일반 실내 |
| 2500 ~ 3199 | Dim 🌥 | 어두운 실내 |
| 3200 ~ 3799 | Dark 🌙 | 저녁/조명 없음 |
| 3800 ~ 4095 | Very Dark 🌑 | 완전 어둠 |

### 핵심 코드 설명

```c
/* DMA를 이용한 연속 ADC 변환 */
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLES_TO_AVG);

/* 평균값 계산 */
uint16_t Get_Average_ADC(void) {
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES_TO_AVG; i++) {
        sum += adc_buffer[i];
    }
    return (uint16_t)(sum / SAMPLES_TO_AVG);
}

/* 조도에 따른 자동 LED 제어 */
if (adc_value > LIGHT_DIM) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);   /* LED ON */
} else {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); /* LED OFF */
}
```

## 📁 프로젝트 구조

```
04_light_sensor/
├── main.c              # 메인 애플리케이션
├── README.md           # 프로젝트 문서
└── STM32CubeIDE/       # (선택) IDE 프로젝트 파일
```

## 🚀 빌드 및 실행

### STM32CubeIDE 사용

1. 새 STM32 프로젝트 생성 (NUCLEO-F103RB 선택)
2. `main.c` 내용을 프로젝트에 복사
3. 빌드 및 다운로드

<img width="1349" height="1035" alt="002" src="https://github.com/user-attachments/assets/c7dd90e7-6c2c-4ecd-9c6b-6c4c883deefa" />

<img width="1349" height="1035" alt="003" src="https://github.com/user-attachments/assets/f5682ed9-945e-4d8e-8f35-79a0ead9fc87" />


```c
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
#define ADC_RESOLUTION      4096    /* 12-bit ADC */
#define SAMPLES_TO_AVG      16      /* 평균 샘플 수 */

/* 조도 레벨 임계값 (ADC 값 기준, 밝으면 값이 낮음) */
#define LIGHT_VERY_BRIGHT   500
#define LIGHT_BRIGHT        1500
#define LIGHT_NORMAL        2500
#define LIGHT_DIM           3200
#define LIGHT_DARK          3800
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
volatile uint16_t adc_buffer[SAMPLES_TO_AVG];
volatile uint8_t adc_complete = 0;
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
uint16_t Get_Average_ADC(void);
const char* Get_Light_Level_String(uint16_t adc_value);
void Print_Light_Bar(uint16_t adc_value);

/* Printf redirect -----------------------------------------------------------*/
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* DMA Complete Callback -----------------------------------------------------*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        adc_complete = 1;
    }
}
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
/**
 * @brief Calculate average ADC value from DMA buffer
 */
uint16_t Get_Average_ADC(void) {
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES_TO_AVG; i++) {
        sum += adc_buffer[i];
    }
    return (uint16_t)(sum / SAMPLES_TO_AVG);
}

/**
 * @brief Convert ADC value to light level string
 */
const char* Get_Light_Level_String(uint16_t adc_value) {
    if (adc_value < LIGHT_VERY_BRIGHT) return "Very Bright ☀☀";
    if (adc_value < LIGHT_BRIGHT)      return "Bright ☀";
    if (adc_value < LIGHT_NORMAL)      return "Normal 🌤";
    if (adc_value < LIGHT_DIM)         return "Dim 🌥";
    if (adc_value < LIGHT_DARK)        return "Dark 🌙";
    return "Very Dark 🌑";
}

/**
 * @brief Print graphical light bar
 */
void Print_Light_Bar(uint16_t adc_value) {
    /* ADC 값을 0-20 범위로 변환 (밝으면 바가 김) */
    int bar_length = 20 - (adc_value * 20 / ADC_RESOLUTION);
    if (bar_length < 0) bar_length = 0;
    if (bar_length > 20) bar_length = 20;

    printf("[");
    for (int i = 0; i < 20; i++) {
        if (i < bar_length) {
            printf("█");
        } else {
            printf("░");
        }
    }
    printf("] %d%%", bar_length * 5);
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  printf("\r\n========================================\r\n");
  printf("  Light Sensor (LDR) Module Test\r\n");
  printf("  Board: NUCLEO-F103RB\r\n");
  printf("========================================\r\n\n");
  printf("ADC Clock: 8MHz (PCLK2/2)\r\n");
  printf("ADC Resolution: 12-bit (0-4095)\r\n");
  printf("Samples averaged: %d\r\n\n", SAMPLES_TO_AVG);

  /* ADC Calibration */
  HAL_ADCEx_Calibration_Start(&hadc1);

  /* Start DMA-based ADC conversion */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLES_TO_AVG);

  uint16_t adc_value = 0;
  uint16_t prev_adc_value = 0;
  uint32_t last_print_time = 0;
  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
      if (adc_complete) {
          adc_complete = 0;

          /* 평균값 계산 */
          adc_value = Get_Average_ADC();

          /* 500ms마다 또는 큰 변화 시 출력 */
          if (HAL_GetTick() - last_print_time >= 500 ||
              (adc_value > prev_adc_value + 100) ||
              (prev_adc_value > adc_value + 100)) {

              printf("\r\n[Light Sensor Reading]\r\n");
              printf("  ADC Value: %4d / %d\r\n", adc_value, ADC_RESOLUTION - 1);
              printf("  Voltage  : %.2f V\r\n", (float)adc_value * 3.3f / ADC_RESOLUTION);
              printf("  Level    : %s\r\n", Get_Light_Level_String(adc_value));
              printf("  ");
              Print_Light_Bar(adc_value);
              printf("\r\n");

              last_print_time = HAL_GetTick();
              prev_adc_value = adc_value;
          }

          /* LED 밝기에 따른 제어 */
          if (adc_value > LIGHT_DIM) {
              /* 어두우면 LED ON */
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
          } else {
              /* 밝으면 LED OFF */
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
          }

          /* 다음 DMA 변환 시작 */
          HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLES_TO_AVG);
      }

      HAL_Delay(10);
    /* USER CODE END WHILE */
```

### 예상 출력

```
========================================
  Light Sensor (LDR) Module Test
  Board: NUCLEO-F103RB
========================================

ADC Clock: 8MHz (PCLK2/2)
ADC Resolution: 12-bit (0-4095)
Samples averaged: 16

[Light Sensor Reading]
  ADC Value: 1842 / 4095
  Voltage  : 1.48 V
  Level    : Normal 🌤
  [██████████░░░░░░░░░░] 50%

[Light Sensor Reading]
  ADC Value: 3521 / 4095
  Voltage  : 2.84 V
  Level    : Dark 🌙
  [████░░░░░░░░░░░░░░░░] 20%
```

## 🎯 동작 원리

### 광저항 특성 곡선

```
저항 (Ω)
    │
10MΩ├─────○
    │       \
 1MΩ├────────○
    │          \
100kΩ├───────────○
    │             \
 10kΩ├──────────────○
    │                \
  1kΩ├───────────────────○
    └─────┬─────┬─────┬─────> 조도 (Lux)
         10   100  1000  10000
```

### 전압 출력 계산

```
Vout = Vcc × LDR / (R1 + LDR)

예시 (Vcc = 3.3V, R1 = 10kΩ):
- 밝음 (LDR = 1kΩ):  Vout = 3.3 × 1k / (10k + 1k) = 0.3V → ADC ≈ 372
- 어두움 (LDR = 100kΩ): Vout = 3.3 × 100k / (10k + 100k) = 3.0V → ADC ≈ 3723
```

## 📌 응용 아이디어

1. **자동 조명 시스템**: 어두워지면 자동으로 조명 ON
2. **일출/일몰 감지**: 시간대별 조도 변화 로깅
3. **화면 밝기 자동 조절**: 주변 밝기에 따른 디스플레이 밝기 조절
4. **식물 성장 모니터링**: 광량 측정 및 기록
5. **사진/영상**: 노출 보조 장치

### 자동 조명 히스테리시스 예제

```c
#define THRESHOLD_ON    3000    /* 조명 켜기 임계값 */
#define THRESHOLD_OFF   2500    /* 조명 끄기 임계값 */
#define HYSTERESIS      500     /* 히스테리시스 폭 */

static uint8_t light_on = 0;

void Auto_Light_Control(uint16_t adc_value) {
    if (!light_on && adc_value > THRESHOLD_ON) {
        light_on = 1;
        /* 조명 ON */
    } else if (light_on && adc_value < THRESHOLD_OFF) {
        light_on = 0;
        /* 조명 OFF */
    }
    /* 히스테리시스 구간에서는 현재 상태 유지 */
}
```

## ⚠️ 주의사항

- CdS는 온도에 민감하므로 고온 환경 주의
- 직사광선에서는 포화 상태가 될 수 있음
- CdS에는 카드뮴이 포함되어 환경 규제 대상일 수 있음
- 응답 속도가 느리므로 빠른 변화 감지에는 부적합
- 적외선(IR)에도 반응하므로 정밀 측정 시 고려 필요

## 🔍 필터링 기법

### 이동 평균 필터

```c
#define FILTER_SIZE 8
uint16_t filter_buffer[FILTER_SIZE];
uint8_t filter_index = 0;

uint16_t Moving_Average_Filter(uint16_t new_value) {
    filter_buffer[filter_index] = new_value;
    filter_index = (filter_index + 1) % FILTER_SIZE;
    
    uint32_t sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += filter_buffer[i];
    }
    return sum / FILTER_SIZE;
}
```

### 지수 가중 이동 평균 (EWMA)

```c
#define ALPHA 0.1f  /* 0~1, 작을수록 더 부드러움 */

float ewma_filter(float new_value, float prev_filtered) {
    return ALPHA * new_value + (1.0f - ALPHA) * prev_filtered;
}
```

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [KY-018 Photoresistor Module](https://arduinomodules.info/ky-018-photoresistor-module/)
