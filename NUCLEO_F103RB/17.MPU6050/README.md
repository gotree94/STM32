# MPU6050

<img width="382" height="289" alt="124" src="https://github.com/user-attachments/assets/66e7c924-b961-4ac9-84db-a953738b1a70" />


# STM32F103 + MPU-6050 CubeMX Configuration Guide

## 🎯 프로젝트 개요
- **MCU**: STM32F103C8T6 (Blue Pill) 또는 STM32F103RB (Nucleo)
- **센서**: MPU-6050 (가속도계 + 자이로스코프)
- **통신**: I2C (센서), UART (PC 통신)
- **클럭**: 64MHz
- **기능**: 실시간 IMU 데이터 Processing 시각화

---

## 📋 1. 새 프로젝트 생성

### 1.1 STM32CubeMX 실행
1. **File** → **New STM32 Project**
2. **Board Selector** 탭에서:
   - **Blue Pill**: `STM32F103C8` 검색 후 선택
   - **Nucleo**: `NUCLEO-F103RB` 검색 후 선택
3. **Start Project** 클릭

### 1.2 프로젝트 설정
- **Project Name**: `MPU6050_Project`
- **Toolchain**: `STM32CubeIDE`
- **Language**: `C`

---

## ⚙️ 2. 클럭 설정 (Clock Configuration)

### 2.1 시스템 클럭 구성
```
HSE (High Speed External): 8MHz ✓ Crystal/Ceramic Resonator
PLL (Phase Locked Loop): ENABLE ✓
PLL Source: HSE
PLL Multiplication Factor: x8
System Clock Source: PLLCLK

결과:
- SYSCLK: 64MHz
- AHB: 64MHz  
- APB1: 32MHz (Prescaler: /2)
- APB2: 64MHz (Prescaler: /1)
```

### 2.2 설정 순서
1. **RCC** → **HSE** → **Crystal/Ceramic Resonator** 선택
2. **Clock Configuration** 탭 이동
3. **Input frequency**: `8` 입력
4. **PLL** 체크박스 ✓
5. **x8** 선택 (Multiplication factor)
6. **System Clock Mux**: **PLLCLK** 선택
7. **HCLK**: `64` 확인
8. **APB1 Prescaler**: `/2` 설정
9. **APB2 Prescaler**: `/1` 설정

---

## 🔌 3. 핀 설정 (Pinout & Configuration)

### 3.1 GPIO 설정 (LED)

#### Blue Pill (STM32F103C8)
| 핀 | 기능 | 설정 |
|---|---|---|
| **PA5** | LD2 (User LED) | GPIO_Output |

#### Nucleo (STM32F103RB)  
| 핀 | 기능 | 설정 |
|---|---|---|
| **PA5** | LD2 (User LED) | GPIO_Output (기본 설정됨) |

**설정 방법:**
- **PA5** 핀 클릭 → **GPIO_Output** 선택

### 3.2 I2C1 설정 (MPU-6050 통신)

| 핀 | 기능 | 연결 |
|---|---|---|
| **PB8** | I2C1_SCL | MPU-6050 SCL |
| **PB9** | I2C1_SDA | MPU-6050 SDA |

**설정 방법:**
1. **PB8** 핀 클릭 → **I2C1_SCL** 선택
2. **PB9** 핀 클릭 → **I2C1_SDA** 선택

### 3.3 USART1 설정 (PC 통신)

| 핀 | 기능 | 연결 |
|---|---|---|
| **PA9** | USART1_TX | USB-Serial TX |
| **PA10** | USART1_RX | USB-Serial RX |

**설정 방법:**
1. **PA9** 핀 클릭 → **USART1_TX** 선택  
2. **PA10** 핀 클릭 → **USART1_RX** 선택

### 3.4 TIM2 설정 (타이머)

**Connectivity** → **TIM2** → **Activated** ✓

---

## 🛠️ 4. 주변 장치 구성

### 4.1 I2C1 Configuration
**Connectivity** → **I2C1**

| 파라미터 | 설정값 |
|---------|--------|
| **I2C Speed Mode** | Standard Mode |
| **I2C Clock Speed** | 400000 Hz (400kHz) |
| **Duty Cycle** | 2 |
| **Own Address Length** | 7-bit |
| **Primary Address Length** | 7-bit |
| **General Call Address Detection** | Disable |
| **Clock No Stretch Mode** | Disable |

### 4.2 USART1 Configuration  
**Connectivity** → **USART1**

| 파라미터 | 설정값 |
|---------|--------|
| **Mode** | Asynchronous |
| **Baud Rate** | 115200 Bits/s |
| **Word Length** | 8 Bits |
| **Parity** | None |
| **Stop Bits** | 1 |
| **Data Direction** | Receive and Transmit |
| **Over Sampling** | 16 Samples |

### 4.3 TIM2 Configuration
**Timers** → **TIM2**

| 파라미터 | 설정값 | 계산 |
|---------|--------|-----|
| **Prescaler** | 63999 | 64MHz / 64000 = 1kHz |
| **Counter Mode** | Up |  
| **Counter Period** | 999 | 1000 counts |
| **auto-reload preload** | Disable |

**결과**: 1ms 해상도, 1초 오버플로우

### 4.4 GPIO Configuration

#### PA5 (LD2) 설정
**System Core** → **GPIO** → **PA5**

| 파라미터 | 설정값 |
|---------|--------|
| **GPIO mode** | Output Push Pull |
| **GPIO Pull-up/Pull-down** | No pull-up and no pull-down |
| **Maximum output speed** | Low |
| **User Label** | LD2 |

---

## 🔧 5. 시스템 구성

### 5.1 SYS Configuration
**System Core** → **SYS**

| 파라미터 | 설정값 |
|---------|--------|
| **Timebase Source** | SysTick |
| **Debug** | Serial Wire |

### 5.2 NVIC Configuration
**System Core** → **NVIC**

**Enable interrupts:**
- ✅ **USART1 global interrupt**
- ✅ **TIM2 global interrupt** (optional)

---

## 📦 6. 프로젝트 생성 및 설정

### 6.1 Project Manager Settings
**Project Manager** 탭

| 설정 | 값 |
|------|---|
| **Project Name** | MPU6050_Project |
| **Project Location** | 원하는 경로 |
| **Toolchain / IDE** | STM32CubeIDE |
| **Project Settings** | |
| **Application Structure** | Basic |
| **Code Generation** | |
| **Generate peripheral initialization as a pair of '.c/.h' files per peripheral** | ✅ |
| **Backup previously generated files when re-generating** | ✅ |
| **Delete previously generated files when not re-generated** | ✅ |

### 6.2 Code Generation Options
**Advanced Settings:**

| Driver | Class | ✅/❌ |
|--------|-------|------|
| **I2C** | HAL | ✅ |
| **USART** | HAL | ✅ |
| **GPIO** | HAL | ✅ |
| **TIM** | HAL | ✅ |

---

## 🔌 7. 하드웨어 연결

### 7.1 MPU-6050 연결
```
STM32F103        MPU-6050
━━━━━━━━━        ━━━━━━━━
PB8 (SCL)   ←→   SCL
PB9 (SDA)   ←→   SDA  
3.3V        ←→   VCC
GND         ←→   GND
```

### 7.2 USB-Serial 연결 (디버깅)
```
STM32F103        USB-Serial
━━━━━━━━━        ━━━━━━━━━━
PA9 (TX)    ←→   RX
PA10 (RX)   ←→   TX
GND         ←→   GND
```

### 7.3 전원 공급
- **USB**: 5V → 3.3V 레귤레이터 통해 공급
- **External**: 3.3V 직접 공급 (권장)

---

## ✅ 8. 최종 확인 체크리스트

### 8.1 클럭 설정 ✓
- [ ] HSE: 8MHz Crystal/Ceramic
- [ ] PLL: ENABLE, x8 multiplier  
- [ ] SYSCLK: 64MHz
- [ ] APB1: 32MHz, APB2: 64MHz

### 8.2 핀 배치 ✓
- [ ] PA5: GPIO_Output (LD2)
- [ ] PB8: I2C1_SCL
- [ ] PB9: I2C1_SDA
- [ ] PA9: USART1_TX
- [ ] PA10: USART1_RX

### 8.3 주변장치 설정 ✓
- [ ] I2C1: 400kHz, 7-bit addressing
- [ ] USART1: 115200 baud, 8N1
- [ ] TIM2: 1ms resolution
- [ ] GPIO: Push-pull output

### 8.4 인터럽트 설정 ✓  
- [ ] USART1 global interrupt: ENABLE
- [ ] SysTick: Timebase source

---

## 🚀 9. 코드 생성 및 빌드

### 9.1 코드 생성
1. **GENERATE CODE** 버튼 클릭
2. STM32CubeIDE 자동 실행 대기
3. 프로젝트 자동 import 확인

### 9.2 사용자 코드 추가
생성된 `main.c` 파일에 MPU-6050 관련 코드를 추가합니다.

### 9.3 빌드 및 플래시
1. **Project** → **Build All** (Ctrl+B)
2. **Run** → **Debug As** → **STM32 MCU C/C++ Application**
3. 디버거 설정 후 플래시

---

## 🔍 10. 트러블슈팅

### 10.1 일반적인 문제

**클럭 설정 오류**
- 증상: 시스템이 부팅되지 않음
- 해결: HSE 설정 확인, 크리스털 연결 점검

**I2C 통신 실패**  
- 증상: MPU-6050 인식 실패
- 해결: 풀업 저항 확인, 연결 점검

**UART 통신 문제**
- 증상: 시리얼 데이터 수신 안됨  
- 해결: 보드레이트 확인, TX/RX 핀 교차 연결

### 10.2 디버깅 방법
1. **LED 상태 확인**: PA5 LED로 시스템 상태 파악
2. **시리얼 모니터**: 115200 baud로 디버그 메시지 확인
3. **I2C 스캔**: 0x68 주소에서 MPU-6050 감지 확인

---

## 📈 성능 최적화

### 클럭 최적화
- **64MHz**: 충분한 처리 능력
- **APB1 32MHz**: I2C, TIM 동작
- **APB2 64MHz**: USART, GPIO 고속 동작

### 샘플링 최적화  
- **100Hz**: 실시간 시각화에 적합
- **10ms 간격**: 안정적인 데이터 수집
- **상보 필터**: 노이즈 제거 및 안정성

이 가이드를 따라 설정하면 MPU-6050과 STM32F103 간의 완벽한 통신이 가능합니다! 🎉

<img width="800" height="600" alt="LCD-SPI" src="https://github.com/user-attachments/assets/beee2466-55d7-44cf-956a-0a860e1a189a" />
<br>
<img width="800" height="600" alt="LCD-SPI_008" src="https://github.com/user-attachments/assets/8acc11bd-f882-4708-a598-880511e50ea9" />
<br>
<img width="800" height="600" alt="LCD-SPI_001" src="https://github.com/user-attachments/assets/1e88e930-a23f-40ab-aaad-bf303d965c89" />
<br>
<img width="800" height="600" alt="LCD-SPI_002" src="https://github.com/user-attachments/assets/00aacce1-a6b2-47e0-afcf-fcdcab8ce2dd" />
<br>
<img width="800" height="600" alt="LCD-SPI_003" src="https://github.com/user-attachments/assets/763c200e-ad62-4c16-a5c0-e8b3bf83ee5c" />
<br>
<img width="800" height="600" alt="LCD-SPI_004" src="https://github.com/user-attachments/assets/27e07481-147b-4780-868e-ffdc52aeed1a" />
<br>
<img width="800" height="600" alt="LCD-SPI_005" src="https://github.com/user-attachments/assets/0168f464-c43b-42ec-9581-a58563ba8a6e" />
<br>
<img width="800" height="600" alt="LCD-SPI_006" src="https://github.com/user-attachments/assets/93d2b905-a4a0-4168-8c3b-b0bce164960f" />
<br>
<img width="800" height="600" alt="LCD-SPI_007" src="https://github.com/user-attachments/assets/e85b0b5e-5ab9-4737-a56c-bd0af6b6b834" />
<br>

```c
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */

// ST7735S Commands
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09
#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E
#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// LCD dimensions
#define LCD_WIDTH  160
#define LCD_HEIGHT 120 //80

// Colors (RGB565)
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0

/* USER CODE END PD */

```

```c
/* USER CODE BEGIN PM */

// Pin control macros
#define LCD_CS_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define LCD_CS_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)
#define LCD_DC_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET)
#define LCD_DC_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET)
#define LCD_RES_LOW()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET)
#define LCD_RES_HIGH() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET)

/* USER CODE END PM */
```

```c
/* USER CODE BEGIN PV */

// Simple 8x8 font (ASCII 32-127) - subset for demonstration
static const uint8_t font8x8[][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (Space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // '!'
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // '"'
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // '#'
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // '$'
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // '%'
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // '&'
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // '''
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // '('
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // ')'
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // '*'
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // '+'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x06, 0x00}, // ','
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // '-'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // '.'
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // '/'
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // '0'
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // '1'
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // '2'
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // '3'
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // '4'
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // '5'
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // '6'
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // '7'
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // '8'
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // '9'
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // ':'
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x06, 0x00}, // ';'
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // '<'
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // '='
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // '>'
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // '?'
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // '@'
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // 'A'
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // 'B'
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // 'C'
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // 'D'
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // 'E'
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // 'F'
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // 'G'
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // 'H'
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'I'
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // 'J'
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // 'K'
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // 'L'
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // 'M'
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // 'N'
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // 'O'
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // 'P'
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // 'Q'
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // 'R'
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // 'S'
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'T'
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // 'U'
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // 'V'
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // 'W'
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // 'X'
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // 'Y'
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // 'Z'
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00}, // '['
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00}, // '\'
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00}, // ']'
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // '^'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, // '_'
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // '`'
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}, // 'a'
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}, // 'b'
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}, // 'c'
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}, // 'd'
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}, // 'e'
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}, // 'f'
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // 'g'
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}, // 'h'
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'i'
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}, // 'j'
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}, // 'k'
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'l'
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // 'm'
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}, // 'n'
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}, // 'o'
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}, // 'p'
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}, // 'q'
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}, // 'r'
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}, // 's'
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}, // 't'
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}, // 'u'
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // 'v'
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}, // 'w'
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}, // 'x'
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // 'y'
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}, // 'z'
    {0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00}, // '{'
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00}, // '|'
    {0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00}, // '}'
    {0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // '~'
};

/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */

// LCD function prototypes
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_WriteData16(uint16_t data);
void LCD_Init(void);
void LCD_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void LCD_DrawPixel(uint8_t x, uint8_t y, uint16_t color);
void LCD_Fill(uint16_t color);
void LCD_DrawChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg_color);
void LCD_DrawString(uint8_t x, uint8_t y, const char* str, uint16_t color, uint16_t bg_color);

/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */

void LCD_WriteCommand(uint8_t cmd) {
    LCD_CS_LOW();
    LCD_DC_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

void LCD_WriteData(uint8_t data) {
    LCD_CS_LOW();
    LCD_DC_HIGH();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

void LCD_WriteData16(uint16_t data) {
    uint8_t buffer[2];
    buffer[0] = (data >> 8) & 0xFF;
    buffer[1] = data & 0xFF;

    LCD_CS_LOW();
    LCD_DC_HIGH();
    HAL_SPI_Transmit(&hspi1, buffer, 2, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

void LCD_Init(void) {
    // Hardware reset
    LCD_RES_LOW();
    HAL_Delay(100);
    LCD_RES_HIGH();
    HAL_Delay(100);

    // Software reset
    LCD_WriteCommand(ST7735_SWRESET);
    HAL_Delay(150);

    // Out of sleep mode
    LCD_WriteCommand(ST7735_SLPOUT);
    HAL_Delay(500);

    // Frame rate control - normal mode
    LCD_WriteCommand(ST7735_FRMCTR1);
    LCD_WriteData(0x01);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x2D);

    // Frame rate control - idle mode
    LCD_WriteCommand(ST7735_FRMCTR2);
    LCD_WriteData(0x01);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x2D);

    // Frame rate control - partial mode
    LCD_WriteCommand(ST7735_FRMCTR3);
    LCD_WriteData(0x01);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x2D);
    LCD_WriteData(0x01);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x2D);

    // Display inversion control
    LCD_WriteCommand(ST7735_INVCTR);
    LCD_WriteData(0x07);

    // Power control
    LCD_WriteCommand(ST7735_PWCTR1);
    LCD_WriteData(0xA2);
    LCD_WriteData(0x02);
    LCD_WriteData(0x84);

    LCD_WriteCommand(ST7735_PWCTR2);
    LCD_WriteData(0xC5);

    LCD_WriteCommand(ST7735_PWCTR3);
    LCD_WriteData(0x0A);
    LCD_WriteData(0x00);

    LCD_WriteCommand(ST7735_PWCTR4);
    LCD_WriteData(0x8A);
    LCD_WriteData(0x2A);

    LCD_WriteCommand(ST7735_PWCTR5);
    LCD_WriteData(0x8A);
    LCD_WriteData(0xEE);

    // VCOM control
    LCD_WriteCommand(ST7735_VMCTR1);
    LCD_WriteData(0x0E);

    // Display inversion off
    LCD_WriteCommand(ST7735_INVOFF);

    // Memory access control (rotation)
    LCD_WriteCommand(ST7735_MADCTL);
    // 1. 기본 90도 회전 (추천)
    //LCD_WriteData(0x20); // MY=0, MX=0, MV=1
    // 2. 현재 사용중
    //LCD_WriteData(0xE0); // MY=1, MX=1, MV=1
    // 3. 90도 + X축만 미러링
    LCD_WriteData(0x60); // MY=0, MX=1, MV=1
    // 4. 90도 + Y축만 미러링
    //LCD_WriteData(0xA0); // MY=1, MX=0, MV=1

    // Color mode: 16-bit color
    LCD_WriteCommand(ST7735_COLMOD);
    LCD_WriteData(0x05);

    // Column address set
    LCD_WriteCommand(ST7735_CASET);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    //LCD_WriteData(0x4F); // 79
    LCD_WriteData(0x9F); // 159


    // Row address set
    LCD_WriteCommand(ST7735_RASET);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    //LCD_WriteData(0x9F); // 159
    // Row address set (80픽셀)
	LCD_WriteData(0x4F); // 79

    // Gamma correction
    LCD_WriteCommand(ST7735_GMCTRP1);
    LCD_WriteData(0x0f);
    LCD_WriteData(0x1a);
    LCD_WriteData(0x0f);
    LCD_WriteData(0x18);
    LCD_WriteData(0x2f);
    LCD_WriteData(0x28);
    LCD_WriteData(0x20);
    LCD_WriteData(0x22);
    LCD_WriteData(0x1f);
    LCD_WriteData(0x1b);
    LCD_WriteData(0x23);
    LCD_WriteData(0x37);
    LCD_WriteData(0x00);
    LCD_WriteData(0x07);
    LCD_WriteData(0x02);
    LCD_WriteData(0x10);

    LCD_WriteCommand(ST7735_GMCTRN1);
    LCD_WriteData(0x0f);
    LCD_WriteData(0x1b);
    LCD_WriteData(0x0f);
    LCD_WriteData(0x17);
    LCD_WriteData(0x33);
    LCD_WriteData(0x2c);
    LCD_WriteData(0x29);
    LCD_WriteData(0x2e);
    LCD_WriteData(0x30);
    LCD_WriteData(0x30);
    LCD_WriteData(0x39);
    LCD_WriteData(0x3f);
    LCD_WriteData(0x00);
    LCD_WriteData(0x07);
    LCD_WriteData(0x03);
    LCD_WriteData(0x10);

    // Normal display on
    LCD_WriteCommand(ST7735_NORON);
    HAL_Delay(10);

    // Main screen turn on
    LCD_WriteCommand(ST7735_DISPON);
    HAL_Delay(100);
}

void LCD_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // 0.96" ST7735S LCD 오프셋 적용
    uint8_t x_offset = 0;  // X축 오프셋
    uint8_t y_offset = 0;   // Y축 오프셋

    // Column address set (X축)
    LCD_WriteCommand(ST7735_CASET);
    LCD_WriteData(0x00);
    LCD_WriteData(x0 + x_offset);
    LCD_WriteData(0x00);
    LCD_WriteData(x1 + x_offset);

    // Row address set (Y축)
    LCD_WriteCommand(ST7735_RASET);
    LCD_WriteData(0x00);
    LCD_WriteData(y0 + y_offset);
    LCD_WriteData(0x00);
    LCD_WriteData(y1 + y_offset);

    // Write to RAM
    LCD_WriteCommand(ST7735_RAMWR);
}

void LCD_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

    LCD_SetWindow(x, y, x, y);
    LCD_WriteData16(color);
}

void LCD_Fill(uint16_t color) {
    LCD_SetWindow(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);

    LCD_CS_LOW();
    LCD_DC_HIGH();

    for(uint16_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        uint8_t buffer[2];
        buffer[0] = (color >> 8) & 0xFF;
        buffer[1] = color & 0xFF;
        HAL_SPI_Transmit(&hspi1, buffer, 2, HAL_MAX_DELAY);
    }

    LCD_CS_HIGH();
}

void LCD_DrawChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg_color) {
    if(ch < 32 || ch > 126) ch = 32; // Replace invalid chars with space

    const uint8_t* font_char = font8x8[ch - 32];

    for(uint8_t i = 0; i < 8; i++) {
        uint8_t line = font_char[i];
        for(uint8_t j = 0; j < 8; j++) {
            //if(line & (0x80 >> j)) {
        	if(line & (0x01 << j)) { // LSB부터 읽기
                LCD_DrawPixel(x + j, y + i, color);
            } else {
                LCD_DrawPixel(x + j, y + i, bg_color);
            }
        }
    }
}

void LCD_DrawString(uint8_t x, uint8_t y, const char* str, uint16_t color, uint16_t bg_color) {
    uint8_t orig_x = x;

    while(*str) {
        if(*str == '\n') {
            y += 8;
            x = orig_x;
        } else if(*str == '\r') {
            x = orig_x;
        } else {
            if(x + 8 > LCD_WIDTH) {
                x = orig_x;
                y += 8;
            }
            if(y + 8 > LCD_HEIGHT) {
                break;
            }

            LCD_DrawChar(x, y, *str, color, bg_color);
            x += 8;
        }
        str++;
    }
}

/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */

  // Initialize LCD
  LCD_Init();

  // Clear screen with black background
  LCD_Fill(BLACK);

  LCD_DrawString(10, 30, "Hello World!", WHITE, BLACK);
  LCD_DrawString(10, 45, "STM32F103", GREEN, BLACK);
  LCD_DrawString(10, 60, "ST7735S LCD", CYAN, BLACK);
  LCD_DrawString(10, 75, "160x80", YELLOW, BLACK);

  /* USER CODE END 2 */
```

```c
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  HAL_Delay(1000);
  }
  /* USER CODE END 3 */
```

