# JoyStick

<img width="250" height="250" alt="134" src="https://github.com/user-attachments/assets/f7f2b0c1-67c2-4f8a-9eb3-1fbc0cf3060c" />
<br>


<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/776ad826-64e2-421f-9d18-10c543671cbf" />
<br>

### 0. RCC
<img width="1590" height="908" alt="JoyStick_001" src="https://github.com/user-attachments/assets/7c904c16-ccdd-42f5-8964-38ec31742f0a" />
<br>

### 1. ADC
#### 1.1 ADC1 기본 설정
 - Mode: Independent mode
 - Data Alignment: Right alignment
 - Scan Conversion Mode: Enabled (다중 채널용)
 - Continuous Conversion Mode: Enabled
 - Discontinuous Conversion Mode: Disabled
 - Number of Conversion: 2
 - External Trigger Conversion Source: Regular Conversion launched by software

#### 1.2 ADC 채널 설정
##### Channel 0 (PA0 - 조이스틱 X축):
 - Rank: 1
 - Channel: IN0
 - Sampling Time: 239.5 Cycles

##### Channel 1 (PA1 - 조이스틱 Y축):
 - Rank: 2
 - Channel: IN1
 - Sampling Time: 239.5 Cycles

<img width="1590" height="908" alt="JoyStick_008" src="https://github.com/user-attachments/assets/d50097cc-3b15-4190-8378-1369cddd7647" />

### 2. DMA
#### 2.1 DMA 설정
 - DMA Request: ADC1
 - Stream: DMA1 Channel1
 - Direction: Peripheral to Memory
 - Priority: Medium
 - Mode: Circular
 - Data Width: Half Word (16 bit)

<img width="1590" height="908" alt="JoyStick_002" src="https://github.com/user-attachments/assets/ad2b9640-f1a9-4ade-a354-30f18fdea471" />
<br>
<img width="1590" height="908" alt="JoyStick_003" src="https://github.com/user-attachments/assets/f4d2fcd3-085b-4550-bb63-981509525fbe" />
<br>

### 3. TIMER2
#### 3.1 타이머 설정 (TIM2)
 - Prescaler: 6399 (64MHz → 10kHz)
 - Auto-reload value: 499 (50ms 주기)
 - Counter Mode: Up
 - Auto-reload preload: Disabled
 - 
<img width="1590" height="908" alt="JoyStick_007" src="https://github.com/user-attachments/assets/c876d8c8-0f72-4a18-bf5f-90af1e1a4fa2" />
<br>

### 4.NVC
<img width="1590" height="811" alt="JoyStick_009" src="https://github.com/user-attachments/assets/6a936b1e-64b8-4aed-a805-6e9df8a8ce50" />
<br>
<img width="1590" height="811" alt="JoyStick_010" src="https://github.com/user-attachments/assets/50417a08-1165-4ecc-9406-185de267f636" />

### 5. CLOCK Fix
#### 5.1. 클럭 설정
 - ADC Clock Prescaler: PCLK2 divided by 6 (약 10.67MHz)
 - System Clock: 64MHz (일반적인 STM32F103 설정)

<img width="1590" height="908" alt="JoyStick_004" src="https://github.com/user-attachments/assets/31810fd1-7e02-4ee5-8e7e-3b4f6375ef93" />
<br>
<img width="1590" height="908" alt="JoyStick_005" src="https://github.com/user-attachments/assets/990ab66d-569e-4802-b185-2187e4f89c4c" />
<br>
<img width="1590" height="908" alt="JoyStick_006" src="https://github.com/user-attachments/assets/689ceff1-105a-4b55-ad20-611e44fd2303" />
<br>

### 6. Result
<img width="995" height="550" alt="JoyStick_011" src="https://github.com/user-attachments/assets/68c84c4d-5152-4c73-89d5-93761282f754" />
<br>

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
#define ADC_BUFFER_SIZE 2
#define FILTER_SIZE 8        // 이동평균 필터 크기
#define ADC_MAX_VALUE 4095   // 12bit ADC 최대값
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
uint16_t adc_buffer[ADC_BUFFER_SIZE];  // DMA 버퍼
uint16_t joystick_x_raw = 0;           // 조이스틱 X축 원시값
uint16_t joystick_y_raw = 0;           // 조이스틱 Y축 원시값

// 이동평균 필터를 위한 배열
uint32_t x_filter_buffer[FILTER_SIZE] = {0};
uint32_t y_filter_buffer[FILTER_SIZE] = {0};
uint8_t filter_index = 0;

// 필터링된 값
uint16_t joystick_x_filtered = 0;
uint16_t joystick_y_filtered = 0;

// 백분율로 변환된 값 (-100 ~ +100)
int16_t joystick_x_percent = 0;
int16_t joystick_y_percent = 0;

char uart_buffer[100];  // 필요시 사용할 버퍼 (현재는 printf 사용)
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
void process_joystick_data(void);
uint16_t apply_moving_average_filter(uint16_t new_value, uint32_t *filter_buffer);
int16_t convert_to_percentage(uint16_t adc_value);
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  if (ch == '\n')
    HAL_UART_Transmit (&huart2, (uint8_t*) "\r", 1, 0xFFFF);
  HAL_UART_Transmit (&huart2, (uint8_t*) &ch, 1, 0xFFFF);

  return ch;
}

/**
  * @brief  이동평균 필터 적용
  * @param  new_value: 새로운 ADC 값
  * @param  filter_buffer: 필터 버퍼 포인터
  * @retval 필터링된 값
  */
uint16_t apply_moving_average_filter(uint16_t new_value, uint32_t *filter_buffer)
{
    static uint8_t x_init = 0, y_init = 0;
    uint32_t sum = 0;

    // 필터 버퍼 구분 (X축 또는 Y축)
    if (filter_buffer == x_filter_buffer) {
        if (!x_init) {
            // 초기화: 모든 버퍼를 첫 번째 값으로 채움
            for (int i = 0; i < FILTER_SIZE; i++) {
                filter_buffer[i] = new_value;
            }
            x_init = 1;
            return new_value;
        }
    } else {
        if (!y_init) {
            // 초기화: 모든 버퍼를 첫 번째 값으로 채움
            for (int i = 0; i < FILTER_SIZE; i++) {
                filter_buffer[i] = new_value;
            }
            y_init = 1;
            return new_value;
        }
    }

    // 새로운 값을 버퍼에 추가
    filter_buffer[filter_index] = new_value;

    // 평균 계산
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += filter_buffer[i];
    }

    return (uint16_t)(sum / FILTER_SIZE);
}

/**
  * @brief  ADC 값을 백분율로 변환 (-100 ~ +100)
  * @param  adc_value: ADC 값 (0 ~ 4095)
  * @retval 백분율 값
  */
int16_t convert_to_percentage(uint16_t adc_value)
{
    // ADC 중앙값을 기준으로 -100 ~ +100으로 변환
    int16_t centered_value = (int16_t)adc_value - (ADC_MAX_VALUE / 2);
    int16_t percentage = (centered_value * 100) / (ADC_MAX_VALUE / 2);

    // 범위 제한
    if (percentage > 100) percentage = 100;
    if (percentage < -100) percentage = -100;

    return percentage;
}

/**
  * @brief  조이스틱 데이터 처리
  * @param  None
  * @retval None
  */
void process_joystick_data(void)
{
    // 원시 ADC 값 읽기
    joystick_x_raw = adc_buffer[0];  // ADC Channel 0 (PA0)
    joystick_y_raw = adc_buffer[1];  // ADC Channel 1 (PA1)

    // 이동평균 필터 적용
    joystick_x_filtered = apply_moving_average_filter(joystick_x_raw, x_filter_buffer);
    joystick_y_filtered = apply_moving_average_filter(joystick_y_raw, y_filter_buffer);

    // 필터 인덱스 업데이트 (두 축 공통 사용)
    filter_index = (filter_index + 1) % FILTER_SIZE;

    // 백분율로 변환
    joystick_x_percent = convert_to_percentage(joystick_x_filtered);
    joystick_y_percent = convert_to_percentage(joystick_y_filtered);
}

/**
  * @brief  타이머 콜백 함수 (주기적 ADC 읽기용)
  * @param  htim: 타이머 핸들
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        // 조이스틱 데이터 처리
        process_joystick_data();

        // UART로 데이터 출력 (디버깅용)
        printf("X: %d%% (%d), Y: %d%% (%d)\n",
                joystick_x_percent, joystick_x_filtered,
                joystick_y_percent, joystick_y_filtered);
    }
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
      Error_Handler();
  }

  // ADC1 핸들과 DMA 링크
  __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

  // ADC 캘리브레이션
  HAL_ADCEx_Calibration_Start(&hadc1);

  // DMA를 사용한 연속 ADC 변환 시작
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_BUFFER_SIZE);

  // 타이머 시작 (50ms 주기로 데이터 처리)
  HAL_TIM_Base_Start_IT(&htim2);

  // 시작 메시지
  printf("STM32F103 조이스틱 ADC 읽기 시작\n");
  /* USER CODE END 2 */
```

```c
    /* USER CODE BEGIN 3 */
	  HAL_Delay(10);  // 메인 루프 딜레이
  }
  /* USER CODE END 3 */
```

---

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define ADC_BUFFER_SIZE 2
#define FILTER_SIZE 8        // 이동평균 필터 크기
#define ADC_MAX_VALUE 4095   // 12bit ADC 최대값
#define DEADZONE_THRESHOLD 20  // 중앙 데드존 임계값 (%)
/* USER CODE END PD */

/* USER CODE BEGIN PV */
uint16_t adc_buffer[ADC_BUFFER_SIZE];  // DMA 버퍼
uint16_t joystick_x_raw = 0;           // 조이스틱 X축 원시값
uint16_t joystick_y_raw = 0;           // 조이스틱 Y축 원시값

// 이동평균 필터를 위한 배열
uint32_t x_filter_buffer[FILTER_SIZE] = {0};
uint32_t y_filter_buffer[FILTER_SIZE] = {0};
uint8_t filter_index = 0;

// 필터링된 값
uint16_t joystick_x_filtered = 0;
uint16_t joystick_y_filtered = 0;

// 백분율로 변환된 값 (-100 ~ +100)
int16_t joystick_x_percent = 0;
int16_t joystick_y_percent = 0;

// 방향 문자
char direction_char = 'X';
char prev_direction_char = 'X';

char uart_buffer[100];
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
void process_joystick_data(void);
uint16_t apply_moving_average_filter(uint16_t new_value, uint32_t *filter_buffer);
int16_t convert_to_percentage(uint16_t adc_value);
char get_direction_char(int16_t x_percent, int16_t y_percent);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  if (ch == '\n')
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r", 1, 0xFFFF);
  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 0xFFFF);
  return ch;
}

uint16_t apply_moving_average_filter(uint16_t new_value, uint32_t *filter_buffer)
{
    static uint8_t x_init = 0, y_init = 0;
    uint32_t sum = 0;

    if (filter_buffer == x_filter_buffer) {
        if (!x_init) {
            for (int i = 0; i < FILTER_SIZE; i++) {
                filter_buffer[i] = new_value;
            }
            x_init = 1;
            return new_value;
        }
    } else {
        if (!y_init) {
            for (int i = 0; i < FILTER_SIZE; i++) {
                filter_buffer[i] = new_value;
            }
            y_init = 1;
            return new_value;
        }
    }

    filter_buffer[filter_index] = new_value;

    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += filter_buffer[i];
    }

    return (uint16_t)(sum / FILTER_SIZE);
}

int16_t convert_to_percentage(uint16_t adc_value)
{
    int16_t centered_value = (int16_t)adc_value - (ADC_MAX_VALUE / 2);
    int16_t percentage = (centered_value * 100) / (ADC_MAX_VALUE / 2);

    if (percentage > 100) percentage = 100;
    if (percentage < -100) percentage = -100;

    return percentage;
}

/**
  * @brief  조이스틱 위치에 따른 방향 문자 반환
  * @param  x_percent: X축 백분율 (-100 ~ +100), 좌(-) / 우(+)
  * @param  y_percent: Y축 백분율 (-100 ~ +100), 후(-) / 전(+)
  * @retval 방향 문자 (W/A/S/D/X)
  */
char get_direction_char(int16_t x_percent, int16_t y_percent)
{
    int16_t abs_x = (x_percent >= 0) ? x_percent : -x_percent;
    int16_t abs_y = (y_percent >= 0) ? y_percent : -y_percent;

    // 데드존 내부 = 중앙 (X)
    if (abs_x < DEADZONE_THRESHOLD && abs_y < DEADZONE_THRESHOLD) {
        return 'X';
    }

    // Y축이 더 크면 전진/후진 우선
    if (abs_y >= abs_x) {
        if (y_percent >= DEADZONE_THRESHOLD) {
            return 'W';  // 전진
        } else if (y_percent <= -DEADZONE_THRESHOLD) {
            return 'S';  // 후진
        }
    }

    // X축이 더 크면 좌회전/우회전
    if (abs_x > abs_y) {
        if (x_percent >= DEADZONE_THRESHOLD) {
            return 'D';  // 우회전
        } else if (x_percent <= -DEADZONE_THRESHOLD) {
            return 'A';  // 좌회전
        }
    }

    return 'X';  // 기본값: 중앙
}

void process_joystick_data(void)
{
    joystick_x_raw = adc_buffer[0];
    joystick_y_raw = adc_buffer[1];

    joystick_x_filtered = apply_moving_average_filter(joystick_x_raw, x_filter_buffer);
    joystick_y_filtered = apply_moving_average_filter(joystick_y_raw, y_filter_buffer);

    filter_index = (filter_index + 1) % FILTER_SIZE;

    joystick_x_percent = convert_to_percentage(joystick_x_filtered);
    joystick_y_percent = convert_to_percentage(joystick_y_filtered);

    // 방향 문자 결정
    direction_char = get_direction_char(joystick_x_percent, joystick_y_percent);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        process_joystick_data();

        // 방향이 변경되었을 때만 출력 (또는 항상 출력하려면 조건 제거)
        if (direction_char != prev_direction_char) {
            printf("%c\n", direction_char);
            prev_direction_char = direction_char;
        }

        // 디버깅용: 항상 상세 정보 출력 (필요시 주석 해제)
        // printf("Dir: %c | X: %d%%, Y: %d%%\n", 
        //        direction_char, joystick_x_percent, joystick_y_percent);
    }
}
/* USER CODE END 0 */
```

## 주요 변경 사항

| 항목 | 설명 |
|------|------|
| **DEADZONE_THRESHOLD** | 중앙 감지 임계값 (기본 20%) |
| **get_direction_char()** | X/Y 백분율을 분석하여 방향 문자 반환 |
| **방향 출력** | 변경 시에만 출력 (채터링 방지) |

## 방향 매핑
```
        W (전진)
        ↑
        |
A (좌) ←─X─→ D (우)
        |
        ↓
        S (후진)

```




