# Laser Module - STM32F103 Test

## 📌 개요

레이저 모듈을 NUCLEO-F103RB 보드에 연결하여 다양한 동작 모드를 테스트하는 프로젝트입니다. PWM을 이용한 밝기 조절, 점멸, SOS 신호 등의 기능을 구현합니다.

⚠️ **경고**: 레이저를 절대 눈에 직접 비추지 마세요! 적절한 레이저 안전 수칙을 준수하세요.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| Laser Module | 1 | KY-008 또는 호환 레이저 모듈 |
| Jumper Wires | 3 | 연결용 점퍼선 |

### 핀 연결

```
┌─────────────────┐         ┌──────────────────┐
│  Laser Module   │         │  NUCLEO-F103RB   │
├─────────────────┤         ├──────────────────┤
│     VCC     ────┼─────────┼──── 5V           │
│     GND     ────┼─────────┼──── GND          │
│     S       ────┼─────────┼──── PA0 (PWM)    │
└─────────────────┘         │                  │
                            │     PC13 ────────│──── User Button
                            │     PA5 ─────────│──── Built-in LED
                            └──────────────────┘
```

### 회로도

```
       +5V
        │
   ┌────┴────┐
   │  Laser  │
   │ Module  │
   │  ┌───┐  │
   │  │LD │  │          ┌──────────┐
   │  └───┘  │          │          │
   │    S────┼──────────┤ PA0(PWM) │
   │         │          │          │
   │   GND───┼──────────┤ GND      │
   └─────────┘          │          │
                        │  PC13    │◄── User Button
                        └──────────┘
```

## 📂 프로젝트 구조

```
laser/
├── main.c          # 메인 소스 코드
├── README.md       # 프로젝트 문서
└── Makefile        # 빌드 설정 (선택)
```

## 💻 소프트웨어 동작

### 동작 모드

| 모드 | 번호 | 설명 |
|------|------|------|
| OFF | 0 | 레이저 완전 꺼짐 |
| ON | 1 | 레이저 연속 켜짐 |
| BLINK | 2 | 500ms 간격 점멸 |
| PWM | 3 | 부드러운 밝기 변화 (Breathing) |
| SOS | 4 | 모스 부호 SOS 신호 |

### 모드 전환
- **User Button (PC13)**: 버튼을 누를 때마다 모드 순환 (OFF → ON → BLINK → PWM → SOS → OFF...)

### SOS 모스 부호 패턴

```
S = ・・・ (짧음 3회)
O = ─ ─ ─ (길음 3회)  
S = ・・・ (짧음 3회)

타이밍:
・ (짧음) = 200ms
─ (길음) = 600ms
간격     = 200ms
단어 간격 = 1000ms
```

### PWM 설정
| 파라미터 | 값 | 설명 |
|----------|-----|------|
| PWM 주파수 | 1kHz | 부드러운 밝기 제어 |
| Duty Cycle 범위 | 0-100% | 0-1000 값 |
| Breathing 속도 | ~3초 주기 | 한 사이클 (밝아짐→어두워짐) |

## 🚀 빌드 및 실행

### STM32CubeIDE 사용

1. **새 프로젝트 생성**
   - File → New → STM32 Project
   - Board Selector에서 NUCLEO-F103RB 선택

2. **핀 설정 (CubeMX)**
   - PA0: TIM2_CH1 (PWM Output)
   - PA5: GPIO_Output (LED)
   - PC13: GPIO_Input (User Button)
   - USART2: Asynchronous, 115200 baud
   - TIM2: PWM Generation CH1

3. **타이머 설정**
   ```
   TIM2 Configuration:
   - Prescaler: 63 (64MHz / 64 = 1MHz)
   - Counter Period: 999 (1MHz / 1000 = 1kHz PWM)
   - PWM Mode: PWM Mode 1
   ```

4. **코드 추가**
   - `main.c` 내용을 생성된 프로젝트에 적용

5. **빌드 및 다운로드**
   - Project → Build All
   - Run → Debug

### 시리얼 모니터 설정
- Baud Rate: 115200
- Data Bits: 8
- Stop Bits: 1
- Parity: None


```c
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MODE_OFF        0
#define MODE_ON         1
#define MODE_BLINK      2
#define MODE_PWM        3
#define MODE_SOS        4
#define NUM_MODES       5

#define BLINK_PERIOD_MS 500
#define PWM_PERIOD      1000    /* PWM period (ARR value) */
#define PWM_STEP        100     /* PWM duty cycle step */
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
volatile uint8_t current_mode = MODE_OFF;
volatile uint8_t button_pressed = 0;
volatile uint16_t pwm_duty = 500;  /* 50% duty cycle */
volatile uint8_t pwm_direction = 1;  /* 1: increasing, 0: decreasing */

/* SOS pattern: ... --- ... (3 short, 3 long, 3 short) */
const uint16_t sos_pattern[] = {200, 200, 200, 200, 200, 600,
                                600, 200, 600, 200, 600, 600,
                                200, 200, 200, 200, 200, 1000};
const uint8_t sos_length = 18;
/* USER CODE END PV */
```

```c
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief  Turn laser ON (100% duty)
 * @retval None
 */
void Laser_On(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, PWM_PERIOD);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

/**
 * @brief  Turn laser OFF (0% duty)
 * @retval None
 */
void Laser_Off(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief  Set laser PWM duty cycle
 * @param  duty: Duty cycle value (0-1000)
 * @retval None
 */
void Laser_SetPWM(uint16_t duty)
{
    if (duty > PWM_PERIOD) duty = PWM_PERIOD;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);

    /* LED brightness indication */
    if (duty > 500)
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief  Mode OFF - Laser completely off
 * @retval None
 */
void Run_Mode_Off(void)
{
    Laser_Off();
    HAL_Delay(100);
}

/**
 * @brief  Mode ON - Laser continuously on
 * @retval None
 */
void Run_Mode_On(void)
{
    Laser_On();
    HAL_Delay(100);
}

/**
 * @brief  Mode BLINK - Simple on/off blinking
 * @retval None
 */
void Run_Mode_Blink(void)
{
    static uint32_t last_toggle = 0;
    static uint8_t state = 0;

    if (HAL_GetTick() - last_toggle >= BLINK_PERIOD_MS)
    {
        state = !state;
        if (state)
            Laser_On();
        else
            Laser_Off();
        last_toggle = HAL_GetTick();
    }
}

/**
 * @brief  Mode PWM - Breathing effect with PWM
 * @retval None
 */
void Run_Mode_PWM(void)
{
    Laser_SetPWM(pwm_duty);

    /* Update duty cycle for breathing effect */
    if (pwm_direction)
    {
        pwm_duty += PWM_STEP / 5;
        if (pwm_duty >= PWM_PERIOD)
        {
            pwm_duty = PWM_PERIOD;
            pwm_direction = 0;
        }
    }
    else
    {
        if (pwm_duty >= PWM_STEP / 5)
            pwm_duty -= PWM_STEP / 5;
        else
        {
            pwm_duty = 0;
            pwm_direction = 1;
        }
    }

    HAL_Delay(20);
}

/**
 * @brief  Mode SOS - Morse code SOS pattern
 * @retval None
 */
void Run_Mode_SOS(void)
{
    static uint8_t pattern_index = 0;
    static uint8_t laser_state = 0;

    if (laser_state == 0)
    {
        Laser_On();
        laser_state = 1;
    }
    else
    {
        Laser_Off();
        laser_state = 0;
    }

    HAL_Delay(sos_pattern[pattern_index]);
    pattern_index = (pattern_index + 1) % sos_length;
}

/**
 * @brief  Print current mode to UART
 * @retval None
 */
void Print_Mode(void)
{
    char msg[50];
    const char* mode_names[] = {"OFF", "ON", "BLINK", "PWM", "SOS"};

    sprintf(msg, "[MODE] Current: %s\r\n", mode_names[current_mode]);
    UART_SendString(msg);
}

/**
 * @brief  Send string via UART
 * @param  str: String to send
 * @retval None
 */
void UART_SendString(char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  /* Start PWM */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  /* Welcome message */
  UART_SendString("\r\n========================================\r\n");
  UART_SendString("  Laser Module Test\r\n");
  UART_SendString("  NUCLEO-F103RB\r\n");
  UART_SendString("========================================\r\n");
  UART_SendString("WARNING: Never point laser at eyes!\r\n\r\n");
  UART_SendString("Modes (Press User Button to change):\r\n");
  UART_SendString("  0: OFF\r\n");
  UART_SendString("  1: ON (Continuous)\r\n");
  UART_SendString("  2: BLINK\r\n");
  UART_SendString("  3: PWM (Breathing)\r\n");
  UART_SendString("  4: SOS\r\n\r\n");

  /* Initial state */
  Laser_Off();
  Print_Mode();
  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* Check for button press (mode change) */
      if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
      {
          HAL_Delay(50);  /* Debounce */
          if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
          {
              /* Wait for button release */
              while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET);
              HAL_Delay(50);

              /* Change mode */
              current_mode = (current_mode + 1) % NUM_MODES;
              Print_Mode();

              /* Reset PWM for mode transitions */
              pwm_duty = 500;
              pwm_direction = 1;
          }
      }

      /* Execute current mode */
      switch (current_mode)
      {
          case MODE_OFF:
              Run_Mode_Off();
              break;
          case MODE_ON:
              Run_Mode_On();
              break;
          case MODE_BLINK:
              Run_Mode_Blink();
              break;
          case MODE_PWM:
              Run_Mode_PWM();
              break;
          case MODE_SOS:
              Run_Mode_SOS();
              break;
          default:
              Run_Mode_Off();
              break;
      }
    /* USER CODE END WHILE */
```
## 📊 출력 예시

```
========================================
  Laser Module Test
  NUCLEO-F103RB
========================================
WARNING: Never point laser at eyes!

Modes (Press User Button to change):
  0: OFF
  1: ON (Continuous)
  2: BLINK
  3: PWM (Breathing)
  4: SOS

[MODE] Current: OFF
[MODE] Current: ON
[MODE] Current: BLINK
[MODE] Current: PWM
[MODE] Current: SOS
[MODE] Current: OFF
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결 방법 |
|------|------|----------|
| 레이저가 켜지지 않음 | 전원 불량 | 5V 전원 확인 |
| PWM이 동작하지 않음 | 타이머 설정 오류 | TIM2_CH1 설정 확인 |
| 버튼 반응 없음 | 디바운싱 문제 | 버튼 연결 및 딜레이 확인 |
| 밝기 조절 안됨 | 모듈 PWM 미지원 | 모듈 사양 확인 |

## ⚠️ 안전 주의사항

1. **눈 보호**: 레이저를 절대 눈에 직접 비추지 마세요
2. **반사 주의**: 거울 등 반사체 주변에서 주의
3. **어린이 주의**: 어린이의 손이 닿지 않는 곳에 보관
4. **클래스 확인**: 대부분의 모듈은 Class 3R 이하 (5mW 미만)

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [Laser Safety Standards (IEC 60825)](https://www.iec.ch/homepage)


