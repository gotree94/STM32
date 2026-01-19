# Button Switch Module - STM32F103 Test

## 📌 개요

버튼 스위치 모듈을 NUCLEO-F103RB 보드에 연결하여 다양한 버튼 이벤트를 감지하는 프로젝트입니다. 짧은 누름, 긴 누름, 더블 클릭을 구분하여 감지하며, 소프트웨어 디바운싱을 구현합니다.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| Button Switch Module | 1 | 버튼 스위치 모듈 (KY-004 등) |
| Jumper Wires | 3 | 연결용 점퍼선 |

### 핀 연결

```
┌─────────────────┐         ┌──────────────────┐
│  Button Module  │         │  NUCLEO-F103RB   │
├─────────────────┤         ├──────────────────┤
│     VCC     ────┼─────────┼──── 3.3V         │
│     GND     ────┼─────────┼──── GND          │
│     S       ────┼─────────┼──── PA0          │
└─────────────────┘         │                  │
                            │     PA5 ─────────│──── Built-in LED
                            └──────────────────┘
```

### 회로도

```
       +3.3V
         │
    ┌────┴────┐
    │ Button  │
    │ Module  │        Internal Pull-up
    │  ┌───┐  │              │
    │  │ SW│──┼──────────────┼──── PA0
    │  └───┘  │              │
    │   GND───┼──────────────┴──── GND
    └─────────┘

    동작: 버튼 미누름 = HIGH (Pull-up)
          버튼 누름   = LOW (GND 연결)
```

## 📂 프로젝트 구조

```
button_switch/
├── main.c          # 메인 소스 코드
├── README.md       # 프로젝트 문서
└── Makefile        # 빌드 설정 (선택)
```

## 💻 소프트웨어 동작

### 감지 이벤트

| 이벤트 | 조건 | LED 피드백 |
|--------|------|-----------|
| Short Press | 누름 < 1초 | 누르는 동안 켜짐 |
| Long Press | 누름 >= 1초 | 3회 깜빡임 |
| Double Click | 2회 클릭 간격 < 300ms | 5회 빠른 깜빡임 |

### 상태 다이어그램

```
                    ┌────────────────┐
                    │     IDLE       │
                    │  (Button HIGH) │
                    └───────┬────────┘
                            │ Button Pressed
                            ▼
                    ┌────────────────┐
                    │   PRESSED      │
                    │  Start Timer   │
                    └───────┬────────┘
                            │
              ┌─────────────┼─────────────┐
              │             │             │
              ▼             ▼             ▼
     ┌────────────┐  ┌────────────┐  ┌────────────┐
     │ < 1 sec    │  │ >= 1 sec   │  │ 2nd Click  │
     │            │  │            │  │ < 300ms    │
     └─────┬──────┘  └─────┬──────┘  └─────┬──────┘
           │               │               │
           ▼               ▼               ▼
     ┌──────────┐    ┌──────────┐    ┌──────────┐
     │  SHORT   │    │   LONG   │    │  DOUBLE  │
     │  PRESS   │    │  PRESS   │    │  CLICK   │
     └──────────┘    └──────────┘    └──────────┘
```

### 디바운싱 알고리즘

```c
/* 타이밍 파라미터 */
#define DEBOUNCE_MS         50      /* 채터링 방지 */
#define LONG_PRESS_MS       1000    /* 긴 누름 기준 */
#define DOUBLE_CLICK_MS     300     /* 더블 클릭 창 */
```

## 🚀 빌드 및 실행

### STM32CubeIDE 사용

1. **새 프로젝트 생성**
   - File → New → STM32 Project
   - Board Selector에서 NUCLEO-F103RB 선택

2. **핀 설정 (CubeMX)**
   - PA0: GPIO_Input (Pull-up)
   - PA5: GPIO_Output (LED)
   - USART2: Asynchronous, 115200 baud

3. **코드 추가**
   - `main.c` 내용을 생성된 프로젝트에 적용

4. **빌드 및 다운로드**
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
/* USER CODE BEGIN PD */
#define DEBOUNCE_MS         50      /* Debounce time in ms */
#define LONG_PRESS_MS       1000    /* Long press threshold */
#define DOUBLE_CLICK_MS     300     /* Double click window */
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
volatile uint32_t button_press_time = 0;
volatile uint32_t button_release_time = 0;
volatile uint32_t last_click_time = 0;
volatile uint8_t button_state = 0;
volatile uint8_t click_count = 0;
volatile uint32_t press_count = 0;
volatile uint8_t pending_click = 0;

/* Event flags */
volatile uint8_t event_short_press = 0;
volatile uint8_t event_long_press = 0;
volatile uint8_t event_double_click = 0;
/* USER CODE END PV */
```

```c
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief  Process button state and detect events
 * @retval None
 */
void Process_Button(void)
{
    static uint8_t prev_state = 1;  /* Assume pull-up (idle high) */
    static uint32_t state_change_time = 0;
    static uint8_t debounced_state = 1;
    static uint8_t is_pressed = 0;

    uint8_t current_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    uint32_t current_time = HAL_GetTick();

    /* State change detection with debounce */
    if (current_state != prev_state)
    {
        state_change_time = current_time;
        prev_state = current_state;
    }

    /* Apply debounce */
    if ((current_time - state_change_time) >= DEBOUNCE_MS)
    {
        if (current_state != debounced_state)
        {
            debounced_state = current_state;

            /* Button pressed (active low) */
            if (debounced_state == GPIO_PIN_RESET)
            {
                button_press_time = current_time;
                is_pressed = 1;
                press_count++;

                /* LED ON */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

                char msg[50];
                sprintf(msg, "[PRESS] Button pressed (#%lu)\r\n", press_count);
                UART_SendString(msg);
            }
            /* Button released */
            else if (is_pressed)
            {
                button_release_time = current_time;
                uint32_t press_duration = button_release_time - button_press_time;
                is_pressed = 0;

                /* LED OFF */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

                char msg[80];
                sprintf(msg, "[RELEASE] Duration: %lu ms\r\n", press_duration);
                UART_SendString(msg);

                /* Determine event type */
                if (press_duration >= LONG_PRESS_MS)
                {
                    /* Long press detected */
                    event_long_press = 1;
                    pending_click = 0;
                    click_count = 0;
                }
                else
                {
                    /* Short press - check for double click */
                    if ((current_time - last_click_time) < DOUBLE_CLICK_MS && pending_click)
                    {
                        /* Double click detected */
                        event_double_click = 1;
                        pending_click = 0;
                        click_count = 0;
                    }
                    else
                    {
                        /* Potential single click - wait for double click window */
                        pending_click = 1;
                        click_count = 1;
                    }
                    last_click_time = current_time;
                }
            }
        }
    }

    /* Check for single click timeout (no second click) */
    if (pending_click && (current_time - last_click_time) >= DOUBLE_CLICK_MS)
    {
        event_short_press = 1;
        pending_click = 0;
        click_count = 0;
    }

    /* Check for long press while still holding */
    if (is_pressed && (current_time - button_press_time) >= LONG_PRESS_MS)
    {
        static uint8_t long_press_reported = 0;
        if (!long_press_reported)
        {
            UART_SendString("[HOLDING] Long press threshold reached\r\n");
            long_press_reported = 1;
        }
    }
    else
    {
        static uint8_t long_press_reported = 0;
        long_press_reported = 0;
    }
}

/**
 * @brief  Handle detected button events
 * @retval None
 */
void Handle_Events(void)
{
    if (event_short_press)
    {
        event_short_press = 0;
        UART_SendString(">>> EVENT: SHORT PRESS <<<\r\n");
        UART_SendString("    Action: Toggle something\r\n\r\n");

        /* Add your short press action here */
    }

    if (event_long_press)
    {
        event_long_press = 0;
        UART_SendString(">>> EVENT: LONG PRESS <<<\r\n");
        UART_SendString("    Action: Enter settings mode\r\n\r\n");

        /* Add your long press action here */
        /* Blink LED to indicate long press */
        for (int i = 0; i < 3; i++)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            HAL_Delay(100);
        }
    }

    if (event_double_click)
    {
        event_double_click = 0;
        UART_SendString(">>> EVENT: DOUBLE CLICK <<<\r\n");
        UART_SendString("    Action: Special function\r\n\r\n");

        /* Add your double click action here */
        /* Quick blinks to indicate double click */
        for (int i = 0; i < 5; i++)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            HAL_Delay(50);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            HAL_Delay(50);
        }
    }
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
  /* Welcome message */
  UART_SendString("\r\n========================================\r\n");
  UART_SendString("  Button Switch Module Test\r\n");
  UART_SendString("  NUCLEO-F103RB\r\n");
  UART_SendString("========================================\r\n");
  UART_SendString("Button Events Detected:\r\n");
  UART_SendString("  - Short Press (< 1 sec)\r\n");
  UART_SendString("  - Long Press (>= 1 sec)\r\n");
  UART_SendString("  - Double Click (< 300ms)\r\n\r\n");
  UART_SendString("Press the button to start...\r\n\r\n");

  /* Initial state read */
  button_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* Process button state */
      Process_Button();

      /* Handle detected events */
      Handle_Events();

      HAL_Delay(5);
    /* USER CODE END WHILE */
```

## 📊 출력 예시

```
========================================
  Button Switch Module Test
  NUCLEO-F103RB
========================================
Button Events Detected:
  - Short Press (< 1 sec)
  - Long Press (>= 1 sec)
  - Double Click (< 300ms)

Press the button to start...

[PRESS] Button pressed (#1)
[RELEASE] Duration: 150 ms
>>> EVENT: SHORT PRESS <<<
    Action: Toggle something

[PRESS] Button pressed (#2)
[RELEASE] Duration: 1250 ms
>>> EVENT: LONG PRESS <<<
    Action: Enter settings mode

[PRESS] Button pressed (#3)
[RELEASE] Duration: 80 ms
[PRESS] Button pressed (#4)
[RELEASE] Duration: 90 ms
>>> EVENT: DOUBLE CLICK <<<
    Action: Special function
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결 방법 |
|------|------|----------|
| 버튼 반응 없음 | 배선 오류 | S핀 연결 확인 |
| 채터링 발생 | 디바운스 부족 | DEBOUNCE_MS 값 증가 |
| 더블클릭 감지 안됨 | 클릭 간격 너무 김 | DOUBLE_CLICK_MS 값 증가 |
| 긴 누름 감지 안됨 | 임계값 문제 | LONG_PRESS_MS 값 조정 |
| 항상 눌림 상태 | Pull-up 설정 오류 | GPIO_PULLUP 설정 확인 |

### 파라미터 튜닝 가이드

사용자 특성에 따라 조정:

```c
/* 노인 또는 느린 반응 */
#define DEBOUNCE_MS         100
#define LONG_PRESS_MS       1500
#define DOUBLE_CLICK_MS     500

/* 게이머 또는 빠른 반응 */
#define DEBOUNCE_MS         30
#define LONG_PRESS_MS       800
#define DOUBLE_CLICK_MS     200
```

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [Debouncing Techniques](https://www.embedded.com/my-favorite-software-debouncers/)


