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

# JoyStick (W/A/S/D/X)

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

---

# JoyStick (W/A/S/D/X) - serial_bridge.py

```
///////////////////////////////////////
// Serial Bridge (python serial_bridge.py)
/////////////////////////////////////////
import serial
import serial.tools.list_ports
import time
import sys

# ============== 설정 ==============
INPUT_PORT = "COM3"      # STM32 연결 포트 (수신)
OUTPUT_PORT = "COM4"     # 출력 포트 (송신)
BAUD_RATE = 115200       # 통신 속도
# ==================================

def list_available_ports():
    """사용 가능한 시리얼 포트 목록 출력"""
    ports = serial.tools.list_ports.comports()
    print("\n[사용 가능한 시리얼 포트]")
    if not ports:
        print("  포트를 찾을 수 없습니다.")
    for port in ports:
        print(f"  {port.device}: {port.description}")
    print()

def serial_bridge():
    """시리얼 포트 브릿지 메인 함수"""
   
    list_available_ports()
   
    input_serial = None
    output_serial = None
   
    try:
        # 입력 포트 연결 (STM32)
        print(f"입력 포트 연결 중: {INPUT_PORT}")
        input_serial = serial.Serial(
            port=INPUT_PORT,
            baudrate=BAUD_RATE,
            timeout=0.1
        )
        print(f"  ✓ {INPUT_PORT} 연결 성공")
       
        # 출력 포트 연결
        print(f"출력 포트 연결 중: {OUTPUT_PORT}")
        output_serial = serial.Serial(
            port=OUTPUT_PORT,
            baudrate=BAUD_RATE,
            timeout=0.1
        )
        print(f"  ✓ {OUTPUT_PORT} 연결 성공")
       
        print("\n" + "="*50)
        print("시리얼 브릿지 시작")
        print(f"  {INPUT_PORT} → {OUTPUT_PORT}")
        print("종료: Ctrl+C")
        print("="*50 + "\n")
       
        valid_commands = {'W', 'A', 'S', 'D', 'X'}
       
        while True:
            # 입력 포트에서 데이터 수신
            if input_serial.in_waiting > 0:
                data = input_serial.readline().decode('utf-8', errors='ignore').strip()
               
                if data:
                    # 유효한 명령어인지 확인
                    if data in valid_commands:
                        # 출력 포트로 전송
                        output_serial.write(f"{data}\n".encode('utf-8'))
                        print(f"[전송] {data}")
                    else:
                        # 디버그 메시지는 화면에만 표시
                        print(f"[수신] {data}")
           
            time.sleep(0.001)  # CPU 부하 감소
           
    except serial.SerialException as e:
        print(f"\n[오류] 시리얼 포트 오류: {e}")
    except KeyboardInterrupt:
        print("\n\n프로그램 종료")
    finally:
        if input_serial and input_serial.is_open:
            input_serial.close()
            print(f"{INPUT_PORT} 닫힘")
        if output_serial and output_serial.is_open:
            output_serial.close()
            print(f"{OUTPUT_PORT} 닫힘")

if __name__ == "__main__":
    serial_bridge()
```

---

# Commands: W/A/S/D/X + B(Button)


<img width="643" height="549" alt="JOYSTICK_B_005" src="https://github.com/user-attachments/assets/821d4489-5200-4b9d-8b02-aab50e7d5dac" />
<br>
<img width="806" height="888" alt="JOYSTICK_B_001" src="https://github.com/user-attachments/assets/bbeecb40-bbad-4a1f-8f51-5d5067fc258f" />
<br>
<img width="827" height="777" alt="JOYSTICK_B_002" src="https://github.com/user-attachments/assets/95ec5554-a895-4152-ac57-e1a153a9c217" />
<br>
<img width="654" height="484" alt="JOYSTICK_B_003" src="https://github.com/user-attachments/assets/728e2025-42a0-4d24-b636-5e09264636bf" />
<br>
<img width="723" height="550" alt="JOYSTICK_B_004" src="https://github.com/user-attachments/assets/12582a79-351f-4b36-a0ec-2604a10cd62a" />
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
#define FILTER_SIZE 8
#define ADC_MAX_VALUE 4095
#define DEADZONE_THRESHOLD 20

// 스위치 디바운싱 설정
#define DEBOUNCE_TIME_MS 200

// 캘리브레이션 샘플 수
#define CALIBRATION_SAMPLES 32
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
uint16_t adc_buffer[ADC_BUFFER_SIZE];
uint16_t joystick_x_raw = 0;
uint16_t joystick_y_raw = 0;

// X축 필터 변수
uint32_t x_filter_buffer[FILTER_SIZE] = {0};
uint8_t x_filter_index = 0;
uint8_t x_filter_init = 0;

// Y축 필터 변수
uint32_t y_filter_buffer[FILTER_SIZE] = {0};
uint8_t y_filter_index = 0;
uint8_t y_filter_init = 0;

uint16_t joystick_x_filtered = 0;
uint16_t joystick_y_filtered = 0;

// 캘리브레이션 값 (중립 위치)
uint16_t joystick_x_center = ADC_MAX_VALUE / 2;  // 기본값 2048
uint16_t joystick_y_center = ADC_MAX_VALUE / 2;  // 기본값 2048
uint8_t calibration_done = 0;

int16_t joystick_x_percent = 0;
int16_t joystick_y_percent = 0;

char direction_char = 'X';
char prev_direction_char = 'X';

// 스위치 관련 변수
volatile uint8_t switch_pressed = 0;
volatile uint32_t last_switch_time = 0;

char uart_buffer[100];
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
void process_joystick_data(void);
void calibrate_joystick(void);
uint16_t apply_moving_average_x(uint16_t new_value);
uint16_t apply_moving_average_y(uint16_t new_value);
int16_t convert_to_percentage_calibrated(uint16_t adc_value, uint16_t center);
char get_direction_char(int16_t x_percent, int16_t y_percent);
/* USER CODE END PFP */
```

```c
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

// X축 전용 이동 평균 필터
uint16_t apply_moving_average_x(uint16_t new_value)
{
    uint32_t sum = 0;

    // 첫 호출 시 버퍼 초기화
    if (!x_filter_init) {
        for (int i = 0; i < FILTER_SIZE; i++) {
            x_filter_buffer[i] = new_value;
        }
        x_filter_init = 1;
        return new_value;
    }

    // 현재 인덱스에 새 값 저장
    x_filter_buffer[x_filter_index] = new_value;
    x_filter_index = (x_filter_index + 1) % FILTER_SIZE;

    // 평균 계산
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += x_filter_buffer[i];
    }

    return (uint16_t)(sum / FILTER_SIZE);
}

// Y축 전용 이동 평균 필터
uint16_t apply_moving_average_y(uint16_t new_value)
{
    uint32_t sum = 0;

    // 첫 호출 시 버퍼 초기화
    if (!y_filter_init) {
        for (int i = 0; i < FILTER_SIZE; i++) {
            y_filter_buffer[i] = new_value;
        }
        y_filter_init = 1;
        return new_value;
    }

    // 현재 인덱스에 새 값 저장
    y_filter_buffer[y_filter_index] = new_value;
    y_filter_index = (y_filter_index + 1) % FILTER_SIZE;

    // 평균 계산
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += y_filter_buffer[i];
    }

    return (uint16_t)(sum / FILTER_SIZE);
}

// 캘리브레이션된 퍼센트 변환
int16_t convert_to_percentage_calibrated(uint16_t adc_value, uint16_t center)
{
    int16_t centered_value = (int16_t)adc_value - (int16_t)center;
    int16_t percentage;

    if (centered_value >= 0) {
        // 양의 방향: center ~ ADC_MAX_VALUE
        uint16_t range = ADC_MAX_VALUE - center;
        if (range == 0) range = 1;  // 0으로 나누기 방지
        percentage = (centered_value * 100) / range;
    } else {
        // 음의 방향: 0 ~ center
        uint16_t range = center;
        if (range == 0) range = 1;  // 0으로 나누기 방지
        percentage = (centered_value * 100) / range;
    }

    // 범위 제한
    if (percentage > 100) percentage = 100;
    if (percentage < -100) percentage = -100;

    return percentage;
}

// 조이스틱 캘리브레이션 (시작 시 호출)
void calibrate_joystick(void)
{
    uint32_t x_sum = 0;
    uint32_t y_sum = 0;

    printf("Calibrating joystick... Keep neutral position!\n");
    HAL_Delay(500);  // 안정화 대기

    // 여러 샘플 수집
    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        x_sum += adc_buffer[0];
        y_sum += adc_buffer[1];
        HAL_Delay(10);
    }

    joystick_x_center = x_sum / CALIBRATION_SAMPLES;
    joystick_y_center = y_sum / CALIBRATION_SAMPLES;
    calibration_done = 1;

    printf("Calibration done! Center: X=%d, Y=%d\n",
           joystick_x_center, joystick_y_center);
}

// 방향 문자 결정
char get_direction_char(int16_t x_percent, int16_t y_percent)
{
    int16_t abs_x = (x_percent >= 0) ? x_percent : -x_percent;
    int16_t abs_y = (y_percent >= 0) ? y_percent : -y_percent;

    // 데드존 내에 있으면 중립
    if (abs_x < DEADZONE_THRESHOLD && abs_y < DEADZONE_THRESHOLD) {
        return 'X';
    }

    // Y축이 더 크거나 같으면 W/S
    if (abs_y >= abs_x) {
        if (y_percent >= DEADZONE_THRESHOLD) {
            return 'W';
        } else if (y_percent <= -DEADZONE_THRESHOLD) {
            return 'S';
        }
    }

    // X축이 더 크면 D/A
    if (abs_x > abs_y) {
        if (x_percent >= DEADZONE_THRESHOLD) {
            return 'D';
        } else if (x_percent <= -DEADZONE_THRESHOLD) {
            return 'A';
        }
    }

    return 'X';
}

// 조이스틱 데이터 처리
void process_joystick_data(void)
{
    // 캘리브레이션 완료 전에는 처리하지 않음
    if (!calibration_done) return;

    joystick_x_raw = adc_buffer[0];
    joystick_y_raw = adc_buffer[1];

    // 각 축별 독립적인 필터 적용
    joystick_x_filtered = apply_moving_average_x(joystick_x_raw);
    joystick_y_filtered = apply_moving_average_y(joystick_y_raw);

    // 캘리브레이션된 중심값 기준으로 퍼센트 계산
    joystick_x_percent = convert_to_percentage_calibrated(joystick_x_filtered, joystick_x_center);
    joystick_y_percent = convert_to_percentage_calibrated(joystick_y_filtered, joystick_y_center);

    direction_char = get_direction_char(joystick_x_percent, joystick_y_percent);
}

/**
  * @brief  외부 인터럽트 콜백 (조이스틱 스위치)
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t current_time = HAL_GetTick();

    // PA4 (조이스틱 스위치) 또는 PC13 (Blue Button)
    if (GPIO_Pin == GPIO_PIN_4 || GPIO_Pin == GPIO_PIN_13) {
        // 디바운싱: 마지막 입력 후 일정 시간 경과했는지 확인
        if ((current_time - last_switch_time) > DEBOUNCE_TIME_MS) {
            switch_pressed = 1;
            last_switch_time = current_time;
        }
    }
}

// 타이머 인터럽트 콜백
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        process_joystick_data();

        // 스위치가 눌렸으면 'B' 전송 (Button)
        if (switch_pressed) {
            printf("B\n");
            switch_pressed = 0;
        }
        // 방향이 변경되었을 때만 출력
        else if (direction_char != prev_direction_char) {
            printf("%c\n", direction_char);
            prev_direction_char = direction_char;
        }
    }
}
/* USER CODE END 0 */
```


```c
  /* USER CODE BEGIN 2 */
  if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
      Error_Handler();
  }

  __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_BUFFER_SIZE);

  // ADC DMA 안정화 대기 후 캘리브레이션 수행
  HAL_Delay(100);
  calibrate_joystick();

  // 타이머 인터럽트 시작
  HAL_TIM_Base_Start_IT(&htim2);

  printf("Joystick Control Started\n");
  printf("Commands: W/A/S/D/X + B(Button)\n");
  /* USER CODE END 2 */
```

```c
    /* USER CODE BEGIN 3 */
    HAL_Delay(10);  // 메인 루프 딜레이
  }
  /* USER CODE END 3 */
```


## 출력 화면

```
Calibrating joystick... Keep neutral position!
Calibration done! Center: X=3200, Y=3217
Joystick Control Started
Commands: W/A/S/D/X + B(Button)
W
X
S
X
A
X
D
X
B
W
X
S
X
A
X
D
X
W
X
S
X
A
X
D
X
```
---

# Commands: W/A/S/D/X + B(Button) + serial_bridge.py

```python
import serial
import serial.tools.list_ports
import time

INPUT_PORT = "COM3"
OUTPUT_PORT = "COM4"
BAUD_RATE = 115200

def list_available_ports():
    ports = serial.tools.list_ports.comports()
    print("\n[사용 가능한 시리얼 포트]")
    for port in ports:
        print(f"  {port.device}: {port.description}")
    print()

def serial_bridge():
    list_available_ports()
    
    input_serial = None
    output_serial = None
    
    try:
        print(f"입력 포트 연결 중: {INPUT_PORT}")
        input_serial = serial.Serial(INPUT_PORT, BAUD_RATE, timeout=0.1)
        print(f"  ✓ {INPUT_PORT} 연결 성공")
        
        print(f"출력 포트 연결 중: {OUTPUT_PORT}")
        output_serial = serial.Serial(OUTPUT_PORT, BAUD_RATE, timeout=0.1)
        print(f"  ✓ {OUTPUT_PORT} 연결 성공")
        
        print("\n" + "="*50)
        print("시리얼 브릿지 시작")
        print(f"  {INPUT_PORT} → {OUTPUT_PORT}")
        print("명령어: W(전진), S(후진), A(좌), D(우), X(정지), B(버튼)")
        print("종료: Ctrl+C")
        print("="*50 + "\n")
        
        # B(Button) 추가
        valid_commands = {'W', 'A', 'S', 'D', 'X', 'B'}
        
        while True:
            if input_serial.in_waiting > 0:
                data = input_serial.readline().decode('utf-8', errors='ignore').strip()
                
                if data and data in valid_commands:
                    output_serial.write(f"{data}\n".encode('utf-8'))
                    
                    # 버튼 입력 시 다르게 표시
                    if data == 'B':
                        print(f"[버튼] ●")
                    else:
                        print(f"[전송] {data}")
            
            time.sleep(0.001)
            
    except serial.SerialException as e:
        print(f"\n[오류] 시리얼 포트 오류: {e}")
    except KeyboardInterrupt:
        print("\n\n프로그램 종료")
    finally:
        if input_serial and input_serial.is_open:
            input_serial.close()
        if output_serial and output_serial.is_open:
            output_serial.close()

if __name__ == "__main__":
    serial_bridge()
```
