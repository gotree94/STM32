# VL53L0X

## STM32CubeMX 설정 가이드
### VL53L0X 거리 센서와 I2C 주소 스캐너

<img width="349" height="192" alt="1002" src="https://github.com/user-attachments/assets/2ce67ecd-f487-4eb1-b083-e5823e572a55" />

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/d80c0cce-15b0-4676-b297-6b326258817e" />

<img width="600" height="800" alt="400" src="https://github.com/user-attachments/assets/3e7c9608-096b-460e-b5f0-a5862f3dd39d" />

<img width="600" height="400" alt="003" src="https://github.com/user-attachments/assets/fea16690-c450-40bf-ab20-4981d3e91586" />


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
XSHUT             →    3.3V
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
  - I2C Speed Mode: Fast Mode
  - I2C Clock Speed: 400000 Hz (400 kHz)
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
===========================================
  VL53L0X Distance Sensor - NUCLEO-F103RB
===========================================
System Clock: 64 MHz
APB1 Clock (I2C): 32 MHz
APB2 Clock: 64 MHz
-------------------------------------------
Initializing VL53L0X sensor...
SUCCESS: VL53L0X initialized!
Device Model ID: 0xEE (Expected: 0xEE)
-------------------------------------------
Starting distance measurements...
===========================================

Distance:  245 mm  [** MEDIUM **]
Distance:  247 mm  [** MEDIUM **]
Distance:  250 mm  [** MEDIUM **]
Distance:  105 mm  [*** CLOSE ***]
Distance:   85 mm  [**** VERY CLOSE ****]
Distance:  890 mm  [** MEDIUM **]
Distance: 1520 mm  [* FAR *]
Distance:  320 mm  [** MEDIUM **]
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

```c
/* USER CODE BEGIN Includes */
#include "vl53l0x_platform.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```


```c
/* USER CODE BEGIN PFP */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END PFP */
```


```c
  /* USER CODE BEGIN 2 */
  printf("\r\n");
  printf("===========================================\r\n");
  printf("  VL53L0X Distance Sensor - NUCLEO-F103RB\r\n");
  printf("===========================================\r\n");
  printf("System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
  printf("APB1 Clock (I2C): %lu MHz\r\n", HAL_RCC_GetPCLK1Freq() / 1000000);
  printf("APB2 Clock: %lu MHz\r\n", HAL_RCC_GetPCLK2Freq() / 1000000);
  printf("-------------------------------------------\r\n");

  HAL_Delay(100);

  // I2C 버스 스캔 추가
  printf("Scanning I2C bus...\r\n");
  uint8_t found = 0;
  for(uint8_t i=1; i<128; i++)
  {
      if(HAL_I2C_IsDeviceReady(&hi2c1, i<<1, 1, 10) == HAL_OK)
      {
          printf("  [OK] Found device at address: 0x%02X\r\n", i<<1);
          found++;
      }
  }
  if(found == 0)
  {
      printf("  [ERROR] No I2C devices found!\r\n");
      printf("  Check connections:\r\n");
      printf("    - VIN to 3.3V\r\n");
      printf("    - GND to GND\r\n");
      printf("    - SCL to PB8\r\n");
      printf("    - SDA to PB9\r\n");
  }
  else
  {
      printf("  Total %d device(s) found.\r\n", found);
  }
  printf("-------------------------------------------\r\n\r\n");

  printf("Initializing VL53L0X sensor...\r\n");

  if(VL53L0X_Init(&hi2c1) == 0)
  {
      uint8_t id;
      VL53L0X_ReadID(&hi2c1, &id);
      printf("SUCCESS: VL53L0X initialized!\r\n");
      printf("Device Model ID: 0x%02X (Expected: 0xEE)\r\n", id);
      printf("-------------------------------------------\r\n");
      printf("Starting distance measurements...\r\n");
      printf("===========================================\r\n\r\n");
  }
  else
  {
      printf("ERROR: VL53L0X initialization failed!\r\n");
      printf("-------------------------------------------\r\n");
      printf("Please check the following connections:\r\n");
      printf("  VL53L0X VIN  -> NUCLEO 3.3V (CN7 pin 16)\r\n");
      printf("  VL53L0X GND  -> NUCLEO GND  (CN7 pin 20)\r\n");
      printf("  VL53L0X SCL  -> NUCLEO PB8  (CN10 pin 3)\r\n");
      printf("  VL53L0X SDA  -> NUCLEO PB9  (CN10 pin 5)\r\n");
      printf("===========================================\r\n");

      // Error indication with LED blinking
      while(1)
      {
          HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
          HAL_Delay(200);
      }
  }

  HAL_Delay(500);
  /* USER CODE END 2 */
```


```c
    /* USER CODE BEGIN 3 */
      // Start new measurement
      VL53L0X_StartMeasurement(&hi2c1);

      // Read distance
      if(VL53L0X_ReadDistance(&hi2c1, &distance) == 0)
      {
          // Print distance with status indicator
          printf("Distance: %4d mm", distance);

          // Distance range indicator
          if(distance < 100)
              printf("  [**** VERY CLOSE ****]");
          else if(distance < 300)
              printf("  [*** CLOSE ***]");
          else if(distance < 1000)
              printf("  [** MEDIUM **]");
          else if(distance < 2000)
              printf("  [* FAR *]");
          else
              printf("  [OUT OF RANGE]");

          printf("\r\n");

          // Toggle LED to indicate successful measurement
          HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      }
      else
      {
          printf("ERROR: Measurement failed!\r\n");
      }

      // Measurement rate: 10Hz (100ms delay)
      HAL_Delay(100);
  }
  /* USER CODE END 3 */
```



