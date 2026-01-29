# NUCLEO-F767ZI Timer TimeBase Interrupt

STM32 NUCLEO-F767ZI 보드의 타이머를 이용한 주기적 인터럽트(TimeBase) 예제입니다.

## 📋 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F767ZI |
| MCU | STM32F767ZIT6 (ARM Cortex-M7, 216MHz) |
| IDE | STM32CubeIDE |
| 기능 | TIM6를 이용한 1초 주기 LED 토글 + USART3 상태 출력 |

## 🔧 하드웨어 구성

### LED 핀 매핑

| LED | 색상 | GPIO |
|-----|------|------|
| LD1 | Green | PB0 |
| LD2 | Blue | PB7 |
| LD3 | Red | PB14 |

### USART3 (ST-LINK VCP)

| 기능 | GPIO |
|------|------|
| TX | PD8 |
| RX | PD9 |

## ⏱️ 타이머 개요

### STM32F767 타이머 종류

| 타이머 | 종류 | 비트 | 주요 용도 |
|--------|------|------|----------|
| TIM1, TIM8 | Advanced | 16-bit | PWM, 모터 제어 |
| TIM2, TIM5 | General Purpose | **32-bit** | 범용, 긴 주기 |
| TIM3, TIM4, TIM9~14 | General Purpose | 16-bit | 범용 |
| **TIM6, TIM7** | **Basic** | 16-bit | **TimeBase 전용** |

> 💡 이 예제에서는 가장 단순한 **TIM6 (Basic Timer)**를 사용합니다.

### 타이머 클럭 계산

NUCLEO-F767ZI (216MHz 설정 시):

```
APB1 Timer Clock = APB1 × 2 = 54MHz × 2 = 108MHz
```

| 파라미터 | 공식 |
|----------|------|
| Timer Clock | APB1 × 2 = 108 MHz |
| Counter Clock | Timer Clock / (Prescaler + 1) |
| Update Period | (ARR + 1) / Counter Clock |

### 1초 주기 설정 예시

```
목표: 1초 (1Hz) 주기

Timer Clock = 108 MHz
Prescaler = 10800 - 1 = 10799
Counter Clock = 108MHz / 10800 = 10 kHz (0.1ms per tick)

ARR = 10000 - 1 = 9999
Update Period = 10000 / 10kHz = 1초
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
| APB1 Prescaler | /4 (54 MHz) |
| APB1 Timer Clock | **108 MHz** (자동 ×2) |

### 2. TIM6 설정

**Pinout & Configuration → Timers → TIM6**

#### 2.1 Mode

| 항목 | 설정값 |
|------|--------|
| Activated | ✅ **체크** |

#### 2.2 Parameter Settings

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| Prescaler (PSC) | **10799** | 108MHz / 10800 = 10kHz |
| Counter Mode | Up | |
| Counter Period (ARR) | **9999** | 10000 / 10kHz = 1초 |
| Auto-reload preload | Enable | ARR 버퍼링 |

#### 2.3 NVIC Settings

| 항목 | 설정값 |
|------|--------|
| TIM6 global interrupt | ✅ **체크** |

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

/* USER CODE BEGIN PV */
volatile uint32_t tim6_counter = 0;
volatile uint8_t tim6_flag = 0;
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

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
     printf("\r\n========================================\r\n");
     printf("  NUCLEO-F767ZI TIM6 TimeBase Demo\r\n");
     printf("  System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
     printf("  Timer Period: 1 second\r\n");
     printf("========================================\r\n\n");

     // 타이머 인터럽트 시작
     HAL_TIM_Base_Start_IT(&htim6);

     printf("Timer started. LEDs will toggle every 1 second.\r\n\n");
     /* USER CODE END 2 */

     /* Infinite loop */
     /* USER CODE BEGIN WHILE */
     while (1)
     {
         if (tim6_flag)
         {
             tim6_flag = 0;

             printf("[%5lu] Timer Tick - LD1: %s, LD2: %s, LD3: %s\r\n",
                    tim6_counter,
                    HAL_GPIO_ReadPin(LD1_GPIO_Port, LD1_Pin) ? "ON " : "OFF",
					HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) ? "ON " : "OFF",
                    HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin) ? "ON " : "OFF");
         }

         /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
```

### Timer Callback 함수 (main.c의 USER CODE BEGIN 4)

```c
/* USER CODE BEGIN 4 */

/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim: TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        // LED 토글
        HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

        // 카운터 증가 및 플래그 설정
        tim6_counter++;
        tim6_flag = 1;
    }
}

/* USER CODE END 4 */
```

### stm32f7xx_it.c (자동 생성됨)

```c
/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
    /* USER CODE BEGIN TIM6_DAC_IRQn 0 */

    /* USER CODE END TIM6_DAC_IRQn 0 */
    HAL_TIM_IRQHandler(&htim6);
    /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

    /* USER CODE END TIM6_DAC_IRQn 1 */
}
```

## 🔄 동작 방식

```
┌─────────────────────────────────────────────────────────────┐
│                    Timer Clock (108 MHz)                     │
│                            │                                 │
│                            ▼                                 │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              Prescaler (PSC = 10799)                 │    │
│  │              108MHz / 10800 = 10kHz                  │    │
│  └─────────────────────────────────────────────────────┘    │
│                            │                                 │
│                            ▼                                 │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                   Counter                            │    │
│  │              0 → 1 → 2 → ... → 9999 → 0             │    │
│  │                    (10kHz 클럭)                      │    │
│  └─────────────────────────────────────────────────────┘    │
│                            │                                 │
│                            ▼ (ARR = 9999 도달 시)           │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              Update Event (UEV)                      │    │
│  │              주기: 10000 / 10kHz = 1초               │    │
│  └─────────────────────────────────────────────────────┘    │
│                            │                                 │
│                            ▼                                 │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              TIM6_DAC_IRQHandler()                   │    │
│  │                       │                              │    │
│  │                       ▼                              │    │
│  │           HAL_TIM_PeriodElapsedCallback()            │    │
│  │                - Toggle LEDs                         │    │
│  │                - Set flag                            │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## 📊 다양한 주기 설정 예시

### Timer Clock = 108 MHz 기준

| 목표 주기 | Prescaler (PSC) | Period (ARR) | Counter Clock |
|-----------|-----------------|--------------|---------------|
| 100μs | 107 | 99 | 1 MHz |
| 1ms | 1079 | 99 | 100 kHz |
| 10ms | 10799 | 99 | 10 kHz |
| 100ms | 10799 | 999 | 10 kHz |
| 500ms | 10799 | 4999 | 10 kHz |
| **1초** | **10799** | **9999** | **10 kHz** |
| 2초 | 10799 | 19999 | 10 kHz |
| 5초 | 10799 | 49999 | 10 kHz |

> ⚠️ **주의**: ARR은 16-bit이므로 최대 65535. 더 긴 주기가 필요하면 Prescaler를 늘리거나 32-bit 타이머(TIM2, TIM5) 사용

### 주기 계산 공식

```c
// 원하는 주기(초)로 PSC, ARR 계산
#define TIMER_CLOCK     108000000UL  // 108 MHz
#define DESIRED_FREQ    1            // 1 Hz (1초 주기)

// 방법 1: PSC 고정, ARR 계산
#define PSC_VALUE       10800 - 1    // Counter Clock = 10 kHz
#define ARR_VALUE       (TIMER_CLOCK / (PSC_VALUE + 1) / DESIRED_FREQ) - 1

// 방법 2: 원하는 주기(ms)로 계산
uint32_t calculate_arr(uint32_t period_ms, uint32_t psc)
{
    uint32_t counter_clock = TIMER_CLOCK / (psc + 1);
    return (counter_clock * period_ms / 1000) - 1;
}
```

## 🔧 런타임 주기 변경

```c
// 타이머 주기를 동적으로 변경
void set_timer_period_ms(TIM_HandleTypeDef *htim, uint32_t period_ms)
{
    uint32_t psc = htim->Instance->PSC;
    uint32_t timer_clock = 108000000UL;  // APB1 Timer Clock
    uint32_t counter_clock = timer_clock / (psc + 1);
    uint32_t arr = (counter_clock * period_ms / 1000) - 1;

    __HAL_TIM_SET_AUTORELOAD(htim, arr);
}

// 사용 예
set_timer_period_ms(&htim6, 500);   // 500ms로 변경
set_timer_period_ms(&htim6, 2000);  // 2초로 변경
```

## 📺 예상 출력

```

========================================
  NUCLEO-F767ZI TIM6 TimeBase Demo
  System Clock: 96 MHz
  Timer Period: 1 second
========================================

Timer started. LEDs will toggle every 1 second.

[    1] Timer Tick - LD1: ON , LD2: ON , LD3: ON
[    2] Timer Tick - LD1: OFF, LD2: OFF, LD3: OFF
[    3] Timer Tick - LD1: ON , LD2: ON , LD3: ON
[    4] Timer Tick - LD1: OFF, LD2: OFF, LD3: OFF
[    5] Timer Tick - LD1: ON , LD2: ON , LD3: ON
[    6] Timer Tick - LD1: OFF, LD2: OFF, LD3: OFF
[    7] Timer Tick - LD1: ON , LD2: ON , LD3: ON
[    8] Timer Tick - LD1: OFF, LD2: OFF, LD3: OFF
...
```

## 🔍 트러블슈팅

### 타이머가 동작하지 않는 경우

- [ ] `HAL_TIM_Base_Start_IT(&htim6)` 호출 확인
- [ ] NVIC에서 **TIM6 global interrupt** 활성화 확인
- [ ] `HAL_TIM_PeriodElapsedCallback()` 함수 구현 확인

### 주기가 맞지 않는 경우

- [ ] APB1 Timer Clock 확인 (54MHz × 2 = 108MHz)
- [ ] PSC, ARR 값 계산 재확인
- [ ] Clock Configuration 설정 확인

### Callback이 호출되지 않는 경우

- [ ] `stm32f7xx_it.c`에 `TIM6_DAC_IRQHandler()` 존재 확인
- [ ] `HAL_TIM_IRQHandler(&htim6)` 호출 확인
- [ ] `htim6` extern 선언 확인

### 주기가 2배인 경우

- [ ] APB1 Prescaler 확인 (1이 아니면 Timer Clock은 ×2)

## 📁 프로젝트 구조

```
04_TIM_TimeBase/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f7xx_hal_conf.h
│   │   └── stm32f7xx_it.h
│   └── Src/
│       ├── main.c                    # 메인 로직 + Callback
│       ├── stm32f7xx_hal_msp.c       # TIM6 MSP Init
│       ├── stm32f7xx_it.c            # TIM6_DAC_IRQHandler
│       └── system_stm32f7xx.c
├── Drivers/
│   ├── CMSIS/
│   └── STM32F7xx_HAL_Driver/
├── 04_TIM_TimeBase.ioc
└── README.md
```

## 🆚 HAL_Delay vs Timer Interrupt

| 항목 | HAL_Delay() | Timer Interrupt |
|------|-------------|-----------------|
| 동작 방식 | Blocking (CPU 점유) | Non-blocking |
| CPU 효율 | ❌ 낮음 | ✅ 높음 |
| 정확도 | 보통 | ✅ 높음 |
| 다른 작업 병행 | ❌ 불가 | ✅ 가능 |
| 복잡도 | 간단 | 약간 복잡 |
| 용도 | 간단한 딜레이 | 주기적 작업, 실시간 시스템 |

## 📚 참고 자료

- [NUCLEO-F767ZI User Manual (UM1974)](https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf)
- [STM32F767ZI Reference Manual (RM0410) - Basic Timers](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F7 HAL Driver - TIM](https://www.st.com/resource/en/user_manual/um1905-description-of-stm32f7-hal-and-lowlayer-drivers-stmicroelectronics.pdf)

## 📝 라이선스

This project is licensed under the MIT License.

## ✍️ Author

Created for STM32 embedded systems learning and development.
