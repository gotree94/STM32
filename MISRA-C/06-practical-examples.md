# 06. 실습 예제

[← 이전: 준수 및 문서화](05-compliance.md) | [메인으로](../README.md) | [다음: 교육 커리큘럼 →](07-curriculum.md)

---

## 📚 학습 목표

이 장을 완료하면 다음을 수행할 수 있습니다:
- MISRA-C 규칙 위반 코드 식별 및 수정
- STM32 프로젝트에서 MISRA 준수 코드 작성
- Cppcheck로 실제 프로젝트 검사
- 일반적인 임베디드 패턴의 MISRA 준수 구현

---

## 6.1 위반 코드 식별 및 수정 실습

### 실습 1: 기본 위반 사항 수정

다음 코드에서 MISRA 위반을 찾고 수정하세요.

#### 문제 코드

```c
/* exercise1_violations.c */
#include <stdio.h>
#include <stdlib.h>

#define MAX(a,b) a > b ? a : b

int data[10];

int process(int *arr, int size)
{
    int sum = 0;
    int i;
    
    for (i = 0; i <= size; i++)
        sum += arr[i];
    
    if (size > 5)
        printf("Large array\n");
        return sum;
    
    return 0;
}

int main()
{
    int *dynamic = malloc(sizeof(int) * 10);
    int result;
    
    for (int i = 0; i < 10; i++)
        data[i] = MAX(i, 5);
    
    result = process(data, 10);
    
    free(dynamic);
    return 0;
}
```

#### 위반 목록

| 라인 | 규칙 | 설명 |
|------|------|------|
| 1 | Rule 21.6 | stdio.h 사용 |
| 2 | Rule 21.3 | stdlib.h (malloc/free) 사용 |
| 4 | Rule 20.7 | 매크로 매개변수 괄호 누락 |
| 14 | Rule 18.1 | 배열 범위 초과 (i <= size) |
| 14-15 | Rule 15.6 | for 복합문 미사용 |
| 17-19 | Rule 15.6 | if 복합문 미사용 |
| 17-19 | - | 들여쓰기 오류로 인한 논리 오류 |
| 24 | Rule 8.4 | main 함수 프로토타입 |
| 29-30 | Rule 15.6 | for 복합문 미사용 |

#### 수정된 코드

```c
/* exercise1_compliant.c */
#include <stdint.h>
#include <stdbool.h>

/* Rule 20.7: 매크로 매개변수 괄호 */
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/* 정적 배열 사용 (Rule 21.3 준수) */
#define ARRAY_SIZE 10U
static int32_t data[ARRAY_SIZE];

/* 함수 프로토타입 (Rule 8.4) */
static int32_t process(const int32_t *arr, uint32_t size);

static int32_t process(const int32_t *arr, uint32_t size)
{
    int32_t sum = 0;
    
    /* Rule 15.6: 복합문 사용 */
    /* Rule 18.1: i < size (범위 내 접근) */
    for (uint32_t i = 0U; i < size; i++) {
        sum += arr[i];
    }
    
    /* Rule 15.6: 복합문 사용 */
    if (size > 5U) {
        /* Rule 21.6: printf 대신 커스텀 함수 또는 제거 */
        /* debug_print("Large array"); */
    }
    
    return sum;
}

/* Rule 8.4: main 프로토타입 명시 */
int main(void)
{
    int32_t result;
    
    /* Rule 15.6: 복합문 사용 */
    for (uint32_t i = 0U; i < ARRAY_SIZE; i++) {
        data[i] = MAX((int32_t)i, 5);
    }
    
    result = process(data, ARRAY_SIZE);
    
    /* Rule 17.7: 반환값 사용 */
    (void)result;  /* 의도적 미사용 명시 */
    
    return 0;
}
```

---

### 실습 2: 타입 변환 규칙

#### 문제 코드

```c
/* exercise2_type_violations.c */
#include <stdint.h>

void type_examples(void)
{
    /* 다양한 타입 변환 위반 */
    uint32_t u32 = 0x12345678U;
    uint16_t u16 = u32;              /* ? */
    
    int32_t s32 = -100;
    uint32_t u32_2 = s32;            /* ? */
    
    int8_t s8 = 100;
    uint8_t u8 = 200U;
    int16_t result = s8 + u8;        /* ? */
    
    float f = 3.14f;
    int32_t i = f;                   /* ? */
    
    char c = 'A';
    int32_t ascii = c + 1;           /* ? */
}
```

#### 수정된 코드

```c
/* exercise2_compliant.c */
#include <stdint.h>

void type_examples(void)
{
    /* Rule 10.3: Narrowing 변환 - 명시적 캐스팅 */
    uint32_t u32 = 0x12345678U;
    uint16_t u16 = (uint16_t)(u32 & 0xFFFFU);  /* 의도 명확히 */
    
    /* Rule 10.3: 부호 변환 - 값 검증 후 변환 */
    int32_t s32 = -100;
    uint32_t u32_2;
    if (s32 >= 0) {
        u32_2 = (uint32_t)s32;
    } else {
        u32_2 = 0U;  /* 또는 에러 처리 */
    }
    
    /* Rule 10.4: 부호 혼합 연산 - 같은 타입으로 통일 */
    int8_t s8 = 100;
    uint8_t u8 = 200U;
    int16_t result = (int16_t)s8 + (int16_t)u8;  /* 둘 다 int16_t로 */
    
    /* Rule 10.3: 부동소수점 → 정수 */
    float f = 3.14f;
    int32_t i = (int32_t)f;  /* 명시적 캐스팅 */
    
    /* Rule 10.1: char 산술 연산 */
    char c = 'A';
    int32_t ascii = (int32_t)(uint8_t)c + 1;  /* 명시적 변환 */
    
    /* 컴파일러 경고 방지 */
    (void)u16;
    (void)u32_2;
    (void)result;
    (void)i;
    (void)ascii;
}
```

---

## 6.2 STM32 MISRA 준수 프로젝트 구조

### 프로젝트 디렉토리 구조

```
stm32_misra_project/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── gpio_driver.h
│   │   └── uart_driver.h
│   └── Src/
│       ├── main.c
│       ├── gpio_driver.c
│       └── uart_driver.c
├── Drivers/
│   └── STM32F1xx_HAL_Driver/    (Adopted Code)
├── misra/
│   ├── suppressions.txt
│   ├── misra_rules.txt
│   └── GEP.md
└── Makefile
```

### MISRA 준수 GPIO 드라이버

```c
/* gpio_driver.h */
#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/* GPIO 핀 정의 - Rule 7.2: u 접미사 */
#define GPIO_PIN_0   ((uint16_t)0x0001U)
#define GPIO_PIN_1   ((uint16_t)0x0002U)
#define GPIO_PIN_13  ((uint16_t)0x2000U)

/* GPIO 상태 열거형 */
typedef enum {
    GPIO_STATE_RESET = 0,
    GPIO_STATE_SET   = 1
} GPIO_State_t;

/* GPIO 모드 열거형 */
typedef enum {
    GPIO_MODE_INPUT  = 0,
    GPIO_MODE_OUTPUT = 1
} GPIO_Mode_t;

/* 함수 프로토타입 - Rule 8.2 */
void GPIO_Init(uint16_t pin, GPIO_Mode_t mode);
void GPIO_WritePin(uint16_t pin, GPIO_State_t state);
GPIO_State_t GPIO_ReadPin(uint16_t pin);
void GPIO_TogglePin(uint16_t pin);

#endif /* GPIO_DRIVER_H */
```

```c
/* gpio_driver.c */
#include "gpio_driver.h"
#include "stm32f1xx_hal.h"

/* 
 * Rule 8.7: 내부 함수는 static
 * HAL 래퍼 함수들 - Adopted Code 격리
 */

void GPIO_Init(uint16_t pin, GPIO_Mode_t mode)
{
    GPIO_InitTypeDef gpio_config;
    
    /* Rule 9.1: 구조체 초기화 */
    gpio_config.Pin = pin;
    gpio_config.Pull = GPIO_NOPULL;
    gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
    
    /* Rule 16.4: switch에 default */
    switch (mode) {
        case GPIO_MODE_INPUT:
            gpio_config.Mode = GPIO_MODE_INPUT;
            break;
        case GPIO_MODE_OUTPUT:
            gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
            break;
        default:
            /* 예상치 못한 모드 - 기본값 사용 */
            gpio_config.Mode = GPIO_MODE_INPUT;
            break;
    }
    
    /* HAL 함수 호출 (Adopted Code) */
    HAL_GPIO_Init(GPIOC, &gpio_config);
}

void GPIO_WritePin(uint16_t pin, GPIO_State_t state)
{
    GPIO_PinState hal_state;
    
    /* Rule 10.3: 명시적 변환 */
    if (state == GPIO_STATE_SET) {
        hal_state = GPIO_PIN_SET;
    } else {
        hal_state = GPIO_PIN_RESET;
    }
    
    HAL_GPIO_WritePin(GPIOC, pin, hal_state);
}

GPIO_State_t GPIO_ReadPin(uint16_t pin)
{
    GPIO_State_t result;
    GPIO_PinState hal_state;
    
    hal_state = HAL_GPIO_ReadPin(GPIOC, pin);
    
    /* Rule 10.3: 명시적 변환 */
    if (hal_state == GPIO_PIN_SET) {
        result = GPIO_STATE_SET;
    } else {
        result = GPIO_STATE_RESET;
    }
    
    return result;
}

void GPIO_TogglePin(uint16_t pin)
{
    HAL_GPIO_TogglePin(GPIOC, pin);
}
```

### MISRA 준수 UART 드라이버

```c
/* uart_driver.h */
#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/* 에러 코드 정의 */
typedef enum {
    UART_OK      = 0,
    UART_ERROR   = -1,
    UART_TIMEOUT = -2,
    UART_BUSY    = -3
} UART_Status_t;

/* 함수 프로토타입 */
void UART_Init(uint32_t baudrate);
UART_Status_t UART_Transmit(const uint8_t *data, uint16_t size, uint32_t timeout);
UART_Status_t UART_Receive(uint8_t *data, uint16_t size, uint32_t timeout);

/* 디버그 출력 (조건부 컴파일) */
#ifdef DEBUG_ENABLED
void UART_DebugPrint(const char *message);
void UART_DebugPrintInt(const char *label, int32_t value);
#endif

#endif /* UART_DRIVER_H */
```

```c
/* uart_driver.c */
#include "uart_driver.h"
#include "stm32f1xx_hal.h"
#include <string.h>

/* 모듈 내부 변수 - Rule 8.7 */
static UART_HandleTypeDef huart2;

void UART_Init(uint32_t baudrate)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = baudrate;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    
    /* Rule 17.7: 반환값 확인 */
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        /* 에러 처리 */
    }
}

UART_Status_t UART_Transmit(const uint8_t *data, uint16_t size, uint32_t timeout)
{
    UART_Status_t result = UART_ERROR;
    
    /* Rule 18.1: NULL 포인터 검사 */
    if (data != NULL) {
        HAL_StatusTypeDef hal_status;
        
        /* 
         * const 캐스트 필요 - HAL 함수 시그니처 제한
         * Deviation: HAL API 호환성
         */
        hal_status = HAL_UART_Transmit(&huart2, (uint8_t *)data, size, timeout);
        
        /* Rule 16.4: switch default */
        switch (hal_status) {
            case HAL_OK:
                result = UART_OK;
                break;
            case HAL_TIMEOUT:
                result = UART_TIMEOUT;
                break;
            case HAL_BUSY:
                result = UART_BUSY;
                break;
            default:
                result = UART_ERROR;
                break;
        }
    }
    
    return result;
}

UART_Status_t UART_Receive(uint8_t *data, uint16_t size, uint32_t timeout)
{
    UART_Status_t result = UART_ERROR;
    
    if (data != NULL) {
        HAL_StatusTypeDef hal_status;
        
        hal_status = HAL_UART_Receive(&huart2, data, size, timeout);
        
        switch (hal_status) {
            case HAL_OK:
                result = UART_OK;
                break;
            case HAL_TIMEOUT:
                result = UART_TIMEOUT;
                break;
            case HAL_BUSY:
                result = UART_BUSY;
                break;
            default:
                result = UART_ERROR;
                break;
        }
    }
    
    return result;
}

#ifdef DEBUG_ENABLED
/* 디버그 함수 - Rule 21.6 Deviation */
void UART_DebugPrint(const char *message)
{
    if (message != NULL) {
        uint16_t len = (uint16_t)strlen(message);
        (void)UART_Transmit((const uint8_t *)message, len, 1000U);
    }
}

void UART_DebugPrintInt(const char *label, int32_t value)
{
    char buffer[32];
    int32_t len;
    
    if (label != NULL) {
        /* snprintf 사용 - Rule 21.6 Deviation (디버그 전용) */
        len = snprintf(buffer, sizeof(buffer), "%s: %ld\r\n", label, (long)value);
        if ((len > 0) && (len < (int32_t)sizeof(buffer))) {
            (void)UART_Transmit((const uint8_t *)buffer, (uint16_t)len, 1000U);
        }
    }
}
#endif
```

---

## 6.3 LED 점멸 프로젝트 (MISRA 준수)

### main.c

```c
/* main.c - MISRA C:2023 준수 LED 점멸 */
#include "stm32f1xx_hal.h"
#include "gpio_driver.h"
#include <stdint.h>

/* Rule 7.2: 상수에 접미사 */
#define LED_PIN         GPIO_PIN_13
#define BLINK_DELAY_MS  500U

/* 함수 프로토타입 */
static void SystemClock_Config(void);
static void Error_Handler(void);

int main(void)
{
    /* HAL 초기화 */
    if (HAL_Init() != HAL_OK) {
        Error_Handler();
    }
    
    /* 시스템 클럭 설정 */
    SystemClock_Config();
    
    /* GPIO 클럭 활성화 */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /* LED 핀 초기화 */
    GPIO_Init(LED_PIN, GPIO_MODE_OUTPUT);
    
    /* Rule 15.6: 무한 루프 복합문 */
    /* Rule 14.3: volatile로 상수 조건 회피 */
    for (;;) {
        GPIO_TogglePin(LED_PIN);
        HAL_Delay(BLINK_DELAY_MS);
    }
    
    /* Rule 2.1: 도달 불가 코드 - 의도적 (무한 루프) */
    /* return 0; */
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef rcc_osc = {0};
    RCC_ClkInitTypeDef rcc_clk = {0};
    
    /* HSI 사용 설정 */
    rcc_osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    rcc_osc.HSIState = RCC_HSI_ON;
    rcc_osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    rcc_osc.PLL.PLLState = RCC_PLL_NONE;
    
    if (HAL_RCC_OscConfig(&rcc_osc) != HAL_OK) {
        Error_Handler();
    }
    
    /* 클럭 설정 */
    rcc_clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                        RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    rcc_clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    rcc_clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    rcc_clk.APB1CLKDivider = RCC_HCLK_DIV1;
    rcc_clk.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&rcc_clk, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

static void Error_Handler(void)
{
    /* 에러 발생 시 무한 루프 */
    __disable_irq();
    for (;;) {
        /* 대기 */
    }
}
```

---

## 6.4 Cppcheck 검사 실행

### Suppression 파일

```
# misra/suppressions.txt

# HAL 라이브러리 (Adopted Code) 제외
*:Drivers/STM32F1xx_HAL_Driver/*

# 디버그 코드 예외
misra-c2012-21.6:Core/Src/uart_driver.c

# 시스템 코드 예외
misra-c2012-2.1:Core/Src/main.c:45  # 무한 루프 후 도달 불가
```

### Makefile MISRA 타겟

```makefile
# Makefile

CPPCHECK = cppcheck
CPPCHECK_FLAGS = --addon=misra --std=c99 --enable=all
CPPCHECK_SUPPRESS = --suppressions-list=misra/suppressions.txt
CPPCHECK_INC = -I Core/Inc -I Drivers/CMSIS/Include

.PHONY: misra misra-report

misra:
	$(CPPCHECK) $(CPPCHECK_FLAGS) $(CPPCHECK_SUPPRESS) $(CPPCHECK_INC) Core/Src/

misra-report:
	$(CPPCHECK) $(CPPCHECK_FLAGS) $(CPPCHECK_SUPPRESS) $(CPPCHECK_INC) \
		--xml --xml-version=2 Core/Src/ 2> misra_report.xml
```

### 검사 실행

```bash
# MISRA 검사
make misra

# 보고서 생성
make misra-report
```

---

## 6.5 종합 실습 과제

### 과제: 온도 센서 드라이버 작성

다음 요구사항을 만족하는 MISRA 준수 온도 센서 드라이버를 작성하세요.

**요구사항:**
1. ADC로 온도 센서 값 읽기
2. 섭씨 온도로 변환
3. 임계값 초과 시 경고
4. MISRA C:2023 Required 규칙 100% 준수

**인터페이스:**

```c
/* temp_sensor.h */
#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TEMP_OK = 0,
    TEMP_ERROR = -1,
    TEMP_WARNING_HIGH = 1,
    TEMP_WARNING_LOW = 2
} TempStatus_t;

void TempSensor_Init(void);
TempStatus_t TempSensor_Read(int16_t *temperature_celsius);
void TempSensor_SetThreshold(int16_t low, int16_t high);

#endif
```

<details>
<summary>예시 답안 보기</summary>

```c
/* temp_sensor.c */
#include "temp_sensor.h"
#include "stm32f1xx_hal.h"

/* 모듈 상수 */
#define ADC_RESOLUTION      4096U    /* 12-bit ADC */
#define VREF_MV             3300U    /* 3.3V reference */
#define TEMP_SENSOR_V25     1430U    /* 1.43V at 25°C (mV) */
#define TEMP_SENSOR_SLOPE   43U      /* 4.3mV/°C * 10 */

/* 모듈 변수 */
static ADC_HandleTypeDef hadc1;
static int16_t threshold_low = -40;
static int16_t threshold_high = 85;

void TempSensor_Init(void)
{
    ADC_ChannelConfTypeDef config = {0};
    
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1U;
    
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        /* 에러 처리 */
    }
    
    config.Channel = ADC_CHANNEL_TEMPSENSOR;
    config.Rank = ADC_REGULAR_RANK_1;
    config.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    
    if (HAL_ADC_ConfigChannel(&hadc1, &config) != HAL_OK) {
        /* 에러 처리 */
    }
}

TempStatus_t TempSensor_Read(int16_t *temperature_celsius)
{
    TempStatus_t status = TEMP_ERROR;
    
    /* Rule 18.1: NULL 검사 */
    if (temperature_celsius != NULL) {
        uint32_t adc_value;
        uint32_t voltage_mv;
        int32_t temp_calc;
        
        /* ADC 변환 시작 */
        if (HAL_ADC_Start(&hadc1) == HAL_OK) {
            /* 변환 완료 대기 */
            if (HAL_ADC_PollForConversion(&hadc1, 100U) == HAL_OK) {
                /* ADC 값 읽기 */
                adc_value = HAL_ADC_GetValue(&hadc1);
                
                /* 전압 계산 (mV) */
                voltage_mv = (adc_value * VREF_MV) / ADC_RESOLUTION;
                
                /* 온도 계산: T = ((V25 - Vsense) / Slope) + 25 */
                temp_calc = (int32_t)TEMP_SENSOR_V25 - (int32_t)voltage_mv;
                temp_calc = (temp_calc * 10) / (int32_t)TEMP_SENSOR_SLOPE;
                temp_calc = temp_calc + 25;
                
                /* Rule 10.3: 범위 검증 후 변환 */
                if ((temp_calc >= INT16_MIN) && (temp_calc <= INT16_MAX)) {
                    *temperature_celsius = (int16_t)temp_calc;
                    status = TEMP_OK;
                    
                    /* 임계값 검사 */
                    if (*temperature_celsius > threshold_high) {
                        status = TEMP_WARNING_HIGH;
                    } else if (*temperature_celsius < threshold_low) {
                        status = TEMP_WARNING_LOW;
                    } else {
                        /* Rule 15.7: else 절 */
                    }
                }
            }
            
            (void)HAL_ADC_Stop(&hadc1);
        }
    }
    
    return status;
}

void TempSensor_SetThreshold(int16_t low, int16_t high)
{
    /* Rule 15.6: 복합문 */
    if (low < high) {
        threshold_low = low;
        threshold_high = high;
    }
}
```

</details>

---

## 📝 학습 확인 문제

### Q1. 다음 코드의 MISRA 위반을 모두 찾으세요.

```c
#define SQR(x) x*x

int calc(int n) {
    int arr[10];
    for (int i = 0; i <= 10; i++)
        arr[i] = SQR(i+1);
    return arr[0];
}
```

<details>
<summary>정답 보기</summary>

1. **Rule 20.7**: 매크로 괄호 누락 → `#define SQR(x) ((x)*(x))`
2. **Rule 15.6**: for 복합문 미사용 → `{ }` 추가
3. **Rule 18.1**: 배열 범위 초과 (i <= 10) → `i < 10`
4. **Rule 8.4**: 함수 프로토타입 누락

</details>

---

## 📚 다음 학습

다음 장에서는 전체 교육 커리큘럼과 평가 방법을 확인합니다.

[다음: 07. 교육 커리큘럼 →](07-curriculum.md)
