# NUCLEO-F767ZI LED Blink with USART Printf

STM32 NUCLEO-F767ZI 보드를 이용한 LED Blink 및 USART Printf 예제 프로젝트입니다.

## 📋 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F767ZI |
| MCU | STM32F767ZIT6 (ARM Cortex-M7, 216MHz) |
| IDE | STM32CubeIDE |
| 기능 | LD1/LD3 LED 토글 + USART3 Printf 출력 |

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

> NUCLEO 보드의 ST-LINK는 Virtual COM Port를 제공하며, USART3에 연결되어 있습니다.

## ⚙️ 프로젝트 생성 절차

### 1. STM32CubeIDE 프로젝트 생성

1. **File → New → STM32 Project**
2. Board Selector에서 `NUCLEO-F767ZI` 선택
3. 프로젝트 이름 입력 (예: `LED_Blink_USART`)
4. Targeted Language: **C**
5. **Finish** 클릭

### 2. CubeMX 설정 (.ioc 파일)

#### 2.1 RCC 설정

**Pinout & Configuration → System Core → RCC**

| 항목 | 설정값 |
|------|--------|
| HSE | Crystal/Ceramic Resonator |
| LSE | Crystal/Ceramic Resonator (선택) |

**Clock Configuration 탭:**

| 파라미터 | 값 |
|----------|-----|
| PLL Source | HSE |
| PLLM | /8 |
| PLLN | ×432 |
| PLLP | /2 |
| PLLQ | /9 |
| **SYSCLK** | **216 MHz** |
| AHB Prescaler | /1 |
| APB1 Prescaler | /4 (54 MHz) |
| APB2 Prescaler | /2 (108 MHz) |

> 💡 **Tip**: HCLK 입력란에 `216`을 입력하고 Enter를 누르면 자동으로 최적의 PLL 값이 계산됩니다.

#### 2.2 GPIO 설정 (LED)

보드 선택 시 자동 설정됨. 확인만 필요:

**System Core → GPIO**

| 핀 | Mode | Output Level | User Label |
|----|------|--------------|------------|
| PB0 | Output Push Pull | Low | LD1 |
| PB14 | Output Push Pull | Low | LD3 |

#### 2.3 USART3 설정

**Connectivity → USART3**

| 항목 | 설정값 |
|------|--------|
| Mode | Asynchronous |
| Baud Rate | 115200 |
| Word Length | 8 Bits |
| Parity | None |
| Stop Bits | 1 |

#### 2.4 코드 생성

**Ctrl+S** 또는 **Project → Generate Code**

## 💻 소스 코드

### main.c

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* USER CODE BEGIN 0 */

// printf 리다이렉션 (USART3)
#ifdef __GNUC__
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#else
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

// scanf 리다이렉션 (선택)
#ifdef __GNUC__
int __io_getchar(void)
{
    uint8_t ch;
    HAL_UART_Receive(&huart3, &ch, 1, HAL_MAX_DELAY);
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
    printf("\r\n=================================\r\n");
    printf("  NUCLEO-F767ZI LED Blink Demo\r\n");
    printf("  System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    printf("=================================\r\n\n");
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    uint32_t count = 0;
    while (1)
    {
        // LD1, LD3 토글
        HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
        HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

        // 상태 출력
        printf("[%5lu] LD1: %s, LD3: %s\r\n",
               count++,
               HAL_GPIO_ReadPin(LD1_GPIO_Port, LD1_Pin) ? "ON " : "OFF",
               HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin) ? "ON " : "OFF");

        HAL_Delay(500);

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
```

## 🖥️ 시리얼 터미널 설정

### 터미널 프로그램

- **Windows**: PuTTY, Tera Term, RealTerm
- **Linux**: minicom, screen
- **macOS**: screen, CoolTerm

### 연결 설정

| 항목 | 값 |
|------|-----|
| Port | COMx (Windows) / /dev/ttyACMx (Linux) |
| Baud Rate | 115200 |
| Data Bits | 8 |
| Parity | None |
| Stop Bits | 1 |
| Flow Control | None |

### Linux 터미널 명령

```bash
# 포트 확인
ls /dev/ttyACM*

# minicom 사용
sudo minicom -D /dev/ttyACM0 -b 115200

# screen 사용
screen /dev/ttyACM0 115200
```

## 📤 빌드 및 다운로드

1. **빌드**: `Ctrl+B` 또는 Project → Build Project
2. **다운로드/디버그**: `F11` 또는 Run → Debug As → STM32 C/C++ Application
3. **실행**: `F8` (Resume)

## 📺 예상 출력

```
=================================
  NUCLEO-F767ZI LED Blink Demo
  System Clock: 216 MHz
=================================

[    0] LD1: ON , LD3: ON 
[    1] LD1: OFF, LD3: OFF
[    2] LD1: ON , LD3: ON 
[    3] LD1: OFF, LD3: OFF
...
```

## 🔍 트러블슈팅

### LED가 동작하지 않는 경우

- [ ] GPIO 핀이 Output으로 설정되었는지 확인
- [ ] User Label (LD1, LD3)이 올바르게 지정되었는지 확인
- [ ] 코드 생성 후 빌드했는지 확인

### 시리얼 출력이 안 되는 경우

- [ ] USART3 활성화 여부 확인
- [ ] TX/RX 핀이 PD8/PD9로 설정되었는지 확인
- [ ] Baud Rate가 터미널과 일치하는지 확인
- [ ] ST-LINK 펌웨어 업데이트 필요 여부 확인

### 글자가 깨지는 경우

- [ ] Baud Rate 일치 여부 확인
- [ ] RCC 클럭 설정이 올바른지 확인 (잘못된 클럭 설정은 UART Baud Rate에 영향)

## 📁 프로젝트 구조

```
LED_Blink_USART/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f7xx_hal_conf.h
│   │   └── stm32f7xx_it.h
│   └── Src/
│       ├── main.c
│       ├── stm32f7xx_hal_msp.c
│       ├── stm32f7xx_it.c
│       └── system_stm32f7xx.c
├── Drivers/
│   ├── CMSIS/
│   └── STM32F7xx_HAL_Driver/
├── LED_Blink_USART.ioc
└── README.md
```

## 📚 참고 자료

- [NUCLEO-F767ZI User Manual (UM1974)](https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf)
- [STM32F767ZI Datasheet](https://www.st.com/resource/en/datasheet/stm32f767zi.pdf)
- [STM32F7 HAL Driver Manual](https://www.st.com/resource/en/user_manual/um1905-description-of-stm32f7-hal-and-lowlayer-drivers-stmicroelectronics.pdf)

## 📝 라이선스

This project is licensed under the MIT License.

## ✍️ Author

Created for STM32 embedded systems learning and development.
