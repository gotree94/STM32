# Rotary_Encoder

# STM32F103 로터리 인코더 CubeMX 설정 가이드

## 1. 프로젝트 생성

### MCU 선택
- **Part Number**: STM32F411 (또는 사용하는 모델)
- **Package**: LQFP48

### 프로젝트 설정
- **Project Name**: Rotary_Encoder_Project
- **Toolchain/IDE**: STM32CubeIDE
- **Application Structure**: Basic

---

## 2. 클록 설정 (Clock Configuration)

### RCC (Reset and Clock Control)
1. **Pinout & Configuration** → **System Core** → **RCC**
2. **High Speed Clock (HSE)**: BYPASS 또는 Crystal/Ceramic Resonator
3. **Low Speed Clock (LSE)**: Disable

### Clock Configuration 탭
1. **Input frequency**: 8 MHz (HSE 사용 시)
2. **HCLK (MHz)**: 84
3. **설정 방법**:
   - PLL Source: HSI/2 (내부 클록 사용 시) 또는 HSE (외부 클록 사용 시)
   - PLL MUL: x16 (HSI 사용 시) 또는 x8 (HSE 사용 시)
   - System Clock Mux: PLLCLK
   - AHB Prescaler: /1
   - APB1 Prescaler: /2
   - APB2 Prescaler: /1

**결과**: SYSCLK = 84MHz, HCLK = 84MHz, PCLK1 = 42MHz, PCLK2 = 84MHz

---

## 3. GPIO 설정

### 인코더 핀 설정 (PA0, PA1)

#### PA0 (CLK 핀)
1. **Pinout view**에서 **PA0** 클릭
2. **GPIO_EXTI0** 선택

**설정값**:
- **GPIO mode**: External Interrupt Mode with Rising/falling edge trigger detection
- **GPIO Pull-up/Pull-down**: Pull-up
- **Maximum output speed**: Low (입력이므로 관계없음)
- **User Label**: CLK_Pin

#### PA1 (DT 핀)
1. **Pinout view**에서 **PA1** 클릭
2. **GPIO_EXTI1** 선택

**설정값**:
- **GPIO mode**: External Interrupt Mode with Rising/falling edge trigger detection
- **GPIO Pull-up/Pull-down**: Pull-up
- **Maximum output speed**: Low (입력이므로 관계없음)
- **User Label**: DT_Pin

### LED 핀 설정 (선택사항)
- **PA5**: GPIO_Output (LD2_Pin으로 자동 설정됨)

### 버튼 핀 설정 (선택사항)
- **PC13**: GPIO_EXTI13 (B1_Pin으로 자동 설정됨)

---

## 4. NVIC 설정 (인터럽트 우선순위)

1. **Pinout & Configuration** → **System Core** → **NVIC**
2. 다음 인터럽트들을 **Enable**로 설정:

| 인터럽트 이름 | Enable | Preemption Priority | Sub Priority |
|---------------|--------|-------------------|--------------|
| EXTI line0 interrupt | ✓ | 1 | 0 |
| EXTI line1 interrupt | ✓ | 1 | 0 |
| EXTI line[15:10] interrupts | ✓ | 2 | 0 |

**Priority 설정 이유**:
- 인코더 인터럽트(0,1): 높은 우선순위 (1)
- 버튼 인터럽트(13): 낮은 우선순위 (2)

---

## 5. UART 설정 (printf 출력용)

### USART2 설정
1. **Pinout & Configuration** → **Connectivity** → **USART2**
2. **Mode**: Asynchronous

**파라미터 설정**:
- **Baud Rate**: 115200 Bits/s
- **Word Length**: 8 Bits (including Parity)
- **Parity**: None
- **Stop Bits**: 1
- **Data Direction**: Receive and Transmit
- **Over Sampling**: 16 Samples

**GPIO 설정** (자동으로 설정됨):
- **PA2**: USART2_TX
- **PA3**: USART2_RX

---

## 6. 타이머 설정 (선택사항)

### TIM2 설정
1. **Pinout & Configuration** → **Timers** → **TIM2**
2. **Clock Source**: Internal Clock

**파라미터 설정**:
- **Prescaler**: 8399 (84MHz/8400 = 10kHz)
- **Counter Mode**: Up
- **Counter Period**: 9999 (10kHz/10000 = 1Hz)
- **Auto-reload preload**: Disable

---

## 7. 프로젝트 설정

### Project Manager 탭

#### Project Settings
- **Project Name**: 원하는 프로젝트 이름
- **Project Location**: 원하는 경로
- **Toolchain/IDE**: STM32CubeIDE

#### Code Generator Settings
- **STM32Cube MCU packages and embedded software packs**: Latest
- **Generated files**:
  - ✓ Generate peripheral initialization as a pair of '.c/.h' files per peripheral
  - ✓ Backup previously generated files when re-generating
  - ✓ Keep User Code when re-generating
  - ✓ Delete previously generated files when not re-generated

#### Advanced Settings
- **Generated Function Calls**:
  - HAL: ✓ (모든 항목)
  - LL: 필요시에만

---

## 8. 추가 설정사항

### main.h 파일 확인
코드 생성 후 `main.h`에서 다음 정의들이 있는지 확인:

```c
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define CLK_Pin GPIO_PIN_0
#define CLK_GPIO_Port GPIOA
#define DT_Pin GPIO_PIN_1
#define DT_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
```

### 컴파일러 설정 (printf 사용을 위해)
1. **Project Properties** → **C/C++ Build** → **Settings**
2. **Tool Settings** → **MCU GCC Linker** → **Libraries**
3. **Use float with printf from newlib-nano (-u _printf_float)**: Yes
4. **Small printf from newlib-nano**: Yes

---

## 9. 하드웨어 연결

### 로터리 인코더 연결
```
로터리 인코더    STM32F103
CLK      →      PA0
DT       →      PA1  
SW       →      (선택사항)
VCC      →      3.3V
GND      →      GND
```

### UART 연결 (디버깅용)
```
STM32F103    UART-USB 컨버터
PA2 (TX) →   RX
PA3 (RX) →   TX
GND      →   GND
```

---

## 10. 코드 통합 참고사항

### USER CODE 섹션에 추가할 내용
1. **USER CODE BEGIN Includes**: `#include <stdio.h>`
2. **USER CODE BEGIN PV**: 인코더 변수들
3. **USER CODE BEGIN 0**: printf 리다이렉션 및 인코더 함수들
4. **USER CODE BEGIN 2**: 인코더 초기화 및 시작 메시지
5. **USER CODE BEGIN WHILE**: 메인 루프 로직

### 컴파일 시 확인사항
- 모든 인터럽트 핀이 올바르게 설정되었는지 확인
- NVIC 우선순위가 올바른지 확인
- 클록 설정이 64MHz로 올바른지 확인

---

이 가이드를 따라 설정하면 최적화된 로터리 인코더 코드가 정상적으로 동작할 것입니다.
