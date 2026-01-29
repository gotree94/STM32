# NUCLEO-F767ZI ADC Internal Temperature Sensor

STM32 NUCLEO-F767ZI 보드의 ADC를 이용한 내부 온도 센서(CPU 온도) 측정 예제입니다.

## 📋 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F767ZI |
| MCU | STM32F767ZIT6 (ARM Cortex-M7, 216MHz) |
| IDE | STM32CubeIDE |
| 기능 | ADC로 내부 온도 센서 읽기 + USART3 출력 |

## 🌡️ 내부 온도 센서 개요

### STM32F767 내부 온도 센서 특성

| 항목 | 값 |
|------|-----|
| ADC 채널 | **ADC1_IN18** (내부 연결) |
| 측정 범위 | -40°C ~ +125°C |
| 정확도 | ±3°C (보정 없이) |
| 샘플링 시간 | 최소 10μs 권장 |
| VREF+ | 3.3V |

### 온도 계산 공식

STM32F7 Reference Manual 기준:

```
Temperature (°C) = ((V_SENSE - V_25) / Avg_Slope) + 25

V_SENSE = ADC_RAW × (VREF+ / 4096)   // 12-bit ADC
V_25 = 0.76V (25°C에서의 전압, 대략값)
Avg_Slope = 2.5 mV/°C (대략값)
```

### Calibration 값 사용 (더 정확)

STM32F767은 공장에서 보정된 값을 내부 메모리에 저장:

| 주소 | 값 | 설명 |
|------|-----|------|
| 0x1FF0F44C | TS_CAL1 | 30°C, VDDA=3.3V에서의 ADC 값 |
| 0x1FF0F44E | TS_CAL2 | 110°C, VDDA=3.3V에서의 ADC 값 |

```
Temperature = 30 + ((TS_CAL2_TEMP - TS_CAL1_TEMP) / (TS_CAL2 - TS_CAL1)) × (ADC_RAW - TS_CAL1)
            = 30 + (80 / (TS_CAL2 - TS_CAL1)) × (ADC_RAW - TS_CAL1)
```

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
| APB2 Prescaler | /2 (108 MHz) |
| ADC Clock | APB2 / 4 = 27 MHz |

### 2. ADC1 설정

**Pinout & Configuration → Analog → ADC1**

#### 2.1 Mode

| 항목 | 설정값 |
|------|--------|
| Temperature Sensor Channel | ✅ **체크** |

> 💡 체크하면 자동으로 IN18 (내부 온도 센서)이 활성화됩니다.

#### 2.2 Parameter Settings - ADC_Settings

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| Clock Prescaler | PCLK2 divided by 4 | ADC Clock ≤ 36MHz |
| Resolution | 12 bits | 0~4095 |
| Data Alignment | Right alignment | |
| Scan Conversion Mode | Disabled | 단일 채널 |
| Continuous Conversion Mode | Disabled | 폴링 방식 |
| Discontinuous Conversion Mode | Disabled | |
| DMA Continuous Requests | Disabled | |
| End of Conversion Selection | EOC flag at the end of single channel conversion | |

#### 2.3 Parameter Settings - ADC_Regular_ConversionMode

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| Number Of Conversion | 1 | |
| External Trigger Conversion Source | Regular Conversion launched by software | |
| Rank 1 - Channel | **Temperature Sensor Channel** | |
| Rank 1 - Sampling Time | **480 Cycles** | 온도 센서는 긴 샘플링 필요 |

> ⚠️ **중요**: 온도 센서는 최소 10μs 샘플링 시간 필요. 480 Cycles @ 27MHz = ~17.8μs

### 3. GPIO 설정 (LED)

| 핀 | Mode | User Label |
|----|------|------------|
| PB0 | Output Push Pull | LD1 |
| PB14 | Output Push Pull | LD3 |

### 4. USART3 설정

**Connectivity → USART3**

| 항목 | 설정값 |
|------|--------|
| Mode | Asynchronous |
| Baud Rate | 115200 |

### 5. 코드 생성

**Ctrl+S** 또는 **Project → Generate Code**

## 💻 소스 코드

### main.c

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
// 온도 센서 Calibration 값 주소 (STM32F767)
#define TS_CAL1_ADDR    ((uint16_t*)0x1FF0F44C)  // 30°C에서의 ADC 값
#define TS_CAL2_ADDR    ((uint16_t*)0x1FF0F44E)  // 110°C에서의 ADC 값
#define TS_CAL1_TEMP    30                        // Calibration 온도 1
#define TS_CAL2_TEMP    110                       // Calibration 온도 2
#define VREFINT_CAL     ((uint16_t*)0x1FF0F44A)  // VREFINT calibration 값
/* USER CODE END PD */

/* USER CODE BEGIN PV */
uint16_t ts_cal1, ts_cal2;
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
  * @brief  ADC 값 읽기 (폴링 방식)
  * @retval ADC 변환 값 (12-bit)
  */
uint16_t Read_ADC(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}

/**
  * @brief  Calibration 값을 이용한 정확한 온도 계산
  * @param  adc_value: ADC 변환 값
  * @retval 온도 (°C)
  */
float Calculate_Temperature_Calibrated(uint16_t adc_value)
{
    // Temperature = 30 + (80 / (TS_CAL2 - TS_CAL1)) × (ADC_RAW - TS_CAL1)
    float temperature;
    temperature = (float)(TS_CAL2_TEMP - TS_CAL1_TEMP) / (float)(ts_cal2 - ts_cal1);
    temperature *= (float)(adc_value - ts_cal1);
    temperature += TS_CAL1_TEMP;
    return temperature;
}

/**
  * @brief  공식을 이용한 온도 계산 (Calibration 없이)
  * @param  adc_value: ADC 변환 값
  * @retval 온도 (°C)
  */
float Calculate_Temperature_Formula(uint16_t adc_value)
{
    // V_SENSE = ADC × (3.3 / 4096)
    // Temperature = ((V_SENSE - 0.76) / 0.0025) + 25
    float v_sense = (float)adc_value * 3.3f / 4096.0f;
    float temperature = ((v_sense - 0.76f) / 0.0025f) + 25.0f;
    return temperature;
}

/* USER CODE END 0 */

int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART3_UART_Init();
    MX_ADC1_Init();

    /* USER CODE BEGIN 2 */
    // Calibration 값 읽기
    ts_cal1 = *TS_CAL1_ADDR;
    ts_cal2 = *TS_CAL2_ADDR;

    printf("\r\n============================================\r\n");
    printf("  NUCLEO-F767ZI ADC Temperature Sensor Demo\r\n");
    printf("  System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    printf("============================================\r\n");
    printf("Calibration Values:\r\n");
    printf("  TS_CAL1 (30C):  %u\r\n", ts_cal1);
    printf("  TS_CAL2 (110C): %u\r\n", ts_cal2);
    printf("============================================\r\n\n");
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    uint32_t count = 0;
    while (1)
    {
        // ADC 값 읽기
        uint16_t adc_raw = Read_ADC();

        // 온도 계산 (두 가지 방식)
        float temp_cal = Calculate_Temperature_Calibrated(adc_raw);
        float temp_formula = Calculate_Temperature_Formula(adc_raw);

        // 결과 출력
        printf("[%5lu] ADC: %4u | Temp(Cal): %6.2f C | Temp(Formula): %6.2f C\r\n",
               count++, adc_raw, temp_cal, temp_formula);

        // 온도에 따른 LED 제어 (예: 40°C 이상이면 LD3 ON)
        if (temp_cal > 40.0f)
        {
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);   // Red ON
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET); // Green OFF
        }
        else
        {
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); // Red OFF
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);   // Green ON
        }

        HAL_Delay(1000);

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
```

## 🔄 동작 방식

```
┌─────────────────────────────────────────────────────────────┐
│                Internal Temperature Sensor                   │
│                         │                                    │
│                         ▼                                    │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                    ADC1 CH18                         │    │
│  │              (내부 온도 센서 채널)                    │    │
│  └─────────────────────────────────────────────────────┘    │
│                         │                                    │
│                         ▼                                    │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              12-bit ADC Conversion                   │    │
│  │                  0 ~ 4095                            │    │
│  └─────────────────────────────────────────────────────┘    │
│                         │                                    │
│                         ▼                                    │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              Temperature Calculation                 │    │
│  │                                                      │    │
│  │   Using Calibration:                                 │    │
│  │   T = 30 + (80/(CAL2-CAL1)) × (ADC - CAL1)          │    │
│  │                                                      │    │
│  │   Using Formula:                                     │    │
│  │   V = ADC × 3.3/4096                                │    │
│  │   T = (V - 0.76)/0.0025 + 25                        │    │
│  └─────────────────────────────────────────────────────┘    │
│                         │                                    │
│                         ▼                                    │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              USART3 Output + LED Control             │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## 📊 ADC 샘플링 시간 계산

### 온도 센서 최소 샘플링 시간

Reference Manual 권장: **최소 10μs**

```
ADC Clock = APB2 / 4 = 108MHz / 4 = 27MHz
1 ADC Cycle = 1 / 27MHz = 37ns

샘플링 시간 = Cycles × 37ns

480 Cycles: 480 × 37ns = 17.8μs ✅ (권장)
144 Cycles: 144 × 37ns = 5.3μs  ❌ (너무 짧음)
```

### 사용 가능한 Sampling Time

| Cycles | 시간 (27MHz 기준) | 온도 센서 사용 |
|--------|-------------------|----------------|
| 3 | 0.11μs | ❌ |
| 15 | 0.56μs | ❌ |
| 28 | 1.04μs | ❌ |
| 56 | 2.07μs | ❌ |
| 84 | 3.11μs | ❌ |
| 112 | 4.15μs | ❌ |
| 144 | 5.33μs | ❌ |
| **480** | **17.78μs** | ✅ 권장 |

## 🔧 DMA 방식 (연속 측정)

### CubeMX 추가 설정

**ADC1 → DMA Settings:**

| 항목 | 설정값 |
|------|--------|
| DMA Request | ADC1 |
| Stream | DMA2 Stream 0 |
| Direction | Peripheral to Memory |
| Priority | Low |
| Mode | Circular |
| Data Width | Half Word (16-bit) |

**ADC1 → Parameter Settings:**

| 항목 | 변경값 |
|------|--------|
| Continuous Conversion Mode | **Enabled** |
| DMA Continuous Requests | **Enabled** |

### DMA 코드

```c
/* USER CODE BEGIN PV */
volatile uint16_t adc_dma_buffer[10];  // DMA 버퍼
volatile uint8_t adc_conv_complete = 0;
/* USER CODE END PV */

/* USER CODE BEGIN 2 */
// DMA 시작
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 10);
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        adc_conv_complete = 1;
    }
}

// 평균값 계산
uint16_t Get_Average_ADC(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < 10; i++)
    {
        sum += adc_dma_buffer[i];
    }
    return (uint16_t)(sum / 10);
}
/* USER CODE END 4 */
```

## 📺 예상 출력

```
============================================
  NUCLEO-F767ZI ADC Temperature Sensor Demo
  System Clock: 96 MHz
============================================
Calibration Values:
  TS_CAL1 (30C):  951
  TS_CAL2 (110C): 1206
============================================

[    0] ADC:  943 | Temp(Cal):  27.49 C | Temp(Formula):  24.90 C
[    1] ADC:  947 | Temp(Cal):  28.75 C | Temp(Formula):  26.19 C
[    2] ADC:  948 | Temp(Cal):  29.06 C | Temp(Formula):  26.51 C
[    3] ADC:  949 | Temp(Cal):  29.37 C | Temp(Formula):  26.83 C
[    4] ADC:  953 | Temp(Cal):  30.63 C | Temp(Formula):  28.12 C
...
```

> 💡 MCU 동작 중에는 코어 온도가 주변 온도보다 높게 측정됩니다 (정상).

## 🔍 트러블슈팅

### ADC 값이 0 또는 4095로 고정되는 경우

- [ ] Temperature Sensor Channel 활성화 확인
- [ ] ADC Clock이 36MHz 이하인지 확인
- [ ] `HAL_ADC_Start()` 호출 확인

### 온도가 비정상적으로 높거나 낮은 경우

- [ ] Sampling Time이 480 Cycles인지 확인 (10μs 이상 필요)
- [ ] Calibration 주소가 STM32F767에 맞는지 확인
- [ ] VDDA가 3.3V인지 확인

### Calibration 값이 0인 경우

- [ ] Calibration 주소 확인 (STM32F7 시리즈마다 다름)
- [ ] Flash 읽기 권한 확인

### ADC 변환이 완료되지 않는 경우

- [ ] `HAL_ADC_PollForConversion()` 타임아웃 확인
- [ ] ADC 초기화 순서 확인

## 📁 프로젝트 구조

```
05_ADC_Temperature/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f7xx_hal_conf.h
│   │   └── stm32f7xx_it.h
│   └── Src/
│       ├── main.c                    # 메인 로직
│       ├── stm32f7xx_hal_msp.c       # ADC MSP Init
│       ├── stm32f7xx_it.c
│       └── system_stm32f7xx.c
├── Drivers/
│   ├── CMSIS/
│   └── STM32F7xx_HAL_Driver/
├── 05_ADC_Temperature.ioc
└── README.md
```

## 📚 STM32F7 Calibration 주소 참고

| 시리즈 | TS_CAL1 | TS_CAL2 | CAL1 온도 | CAL2 온도 |
|--------|---------|---------|-----------|-----------|
| STM32F767 | 0x1FF0F44C | 0x1FF0F44E | 30°C | 110°C |
| STM32F746 | 0x1FF0F44C | 0x1FF0F44E | 30°C | 110°C |
| STM32F4xx | 0x1FFF7A2C | 0x1FFF7A2E | 30°C | 110°C |

## 📚 참고 자료

- [NUCLEO-F767ZI User Manual (UM1974)](https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf)
- [STM32F767ZI Reference Manual (RM0410) - ADC](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F767ZI Datasheet - Temperature Sensor](https://www.st.com/resource/en/datasheet/stm32f767zi.pdf)

## 📝 라이선스

This project is licensed under the MIT License.

## ✍️ Author

Created for STM32 embedded systems learning and development.
