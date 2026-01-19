# High Sensitivity Sound Sensor Module - STM32F103 NUCLEO

고감도 소리센서 모듈(KY-037 또는 유사 모듈)을 STM32F103 NUCLEO 보드로 테스트하는 예제입니다.

## 하드웨어 구성

### 부품 목록
| 부품 | 수량 | 비고 |
|------|------|------|
| STM32F103 NUCLEO | 1 | NUCLEO-F103RB |
| 고감도 소리센서 모듈 | 1 | KY-037 또는 동급 |
| 점퍼 와이어 | 4+ | M-F 타입 |

### 센서 모듈 사양
- 동작 전압: 3.3V ~ 5V
- 출력: 디지털(DO) + 아날로그(AO)
- 감도 조절: 온보드 가변저항
- 마이크: Electret Condenser Microphone

### 핀 연결

```
소리센서 모듈        STM32F103 NUCLEO
-----------------------------------------
VCC      ────────►  3.3V (CN7-16)
GND      ────────►  GND (CN7-20)
AO       ────────►  PA0 (CN7-28) - ADC
DO       ────────►  PA1 (CN7-30) - Digital
```

### 회로도

```
                     ┌──────────────────┐
                     │  고감도 소리센서  │
    PA0 (ADC) ◄───── │ AO              │
                     │      [POT]      │ ← 감도 조절
    PA1 (DI)  ◄───── │ DO              │
                     │                 │
    3.3V ──────────► │ VCC         MIC │
                     │                 │
    GND  ──────────► │ GND             │
                     └──────────────────┘
                     
    PA5 ──────────► LED (상태 표시)
```

## 기능 설명

1. **아날로그 출력 (AO)**
   - ADC를 통해 소리 크기를 0~4095 값으로 측정
   - 실시간 소리 레벨 모니터링

2. **디지털 출력 (DO)**
   - 설정 임계값 초과 시 LOW 출력
   - 가변저항으로 임계값 조절 가능

3. **LED 표시**
   - DO 신호에 따라 소리 감지 시 LED 점등

4. **시리얼 모니터링**
   - ADC 값 실시간 출력
   - 막대 그래프 시각화
   - 소리 레벨 분류 (Quiet/Normal/Loud/Very Loud)

## 빌드 및 업로드

### STM32CubeIDE 사용 시
1. 새 프로젝트 생성 (STM32F103RB 선택)
2. PA0을 ADC1_IN0으로 설정
3. `main.c` 내용을 프로젝트에 복사
4. 빌드 후 업로드

```c
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```

```c
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SOUND_AO_PIN        GPIO_PIN_0      // Analog Output
#define SOUND_AO_PORT       GPIOA
#define SOUND_DO_PIN        GPIO_PIN_1      // Digital Output
#define SOUND_DO_PORT       GPIOA
#define LED_PIN             GPIO_PIN_5
#define LED_PORT            GPIOA

#define ADC_THRESHOLD_LOW   500             // 조용한 환경
#define ADC_THRESHOLD_MED   1500            // 보통 소리
#define ADC_THRESHOLD_HIGH  2500            // 큰 소리
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
volatile uint8_t sound_detected = 0;
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
uint16_t Read_Sound_ADC(void);
void Print_Sound_Level(uint16_t adc_value);
void Print_Sound_Bar(uint16_t adc_value);

/* Printf redirect */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END PFP */
```

```c
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief  Read Sound Sensor ADC Value
 * @retval ADC value (0-4095)
 */
uint16_t Read_Sound_ADC(void)
{
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        return HAL_ADC_GetValue(&hadc1);
    }
    return 0;
}

/**
 * @brief  Print sound level description
 */
void Print_Sound_Level(uint16_t adc_value)
{
    if (adc_value < ADC_THRESHOLD_LOW)
    {
        printf(" [Quiet]");
    }
    else if (adc_value < ADC_THRESHOLD_MED)
    {
        printf(" [Normal]");
    }
    else if (adc_value < ADC_THRESHOLD_HIGH)
    {
        printf(" [Loud]");
    }
    else
    {
        printf(" [Very Loud!]");
    }
}

/**
 * @brief  Print sound level bar graph
 */
void Print_Sound_Bar(uint16_t adc_value)
{
    uint8_t bar_length = adc_value / 200;  // 0-20 범위
    if (bar_length > 20) bar_length = 20;

    printf("[");
    for (uint8_t i = 0; i < 20; i++)
    {
        if (i < bar_length)
            printf("#");
        else
            printf("-");
    }
    printf("]");
}
/* USER CODE END 0 */
```

```c
 /* USER CODE BEGIN 2 */
  printf("\r\n============================================\r\n");
  printf("  High Sensitivity Sound Sensor Test\r\n");
  printf("  STM32F103 NUCLEO\r\n");
  printf("============================================\r\n");
  printf("PA0: Analog Output (ADC)\r\n");
  printf("PA1: Digital Output (Threshold Detection)\r\n");
  printf("PA5: LED Indicator\r\n\r\n");

  uint16_t adc_value = 0;
  uint16_t adc_max = 0;
  uint16_t adc_min = 4095;
  uint32_t sample_count = 0;
  uint32_t last_print_time = 0;
  /* USER CODE END 2 */
```

```c
 /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* ADC 읽기 (아날로그 출력) */
      adc_value = Read_Sound_ADC();

      /* 최대/최소값 추적 */
      if (adc_value > adc_max) adc_max = adc_value;
      if (adc_value < adc_min) adc_min = adc_value;
      sample_count++;

      /* 디지털 출력 읽기 (임계값 감지) */
      if (HAL_GPIO_ReadPin(SOUND_DO_PORT, SOUND_DO_PIN) == GPIO_PIN_RESET)
      {
          /* 소리 감지됨 (Active Low) */
          HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
          sound_detected = 1;
      }
      else
      {
          HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
      }

      /* 100ms마다 결과 출력 */
      if (HAL_GetTick() - last_print_time >= 100)
      {
          last_print_time = HAL_GetTick();

          printf("ADC: %4d | ", adc_value);
          Print_Sound_Bar(adc_value);
          Print_Sound_Level(adc_value);

          if (sound_detected)
          {
              printf(" [DETECTED!]");
              sound_detected = 0;
          }
          printf("\r\n");

          /* 1초마다 통계 출력 */
          if (sample_count >= 100)
          {
              printf("--- Stats: Min=%d, Max=%d, Range=%d ---\r\n",
                     adc_min, adc_max, adc_max - adc_min);
              adc_max = 0;
              adc_min = 4095;
              sample_count = 0;
          }
      }

      HAL_Delay(10);
    /* USER CODE END WHILE */
```





### CubeMX 설정
```
Pinout:
- PA0: ADC1_IN0
- PA1: GPIO_Input
- PA5: GPIO_Output

ADC1:
- Resolution: 12-bit
- Scan Conversion Mode: Disabled
- Continuous Conversion Mode: Disabled
```

## 시리얼 출력 예시

```
============================================
  High Sensitivity Sound Sensor Test
  STM32F103 NUCLEO
============================================
PA0: Analog Output (ADC)
PA1: Digital Output (Threshold Detection)
PA5: LED Indicator

ADC:  512 | [##------------------] [Quiet]
ADC:  876 | [####----------------] [Normal]
ADC: 2341 | [###########---------] [Loud] [DETECTED!]
ADC: 3567 | [#################---] [Very Loud!] [DETECTED!]
ADC:  234 | [#-------------------] [Quiet]
--- Stats: Min=234, Max=3567, Range=3333 ---
```

## 감도 조절

### 가변저항 조절
1. 작은 드라이버로 센서 모듈의 가변저항(POT) 조절
2. **시계 방향**: 감도 증가 (작은 소리도 감지)
3. **반시계 방향**: 감도 감소 (큰 소리만 감지)

### 소프트웨어 임계값 조절
```c
#define ADC_THRESHOLD_LOW   500     // 조용한 환경 기준
#define ADC_THRESHOLD_MED   1500    // 보통 소리 기준
#define ADC_THRESHOLD_HIGH  2500    // 큰 소리 기준
```

## 응용 예제

### 박수 감지 스위치
```c
#define CLAP_THRESHOLD  2000
#define CLAP_TIMEOUT    300     // ms

uint8_t clap_count = 0;
uint32_t last_clap_time = 0;

void Check_Clap(uint16_t adc_value)
{
    uint32_t current_time = HAL_GetTick();
    
    if (adc_value > CLAP_THRESHOLD)
    {
        if (current_time - last_clap_time > 100)  // Debounce
        {
            clap_count++;
            last_clap_time = current_time;
            printf("Clap detected! Count: %d\r\n", clap_count);
        }
    }
    
    // 타임아웃 체크
    if (current_time - last_clap_time > CLAP_TIMEOUT && clap_count > 0)
    {
        if (clap_count == 2)
        {
            printf("Double clap - Toggle Light!\r\n");
            HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        }
        clap_count = 0;
    }
}
```

### 소음 레벨 로깅
```c
typedef struct {
    uint16_t min;
    uint16_t max;
    uint32_t avg;
    uint32_t count;
} SoundStats;

void Update_Stats(SoundStats *stats, uint16_t value)
{
    if (value < stats->min) stats->min = value;
    if (value > stats->max) stats->max = value;
    stats->avg = ((stats->avg * stats->count) + value) / (stats->count + 1);
    stats->count++;
}
```

## 트러블슈팅

| 증상 | 원인 | 해결방법 |
|------|------|----------|
| ADC 값이 항상 0 | ADC 초기화 오류 | ADC 캘리브레이션 확인 |
| DO 반응 없음 | 감도 설정 | 가변저항 시계방향 조절 |
| 값 불안정 | 노이즈 | 디커플링 캐패시터 추가 |
| LED 계속 켜짐 | 감도 과다 | 가변저항 반시계방향 조절 |

## 노이즈 필터링

### 이동 평균 필터
```c
#define FILTER_SIZE 10
uint16_t adc_buffer[FILTER_SIZE];
uint8_t buffer_index = 0;

uint16_t Filter_ADC(uint16_t new_value)
{
    adc_buffer[buffer_index++] = new_value;
    if (buffer_index >= FILTER_SIZE) buffer_index = 0;
    
    uint32_t sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++)
        sum += adc_buffer[i];
    
    return sum / FILTER_SIZE;
}
```

## 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [KY-037 Datasheet](https://datasheetspdf.com/pdf/1402037/Joy-IT/KY-037/1)

