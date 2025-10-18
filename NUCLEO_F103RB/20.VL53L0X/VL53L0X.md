# VL53L0X

# STM32CubeMX 설정 가이드
## VL53L0X 거리 센서와 I2C 주소 스캐너

<img width="600" height="800" alt="400" src="https://github.com/user-attachments/assets/3e7c9608-096b-460e-b5f0-a5862f3dd39d" />



### 필요한 하드웨어
- STM32F103 NUCLEO 보드
- VL53L0X ToF 거리 센서 모듈
- 점퍼 와이어
- 풀업 저항 (4.7kΩ) - VL53L0X 모듈에 없는 경우

### 하드웨어 연결
```
VL53L0X 모듈      →    STM32F103 NUCLEO
VCC               →    3.3V (CN7-16 또는 CN6-4)
GND               →    GND (CN7-20 또는 CN6-6)
SCL               →    PB8 (CN10-3)
SDA               →    PB9 (CN10-5)
GPIO1 (선택사항)   →    연결하지 않음
XSHUT (선택사항)   →    연결하지 않음
```

---

## STM32CubeMX 설정 단계

### 1. 프로젝트 생성
1. STM32CubeMX 실행
2. Start My project from MCU 클릭
3. STM32F103RB (NUCLEO-F103RB) 선택
4. Start Project 클릭

### 2. 시스템 설정

#### 2.1 RCC (리셋 및 클럭 제어)
- **Pinout & Configuration → System Core → RCC**
- **High Speed Clock (HSE)**: 비활성화
- **Low Speed Clock (LSE)**: 비활성화
- **Master Clock Output**: 비활성화

#### 2.2 SYS (시스템)
- **Pinout & Configuration → System Core → SYS**
- **Debug**: Serial Wire (기본값)
- **Timebase Source**: SysTick (기본값)

### 3. 클럭 설정
- **Clock Configuration 탭**
- **Input frequency**: 8 MHz (HSI)
- **PLLCLK 설정**: 
  - PLL Source Mux: HSI/2
  - PLLMUL: x16 (결과: 64MHz)
- **System Clock Mux**: PLLCLK
- **HCLK**: 64 MHz
- **APB1 Prescaler**: /2 (32 MHz)
- **APB2 Prescaler**: /1 (64 MHz)

### 4. 주변 장치 설정

#### 4.1 USART2 (시리얼 통신)
- **Pinout & Configuration → Connectivity → USART2**
- **Mode**: Asynchronous
- **설정**:
  - Baud Rate: 115200 Bits/s
  - Word Length: 8 Bits
  - Parity: None
  - Stop Bits: 1
  - Data Direction: Receive and Transmit
- **핀 배치**: 
  - PA2: USART2_TX (자동 설정)
  - PA3: USART2_RX (자동 설정)

#### 4.2 I2C1 (센서 통신)
- **Pinout & Configuration → Connectivity → I2C1**
- **I2C**: I2C 선택
- **설정**:
  - I2C Speed Mode: Standard Mode
  - I2C Clock Speed: 100000 Hz (100 kHz)
  - Clock No Stretch Mode: Disable
  - General Call: Disable
  - Primary Address Length selection: 7-bit
- **핀 배치**:
  - PB8: I2C1_SCL (자동 설정)
  - PB9: I2C1_SDA (자동 설정)

### 5. GPIO 설정
- **사용자 LED (LD2)**: PA5 - GPIO_Output (자동 설정)
- **사용자 버튼 (B1)**: PC13 - GPIO_EXTI13 (자동 설정)

### 6. 프로젝트 관리자 설정

#### 6.1 Project 탭
- **Project Name**: VL53L0X_Distance_Sensor
- **Project Location**: 원하는 디렉토리 선택
- **Toolchain/IDE**: STM32CubeIDE

#### 6.2 Code Generator 탭
- **STM32Cube MCU Package**: 최신 버전
- **생성 파일 옵션**:
  - ✅ Generate peripheral initialization as a pair of '.c/.h' files per peripheral
  - ✅ Keep User Code when re-generating
  - ✅ Delete previously generated files when not re-generated

### 7. 추가 설정

#### 7.1 NVIC 설정
- **Pinout & Configuration → System Core → NVIC**
- **EXTI line[15:10] interrupts**: 활성화 (사용자 버튼용)
- 우선순위: 0

#### 7.2 GPIO 설정 확인
다음 핀들이 올바르게 설정되었는지 확인:
- **PA2**: USART2_TX
- **PA3**: USART2_RX  
- **PA5**: GPIO_Output (LD2)
- **PB8**: I2C1_SCL
- **PB9**: I2C1_SDA
- **PC13**: GPIO_EXTI13 (B1)

---

## 코드 생성 및 통합

### 8. 코드 생성
1. **"GENERATE CODE"** 클릭
2. 프롬프트가 나타나면 STM32CubeIDE에서 프로젝트 열기

### 9. STM32CubeIDE 프로젝트 설정

#### 9.1 Printf 지원
1. **프로젝트 우클릭 → Properties**
2. **C/C++ Build → Settings → Tool Settings**
3. **MCU GCC Linker → Libraries**
4. **Libraries (-l)**: `c`, `m`, `nosys` 추가
5. **MCU C Compiler → Miscellaneous**
6. **Other flags**: `-u _printf_float` 추가 (printf에서 float 사용 시)

#### 9.2 최적화 설정
- **C/C++ Build → Settings → MCU GCC Compiler → Optimization**
- **Optimization Level**: Optimize for debug (-Og) 또는 Optimize (-O1)

### 10. 코드 통합
1. 생성된 `main.c`를 제공된 VL53L0X 코드로 교체
2. 모든 USER CODE 섹션이 올바른 위치에 있는지 확인
3. include 파일과 함수 선언 확인

---

## 하드웨어 테스트 체크리스트

### 첫 실행 전 확인사항:
- [ ] VL53L0X가 3.3V에 연결됨 (5V 아님!)
- [ ] 접지 연결 확실
- [ ] I2C 핀 연결: PB8 (SCL), PB9 (SDA)
- [ ] I2C 라인에 풀업 저항 존재 (4.7kΩ)
- [ ] 시리얼 출력용 USB 케이블 연결

### 예상 시리얼 출력:
```
VL53L0X Distance Measurement Test
System Clock: 64MHz
I2C1: PB8-SCL, PB9-SDA

=== I2C Bus Scanner ===
Scanning I2C bus...
Device found at address: 0x52 (7-bit: 0x29)
Total devices found: 1
======================

=== VL53L0X Device Detection ===
Testing address 0x52 (7-bit: 0x29)...
Device responds at 0x52
Device ID: 0xEE
✓ VL53L0X found at address 0x52!

Using VL53L0X at address: 0x52

VL53L0X found! Model ID: 0xEE
VL53L0X simple initialization complete!
Raw distance reading: 245 mm
Distance: 245 mm
```

---

## 문제 해결

### I2C 장치를 찾을 수 없음:
1. 3.3V 전원 공급 확인
2. 접지 연결 확인
3. PB8/PB9 연결 확인
4. SCL/SDA에 4.7kΩ 풀업 저항 추가/확인
5. 다른 VL53L0X 모듈로 테스트

### 장치는 발견되지만 잘못된 ID:
1. VL53L0X 모듈 확인 (VL53L1X나 다른 센서가 아닌지)
2. I2C 타이밍과 클럭 속도 확인
3. 안정적인 전원 공급 확인

### 측정 타임아웃:
1. 장치 초기화가 성공했는지 확인
2. I2C 통신 오류 확인
3. 센서가 가려지거나 방해받지 않는지 확인
4. 다른 측정 타이밍 설정 시도

### 빌드 오류:
1. printf 지원이 활성화되었는지 확인
2. 모든 USER CODE 섹션이 보존되었는지 확인
3. 라이브러리 링크 확인 (c, m, nosys)
4. STM32CubeIDE와 HAL 라이브러리 업데이트

---

## 성능 참고사항

- **측정 범위**: 30mm - 2000mm (일반적)
- **정확도**: ±3% (일반적 조건)
- **측정 속도**: 현재 설정으로 약 10Hz
- **I2C 속도**: 100kHz (필요시 400kHz까지 증가 가능)
- **시스템 클럭**: 64MHz - 성능과 전력의 균형 최적화

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

