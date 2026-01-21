# IR Receiver Module Test

STM32F103 NUCLEO 보드를 이용한 IR 수신 모듈 테스트 프로젝트

## 개요

이 프로젝트는 IR(적외선) 수신 모듈의 기본 동작을 테스트합니다. IR 신호가 감지되면 LED가 토글되고 펄스 정보가 시리얼로 출력됩니다.

## 하드웨어 구성

### 필요 부품

| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103 개발보드 |
| IR Receiver Module | 1 | TSOP1838, VS1838B 등 |

### 핀 연결

```
STM32F103 NUCLEO          IR Receiver Module
-----------------          ------------------
PA0           <--------- OUT (Signal)
3.3V          ---------> VCC
GND           ---------> GND

PA5 (LED)    : 내장 LED (신호 감지 표시)
PA2 (USART2_TX): USB Virtual COM (디버그 출력)
PA3 (USART2_RX): USB Virtual COM
```

### IR 수신 모듈 핀 배치

일반적인 IR 수신 모듈 (TSOP1838/VS1838B):

```
       +-----+
       |     |
       | ○○○ |  <- IR 수신부
       +-----+
        | | |
        G V S
        N C I
        D C G
            N
            A
            L

핀 순서 (모듈에 따라 다를 수 있음):
- TSOP1838: OUT - GND - VCC
- VS1838B:  OUT - VCC - GND
```

### 회로도

```
              +3.3V
                |
                |
         [10kΩ] (선택사항, 풀업)
                |
    PA0 <-------+----[OUT]
                      [IR Receiver]
    GND --------+----[GND]
                |
    3.3V -------+----[VCC]
```

---

## 주변장치 설정 상세

### 1. 시스템 클럭 설정 (64MHz)

이 프로젝트에서는 내부 RC 오실레이터(HSI, 8MHz)와 PLL을 사용하여 64MHz 시스템 클럭을 생성합니다.

#### 클럭 소스 선택: HSI vs HSE

| 항목 | HSI (내부 RC) | HSE (외부 크리스탈) |
|------|---------------|---------------------|
| 주파수 | 8MHz 고정 | 보드에 따라 다름 (NUCLEO: 8MHz) |
| 정밀도 | ±1% (온도에 따라 변동) | ±0.01% (크리스탈 정밀도) |
| 안정화 시간 | 빠름 (~2μs) | 느림 (~수 ms) |
| 외부 부품 | 불필요 | 크리스탈 + 커패시터 필요 |
| 권장 용도 | 일반 용도, 빠른 시작 | 정밀 타이밍, UART 통신 |

> **참고**: NUCLEO 보드에서 HSE를 사용하려면 ST-LINK의 MCO 출력을 활성화하거나 외부 크리스탈을 납땜해야 합니다. 이 프로젝트에서는 간편한 HSI를 사용합니다.

#### 클럭 트리 구조 (64MHz)

```
                          ┌─────────────────────────────────────────────────────────┐
                          │                   Clock Tree (64MHz)                    │
                          └─────────────────────────────────────────────────────────┘

    [HSI 8MHz]                                                      
        │                                                           
        ▼                                                           
   ┌─────────┐     ┌─────────┐     ┌─────────┐                     
   │ HSI/2   │────▶│   PLL   │────▶│ SYSCLK  │                     
   │  4MHz   │     │   x16   │     │  64MHz  │                     
   └─────────┘     └─────────┘     └─────────┘                     
                                        │                          
                    ┌───────────────────┼───────────────────┐      
                    ▼                   ▼                   ▼      
              ┌─────────┐         ┌─────────┐         ┌─────────┐  
              │  AHB    │         │  APB1   │         │  APB2   │  
              │  /1     │         │  /2     │         │  /1     │  
              │ 64MHz   │         │ 32MHz   │         │ 64MHz   │  
              └─────────┘         └─────────┘         └─────────┘  
                    │                   │                   │      
                    ▼                   ▼                   ▼      
               Core/AHB          TIM2,3,4 클럭           TIM1      
               DMA, Memory       = 32MHz × 2            USART1     
                                 = 64MHz               SPI1, ADC  
```

#### 클럭 설정 코드 (64MHz용 수정)

```c
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* 오실레이터 설정 - HSI 사용 */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;               // HSI 활성화
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;  // 기본 캘리브레이션
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;           // PLL 활성화
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;  // HSI/2 = 4MHz
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;          // 4MHz × 16 = 64MHz
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* 버스 클럭 설정 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;  // SYSCLK = PLL 출력
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;    // AHB = 64MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;     // APB1 = 32MHz (최대 36MHz)
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     // APB2 = 64MHz
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}
```

#### 클럭 설정 값 요약 (64MHz)

| 항목 | 설정 값 | 결과 주파수 | 비고 |
|------|---------|-------------|------|
| HSI | 8MHz | 8MHz | 내부 RC 오실레이터 |
| HSI/2 | /2 | 4MHz | PLL 입력 (HSI는 항상 /2 후 PLL 입력) |
| PLLMUL | ×16 | 64MHz | 4 × 16 = 64 |
| SYSCLK | PLL | 64MHz | 시스템 클럭 |
| AHB (HCLK) | /1 | 64MHz | Core, DMA, Memory |
| APB1 (PCLK1) | /2 | 32MHz | TIM2~4, USART2~3, I2C, SPI2 |
| APB2 (PCLK2) | /1 | 64MHz | TIM1, USART1, SPI1, ADC, GPIO |
| Flash Latency | 2 | - | 48~72MHz 동작 시 필수 |
| **APB1 Timer Clock** | - | **64MHz** | APB1 분주 ≠ 1이면 타이머 클럭 ×2 |

#### Flash Latency 설정

STM32F103은 시스템 클럭에 따라 Flash 메모리 접근 대기 사이클을 설정해야 합니다:

| SYSCLK 범위 | Flash Latency | 설명 |
|-------------|---------------|------|
| 0 ~ 24 MHz | 0 (Zero wait state) | 대기 없음 |
| 24 ~ 48 MHz | 1 (One wait state) | 1 사이클 대기 |
| 48 ~ 72 MHz | 2 (Two wait states) | 2 사이클 대기 |

64MHz에서는 반드시 `FLASH_LATENCY_2`를 설정해야 합니다.

---

### 2. PA0 핀 설정 상세

PA0는 IR 수신 모듈의 출력 신호를 읽는 핀입니다. 측정 방식에 따라 설정이 달라집니다.

#### PA0 핀의 대체 기능 (Alternate Functions)

```
┌──────────────────────────────────────────────────────────────────┐
│                    PA0 Pin Alternate Functions                    │
├──────────────────────────────────────────────────────────────────┤
│  기능               │ 설명                                        │
├──────────────────────────────────────────────────────────────────┤
│  GPIO Input         │ 일반 디지털 입력 (본 프로젝트에서 사용)     │
│  TIM2_CH1           │ 타이머2 채널1 (Input Capture/PWM)           │
│  TIM2_ETR           │ 타이머2 외부 트리거                         │
│  ADC12_IN0          │ ADC1/2 채널0 (아날로그 입력)                │
│  WKUP               │ Standby 모드 Wake-up 핀                     │
│  USART2_CTS         │ USART2 CTS (리맵 시)                        │
└──────────────────────────────────────────────────────────────────┘
```

#### 측정 방식 비교: Polling vs Input Capture vs Interrupt

IR 신호의 펄스를 측정하는 방법은 크게 3가지가 있습니다:

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                          IR 펄스 측정 방식 비교                                  │
├────────────────┬──────────────────┬──────────────────┬──────────────────────────┤
│     항목       │   GPIO Polling   │  EXTI Interrupt  │    Input Capture        │
├────────────────┼──────────────────┼──────────────────┼──────────────────────────┤
│ 구현 난이도    │ ★☆☆ 쉬움        │ ★★☆ 보통        │ ★★★ 복잡              │
├────────────────┼──────────────────┼──────────────────┼──────────────────────────┤
│ 측정 정밀도    │ 중 (루프 지연)   │ 상 (ISR 진입)    │ 최상 (하드웨어 캡처)     │
├────────────────┼──────────────────┼──────────────────┼──────────────────────────┤
│ CPU 사용률     │ 높음 (100%)      │ 낮음             │ 매우 낮음               │
├────────────────┼──────────────────┼──────────────────┼──────────────────────────┤
│ 타이밍 지터    │ ~수십 μs         │ ~수 μs           │ < 1μs (1 클럭)          │
├────────────────┼──────────────────┼──────────────────┼──────────────────────────┤
│ 다른 작업 병행 │ 어려움           │ 가능             │ 가능 (DMA 조합 시 최적) │
├────────────────┼──────────────────┼──────────────────┼──────────────────────────┤
│ 적합한 용도    │ 테스트/학습용    │ 일반 IR 수신     │ 정밀 프로토콜 디코딩     │
├────────────────┼──────────────────┼──────────────────┼──────────────────────────┤
│ PA0 설정       │ GPIO_MODE_INPUT  │ GPIO_MODE_IT_*   │ GPIO_MODE_AF_INPUT      │
└────────────────┴──────────────────┴──────────────────┴──────────────────────────┘
```

#### 현재 프로젝트: GPIO Polling 방식

본 프로젝트는 **GPIO Polling** 방식을 사용합니다. 이 방식은 구현이 간단하고 동작 원리를 이해하기 쉽습니다.

```c
/* PA0 GPIO 입력 설정 */
GPIO_InitTypeDef GPIO_InitStruct = {0};

GPIO_InitStruct.Pin = GPIO_PIN_0;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;     // 일반 입력 모드
GPIO_InitStruct.Pull = GPIO_PULLUP;         // 내부 풀업 활성화
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
```

**Polling 동작 원리:**

```
Main Loop
    │
    ├──▶ GPIO 상태 읽기: HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)
    │           │
    │           ▼
    │    ┌──────────────┐
    │    │ 이전=HIGH    │     하강 에지 감지
    │    │ 현재=LOW     │ ──▶ 펄스 시작 시간 기록
    │    └──────────────┘
    │           │
    │           ▼
    │    ┌──────────────┐
    │    │ 이전=LOW     │     상승 에지 감지
    │    │ 현재=HIGH    │ ──▶ 펄스 폭 계산
    │    └──────────────┘
    │           │
    └───────────┘ (반복)
```

#### (참고) Input Capture 방식 설정

향후 정밀한 측정이 필요한 경우 Input Capture 방식으로 업그레이드할 수 있습니다:

```c
/* Input Capture 방식 - PA0을 TIM2_CH1으로 설정 */

/* GPIO 설정 - Alternate Function Input */
GPIO_InitStruct.Pin = GPIO_PIN_0;
GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;   // 대체 기능 입력
GPIO_InitStruct.Pull = GPIO_PULLUP;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* TIM2 Input Capture 설정 */
TIM_IC_InitTypeDef sConfigIC = {0};

htim2.Instance = TIM2;
htim2.Init.Prescaler = 63;                   // 64MHz / 64 = 1MHz
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
htim2.Init.Period = 0xFFFFFFFF;
HAL_TIM_IC_Init(&htim2);

sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;  // 양쪽 에지 캡처
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
sConfigIC.ICFilter = 0x0F;                   // 노이즈 필터 (최대)
HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1);

/* Input Capture 인터럽트 시작 */
HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
```

#### (참고) EXTI Interrupt 방식 설정

```c
/* EXTI Interrupt 방식 - PA0 외부 인터럽트 설정 */
GPIO_InitStruct.Pin = GPIO_PIN_0;
GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  // 양쪽 에지 인터럽트
GPIO_InitStruct.Pull = GPIO_PULLUP;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* NVIC 설정 */
HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(EXTI0_IRQn);
```

---

### 3. TIM2 설정 (마이크로초 타이머) - 64MHz 기준

IR 신호의 펄스 폭을 정확히 측정하려면 마이크로초 단위의 시간 측정이 필요합니다. TIM2를 1MHz(1μs tick)로 설정합니다.

#### APB1 타이머 클럭 계산

```
중요: APB1 프리스케일러가 1이 아닌 경우, 타이머 클럭은 APB1 클럭의 2배입니다.

APB1 Clock (PCLK1) = SYSCLK / APB1_DIV = 64MHz / 2 = 32MHz
APB1 Timer Clock = PCLK1 × 2 = 32MHz × 2 = 64MHz  (∵ APB1_DIV ≠ 1)
```

```
    SYSCLK (64MHz)
         │
         ▼
    ┌─────────┐
    │ APB1    │  /2
    │Prescaler│
    └─────────┘
         │
         ├──────────────────────▶ APB1 Peripheral Clock = 32MHz
         │                        (USART2, I2C, SPI2 등)
         │
         ▼
    ┌─────────┐
    │  ×2     │  (APB1 분주 ≠ 1일 때)
    │Multiplier│
    └─────────┘
         │
         ▼
    APB1 Timer Clock = 64MHz ◀── TIM2, TIM3, TIM4 입력
```

#### 타이머 블록 다이어그램

```
                     ┌────────────────────────────────────────────────┐
                     │              TIM2 Block Diagram                │
                     └────────────────────────────────────────────────┘
                                                                       
    APB1 Timer Clock (64MHz)                                          
           │                                                          
           ▼                                                          
    ┌─────────────┐                                                   
    │  Prescaler  │  PSC = 63                                         
    │   (PSC)     │  64MHz / (63+1) = 1MHz                            
    └─────────────┘                                                   
           │                                                          
           ▼  1MHz (1μs per tick)                                     
    ┌─────────────┐                                                   
    │   Counter   │  CNT: 0 → 1 → 2 → ... → 0xFFFFFFFF → 0 (overflow)
    │   (CNT)     │                                                   
    └─────────────┘                                                   
           │                                                          
           ▼                                                          
    ┌─────────────┐                                                   
    │ Auto-Reload │  ARR = 0xFFFFFFFF (32-bit max)                    
    │   (ARR)     │  오버플로우 주기: ~71.6분                          
    └─────────────┘                                                   
```

#### 프리스케일러 계산 (64MHz 기준)

```
Timer Clock = APB1 Timer Clock / (PSC + 1)

목표: 1MHz (1μs resolution)
입력: 64MHz (APB1 Timer Clock)

PSC = (64MHz / 1MHz) - 1 = 64 - 1 = 63
```

#### TIM2 설정 코드 (64MHz용)

```c
static void MX_TIM2_Init(void) {
    /* TIM2 클럭 활성화 */
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    
    /* 프리스케일러: 64MHz → 1MHz */
    htim2.Init.Prescaler = 63;              // (63 + 1) = 64 분주
    
    /* 카운터 모드: 업 카운트 */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    
    /* Auto-Reload 값: 32비트 최대값 */
    htim2.Init.Period = 0xFFFFFFFF;         // TIM2는 32-bit 타이머
    
    /* 클럭 분주 (입력 필터용) */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    
    /* Auto-Reload 프리로드 비활성화 */
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
}
```

#### TIM2 설정 값 상세

| 레지스터 | 설정 값 | 의미 |
|----------|---------|------|
| **PSC** (Prescaler) | 63 | 입력 클럭을 (63+1)=64로 나눔. 64MHz/64=1MHz |
| **ARR** (Auto-Reload) | 0xFFFFFFFF | 카운터 최대값. TIM2는 32비트 타이머 |
| **CNT** (Counter) | 0부터 시작 | 1μs마다 1씩 증가 |
| **CR1.CEN** | 1 (Start 후) | 카운터 활성화 |
| **CR1.DIR** | 0 (Up) | 업 카운트 모드 |
| **CR1.ARPE** | 0 | Auto-Reload 프리로드 비활성화 |

#### 타이머 동작 타이밍

```
    Clock Input (64MHz)
    ─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─...─┬─┬─┬─┬─
     │ │ │ │ │ │ │ │ │ │ │ │     │ │ │ │
     └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─...─┴─┴─┴─┴─
     |←─────── 64 clocks ───────▶|
                                 
    Timer Counter (1MHz = 1μs tick)
    ─────┬─────┬─────┬─────┬─────┬─────┬─────
         │     │     │     │     │     │
    CNT: 0     1     2     3     4     5  ...
         |←1μs▶|
```

#### 마이크로초 읽기 함수

```c
uint32_t GetMicros(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
}
```

---

### 4. GPIO 설정 상세

#### GPIO 클럭 활성화

STM32F1 시리즈에서는 각 GPIO 포트별로 클럭을 활성화해야 합니다:

```c
/* GPIOA 클럭 활성화 (RCC_APB2ENR 레지스터의 IOPAEN 비트) */
__HAL_RCC_GPIOA_CLK_ENABLE();
```

#### GPIO 모드 옵션 (STM32F1)

STM32F1은 다른 시리즈와 GPIO 설정 방식이 다릅니다:

```
┌─────────────────────────────────────────────────────────────────┐
│                    STM32F1 GPIO Modes                           │
├──────────────────────┬──────────────────────────────────────────┤
│ Mode                 │ 설명                                     │
├──────────────────────┼──────────────────────────────────────────┤
│ GPIO_MODE_INPUT      │ 입력 모드 (Floating, Pull-up, Pull-down) │
│ GPIO_MODE_OUTPUT_PP  │ Push-Pull 출력                           │
│ GPIO_MODE_OUTPUT_OD  │ Open-Drain 출력                          │
│ GPIO_MODE_AF_PP      │ Alternate Function Push-Pull             │
│ GPIO_MODE_AF_OD      │ Alternate Function Open-Drain            │
│ GPIO_MODE_AF_INPUT   │ Alternate Function Input (TIM IC 등)     │
│ GPIO_MODE_ANALOG     │ 아날로그 모드 (ADC/DAC)                   │
│ GPIO_MODE_IT_RISING  │ 상승 에지 인터럽트                        │
│ GPIO_MODE_IT_FALLING │ 하강 에지 인터럽트                        │
│ GPIO_MODE_IT_RISING_FALLING │ 양쪽 에지 인터럽트                │
└──────────────────────┴──────────────────────────────────────────┘
```

#### IR 수신 핀 (PA0) 설정

```c
/* IR Receiver Input - PA0 */
GPIO_InitTypeDef GPIO_InitStruct = {0};

GPIO_InitStruct.Pin = IR_RECV_Pin;           // GPIO_PIN_0
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;      // 디지털 입력 모드
GPIO_InitStruct.Pull = GPIO_PULLUP;          // 내부 풀업 저항 활성화
HAL_GPIO_Init(IR_RECV_GPIO_Port, &GPIO_InitStruct);  // GPIOA
```

**Pull 설정 옵션:**

| 설정 | 동작 | IR 수신 모듈 적합성 |
|------|------|---------------------|
| GPIO_NOPULL | 플로팅 (외부 저항 필요) | 외부 풀업 있을 때 |
| GPIO_PULLUP | 내부 풀업 (~40kΩ) | ✓ 권장 |
| GPIO_PULLDOWN | 내부 풀다운 (~40kΩ) | ✗ 부적합 |

**풀업이 필요한 이유:**

```
IR 수신 모듈 (Active Low 출력)

신호 없을 때:                    신호 있을 때:
    VCC                              VCC
     │                                │
     R (풀업)                         R (풀업)
     │                                │
     ├───── OUT = HIGH               ├───── OUT = LOW
     │                                │
   [OFF]                            [ON] ← 내부 트랜지스터
     │                                │
    GND                              GND
```

#### LED 핀 (PA5) 설정

```c
/* LED Output - PA5 (NUCLEO 보드 내장 LED) */
HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);  // 초기값 LOW (LED OFF)

GPIO_InitStruct.Pin = LED_Pin;                // GPIO_PIN_5
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // Push-Pull 출력
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  // 저속 (2MHz)
HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
```

**GPIO Speed 옵션:**

| 설정 | 최대 주파수 | 슬루율 | 노이즈/EMI | 용도 |
|------|------------|--------|------------|------|
| GPIO_SPEED_FREQ_LOW | 2MHz | 느림 | 낮음 | LED, 릴레이 |
| GPIO_SPEED_FREQ_MEDIUM | 10MHz | 중간 | 중간 | 일반 GPIO |
| GPIO_SPEED_FREQ_HIGH | 50MHz | 빠름 | 높음 | SPI, 고속 통신 |

---

### 5. USART2 설정 (디버그 출력)

NUCLEO 보드는 ST-LINK의 Virtual COM Port를 통해 USB 시리얼 통신을 제공합니다. PA2(TX), PA3(RX)는 ST-LINK에 연결되어 있습니다.

#### USART2 클럭 및 GPIO 설정

```c
static void MX_USART2_UART_Init(void) {
    /* 클럭 활성화 */
    __HAL_RCC_USART2_CLK_ENABLE();  // USART2는 APB1 버스 (32MHz)
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* PA2 - TX (Alternate Function Push-Pull) */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;       // 대체 기능 Push-Pull
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 고속 (50MHz)
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PA3 - RX (Input) */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;       // 입력 (또는 GPIO_MODE_AF_INPUT)
    GPIO_InitStruct.Pull = GPIO_NOPULL;           // 외부 연결이므로 풀 불필요
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
```

#### USART2 통신 파라미터 설정

```c
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;               // 전송 속도
    huart2.Init.WordLength = UART_WORDLENGTH_8B; // 8비트 데이터
    huart2.Init.StopBits = UART_STOPBITS_1;      // 1 스톱 비트
    huart2.Init.Parity = UART_PARITY_NONE;       // 패리티 없음
    huart2.Init.Mode = UART_MODE_TX_RX;          // 송수신 모드
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE; // 하드웨어 흐름 제어 없음
    huart2.Init.OverSampling = UART_OVERSAMPLING_16; // 16배 오버샘플링
    
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}
```

#### Baud Rate 계산 (64MHz, APB1=32MHz)

```
USART2 Clock = APB1 Clock = 32MHz
Baud Rate = USART Clock / (16 × USARTDIV)  (OverSampling 16일 때)

USARTDIV = 32,000,000 / (16 × 115,200) = 17.361...

BRR 레지스터 계산:
- Mantissa (정수부): 17 = 0x11
- Fraction (소수부): 0.361 × 16 = 5.78 ≈ 6 = 0x6
- BRR = (17 << 4) | 6 = 0x116

실제 Baud Rate = 32,000,000 / (16 × 17.375) = 115,108 bps
오차: |115,108 - 115,200| / 115,200 × 100 = 0.08% (허용 범위 내)
```

> **참고**: UART 통신의 Baud Rate 오차는 일반적으로 3% 이내면 안정적으로 동작합니다.

#### printf 리다이렉션

```c
/* syscalls.c 또는 main.c에 추가 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
```

프로젝트 설정에서 `Use float with printf` 옵션을 비활성화하면 코드 크기를 줄일 수 있습니다.

---

### 6. 추가 필수 설정 사항

#### 6.1 HAL 드라이버 초기화

```c
int main(void) {
    /* HAL 라이브러리 초기화 (필수) */
    HAL_Init();
    
    /* 이 함수는 다음을 수행:
     * - NVIC 우선순위 그룹 설정
     * - SysTick 설정 (1ms 인터럽트)
     * - Flash Prefetch 활성화
     */
    ...
}
```

`HAL_Init()` 내부 동작:

```c
HAL_StatusTypeDef HAL_Init(void) {
    /* Flash Prefetch Buffer 활성화 */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
    
    /* NVIC 우선순위 그룹 설정 (4비트 preemption) */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    
    /* SysTick 설정 (1ms 인터럽트) */
    HAL_InitTick(TICK_INT_PRIORITY);
    
    return HAL_OK;
}
```

#### 6.2 SysTick 설정

HAL 라이브러리는 1ms 시스템 틱을 위해 SysTick 타이머를 사용합니다:

```c
/* stm32f1xx_hal.c에서 자동 설정됨 */
HAL_SYSTICK_Config(SystemCoreClock / 1000);  // 64MHz / 1000 = 64000 (1ms)
```

SysTick은 다음 기능에 사용됩니다:
- `HAL_Delay()` 함수
- `HAL_GetTick()` 함수
- HAL 라이브러리 내부 타임아웃

#### 6.3 NVIC (인터럽트) 설정

현재 프로젝트(Polling 방식)에서는 추가 인터럽트 설정이 필요 없습니다. 단, SysTick 인터럽트는 HAL_Init()에서 자동 활성화됩니다.

Interrupt 방식으로 업그레이드 시:

```c
/* EXTI0 인터럽트 (PA0) 활성화 */
HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
HAL_NVIC_EnableIRQ(EXTI0_IRQn);

/* 또는 TIM2 인터럽트 (Input Capture) */
HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
HAL_NVIC_EnableIRQ(TIM2_IRQn);
```

#### 6.4 인터럽트 핸들러 (stm32f1xx_it.c)

HAL 라이브러리 사용 시 필수 인터럽트 핸들러:

```c
/* stm32f1xx_it.c */

void NMI_Handler(void) {
    while (1) {}
}

void HardFault_Handler(void) {
    while (1) {}
}

void MemManage_Handler(void) {
    while (1) {}
}

void BusFault_Handler(void) {
    while (1) {}
}

void UsageFault_Handler(void) {
    while (1) {}
}

void SVC_Handler(void) {
}

void DebugMon_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
    HAL_IncTick();  // 1ms 틱 증가 (필수)
}
```

#### 6.5 Error_Handler 구현

```c
void Error_Handler(void) {
    /* 인터럽트 비활성화 */
    __disable_irq();
    
    /* 오류 발생 시 무한 루프 (디버깅용) */
    while (1) {
        /* LED 빠르게 깜빡이기 등 오류 표시 가능 */
    }
}
```

---

### 7. STM32CubeIDE (.ioc) 설정 가이드

STM32CubeMX 또는 STM32CubeIDE의 Device Configuration Tool 사용 시:

#### 7.1 Pinout & Configuration

```
1. System Core → RCC
   ├─ High Speed Clock (HSE): Disable (또는 BYPASS Clock Source)
   └─ Low Speed Clock (LSE): Disable
   
2. System Core → SYS
   └─ Debug: Serial Wire
   
3. Timers → TIM2
   ├─ Clock Source: Internal Clock
   ├─ Channel1: (비활성 - Polling 사용 시)
   └─ Parameter Settings:
       ├─ Prescaler: 63
       ├─ Counter Mode: Up
       ├─ Counter Period: 4294967295
       ├─ Internal Clock Division: No Division
       └─ auto-reload preload: Disable
   
4. Connectivity → USART2
   ├─ Mode: Asynchronous
   └─ Parameter Settings:
       ├─ Baud Rate: 115200
       ├─ Word Length: 8 Bits (including Parity)
       ├─ Parity: None
       ├─ Stop Bits: 1
       └─ Data Direction: Receive and Transmit
   
5. GPIO
   ├─ PA0: GPIO_Input
   │   ├─ User Label: IR_RECV
   │   └─ GPIO Pull-up/Pull-down: Pull-up
   │
   └─ PA5: GPIO_Output
       ├─ User Label: LED
       ├─ GPIO output level: Low
       ├─ GPIO mode: Output Push Pull
       └─ Maximum output speed: Low
```

#### 7.2 Clock Configuration

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         Clock Configuration Tab                              │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  [HSI RC] ──▶ [HSI] 8 MHz                                                   │
│                 │                                                            │
│                 ▼                                                            │
│              [/2] ──▶ 4 MHz ──▶ [PLL Source Mux] ──▶ HSI                    │
│                                                                              │
│  [PLL Mul] ──▶ × 16                                                         │
│                                                                              │
│  [PLLCLK] ──▶ 64 MHz ──▶ [System Clock Mux] ──▶ PLLCLK                     │
│                                                                              │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │ SYSCLK: 64 MHz                                                       │    │
│  │ HCLK: 64 MHz (AHB Prescaler: /1)                                    │    │
│  │ APB1 Prescaler: /2 → PCLK1: 32 MHz → APB1 Timer clocks: 64 MHz     │    │
│  │ APB2 Prescaler: /1 → PCLK2: 64 MHz                                  │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 7.3 Project Manager 설정

```
Project Manager → Code Generator
├─ STM32Cube MCU packages: Copy only the necessary library files
├─ Generated files:
│   ├─ ☑ Generate peripheral initialization as a pair of '.c/.h' files
│   └─ ☐ Delete previously generated files when not re-generated
└─ HAL Settings:
    └─ ☑ Set all free pins as analog (to optimize power)
```

---

## 소프트웨어 설명

### 동작 원리

1. IR 수신 모듈은 38kHz 변조된 IR 신호를 감지
2. 신호가 있으면 OUT 핀이 LOW
3. 신호가 없으면 OUT 핀이 HIGH (Active Low)
4. MCU는 펄스 수와 폭을 측정하여 출력

### 주요 변수

```c
#define SIGNAL_TIMEOUT_MS   100    // 신호 버스트 종료 감지 시간
#define NOISE_FILTER_US     100    // 노이즈 필터링 최소 펄스 폭
```

### 프로그램 흐름

```
시작
  ↓
HAL_Init()         ← SysTick, NVIC, Flash 설정
  ↓
SystemClock_Config() ← 64MHz 클럭 설정
  ↓
MX_GPIO_Init()     ← PA0(입력), PA5(출력) 설정
  ↓
MX_TIM2_Init()     ← 마이크로초 타이머 설정
  ↓
MX_USART2_UART_Init() ← 시리얼 통신 설정
  ↓
HAL_TIM_Base_Start() ← 타이머 시작
  ↓
┌──────────────────────────┐
│  메인 루프 (Polling)     │
│  ├─ IR 핀 상태 읽기      │
│  ├─ 하강 에지 감지?      │
│  │   └─ 펄스 시작 기록   │
│  ├─ 상승 에지 감지?      │
│  │   └─ 펄스 폭 계산     │
│  └─ 타임아웃?            │
│      └─ 결과 출력        │
└──────────────────────────┘
```

### IR 신호 측정 로직

```
IR 수신 모듈 출력 (Active Low)
     │
HIGH ─────┐     ┌─────┐     ┌─────────────
          │     │     │     │
LOW       └─────┘     └─────┘

          |← pulse_width →|
          ↑               ↑
     하강 에지        상승 에지
     (펄스 시작)      (펄스 종료)
```

---

## 빌드 및 실행

### STM32CubeIDE 사용 시

1. 새 STM32 프로젝트 생성 (STM32F103RB 선택)
2. 위의 .ioc 설정대로 구성
3. `main.c` 파일의 USER CODE 영역에 코드 추가
4. 빌드 후 플래시

### 컴파일 옵션 확인

```
Project Properties → C/C++ Build → Settings → MCU Settings
├─ Runtime library: Reduced C (--specs=nano.specs)
└─ Use float with printf: Disable (코드 크기 최소화)
```

---

## 사용 방법

1. 보드에 프로그램 업로드
2. 시리얼 터미널 연결 (115200 baud, 8N1)
3. IR 리모컨으로 수신 모듈을 향해 버튼 누르기
4. 터미널에서 수신 결과 확인

### 시리얼 출력 예시

```
╔════════════════════════════════════════╗
║   IR Receiver & NEC Decoder - STM32    ║
╠════════════════════════════════════════╣
║ System Clock : 64 MHz                  ║
║ IR Input     : PA0 (Pull-up)           ║
║ Status LED   : PA5                     ║
║ Protocol     : NEC / Extended NEC      ║
╚════════════════════════════════════════╝

Waiting for IR signals...

────────────────────────────────────────
[1] IR Signal Received
    Pulses captured: 33
    ┌──────────────────────────────────┐
    │         NEC Decoded Data         │
    ├──────────────────────────────────┤
    │ Raw Data   : 0xE916FF00           │
    ├──────────────────────────────────┤
    │ Address    : 0x00 (  0)         │
    │ Address~   : 0xFF (255)         │
    │ Command    : 0x16 ( 22)         │
    │ Command~   : 0xE9 (233)         │
    ├──────────────────────────────────┤
    │ ✓ Standard NEC Protocol         │
    │   Device : 0x00                  │
    │   Key    : 0x16                  │
    └──────────────────────────────────┘
    Binary: 11101001 00010110 11111111 00000000
            CMD~     CMD      ADDR~    ADDR

────────────────────────────────────────
[2] IR Signal Received
    Pulses captured: 2
    ◆ REPEAT Code

────────────────────────────────────────
[3] IR Signal Received
    Pulses captured: 33
    ┌──────────────────────────────────┐
    │         NEC Decoded Data         │
    ├──────────────────────────────────┤
    │ Raw Data   : 0xA25D10EF           │
    ├──────────────────────────────────┤
    │ Address    : 0xEF (239)         │
    │ Address~   : 0x10 ( 16)         │
    │ Command    : 0x5D ( 93)         │
    │ Command~   : 0xA2 (162)         │
    ├──────────────────────────────────┤
    │ ✓ Extended NEC Protocol         │
    │   Device : 0x10EF                │
    │   Key    : 0x5D                  │
    └──────────────────────────────────┘
    Binary: 10100010 01011101 00010000 11101111
            CMD~     CMD      ADDR~    ADDR
```

---

## 출력 해석

### 디코딩된 데이터 필드

| 항목 | 설명 |
|------|------|
| Raw Data | 32비트 원시 데이터 (Hex) |
| Address | 8비트 주소 (장치 식별) |
| Address~ | 8비트 반전 주소 (또는 Extended 주소 상위 바이트) |
| Command | 8비트 명령 (버튼 코드) |
| Command~ | 8비트 반전 명령 (검증용) |

### 프로토콜 구분

| 프로토콜 | 조건 | 주소 형식 |
|----------|------|-----------|
| Standard NEC | Address ^ Address~ = 0xFF | 8비트 |
| Extended NEC | Address ^ Address~ ≠ 0xFF, Command ^ Command~ = 0xFF | 16비트 |

### NEC 프로토콜 상세

#### 프레임 구조

```
Standard NEC Frame (32 bits, LSB first):
┌─────────┬─────────┬──────────┬──────────┬─────────┬─────────┬──────┐
│ Leader  │ Leader  │ Address  │ Address  │ Command │ Command │ Stop │
│ Pulse   │ Space   │ (8 bits) │ Inverted │ (8 bits)│ Inverted│ Bit  │
│ 9ms     │ 4.5ms   │          │ (8 bits) │         │ (8 bits)│      │
└─────────┴─────────┴──────────┴──────────┴─────────┴─────────┴──────┘
     │         │         │           │          │          │
     ▼         ▼         ▼           ▼          ▼          ▼
  ┌─────┐  ┌─────┐                                     
  │█████│  │     │   LSB ◄────────── 32 bits ──────────► MSB
  │█████│  │     │
  └──┬──┘  └──┬──┘
   9000us   4500us
```

#### 비트 인코딩

```
Bit 0:                          Bit 1:
┌─────┐                         ┌─────┐
│█████│                         │█████│
│█████│     ┌─────┐             │█████│           ┌─────┐
└──┬──┘     │     │             └──┬──┘           │     │
 562.5us    562.5us               562.5us         1687.5us
 (pulse)    (space)               (pulse)         (space)
 
Total: 1.125ms                  Total: 2.25ms
```

#### Repeat Code

```
버튼을 계속 누르고 있으면 108ms 간격으로 Repeat 코드 전송:

┌─────┐     ┌─────┐
│█████│     │     │     ┌─┐
│█████│     │     │     │█│
└──┬──┘     └──┬──┘     └┬┘
 9000us     2250us    562.5us
 (pulse)    (space)   (stop)
```

#### 타이밍 허용 오차

| 항목 | 표준값 | 허용 범위 |
|------|--------|-----------|
| Leader Pulse | 9000 µs | 8000 ~ 10000 µs |
| Leader Space | 4500 µs | 4000 ~ 5000 µs |
| Repeat Space | 2250 µs | 2000 ~ 2500 µs |
| Bit Pulse | 562.5 µs | 400 ~ 750 µs |
| Bit 0 Space | 562.5 µs | 400 ~ 750 µs |
| Bit 1 Space | 1687.5 µs | 1400 ~ 1900 µs |

---

## 설정 요약 체크리스트

| 항목 | 설정 값 | 확인 |
|------|---------|------|
| **시스템 클럭** | 64MHz (HSI + PLL×16) | ☐ |
| **Flash Latency** | 2 wait states | ☐ |
| **APB1 분주** | /2 (32MHz) | ☐ |
| **APB1 Timer Clock** | 64MHz (×2) | ☐ |
| **TIM2 PSC** | 63 (→ 1MHz) | ☐ |
| **TIM2 ARR** | 0xFFFFFFFF (32-bit) | ☐ |
| **PA0 Mode** | Input + Pull-up | ☐ |
| **PA5 Mode** | Output Push-Pull | ☐ |
| **USART2 Baud** | 115200 | ☐ |
| **HAL_Init()** | 호출 확인 | ☐ |
| **SysTick_Handler** | HAL_IncTick() 호출 | ☐ |

---

## 문제 해결

| 증상 | 원인 | 해결 방법 |
|------|------|----------|
| 신호 미감지 | 핀 연결 오류 | VCC/GND/OUT 핀 순서 확인 |
| 노이즈 검출 | 환경광 영향 | 수신부를 가리거나 방향 조정 |
| 펄스 수 불일치 | 거리 너무 멈 | 리모컨을 가까이 |
| LED 안 깜빡임 | GPIO 클럭 미활성화 | `__HAL_RCC_GPIOA_CLK_ENABLE()` 확인 |
| 시리얼 출력 없음 | UART 설정 오류 | Baud Rate, 핀 연결 확인 |
| 타이밍 부정확 | PSC 값 오류 | 64MHz 기준 PSC=63 확인 |
| HAL_Delay 동작 안함 | SysTick 미설정 | SysTick_Handler 구현 확인 |
| 프로그램 멈춤 | Flash Latency 오류 | FLASH_LATENCY_2 설정 확인 |

---

## 지원 IR 수신 모듈

| 모듈 | 캐리어 주파수 | 전압 |
|------|--------------|------|
| TSOP1838 | 38kHz | 2.5-5.5V |
| VS1838B | 38kHz | 2.7-5.5V |
| TSOP1736 | 36kHz | 2.5-5.5V |
| TSOP1740 | 40kHz | 2.5-5.5V |

---

## 확장 아이디어

- **Input Capture 방식**: 하드웨어 기반 정밀 펄스 측정
- **EXTI 인터럽트 방식**: CPU 사용률 감소
- **DMA + Input Capture**: 무중단 대량 데이터 캡처
- **다중 프로토콜 지원**: NEC, RC5, Sony SIRC 자동 감지

---

## 참고 자료

- [TSOP1838 Datasheet](https://www.vishay.com/docs/82491/tsop18xx.pdf)
- [VS1838B Datasheet](https://www.alldatasheet.com/datasheet-pdf/pdf/1132466/ETC2/VS1838B.html)
- [STM32F103 Reference Manual (RM0008)](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F1 HAL Driver User Manual (UM1850)](https://www.st.com/resource/en/user_manual/um1850-description-of-stm32f1-hal-and-lowlayer-drivers-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

---

## 라이선스

MIT License
