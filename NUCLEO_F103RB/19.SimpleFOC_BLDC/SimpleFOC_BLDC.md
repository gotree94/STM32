# SimpleFOC_BLDC

# 센서리스 BLDC 모터 제어 시스템 - CubeMX 설정 가이드

  * https://docs.simplefoc.com/simplefocmini
  * https://github.com/simplefoc/SimpleFOCMini
  * http://olddocs.simplefoc.com/v2.3.2/examples

<img width="1286" height="726" alt="simplefoc" src="https://github.com/user-attachments/assets/05d07ec9-3a52-4d6c-b681-d4fd5bd6fdd1" />

## STM32F103 NUCLEO + DRV8313 최종 설정

### 1. 클럭 설정 (Clock Configuration)
- **Clock Source**: HSI (Internal 8MHz RC)
- **PLLMUL**: x16 (4MHz × 16 = 64MHz)
- **System Clock (SYSCLK)**: 64MHz
- **AHB Clock (HCLK)**: 64MHz
- **APB1 Clock (PCLK1)**: 32MHz (64MHz ÷ 2)
- **APB2 Clock (PCLK2)**: 64MHz

### 2. GPIO 핀 설정

#### DRV8313 PWM 입력 핀 (모터 3상 제어)
| 핀 | 기능 | 타이머 | 설정 |
|----|------|--------|------|
| **PA8** | TIM1_CH1 (IN1 - Phase A) | TIM1 Channel 1 | Alternate Function Push Pull |
| **PA9** | TIM1_CH2 (IN2 - Phase B) | TIM1 Channel 2 | Alternate Function Push Pull |
| **PA10** | TIM1_CH3 (IN3 - Phase C) | TIM1 Channel 3 | Alternate Function Push Pull |

#### DRV8313 제어 핀
| 핀 | 기능 | 설정 | 초기값 |
|----|------|------|--------|
| **PA4** | DRV_EN (Enable) | GPIO Output Push Pull | LOW |
| **PA6** | DRV_nSP (Sleep) | GPIO Input No Pull | HIGH (Active Low) |
| **PA7** | DRV_nRT (Reset) | GPIO Output Push Pull | HIGH (Active Low) |
| **PA12** | DRV_nFT (Fault) | GPIO Input Pull-up | HIGH (Active Low) |

#### UART 통신 핀
| 핀 | 기능 | 설정 |
|----|------|------|
| **PA2** | USART2_TX | Alternate Function Push Pull |
| **PA3** | USART2_RX | Alternate Function with Pull-up |

#### 상태 표시
| 핀 | 기능 | 설정 |
|----|------|------|
| **PA5** | LD2 (Status LED) | GPIO Output Push Pull |

### 3. 타이머 설정

#### TIM1 (20kHz PWM 생성)
```
Mode: PWM Generation CH1, CH2, CH3
Clock Source: Internal Clock (64MHz)
Prescaler (PSC): 0 (분주 없음)
Counter Period (ARR): 3199
Counter Mode: Up
Clock Division: No Division
Auto-reload preload: Disable

PWM Mode 1: Output Compare
Initial Pulse: 0
Polarity: High
Fast Mode: Disable
```

**계산**: 64MHz ÷ (0+1) ÷ (3199+1) = 20kHz

#### TIM3 (1kHz 제어 루프 타이머)
```
Mode: Internal Clock
Clock Source: Internal Clock (64MHz)
Prescaler (PSC): 63
Counter Period (ARR): 999
Counter Mode: Up
Auto-reload preload: Disable
```

**계산**: 64MHz ÷ (63+1) ÷ (999+1) = 1kHz

### 4. USART2 설정
```
Mode: Asynchronous
Baud Rate: 115200
Word Length: 8 Bits
Parity: None
Stop Bits: 1
Data Direction: Receive and Transmit
Hardware Flow Control: None
Over Sampling: 16
```

### 5. NVIC (인터럽트) 설정
| 인터럽트 | 우선순위 | 기능 |
|----------|----------|------|
| **TIM3 global interrupt** | 0 (highest) | 1kHz 모터 제어 루프 |
| **USART2 global interrupt** | 1 | UART 명령 수신 |

### 6. Project Manager 설정
- **Toolchain/IDE**: STM32CubeIDE
- **Code Generator Options**:
  - Generate peripheral initialization as pair of .c/.h files: Checked
  - Keep User Code when re-generating: Checked
  - Delete previously generated files: Unchecked

## 하드웨어 연결

### DRV8313 SimpleFOC Board → STM32F103 NUCLEO
| DRV8313 핀 | STM32 핀 | 선 색깔 (권장) | 기능 |
|------------|----------|----------------|------|
| **EN** | PA4 | 빨강 | Enable Control |
| **IN1** | PA8 | 노랑 | Phase A PWM |
| **IN2** | PA9 | 초록 | Phase B PWM |
| **IN3** | PA10 | 파랑 | Phase C PWM |
| **nFT** | PA12 | 주황 | Fault Detection |
| **nSP** | PA6 | 보라 | Sleep Control |
| **nRT** | PA7 | 회색 | Reset Control |
| **GND** | GND | 검정 | Ground |
| **3V3** | 3.3V | 빨강 | Logic Power |

### BLDC 모터 연결
| 모터 단자 | DRV8313 출력 | 기능 |
|-----------|--------------|------|
| **Wire 1** | M1 | Phase A |
| **Wire 2** | M2 | Phase B |
| **Wire 3** | M3 | Phase C |

**주의**: 모터 배선 순서가 중요합니다. 진동이나 역회전이 발생하면 두 선을 바꿔 연결해보세요.

### 전원 연결
- **DRV8313 전원**: 6V-40V (HDD 모터는 12V 권장)
- **STM32 전원**: USB 또는 5V 어댑터
- **공통 GND**: 모든 장치의 그라운드 연결 필수

## 소프트웨어 설정

### 컴파일러 설정
1. **Project Properties** → **C/C++ Build** → **Settings**
2. **MCU GCC Linker** → **Libraries**
   - Add library: `c`, `m`, `nosys`
3. **MCU Settings**
   - Use float with printf: Checked
   - Runtime library: Newlib-nano

### 플래시 설정
- **Optimization Level**: 
  - Debug: -Og
  - Release: -O2
- **Enable printf float**: Checked

## 제어 명령어

### 기본 명령
- **'a'**: 정방향 회전 시작
- **'s'**: 모터 정지
- **'d'**: 역방향 회전 시작
- **'w'**: 속도 증가 (+5%)
- **'x'**: 속도 감소 (-5%)

### 운전 범위
- **속도 범위**: 30% ~ 80% (600 ~ 1600 RPM)
- **최대 RPM**: 2000 RPM
- **PWM 주파수**: 20kHz
- **제어 주파수**: 1kHz

## 성능 지표

### 전류 소모 (12V 기준)
- **정지**: 30-50mA
- **저속 (30-50%)**: 150-200mA
- **중속 (50-70%)**: 100-150mA  
- **고속 (70-80%)**: 80-120mA

### 예상 실제 RPM (센서리스)
- **30%**: ~400-500 RPM
- **50%**: ~600-800 RPM
- **80%**: ~900-1200 RPM

## 안전 기능

### 자동 보호
- **DRV8313 고장 감지**: nFT 핀 실시간 모니터링
- **과전류 방지**: 적응형 PWM 제한
- **과열 방지**: 저전류 운전 모드

### 수동 안전 조치
- **전류 모니터링**: 200mA 초과시 즉시 정지
- **온도 확인**: 모터/드라이버 온도 주기적 점검
- **운전 시간 제한**: 연속 운전 후 냉각 시간 확보

## 문제 해결

### 일반적인 문제
1. **모터가 진동만 함**: 배선 순서 확인 (두 선 교체)
2. **과전류 발생**: magnitude 값 감소 (0.3 → 0.2)
3. **속도가 느림**: 전원 전압 확인 (12V 유지)
4. **UART 명령 무응답**: 인터럽트 활성화 확인

## 터미널 입/출력 결과

```
=== Sensorless BLDC Motor Control with DRV8313 ===
Initializing DRV8313...
DRV8313 initialized successfully
Sensorless Motor Control System Initialized
Commands: 'a'=Forward, 's'=Stop, 'd'=Reverse
Speed Control: 'w'=Speed Up, 'x'=Speed Down
Speed Range: 30% - 80%
Ready for commands!

Status: STOP | DRV: READY | Speed: 30% | Target: 0.0 RPM
Status: STOP | DRV: READY | Speed: 30% | Target: 0.0 RPM
RX: 0x61 (a)
Processing command: 0x61 (a)
FORWARD command
Motor:=1624, B=1255, C=1919, Angle=3.6
�Status: FWD | DRV: RUNNING | Speed: 30% | Target: 600.0 RPM
PWM: A=1648, B=1245, C=1905, Angle=7.2°, Inc=3.600°
PWM: A=1671, B=1237, C=1890, Angle=10.8°, Inc=3.600°
Status: FWD | DRV: RUNNING | Speed: 30% | Target: 600.0 RPM
RX: 0x73 (s)
Processing command: 0x73 (s)
STOP command
Motor: STOPPED
RX: 0x64 (d)
Processing command: 0x64 (d)
REVERSE command
Motor:=1575, B=1280, C=1944, Angle=356.Status: REV | DRV: RUNNING | Speed: 30% | Target: -600.0 RPM
PWM: A=1551, B=1294, C=1954, Angle=352.8°, Inc=-3.600°
PWM: A=1527, B=1309, C=1962, Angle=349.2°, Inc=-3.600°
Status: REV | DRV: RUNNING | Speed: 30% | Target: -600.0 RPM
RX: 0x73 (s)
Processing command: 0x73 (s)
STOP command
Motor: STOPPED
RX: 0x61 (a)
Processing command: 0x61 (a)
FORWARD command
Motor:=1624, B=1255, C=1919, Angle=3.6
�RX: 0x77 (w)
Processing command: 0x77 (w)
SPEED UP command
Speed: 35%
Status: FWD | DRV: RUNNING | Speed: 35% | Target: 700.0 RPM
PWM: A=1205, B=1613, C=1980, Angle=298.2°, Inc=4.200°
PWM: A=1548, B=2011, C=1240, Angle=186.6°, Inc=4.200°
RX: 0x77 (w)
Processing command: 0x77 (w)
SPEED UP command
Speed: 40%
RX: 0x77 (w)
Processing command: 0x77 (w)
SPEED UP command
Speed: 45%
Status: FWD | DRV: RUNNING | Speed: 45% | Target: 900.0 RPM
PWM: A=1689, B=1125, C=1985, Angle=10.2°, Inc=5.400°
RX: 0x77 (w)
Processing command: 0x77 (w)
SPEED UP command
Speed: 50%
RX: 0x77 (w)
Processing command: 0x77 (w)
SPEED UP command
Speed: 55%
PWM: A=2094, B=1034, C=1670, Angle=53.4°, Inc=6.600°
RX: 0x77 (w)
Processing command: 0x77 (w)
SPEED UP command
Speed: 60%
Status: FWD | DRV: RUNNING | Speed: 60% | Target: 1200.0 RPM
PWM: A=998, B=2159, C=1642, Angle=243.6°, Inc=7.200°
PWM: A=942, B=2049, C=1807, Angle=258.0°, Inc=7.200°
RX: 0x78 (x)
Processing command: 0x78 (x)
SPEED DOWN command
Speed: 55%
RX: 0x78 (x)
Processing command: 0x78 (x)
SPEED DOWN command
Speed: 50%
RX: 0x78 (x)
Processing command: 0x78 (x)
SPEED DOWN command
Speed: 45%
RX: 0x78 (x)
Processing command: 0x78 (x)
SPEED DOWN command
Speed: 40%
Status: FWD | DRV: RUNNING | Speed: 40% | Target: 800.0 RPM
RX: 0x78 (x)
Processing command: 0x78 (x)
SPEED DOWN command2Speed: 35%
PWM: A=2033, B=1479, C=1286, Angle=104.4°, Inc=4.200°
RX: 0x73 (s)
Processing command: 0x73 (s)
STOP command
Motor: STOPPED
Status: STOP | DRV: READY | Speed: 35% | Target: 0.0 RPM
Status: STOP | DRV: READY | Speed: 35% | Target: 0.0 RPM
Status: STOP | DRV: READY | Speed: 35% | Target: 0.0 RPM
Status: STOP | DRV: READY | Speed: 35% | Target: 0.0 RPM
```

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

