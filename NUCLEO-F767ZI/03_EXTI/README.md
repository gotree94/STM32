# NUCLEO-F767ZI User Button EXTI Interrupt

STM32 NUCLEO-F767ZI 보드의 파란색 User Button을 이용한 외부 인터럽트(EXTI) 예제입니다.

## 📋 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F767ZI |
| MCU | STM32F767ZIT6 (ARM Cortex-M7, 216MHz) |
| IDE | STM32CubeIDE |
| 기능 | User Button(B1) 인터럽트로 LED 토글 + USART3 상태 출력 |

## 🔧 하드웨어 구성

<img width="1193" height="896" alt="2001" src="https://github.com/user-attachments/assets/535743ad-a4a8-4d9f-bdeb-60bcd91bd117" />
<br>
<img width="1193" height="896" alt="2002" src="https://github.com/user-attachments/assets/fee12d3a-760f-454c-9d9a-97d265e10af4" />
<br>
<img width="1193" height="896" alt="2003" src="https://github.com/user-attachments/assets/966def81-cdb9-41ee-823a-c6782c48fc3e" />
<br>

### User Button

| 버튼 | GPIO | 특성 |
|------|------|------|
| B1 (Blue) | **PC13** | Active High (누르면 HIGH) |

> 💡 NUCLEO-F767ZI의 User Button은 **Active High** 방식입니다. 버튼을 누르면 PC13이 HIGH가 됩니다.

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

## ⚙️ CubeMX 설정

### 1. RCC 설정

**Pinout & Configuration → System Core → RCC**

| 항목 | 설정값 |
|------|--------|
| HSE | **BYPASS Clock Source** |

> ⚠️ NUCLEO 보드는 ST-LINK MCO에서 8MHz 클럭을 공급받으므로 BYPASS 선택

**Clock Configuration:**

| 파라미터 | 값 |
|----------|-----|
| SYSCLK | 216 MHz |
| APB1 | 54 MHz |
| APB2 | 108 MHz |

### 2. GPIO 설정 (User Button - EXTI)

**Pinout & Configuration → System Core → GPIO**

Pinout view에서 **PC13** 클릭 → **GPIO_EXTI13** 선택

또는 보드 선택 시 자동 설정된 경우 확인:

**GPIO → PC13 설정:**

| 항목 | 설정값 |
|------|--------|
| GPIO mode | **External Interrupt Mode with Rising edge trigger detection** |
| GPIO Pull-up/Pull-down | **No pull-up and no pull-down** |
| User Label | **USER_Btn** |

> 💡 **Rising Edge**: 버튼을 누르는 순간 인터럽트 발생  
> 💡 **Falling Edge**: 버튼을 떼는 순간 인터럽트 발생  
> 💡 **Rising/Falling Edge**: 누르거나 떼는 순간 모두 인터럽트 발생

### 3. NVIC 설정 (인터럽트 활성화)

**Pinout & Configuration → System Core → NVIC**

| 인터럽트 | Enable | Preemption Priority |
|----------|--------|---------------------|
| EXTI line[15:10] interrupts | ✅ **체크** | 0 (기본값) |

> ⚠️ PC13은 EXTI Line 13이므로 **EXTI line[15:10]** 인터럽트를 활성화해야 합니다.

### 4. GPIO 설정 (LED)

| 핀 | Mode | User Label |
|----|------|------------|
| PB0 | Output Push Pull | LD1 |
| PB14 | Output Push Pull | LD3 |

### 5. USART3 설정

**Connectivity → USART3**

| 항목 | 설정값 |
|------|--------|
| Mode | Asynchronous |
| Baud Rate | 115200 |
| Word Length | 8 Bits |
| Parity | None |
| Stop Bits | 1 |

### 6. 코드 생성

**Ctrl+S** 또는 **Project → Generate Code**

## 💻 소스 코드

### main.c

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
volatile uint32_t button_press_count = 0;
volatile uint8_t button_pressed_flag = 0;
/* USER CODE END PV */

/* USER CODE BEGIN 0 */

// printf 리다이렉션 (USART3)
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
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART3_UART_Init();

    /* USER CODE BEGIN 2 */
    printf("\r\n==========================================\r\n");
    printf("  NUCLEO-F767ZI EXTI Button Interrupt Demo\r\n");
    printf("  System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    printf("==========================================\r\n");
    printf("Press the Blue User Button (B1) to toggle LEDs\r\n\n");
    /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // 버튼 인터럽트 플래그 확인 (메인 루프에서 처리)
      if (button_pressed_flag)
      {
          button_pressed_flag = 0;

          printf("[%3lu] Button Pressed! LD1: %s, LD2: %s, LD3: %s\r\n",
                 button_press_count,
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

### stm32f7xx_it.c (인터럽트 핸들러)

```c
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* USER CODE BEGIN EV */
extern volatile uint32_t button_press_count;
extern volatile uint8_t button_pressed_flag;
/* USER CODE END EV */

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI15_10_IRQn 0 */

    /* USER CODE END EXTI15_10_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(USER_Btn_Pin);
    /* USER CODE BEGIN EXTI15_10_IRQn 1 */

    /* USER CODE END EXTI15_10_IRQn 1 */
}
```

### EXTI Callback 함수 (main.c 또는 별도 파일)

```c
/* USER CODE BEGIN 4 */

/**
  * @brief  EXTI line detection callback
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == USER_Btn_Pin)  // PC13
    {
        // LED 토글
        HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

        // 카운터 증가 및 플래그 설정
        button_press_count++;
        button_pressed_flag = 1;
    }
}

/* USER CODE END 4 */
```

## 🔄 동작 방식

```
┌─────────────────────────────────────────────────────────┐
│                     Button Press                         │
│                         │                                │
│                         ▼                                │
│  ┌─────────────────────────────────────────────────┐    │
│  │              Rising Edge on PC13                 │    │
│  └─────────────────────────────────────────────────┘    │
│                         │                                │
│                         ▼                                │
│  ┌─────────────────────────────────────────────────┐    │
│  │           EXTI15_10_IRQHandler()                 │    │
│  │                     │                            │    │
│  │                     ▼                            │    │
│  │         HAL_GPIO_EXTI_IRQHandler()               │    │
│  │                     │                            │    │
│  │                     ▼                            │    │
│  │         HAL_GPIO_EXTI_Callback()                 │    │
│  │              - Toggle LEDs                       │    │
│  │              - Set flag                          │    │
│  └─────────────────────────────────────────────────┘    │
│                         │                                │
│                         ▼                                │
│  ┌─────────────────────────────────────────────────┐    │
│  │              Main Loop                           │    │
│  │              - Check flag                        │    │
│  │              - Print status via USART            │    │
│  └─────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────┘
```

## 🛡️ 디바운싱 (선택사항)

기계식 버튼은 채터링(Chattering) 현상이 발생할 수 있습니다. 소프트웨어 디바운싱을 추가하려면:

### 방법 1: 간단한 딜레이

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_interrupt_time = 0;
    uint32_t current_time = HAL_GetTick();

    if (GPIO_Pin == USER_Btn_Pin)
    {
        // 50ms 이내 재입력 무시 (디바운싱)
        if (current_time - last_interrupt_time > 50)
        {
            HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

            button_press_count++;
            button_pressed_flag = 1;

            last_interrupt_time = current_time;
        }
    }
}
```

### 방법 2: 타이머 기반 디바운싱

```c
/* USER CODE BEGIN PV */
volatile uint8_t debounce_active = 0;
/* USER CODE END PV */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == USER_Btn_Pin && !debounce_active)
    {
        debounce_active = 1;

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

        button_press_count++;
        button_pressed_flag = 1;

        // 타이머로 50ms 후 debounce_active 해제
        HAL_TIM_Base_Start_IT(&htim6);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        debounce_active = 0;
        HAL_TIM_Base_Stop_IT(&htim6);
    }
}
```

## 📺 예상 출력

```
==========================================
  NUCLEO-F767ZI EXTI Button Interrupt Demo
  System Clock: 216 MHz
==========================================
Press the Blue User Button (B1) to toggle LEDs

[  1] Button Pressed! LD1: ON , LD2: ON , LD3: ON 
[  2] Button Pressed! LD1: OFF, LD2: OFF, LD3: OFF
[  3] Button Pressed! LD1: ON , LD2: ON , LD3: ON 
[  4] Button Pressed! LD1: OFF, LD2: OFF, LD3: OFF
...
```

## 🔍 트러블슈팅

### 버튼을 눌러도 반응이 없는 경우

- [ ] PC13이 GPIO_EXTI13으로 설정되었는지 확인
- [ ] NVIC에서 **EXTI line[15:10] interrupts** 활성화 확인
- [ ] `HAL_GPIO_EXTI_Callback()` 함수가 구현되었는지 확인
- [ ] User Label이 `USER_Btn`으로 설정되었는지 확인

### 버튼 한 번 눌렀는데 여러 번 인식되는 경우

- [ ] 디바운싱 코드 추가 필요
- [ ] Rising Edge만 사용 (Rising/Falling 동시 사용 시 2번 인식됨)

### 인터럽트가 발생하지 않는 경우

- [ ] GPIO mode가 **External Interrupt Mode**인지 확인
- [ ] `MX_GPIO_Init()` 이후에 NVIC 설정이 되는지 확인
- [ ] `stm32f7xx_it.c`에 `EXTI15_10_IRQHandler()` 함수 존재 확인

## 📁 프로젝트 구조

```
EXTI_Button/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f7xx_hal_conf.h
│   │   └── stm32f7xx_it.h
│   └── Src/
│       ├── main.c                 # 메인 로직 + Callback
│       ├── stm32f7xx_hal_msp.c
│       ├── stm32f7xx_it.c         # IRQ Handler
│       └── system_stm32f7xx.c
├── Drivers/
│   ├── CMSIS/
│   └── STM32F7xx_HAL_Driver/
├── EXTI_Button.ioc
└── README.md
```

## 📚 EXTI Line 매핑 참고

| GPIO Pin | EXTI Line | IRQ Handler |
|----------|-----------|-------------|
| Px0 | EXTI0 | EXTI0_IRQHandler |
| Px1 | EXTI1 | EXTI1_IRQHandler |
| Px2 | EXTI2 | EXTI2_IRQHandler |
| Px3 | EXTI3 | EXTI3_IRQHandler |
| Px4 | EXTI4 | EXTI4_IRQHandler |
| Px5~Px9 | EXTI5~9 | EXTI9_5_IRQHandler |
| Px10~Px15 | EXTI10~15 | **EXTI15_10_IRQHandler** |

> PC13은 EXTI Line 13이므로 `EXTI15_10_IRQHandler`에서 처리됩니다.

## 📚 참고 자료

- [NUCLEO-F767ZI User Manual (UM1974)](https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf)
- [STM32F767ZI Reference Manual (RM0410)](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F7 HAL Driver - GPIO](https://www.st.com/resource/en/user_manual/um1905-description-of-stm32f7-hal-and-lowlayer-drivers-stmicroelectronics.pdf)

## 📝 라이선스

This project is licensed under the MIT License.

## ✍️ Author

Created for STM32 embedded systems learning and development.
