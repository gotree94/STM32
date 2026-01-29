# NUCLEO-F767ZI CMSIS-DSP FFT Implementation

STM32 NUCLEO-F767ZI 보드의 CMSIS-DSP 라이브러리를 이용한 FFT(Fast Fourier Transform) 예제입니다.

## 📋 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F767ZI |
| MCU | STM32F767ZIT6 (ARM Cortex-M7, 216MHz, FPU, DSP) |
| IDE | STM32CubeIDE |
| 기능 | CMSIS-DSP를 이용한 FFT 분석 + ADC 신호 입력 + USART3 출력 |

## 🧮 STM32F767 DSP 기능

### Cortex-M7 DSP 특성

| 항목 | 내용 |
|------|------|
| FPU | Single & Double Precision |
| DSP Instructions | SIMD (Single Instruction Multiple Data) |
| MAC | Single-cycle 32-bit Multiply-Accumulate |
| Pipeline | 6-stage superscalar |
| Performance | 462 DMIPS @ 216MHz |

### CMSIS-DSP 라이브러리

ARM에서 제공하는 최적화된 DSP 함수 라이브러리:

| 카테고리 | 주요 함수 |
|----------|----------|
| Transform | FFT, IFFT, DCT |
| Filter | FIR, IIR, Biquad |
| Matrix | 행렬 연산 |
| Statistics | Mean, RMS, Variance, Min, Max |
| Complex Math | 복소수 연산 |
| Fast Math | sin, cos, sqrt |

## ⚙️ CubeMX 설정

### 1. RCC 설정

**Pinout & Configuration → System Core → RCC**

| 항목 | 설정값 |
|------|--------|
| HSE | **BYPASS Clock Source** |

**Clock Configuration:**

| 파라미터 | 값 |
|----------|-----|
| SYSCLK | 216 MHz |
| HCLK | 216 MHz |
| APB1 | 54 MHz |
| APB2 | 108 MHz |

### 2. FPU 설정

**Pinout & Configuration → System Core → CORTEX_M7**

| 항목 | 설정값 |
|------|--------|
| Floating Point Unit | **FPU enabled (single and double precision)** |
| ART Accelerator | Enabled |
| CPU ICache | Enabled |
| CPU DCache | Enabled |

> 💡 FPU와 Cache를 활성화하면 DSP 연산 성능이 크게 향상됩니다.

### 3. ADC1 설정 (신호 입력용)

**Pinout & Configuration → Analog → ADC1**

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| IN0 (PA0) | ✅ 활성화 | 외부 신호 입력 |
| Resolution | 12 bits | |
| Continuous Conversion | Enabled | DMA 연속 변환 |
| DMA Continuous Requests | Enabled | |

**DMA Settings:**

| 항목 | 설정값 |
|------|--------|
| DMA Request | ADC1 |
| Stream | DMA2 Stream 0 |
| Direction | Peripheral to Memory |
| Mode | Circular |
| Data Width | Half Word |

### 4. TIM2 설정 (ADC 트리거용)

**Pinout & Configuration → Timers → TIM2**

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| Clock Source | Internal Clock | |
| Prescaler | 0 | |
| Counter Period | 2250 - 1 | 108MHz / 2250 = 48kHz |
| Trigger Event Selection | Update Event | |

> 💡 48kHz 샘플링으로 최대 24kHz 주파수까지 분석 가능 (Nyquist)

### 5. USART3 설정

| 항목 | 설정값 |
|------|--------|
| Mode | Asynchronous |
| Baud Rate | 115200 |

### 6. GPIO 설정 (LED)

| 핀 | Mode | User Label |
|----|------|------------|
| PB0 | Output Push Pull | LD1 |
| PB14 | Output Push Pull | LD3 |

### 7. 코드 생성

**Ctrl+S** 또는 **Project → Generate Code**

## 📦 CMSIS-DSP 라이브러리 추가

### 방법 1: STM32CubeIDE 패키지 매니저

1. **Project → Properties → C/C++ General → Paths and Symbols**
2. **Source Location → Add Folder**
3. `Drivers/CMSIS/DSP` 추가

### 방법 2: 수동 설정

#### Include Path 추가

**Project → Properties → C/C++ Build → Settings → MCU GCC Compiler → Include paths:**

```
../Drivers/CMSIS/DSP/Include
```

#### Library 링크

**Project → Properties → C/C++ Build → Settings → MCU GCC Linker → Libraries:**

| 항목 | 값 |
|------|-----|
| Libraries (-l) | arm_cortexM7lfsp_math |
| Library search path (-L) | ../Drivers/CMSIS/DSP/Lib/GCC |

> 💡 `arm_cortexM7lfsp_math`: Cortex-M7, Little Endian, Single Precision, Float ABI Soft

#### 또는 소스 직접 포함

CMSIS-DSP 소스를 프로젝트에 직접 포함:

1. `Drivers/CMSIS/DSP/Source` 폴더를 프로젝트에 복사
2. 필요한 소스 파일만 빌드에 포함

### Define 추가

**Project → Properties → C/C++ Build → Settings → MCU GCC Compiler → Preprocessor:**

```
ARM_MATH_CM7
__FPU_PRESENT=1
```

## 💻 소스 코드

### main.c

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>
#include "arm_math.h"
#include "arm_const_structs.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define FFT_SIZE        1024                    // FFT 포인트 수 (2의 거듭제곱)
#define SAMPLE_RATE     48000                   // 샘플링 레이트 (Hz)
#define FREQ_RESOLUTION (SAMPLE_RATE / FFT_SIZE) // 주파수 분해능 (~46.875 Hz)
/* USER CODE END PD */

/* USER CODE BEGIN PV */
// ADC DMA 버퍼
volatile uint16_t adc_buffer[FFT_SIZE];
volatile uint8_t adc_conv_complete = 0;

// FFT 버퍼
float32_t fft_input[FFT_SIZE * 2];      // 복소수 입력 (Real, Imag 교대)
float32_t fft_output[FFT_SIZE * 2];     // FFT 출력
float32_t fft_magnitude[FFT_SIZE / 2];  // 크기 스펙트럼 (양의 주파수만)

// FFT 인스턴스
arm_rfft_fast_instance_f32 fft_instance;
/* USER CODE END PV */

/* USER CODE BEGIN 0 */

// printf 리다이렉션
#ifdef __GNUC__
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

/**
  * @brief  테스트 신호 생성 (사인파 합성)
  * @param  buffer: 출력 버퍼
  * @param  size: 샘플 수
  */
void Generate_Test_Signal(float32_t *buffer, uint32_t size)
{
    // 여러 주파수 성분을 가진 테스트 신호 생성
    // f1 = 1000 Hz, f2 = 2500 Hz, f3 = 5000 Hz
    
    for (uint32_t i = 0; i < size; i++)
    {
        float32_t t = (float32_t)i / SAMPLE_RATE;
        
        // 복소수 입력 형식: [Re0, Im0, Re1, Im1, ...]
        // Real FFT는 실수 입력만 사용
        buffer[i] = 1.0f * arm_sin_f32(2.0f * PI * 1000.0f * t)   // 1 kHz
                  + 0.5f * arm_sin_f32(2.0f * PI * 2500.0f * t)   // 2.5 kHz
                  + 0.3f * arm_sin_f32(2.0f * PI * 5000.0f * t);  // 5 kHz
    }
}

/**
  * @brief  ADC 데이터를 FFT 입력 형식으로 변환
  * @param  adc_data: ADC 데이터 배열
  * @param  fft_data: FFT 입력 배열
  * @param  size: 샘플 수
  */
void Convert_ADC_to_Float(uint16_t *adc_data, float32_t *fft_data, uint32_t size)
{
    // ADC 값 (0-4095)을 -1.0 ~ +1.0 범위로 정규화
    for (uint32_t i = 0; i < size; i++)
    {
        fft_data[i] = ((float32_t)adc_data[i] - 2048.0f) / 2048.0f;
    }
}

/**
  * @brief  FFT 수행 및 크기 스펙트럼 계산
  */
void Perform_FFT(void)
{
    // Real FFT 수행
    arm_rfft_fast_f32(&fft_instance, fft_input, fft_output, 0);
    
    // 크기 스펙트럼 계산 (복소수 -> 크기)
    // 출력: [Re0, Re(N/2), Re1, Im1, Re2, Im2, ...]
    arm_cmplx_mag_f32(fft_output + 2, fft_magnitude + 1, FFT_SIZE / 2 - 1);
    
    // DC 성분
    fft_magnitude[0] = fabsf(fft_output[0]);
}

/**
  * @brief  피크 주파수 찾기
  * @param  num_peaks: 찾을 피크 수
  */
void Find_Peak_Frequencies(uint32_t num_peaks)
{
    float32_t max_value;
    uint32_t max_index;
    
    // 임시 배열 복사 (원본 보존)
    float32_t temp_mag[FFT_SIZE / 2];
    arm_copy_f32(fft_magnitude, temp_mag, FFT_SIZE / 2);
    
    printf("\r\n=== Peak Frequencies ===\r\n");
    
    for (uint32_t p = 0; p < num_peaks; p++)
    {
        // 최대값 찾기
        arm_max_f32(temp_mag, FFT_SIZE / 2, &max_value, &max_index);
        
        if (max_value > 0.01f)  // 노이즈 임계값
        {
            float32_t frequency = (float32_t)max_index * FREQ_RESOLUTION;
            printf("Peak %lu: %.1f Hz (Magnitude: %.4f)\r\n", 
                   p + 1, frequency, max_value);
            
            // 찾은 피크 주변 제거 (다음 피크 찾기 위해)
            int32_t start = (int32_t)max_index - 5;
            int32_t end = (int32_t)max_index + 5;
            if (start < 0) start = 0;
            if (end > FFT_SIZE / 2) end = FFT_SIZE / 2;
            
            for (int32_t i = start; i < end; i++)
            {
                temp_mag[i] = 0;
            }
        }
    }
}

/**
  * @brief  스펙트럼 출력 (간단한 ASCII 그래프)
  */
void Print_Spectrum_ASCII(void)
{
    printf("\r\n=== Frequency Spectrum (0 - 10 kHz) ===\r\n");
    
    // 최대값으로 정규화
    float32_t max_mag;
    uint32_t max_idx;
    arm_max_f32(fft_magnitude, FFT_SIZE / 2, &max_mag, &max_idx);
    
    // 10kHz까지만 표시 (인덱스 계산)
    uint32_t max_bin = (uint32_t)(10000.0f / FREQ_RESOLUTION);
    if (max_bin > FFT_SIZE / 2) max_bin = FFT_SIZE / 2;
    
    // 주파수 대역별로 그룹화하여 출력
    uint32_t bins_per_line = 10;  // 약 469Hz 간격
    
    for (uint32_t i = 0; i < max_bin; i += bins_per_line)
    {
        // 해당 대역의 평균 계산
        float32_t avg = 0;
        uint32_t count = 0;
        for (uint32_t j = i; j < i + bins_per_line && j < max_bin; j++)
        {
            avg += fft_magnitude[j];
            count++;
        }
        avg /= count;
        
        // 주파수 표시
        float32_t freq = (float32_t)i * FREQ_RESOLUTION;
        printf("%5.0f Hz |", freq);
        
        // 바 그래프
        uint32_t bar_length = (uint32_t)((avg / max_mag) * 40);
        for (uint32_t b = 0; b < bar_length; b++)
        {
            printf("#");
        }
        printf("\r\n");
    }
}

/**
  * @brief  통계 정보 출력
  */
void Print_Statistics(void)
{
    float32_t mean, rms, std_dev, max_val, min_val;
    uint32_t max_idx, min_idx;
    
    // 입력 신호 통계
    arm_mean_f32(fft_input, FFT_SIZE, &mean);
    arm_rms_f32(fft_input, FFT_SIZE, &rms);
    arm_std_f32(fft_input, FFT_SIZE, &std_dev);
    arm_max_f32(fft_input, FFT_SIZE, &max_val, &max_idx);
    arm_min_f32(fft_input, FFT_SIZE, &min_val, &min_idx);
    
    printf("\r\n=== Input Signal Statistics ===\r\n");
    printf("Mean:     %+.6f\r\n", mean);
    printf("RMS:      %.6f\r\n", rms);
    printf("Std Dev:  %.6f\r\n", std_dev);
    printf("Max:      %+.6f (sample %lu)\r\n", max_val, max_idx);
    printf("Min:      %+.6f (sample %lu)\r\n", min_val, min_idx);
}

/* USER CODE END 0 */

int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_TIM2_Init();
    MX_USART3_UART_Init();

    /* USER CODE BEGIN 2 */
    printf("\r\n============================================\r\n");
    printf("  NUCLEO-F767ZI CMSIS-DSP FFT Demo\r\n");
    printf("  System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    printf("============================================\r\n");
    printf("FFT Size: %d points\r\n", FFT_SIZE);
    printf("Sample Rate: %d Hz\r\n", SAMPLE_RATE);
    printf("Frequency Resolution: %.2f Hz\r\n", (float)FREQ_RESOLUTION);
    printf("Max Detectable Freq: %d Hz\r\n", SAMPLE_RATE / 2);
    printf("============================================\r\n\n");

    // FFT 초기화
    arm_status status = arm_rfft_fast_init_f32(&fft_instance, FFT_SIZE);
    if (status != ARM_MATH_SUCCESS)
    {
        printf("FFT Init Failed! Error: %d\r\n", status);
        Error_Handler();
    }
    printf("FFT Instance initialized successfully.\r\n\n");

    // LED ON - 준비 완료
    HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    uint32_t iteration = 0;
    while (1)
    {
        printf("\r\n>>> FFT Analysis #%lu <<<\r\n", ++iteration);

        // 방법 1: 테스트 신호 사용 (ADC 없이 테스트)
        Generate_Test_Signal(fft_input, FFT_SIZE);

        // 방법 2: ADC 데이터 사용 (실제 신호 분석)
        // HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, FFT_SIZE);
        // HAL_TIM_Base_Start(&htim2);
        // while (!adc_conv_complete);
        // adc_conv_complete = 0;
        // HAL_TIM_Base_Stop(&htim2);
        // Convert_ADC_to_Float((uint16_t*)adc_buffer, fft_input, FFT_SIZE);

        // FFT 수행 시간 측정
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        uint32_t start_tick = HAL_GetTick();
        
        Perform_FFT();
        
        uint32_t elapsed = HAL_GetTick() - start_tick;
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

        printf("FFT Computation Time: %lu ms\r\n", elapsed);

        // 결과 출력
        Print_Statistics();
        Find_Peak_Frequencies(5);
        Print_Spectrum_ASCII();

        // 다음 분석까지 대기
        HAL_Delay(3000);

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
```

### ADC DMA 콜백 (main.c - USER CODE BEGIN 4)

```c
/* USER CODE BEGIN 4 */

/**
  * @brief  ADC 변환 완료 콜백 (DMA)
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        adc_conv_complete = 1;
    }
}

/* USER CODE END 4 */
```

## 🔧 윈도우 함수 적용 (선택사항)

스펙트럼 누설(Spectral Leakage)을 줄이기 위해 윈도우 함수 적용:

### Hanning Window 구현

```c
/* USER CODE BEGIN PV */
float32_t hanning_window[FFT_SIZE];
/* USER CODE END PV */

/**
  * @brief  Hanning 윈도우 생성
  */
void Generate_Hanning_Window(float32_t *window, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        window[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (size - 1)));
    }
}

/**
  * @brief  윈도우 함수 적용
  */
void Apply_Window(float32_t *signal, float32_t *window, uint32_t size)
{
    arm_mult_f32(signal, window, signal, size);
}
```

### 사용 방법

```c
// 초기화 시
Generate_Hanning_Window(hanning_window, FFT_SIZE);

// FFT 수행 전
Apply_Window(fft_input, hanning_window, FFT_SIZE);
Perform_FFT();
```

## 📊 다양한 FFT 크기

| FFT 크기 | 주파수 분해능 (48kHz) | 시간 분해능 | 용도 |
|----------|----------------------|-------------|------|
| 256 | 187.5 Hz | 5.3 ms | 빠른 응답 |
| 512 | 93.75 Hz | 10.7 ms | 일반 |
| **1024** | **46.875 Hz** | **21.3 ms** | **균형** |
| 2048 | 23.44 Hz | 42.7 ms | 높은 분해능 |
| 4096 | 11.72 Hz | 85.3 ms | 정밀 분석 |

## 🔄 동작 방식

```
┌─────────────────────────────────────────────────────────────┐
│                    Signal Input                              │
│         (Test Signal or ADC from PA0)                       │
│                          │                                   │
│                          ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              Window Function (Optional)              │    │
│  │                   Hanning, etc.                      │    │
│  └─────────────────────────────────────────────────────┘    │
│                          │                                   │
│                          ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              CMSIS-DSP FFT                           │    │
│  │           arm_rfft_fast_f32()                        │    │
│  │                                                      │    │
│  │   Time Domain ──────► Frequency Domain              │    │
│  │   x[n] ─────────────► X[k]                          │    │
│  └─────────────────────────────────────────────────────┘    │
│                          │                                   │
│                          ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              Magnitude Calculation                   │    │
│  │           arm_cmplx_mag_f32()                        │    │
│  │                                                      │    │
│  │   |X[k]| = √(Re² + Im²)                             │    │
│  └─────────────────────────────────────────────────────┘    │
│                          │                                   │
│                          ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              Peak Detection & Analysis               │    │
│  │           arm_max_f32()                              │    │
│  └─────────────────────────────────────────────────────┘    │
│                          │                                   │
│                          ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              USART3 Output                           │    │
│  │         Spectrum, Peaks, Statistics                  │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## 📺 예상 출력

```
============================================
  NUCLEO-F767ZI CMSIS-DSP FFT Demo
  System Clock: 216 MHz
============================================
FFT Size: 1024 points
Sample Rate: 48000 Hz
Frequency Resolution: 46.88 Hz
Max Detectable Freq: 24000 Hz
============================================

FFT Instance initialized successfully.

>>> FFT Analysis #1 <<<
FFT Computation Time: 1 ms

=== Input Signal Statistics ===
Mean:     +0.000012
RMS:      0.583095
Std Dev:  0.583095
Max:      +1.299987 (sample 512)
Min:      -1.299543 (sample 0)

=== Peak Frequencies ===
Peak 1: 1000.0 Hz (Magnitude: 512.0000)
Peak 2: 2500.0 Hz (Magnitude: 256.0000)
Peak 3: 5000.0 Hz (Magnitude: 153.6000)

=== Frequency Spectrum (0 - 10 kHz) ===
    0 Hz |##
  469 Hz |####
  938 Hz |########################################
 1406 Hz |####
 1875 Hz |###
 2344 Hz |####################
 2813 Hz |###
 3281 Hz |##
 3750 Hz |##
 4219 Hz |##
 4688 Hz |############
 5156 Hz |##
 5625 Hz |#
...
```

## 🔌 ADC 입력 회로 (실제 신호 분석 시)

```
                    ┌─────────────┐
 Signal Input ──┬───┤   C = 100nF ├───┐
                │   └─────────────┘   │
                │                     │
               ┌┴┐                   ┌┴┐
               │ │ R1                │ │ R2
               │ │ 10k               │ │ 10k
               └┬┘                   └┬┘
                │                     │
                └──────────┬──────────┘
                           │
                           ├────────► PA0 (ADC1_IN0)
                           │
                          ┌┴┐
                          │ │ R3 = 10k
                          └┬┘
                           │
                          ─┴─ 3.3V
                          ───
                           ─

DC Bias = 3.3V × (R2/(R1+R2)) = 1.65V (ADC 중간값)
```

## 🔍 트러블슈팅

### FFT 초기화 실패

- [ ] FFT_SIZE가 2의 거듭제곱인지 확인 (64, 128, 256, 512, 1024, 2048, 4096)
- [ ] CMSIS-DSP 라이브러리 링크 확인
- [ ] `ARM_MATH_CM7` define 확인

### 링크 에러 (undefined reference)

- [ ] 라이브러리 파일 경로 확인
- [ ] `arm_cortexM7lfsp_math` 라이브러리 추가 확인
- [ ] FPU 설정과 라이브러리 일치 확인

### 결과가 비정상적인 경우

- [ ] 입력 데이터 정규화 확인 (-1.0 ~ +1.0 권장)
- [ ] 샘플링 레이트와 FFT 크기 확인
- [ ] DC 오프셋 제거 확인

### 실행 속도가 느린 경우

- [ ] FPU 활성화 확인
- [ ] ICache/DCache 활성화 확인
- [ ] 최적화 레벨 확인 (-O2 또는 -O3)

## 📁 프로젝트 구조

```
07_DSP_FFT/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f7xx_hal_conf.h
│   │   └── stm32f7xx_it.h
│   └── Src/
│       ├── main.c                     # 메인 로직 + FFT
│       ├── stm32f7xx_hal_msp.c
│       ├── stm32f7xx_it.c
│       └── system_stm32f7xx.c
├── Drivers/
│   ├── CMSIS/
│   │   ├── DSP/
│   │   │   ├── Include/               # arm_math.h 등
│   │   │   └── Lib/
│   │   │       └── GCC/
│   │   │           └── libarm_cortexM7lfsp_math.a
│   │   └── Include/
│   └── STM32F7xx_HAL_Driver/
├── 07_DSP_FFT.ioc
└── README.md
```

## 📚 CMSIS-DSP 주요 함수 레퍼런스

### FFT 함수

| 함수 | 설명 |
|------|------|
| `arm_rfft_fast_init_f32()` | Real FFT 초기화 |
| `arm_rfft_fast_f32()` | Real FFT 수행 |
| `arm_cfft_f32()` | Complex FFT 수행 |

### 복소수 연산

| 함수 | 설명 |
|------|------|
| `arm_cmplx_mag_f32()` | 복소수 크기 계산 |
| `arm_cmplx_mag_squared_f32()` | 크기의 제곱 |

### 통계 함수

| 함수 | 설명 |
|------|------|
| `arm_mean_f32()` | 평균 |
| `arm_rms_f32()` | RMS (Root Mean Square) |
| `arm_std_f32()` | 표준편차 |
| `arm_max_f32()` | 최대값 및 인덱스 |
| `arm_min_f32()` | 최소값 및 인덱스 |

### 기본 연산

| 함수 | 설명 |
|------|------|
| `arm_mult_f32()` | 요소별 곱셈 |
| `arm_add_f32()` | 요소별 덧셈 |
| `arm_scale_f32()` | 스케일링 |
| `arm_copy_f32()` | 배열 복사 |

## 📚 참고 자료

- [CMSIS-DSP Documentation](https://arm-software.github.io/CMSIS_5/DSP/html/index.html)
- [CMSIS-DSP GitHub](https://github.com/ARM-software/CMSIS-DSP)
- [STM32F767ZI Reference Manual (RM0410)](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [AN4841: Digital signal processing for STM32 microcontrollers using CMSIS](https://www.st.com/resource/en/application_note/an4841-digital-signal-processing-for-stm32-microcontrollers-using-cmsis-stmicroelectronics.pdf)

## 📝 라이선스

This project is licensed under the MIT License.

## ✍️ Author

Created for STM32 embedded systems learning and development.
