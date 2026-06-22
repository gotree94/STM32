# MPU6050 (STM32F411RETx) 프로젝트

NUCLEO-F411RE 보드에서 MPU6050 6축 IMU 센서(I2C)를 구동하여 가속도/자이로/온도 데이터를 UART로 출력하는 펌웨어 프로젝트입니다.

---

## 목차

1. [프로젝트 개요](#1-프로젝트-개요)
2. [하드웨어 구성](#2-하드웨어-구성)
3. [STM32CubeIDE 프로젝트 생성](#3-stm32cubeide-프로젝트-생성)
4. [프로젝트 구조](#4-프로젝트-구조)
5. [소스 코드 설명](#5-소스-코드-설명)
6. [동작 이론](#6-동작-이론)
7. [Python 3D 시각화](#7-python-3d-시각화)
8. [빌드 및 디버그](#8-빌드-및-디버그)
9. [시리얼 출력 예시](#9-시리얼-출력-예시)

1. [프로젝트 개요](#1-프로젝트-개요)
2. [하드웨어 구성](#2-하드웨어-구성)
3. [STM32CubeIDE 프로젝트 생성](#3-stm32cubeide-프로젝트-생성)
4. [프로젝트 구조](#4-프로젝트-구조)
5. [소스 코드 설명](#5-소스-코드-설명)
6. [동작 이론](#6-동작-이론)
7. [Python 3D 시각화](#7-python-3d-시각화)
8. [빌드 및 디버그](#8-빌드-및-디버그)
9. [시리얼 출력 예시](#9-시리얼-출력-예시)

---

## 1. 프로젝트 개요

| 항목 | 내용 |
|---|---|
| **MCU** | STM32F411RETx (ARM Cortex-M4, FPU, 84MHz) |
| **보드** | NUCLEO-F103 / NUCLEO-F411RE |
| **센서** | MPU6050 (6축 IMU: 3축 가속도 + 3축 자이로 + 온도) |
| **통신** | I2C1 (100kHz, 7비트 어드레싱) |
| **출력** | USART2 (115200 baud, 8N1) |
| **개발 환경** | STM32CubeIDE 1.18.1 |
| **HAL 버전** | STM32Cube FW_F4 V1.28.3 |
| **클럭** | HSI 16MHz → PLL → 84MHz SYSCLK |

### 주요 기능

- I2C 버스 스캔 (`I2C_Scan`)
- MPU6050 초기화 (WHO_AM_I 확인, PWR_MGMT_1 설정으로 Wakeup)
- 자이로 바이어스 보정 (`MPU6050_CalibrateGyro`, 64샘플 평균)
- 가속도/자이로/온도 읽기 (`MPU6050_ReadData`)
- 가속도 기반 Roll/Pitch 계산 (atan2)
- UART를 통한 데이터 출력 (500ms 주기)
- Python 실시간 3D 시각화 병행 가능

---

## 2. 하드웨어 구성

### 핀맵

| NUCLEO-F411RE 핀 | 연결 | 용도 |
|---|---|---|
| PB8 | MPU6050 SCL | I2C1 클럭 |
| PB9 | MPU6050 SDA | I2C1 데이터 |
| PA2 | USB-UART (TX) | USART2 송신 (시리얼 모니터) |
| PA3 | USB-UART (RX) | USART2 수신 |
| PA5 | LD2 (Green LED) | 보드 내장 LED |
| PC13 | B1 (Blue Button) | 사용자 버튼 |
| 3.3V | MPU6050 VCC | 전원 |
| GND | MPU6050 GND | 접지 |

### MPU6050 모듈 (GY-521) 핀 연결

```
MPU6050 (GY-521)      NUCLEO-F411RE
-----------------     ----------------
VCC  (핀1)     ---    3.3V
GND  (핀2)     ---    GND
SCL  (핀3)     ---    PB8  (I2C1_SCL)
SDA  (핀4)     ---    PB9  (I2C1_SDA)
XDA  (핀5)     ---    NC   (보조 I2C, 미사용)
XCL  (핀6)     ---    NC   (보조 I2C 클럭, 미사용)
AD0  (핀7)     ---    GND  (I2C 주소=0x68)
INT  (핀8)     ---    NC   (인터럽트, 미사용)
```

> **참고:** AD0 핀을 GND에 연결하면 I2C 주소가 `0x68`이 되고, VCC에 연결하면 `0x69`가 됩니다.

### I2C 풀업 저항

MPU6050(GY-521) 모듈에는 SCL/SDA에 4.7kΩ 풀업 저항이 내장되어 있어 별도 추가 불필요합니다. 보드에 내장된 I2C 풀업이 없을 경우 4.7kΩ ~ 10kΩ 저항을 VCC(3.3V)로 연결해야 합니다.

---

## 3. STM32CubeIDE 프로젝트 생성

### 3.1 새 프로젝트 생성

1. STM32CubeIDE 실행
2. `File` → `New` → `STM32 Project`
3. **Target Selection** 창에서 `STM32F411RETx` 검색 및 선택 → `Next`
4. 프로젝트 이름: `MPU6050` → `Finish`

### 3.2 CubeMX Pinout & Configuration

#### (1) 클럭 설정 (Clock Configuration 탭)

```
HSI (16MHz) → PLL
  - PLLM = 16
  - PLLN = 336
  - PLLP = 4 (DIV4)
→ SYSCLK = 84 MHz
  - APB1 = 42 MHz (DIV2)
  - APB2 = 84 MHz (DIV1)
```

#### (2) 핀 설정 (Pinout View)

| 핀 | Mode | Signal |
|---|---|---|
| PB8 | Alternate Function - Open Drain | I2C1_SCL |
| PB9 | Alternate Function - Open Drain | I2C1_SDA |
| PA2 | Alternate Function - Push-Pull | USART2_TX |
| PA3 | Alternate Function - Push-Pull | USART2_RX |
| PA5 | GPIO_Output | LD2 (Green LED) |
| PC13 | GPIO_EXTI13 | B1 (Blue Button) |
| PA13 | Serial_Wire | SWDIO (TMS) |
| PA14 | Serial_Wire | SWCLK (TCK) |

#### (3) I2C1 설정 (Pinout → I2C1)

| 파라미터 | 값 |
|---|---|
| I2C Speed Mode | Standard Mode |
| Speed (Hz) | 100000 |
| Clock No Stretch Mode | Disable |

#### (4) USART2 설정 (Pinout → USART2)

| 파라미터 | 값 |
|---|---|
| Mode | Asynchronous |
| Baud Rate | 115200 |
| Word Length | 8 bits |
| Parity | None |
| Stop Bits | 1 |

#### (5) NVIC 설정

| 인터럽트 | Priority | Enable |
|---|---|---|
| SysTick | 0 | Enabled (Preemption, Sub) |

#### (6) 프로젝트 코드 생성

`Project Manager` 탭에서 설정 확인 후, `Ctrl+S`로 코드 생성 (`GENERATE CODE`).

---

## 4. 프로젝트 구조

```
MPU6050/
├── .cproject                     # Eclipse CDT 설정
├── .project                      # Eclipse 프로젝트 설명
├── .mxproject                    # CubeMX 메타데이터
├── MPU6050.ioc                   # CubeMX 핀/주변장치 설정
├── MPU6050 Debug.launch          # 디버그 실행 설정
├── STM32F411RETX_FLASH.ld        # 링커 스크립트 (FLASH)
├── STM32F411RETX_RAM.ld          # 링커 스크립트 (RAM)
├── mpu6050_3d_viewer.py          # Python 실시간 3D 시각화
├── Core/
│   ├── Inc/
│   │   ├── main.h                # 핀 정의
│   │   ├── stm32f4xx_hal_conf.h  # HAL 모듈 설정
│   │   └── stm32f4xx_it.h        # 인터럽트 핸들러 헤더
│   ├── Src/
│   │   ├── main.c                # 메인 응용 (MPU6050 드라이버 전체)
│   │   ├── stm32f4xx_hal_msp.c   # MCU 특화 초기화 (I2C, UART GPIO)
│   │   ├── stm32f4xx_it.c        # 인터럽트 서비스 루틴
│   │   ├── syscalls.c            # Newlib syscall 스텁
│   │   ├── sysmem.c              # 힙 관리 (_sbrk)
│   │   └── system_stm32f4xx.c    # 시스템 초기화
│   └── Startup/
│       └── startup_stm32f411retx.s # 부트코드/벡터테이블
├── Drivers/
│   ├── CMSIS/                    # CMSIS 코어 헤더
│   └── STM32F4xx_HAL_Driver/    # STM32F4 HAL 드라이버
└── Debug/
    ├── MPU6050.elf               # 컴파일된 바이너리
    ├── MPU6050.list              # 디스어셈블리 목록
    ├── MPU6050.map               # 링커 맵
    └── makefile                  # 빌드 메이크파일
```

---

## 5. 소스 코드 설명

### 5.1 `main.c` — 전체 애플리케이션

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */
```

MPU6050 드라이버 코드는 별도의 파일로 분리하지 않고 `main.c` 내에 `static` 함수로 구현되어 있습니다.

#### 상수 정의 (`main.c:36-44`)

```c
/* USER CODE BEGIN PD */
#define MPU6050_ADDR         0x68    // MPU6050 I2C 주소 (AD0=GND)
#define MPU6050_WHO_AM_I     0x75    // WHO_AM_I 레지스터
#define MPU6050_PWR_MGMT_1   0x6B    // 전원 관리 레지스터
#define MPU6050_ACCEL_XOUT_H 0x3B    // 가속도 X 상위 바이트
#define MPU6050_GYRO_XOUT_H  0x43    // 자이로 X 상위 바이트
#define MPU6050_TEMP_OUT_H   0x41    // 온도 상위 바이트
#define ACCEL_SCALE          16384.0f  // ±2g 기준 스케일 팩터
#define GYRO_SCALE           131.0f    // ±250°/s 기준 스케일 팩터
#define GYRO_CALIB_SAMPLES   64        // 자이로 캘리브레이션 샘플 수
/* USER CODE END PD */
```

#### 정적 변수 (`main.c:58-60`)

```c
/* USER CODE BEGIN PV */
static uint8_t mpu6050_rx_buf[14];   // I2C 수신 버퍼 (14바이트)
static char uart_tx_buf[128];        // UART 송신 버퍼
static volatile int16_t gyro_bias_x, gyro_bias_y, gyro_bias_z; // 자이로 바이어스
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
static void I2C_Scan(void);
static HAL_StatusTypeDef MPU6050_Init(void);
static void MPU6050_CalibrateGyro(void);
static void MPU6050_ReadData(void);
/* USER CODE END PFP */
```

#### 함수 목록

| 함수 | 설명 |
|---|---|
| `I2C_Scan()` | I2C 버스에 연결된 장치 검색 (주소 1~126) |
| `MPU6050_Init()` | MPU6050 초기화 (WHO_AM_I 확인 → Wakeup) |
| `MPU6050_CalibrateGyro()` | 자이로 바이어스 측정 (64샘플 평균) |
| `MPU6050_ReadData()` | 6축 + 온도 데이터 읽기 및 Roll/Pitch 계산 |
| `main()` | 시스템 초기화 → 센서 초기화 → 루프 |
| `SystemClock_Config()` | HSI→PLL 84MHz 클럭 설정 |
| `MX_I2C1_Init()` | I2C1 100kHz 초기화 |
| `MX_USART2_UART_Init()` | USART2 115200 baud 초기화 |
| `MX_GPIO_Init()` | GPIO (LED, 버튼) 초기화 |

#### 5.1.1 `I2C_Scan()` (`main.c:77-107`)

I2C 버스의 모든 7비트 주소(1~126)에 대해 `HAL_I2C_IsDeviceReady()`로 장치 존재 여부를 확인합니다.

```c
for (i = 1; i < 127; i++) {
    ret = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 1, 10);
    if (ret == HAL_OK) {
        // 장치 발견: 0x68 출력
    }
}
```

```c
/* USER CODE BEGIN 0 */
static void I2C_Scan(void)
{
  HAL_StatusTypeDef ret;
  uint8_t i;
  uint8_t found = 0;

  sprintf(uart_tx_buf, "\r\nI2C Scanning...\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);

  for (i = 1; i < 127; i++)
  {
    ret = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 1, 10);
    if (ret == HAL_OK)
    {
      sprintf(uart_tx_buf, "I2C device found at 0x%02X\r\n", i);
      HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
      found++;
    }
  }

  if (found == 0)
  {
    sprintf(uart_tx_buf, "No I2C devices found!\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
  }
  else
  {
    sprintf(uart_tx_buf, "Found %d device(s)\r\n", found);
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
  }
}

static HAL_StatusTypeDef MPU6050_Init(void)
{
  uint8_t whoami;
  uint8_t pwr;

  HAL_Delay(100);

  HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, MPU6050_WHO_AM_I, I2C_MEMADD_SIZE_8BIT, &whoami, 1, 100);
  sprintf(uart_tx_buf, "WHO_AM_I = 0x%02X\r\n", whoami);
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);

  if (whoami != 0x68 && whoami != 0x98)
  {
    sprintf(uart_tx_buf, "Unknown device! (WHO_AM_I = 0x%02X)\r\n", whoami);
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
    return HAL_ERROR;
  }

  pwr = 0x00;
  if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &pwr, 1, 100) != HAL_OK)
  {
    sprintf(uart_tx_buf, "PWR_MGMT_1 write failed!\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
    return HAL_ERROR;
  }

  HAL_Delay(100);

  sprintf(uart_tx_buf, "MPU6050 initialized\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);

  return HAL_OK;
}

static void MPU6050_CalibrateGyro(void)
{
  int32_t sum_x = 0, sum_y = 0, sum_z = 0;
  uint8_t i;

  sprintf(uart_tx_buf, "Gyro calibrating... keep sensor still\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);

  for (i = 0; i < GYRO_CALIB_SAMPLES; i++)
  {
    if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, MPU6050_GYRO_XOUT_H, I2C_MEMADD_SIZE_8BIT, mpu6050_rx_buf, 6, 100) == HAL_OK)
    {
      sum_x += (int16_t)((mpu6050_rx_buf[0] << 8) | mpu6050_rx_buf[1]);
      sum_y += (int16_t)((mpu6050_rx_buf[2] << 8) | mpu6050_rx_buf[3]);
      sum_z += (int16_t)((mpu6050_rx_buf[4] << 8) | mpu6050_rx_buf[5]);
    }
    HAL_Delay(5);
  }

  gyro_bias_x = (int16_t)(sum_x / GYRO_CALIB_SAMPLES);
  gyro_bias_y = (int16_t)(sum_y / GYRO_CALIB_SAMPLES);
  gyro_bias_z = (int16_t)(sum_z / GYRO_CALIB_SAMPLES);

  sprintf(uart_tx_buf, "Gyro bias: %4d %4d %4d\r\n", gyro_bias_x, gyro_bias_y, gyro_bias_z);
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
}

static void MPU6050_ReadData(void)
{
  int16_t accel_x, accel_y, accel_z;
  int16_t gyro_x, gyro_y, gyro_z;
  int16_t temp_raw;
  int16_t temp_int, temp_whole, temp_frac;
  float ax, ay, az;
  int16_t roll, pitch;

  if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, MPU6050_ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT, mpu6050_rx_buf, 14, 100) != HAL_OK)
  {
    sprintf(uart_tx_buf, "I2C read failed!\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
    return;
  }

  accel_x = (int16_t)((mpu6050_rx_buf[0] << 8) | mpu6050_rx_buf[1]);
  accel_y = (int16_t)((mpu6050_rx_buf[2] << 8) | mpu6050_rx_buf[3]);
  accel_z = (int16_t)((mpu6050_rx_buf[4] << 8) | mpu6050_rx_buf[5]);
  temp_raw = (int16_t)((mpu6050_rx_buf[6] << 8) | mpu6050_rx_buf[7]);
  gyro_x  = (int16_t)((mpu6050_rx_buf[8] << 8) | mpu6050_rx_buf[9]) - gyro_bias_x;
  gyro_y  = (int16_t)((mpu6050_rx_buf[10] << 8) | mpu6050_rx_buf[11]) - gyro_bias_y;
  gyro_z  = (int16_t)((mpu6050_rx_buf[12] << 8) | mpu6050_rx_buf[13]) - gyro_bias_z;

  ax = (float)accel_x / ACCEL_SCALE;
  ay = (float)accel_y / ACCEL_SCALE;
  az = (float)accel_z / ACCEL_SCALE;

  roll  = (int16_t)(atan2f(-ay, az) * 57.29578f);
  pitch = (int16_t)(atan2f(ax, sqrtf(ay*ay + az*az)) * 57.29578f);

  temp_int = (int16_t)(((int32_t)temp_raw * 100 / 340) + 3653);
  temp_whole = temp_int / 100;
  temp_frac = temp_int % 100;
  if (temp_frac < 0) temp_frac = -temp_frac;

  sprintf(uart_tx_buf, "ACC: %6d %6d %6d  GYRO: %6d %6d %6d  RPY: %4d %4d %4d  TEMP: %d.%02d C\r\n",
          accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z,
          roll, pitch, 0,
          temp_whole, temp_frac);
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */
  HAL_Delay(500);

  I2C_Scan();

  ret = MPU6050_Init();
  if (ret != HAL_OK)
  {
    sprintf(uart_tx_buf, "MPU6050 init failed. Check wiring!\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
    while (1);
  }

  MPU6050_CalibrateGyro();

  HAL_Delay(200);
  /* USER CODE END 2 */
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    MPU6050_ReadData();
    HAL_Delay(500);
    /* USER CODE END WHILE */
```


- MPU6050의 기본 주소는 `0x68`입니다.
- 발견된 모든 장치 주소를 UART로 출력합니다.

#### 5.1.2 `MPU6050_Init()` (`main.c:109-141`)

```c
// 1) WHO_AM_I 레지스터 읽기 (0x75)
HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, MPU6050_WHO_AM_I,
                 I2C_MEMADD_SIZE_8BIT, &whoami, 1, 100);
// 예상값: 0x68 또는 0x98

// 2) PWR_MGMT_1 레지스터에 0x00 쓰기 (0x6B)
//    = Sleep 모드 해제, 내부 발진기 사용
pwr = 0x00;
HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_PWR_MGMT_1,
                  I2C_MEMADD_SIZE_8BIT, &pwr, 1, 100);
```

- **WHO_AM_I** (0x75): MPU6050의 고정 식별값 (0x68, 최신버전은 0x98)
- **PWR_MGMT_1** (0x6B):
  - Bit 6: `SLEEP` (1=슬립모드, 0=정상동작)
  - Bit 3: `CYCLE` (0=연속 측정)
  - Bits 2:0: `CLKSEL` (000=내부 8MHz 발진기, 001=자이로 X축 PLL)

#### 5.1.3 `MPU6050_CalibrateGyro()` (`main.c:143-168`)

정지 상태에서 자이로 값 64회를 읽어 평균을 바이어스로 저장합니다.

```c
for (i = 0; i < GYRO_CALIB_SAMPLES; i++) {
    // GYRO_XOUT_H (0x43)에서 6바이트 읽기
    HAL_I2C_Mem_Read(&hi2c1, ..., MPU6050_GYRO_XOUT_H, ...,
                     mpu6050_rx_buf, 6, 100);
    sum_x += (int16_t)((mpu6050_rx_buf[0] << 8) | mpu6050_rx_buf[1]);
    sum_y += (int16_t)((mpu6050_rx_buf[2] << 8) | mpu6050_rx_buf[3]);
    sum_z += (int16_t)((mpu6050_rx_buf[4] << 8) | mpu6050_rx_buf[5]);
    HAL_Delay(5);
}
gyro_bias_x = (int16_t)(sum_x / GYRO_CALIB_SAMPLES);
gyro_bias_y = ...;
gyro_bias_z = ...;
```

이상적인 정지 상태 자이로 값은 `0 (0°/s)`이지만, 실제 센서는 DC 오프셋이 존재합니다. 이 바이어스를 `MPU6050_ReadData()`에서 읽은 값에서 실시간으로 빼줍니다.

#### 5.1.4 `MPU6050_ReadData()` (`main.c:170-211`)

```c
// 1) ACCEL_XOUT_H (0x3B)에서 14바이트 읽기
//    = ACCEL_X(2) + ACCEL_Y(2) + ACCEL_Z(2) + TEMP(2) + GYRO_X(2) + GYRO_Y(2) + GYRO_Z(2)
HAL_I2C_Mem_Read(&hi2c1, ..., MPU6050_ACCEL_XOUT_H, ...,
                 mpu6050_rx_buf, 14, 100);

// 2) 빅엔디안 → int16_t 변환
accel_x = (int16_t)((rx_buf[0] << 8) | rx_buf[1]);   // ACCEL_XOUT_H/L
accel_y = (int16_t)((rx_buf[2] << 8) | rx_buf[3]);   // ACCEL_YOUT_H/L
accel_z = (int16_t)((rx_buf[4] << 8) | rx_buf[5]);   // ACCEL_ZOUT_H/L
temp_raw = (int16_t)((rx_buf[6] << 8) | rx_buf[7]);  // TEMP_OUT_H/L
gyro_x = (int16_t)((rx_buf[8] << 8) | rx_buf[9]) - gyro_bias_x;  // GYRO_XOUT_H/L
gyro_y = (int16_t)((rx_buf[10] << 8) | rx_buf[11]) - gyro_bias_y;
gyro_z = (int16_t)((rx_buf[12] << 8) | rx_buf[13]) - gyro_bias_z;

// 3) 가속도 g 단위 변환
ax = (float)accel_x / ACCEL_SCALE;  // ±2g → 16384 LSB/g
ay = (float)accel_y / ACCEL_SCALE;
az = (float)accel_z / ACCEL_SCALE;

// 4) Roll/Pitch 계산 (가속도 기반)
roll  = (int16_t)(atan2f(-ay, az) * 57.29578f);         // degrees
pitch = (int16_t)(atan2f(ax, sqrtf(ay*ay + az*az)) * 57.29578f);

// 5) 온도 계산
//    Temp(°C) = (TEMP_OUT / 340) + 36.53
temp_int = ((int32_t)temp_raw * 100 / 340) + 3653;  // 100배 스케일
temp_whole = temp_int / 100;
temp_frac = temp_int % 100;

// 6) UART 출력
sprintf(uart_tx_buf, "ACC: %6d %6d %6d  GYRO: %6d %6d %6d  RPY: %4d %4d %4d  TEMP: %d.%02d C\r\n",
        accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z,
        roll, pitch, 0, temp_whole, temp_frac);
HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buf, strlen(uart_tx_buf), 100);
```

#### 5.1.5 `main()` (`main.c:218-274`)

```c
int main(void) {
    HAL_Init();                    // HAL 라이브러리 초기화
    SystemClock_Config();          // 84MHz 클럭 설정
    MX_GPIO_Init();                // GPIO 초기화 (LED, 버튼)
    MX_USART2_UART_Init();         // UART 115200 baud
    MX_I2C1_Init();                // I2C 100kHz

    HAL_Delay(500);                // 센서 안정화 대기
    I2C_Scan();                    // I2C 장치 스캔
    MPU6050_Init();                // MPU6050 초기화
    MPU6050_CalibrateGyro();       // 자이로 바이어스 측정

    while (1) {
        MPU6050_ReadData();        // 데이터 읽기 + UART 출력
        HAL_Delay(500);            // 500ms 대기
    }
}
```

### 5.2 `main.h` — 핀 정의 (`main.h:60-73`)

| 매크로 | 값 | 설명 |
|---|---|---|
| `B1_Pin` | `GPIO_PIN_13` | PC13, Blue Push Button |
| `USART_TX_Pin` | `GPIO_PIN_2` | PA2, USART2 TX |
| `USART_RX_Pin` | `GPIO_PIN_3` | PA3, USART2 RX |
| `LD2_Pin` | `GPIO_PIN_5` | PA5, Green LED |
| `TMS_Pin` | `GPIO_PIN_13` | PA13, SWDIO |
| `TCK_Pin` | `GPIO_PIN_14` | PA14, SWCLK |

### 5.3 `stm32f4xx_hal_msp.c` — MCU 특화 핀 초기화

#### `HAL_I2C_MspInit()` (`hal_msp.c:88-117`)

I2C1에 필요한 GPIO와 클럭을 초기화합니다:

```c
__HAL_RCC_GPIOB_CLK_ENABLE();               // GPIOB 클럭 활성화
GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;      // 오픈 드레인
GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;   // AF4 = I2C1
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);      // PB8(SCL), PB9(SDA)
__HAL_RCC_I2C1_CLK_ENABLE();                 // I2C1 클럭 활성화
```

#### `HAL_UART_MspInit()` (`hal_msp.c:156-185`)

USART2에 필요한 GPIO와 클럭을 초기화합니다:

```c
__HAL_RCC_USART2_CLK_ENABLE();
__HAL_RCC_GPIOA_CLK_ENABLE();
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;      // 푸시풀
GPIO_InitStruct.Alternate = GPIO_AF7_USART2; // AF7 = USART2
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);      // PA2(TX), PA3(RX)
```

### 5.4 `MPU6050.ioc` — CubeMX 설정 요약

```
Mcu.Name = STM32F411R(C-E)Tx
Mcu.Package = LQFP64
ProjectManager.TargetToolchain = STM32CubeIDE
ProjectManager.FirmwarePackage = STM32Cube FW_F4 V1.28.3

RCC.PLLN = 336
RCC.PLLP = RCC_PLLP_DIV4
RCC.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK
RCC.SYSCLKFreq_VALUE = 84000000
```

---

## 6. 동작 이론

### 6.1 MPU6050 센서 개요

MPU6050은 InvenSense(TDK)사의 6축 IMU(Inertial Measurement Unit)입니다.

- **3축 MEMS 가속도계**: ±2g / ±4g / ±8g / ±16g (I2C로 설정 가능)
- **3축 MEMS 자이로스코프**: ±250 / ±500 / ±1000 / ±2000 °/s (I2C로 설정 가능)
- **온도 센서**: 내장 다이오드 기반
- **DMP (Digital Motion Processor)**: 내장 (이 프로젝트에서는 미사용)
- **I2C 인터페이스**: 최대 400kHz (Fast Mode), 기본 7비트 주소 `0x68`/`0x69`
- **보조 I2C 마스터**: 외부 센서(자기계 등) 연결 가능 (미사용)

### 6.2 I2C 통신 프로토콜

#### I2C 메모리 읽기 (HAL_I2C_Mem_Read)

STM32 HAL에서는 I2C 메모리 읽기 함수로 MPU6050의 레지스터를 읽습니다:

```
START + [0x68 << 1 | W] + ACK + [REG_ADDR] + ACK
     → REPEATED START
     → [0x68 << 1 | R] + ACK + [DATA_BYTE_1] + ACK + ... + [DATA_BYTE_N] + NAK + STOP
```

- `HAL_I2C_Mem_Read(&hi2c1, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, pData, len, timeout)`

#### I2C 메모리 쓰기 (HAL_I2C_Mem_Write)

```
START + [0x68 << 1 | W] + ACK + [REG_ADDR] + ACK + [DATA] + ACK + STOP
```

- `HAL_I2C_Mem_Write(&hi2c1, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, pData, len, timeout)`

> **주의:** HAL 함수의 `DevAddr` 파라미터는 **8비트 I2C 주소**를 요구하므로, 7비트 주소(`0x68`)를 왼쪽으로 1 시프트(`0xD0`)하여 전달해야 합니다.

### 6.3 레지스터 맵

| 주소 | 이름 | 읽기/쓰기 | 설명 |
|---|---|---|---|
| `0x6B` | PWR_MGMT_1 | R/W | 전원 관리 (Bit 6=1: Sleep, Bit 6=0: Wake) |
| `0x68` | PWR_MGMT_2 | R/W | 개별 축 전원 관리 |
| `0x1A` | CONFIG | R/W | 디지털 저역통과필터(DLPF) 설정 |
| `0x1B` | GYRO_CONFIG | R/W | 자이로 풀스케일 (±250/500/1000/2000 °/s) |
| `0x1C` | ACCEL_CONFIG | R/W | 가속도 풀스케일 (±2/4/8/16 g) |
| `0x3B` | ACCEL_XOUT_H | R | 가속도 X 상위 바이트 (이하 14바이트 연속) |
| `0x3D` | ACCEL_YOUT_H | R | 가속도 Y 상위 바이트 |
| `0x3F` | ACCEL_ZOUT_H | R | 가속도 Z 상위 바이트 |
| `0x41` | TEMP_OUT_H | R | 온도 상위 바이트 |
| `0x43` | GYRO_XOUT_H | R | 자이로 X 상위 바이트 |
| `0x45` | GYRO_YOUT_H | R | 자이로 Y 상위 바이트 |
| `0x47` | GYRO_ZOUT_H | R | 자이로 Z 상위 바이트 |
| `0x75` | WHO_AM_I | R | 장치 ID (0x68 또는 0x98) |

### 6.4 가속도계 (Accelerometer)

#### 원리

MEMS 가속도계는 미세한 실리콘 구조물(proof mass)이 가속도에 의해 변위되는 정전용량(capacitance) 변화를 측정합니다.

#### 측정 범위

기본 설정(±2g)에서 스케일 팩터는 `16384 LSB/g`입니다.

```
가속도(g) = raw_value / 16384.0
```

예: raw = 16384 → 1g, raw = 0 → 0g

#### Roll/Pitch 계산

정지 상태에서 가속도계는 중력 가속도(1g)를 측정합니다. 이를 이용해 Roll(회전)과 Pitch(기울기)를 계산합니다.

```
Roll  = atan2(-Ay, Az) * (180 / π)
Pitch = atan2(Ax, sqrt(Ay² + Az²)) * (180 / π)
```

- **Roll**: X축 중심 회전 (좌우 기울기)
- **Pitch**: Y축 중심 회전 (앞뒤 기울기)
- **Yaw**: Z축 중심 회전 (방위각) — 가속도만으로는 계산 불가 (지자기 또는 자이로 융합 필요)

### 6.5 자이로스코프 (Gyroscope)

#### 원리

MEMS 자이로스코프는 코리올리 효과(Coriolis effect)를 이용합니다. 진동하는 구조물에 회전이 가해지면 코리올리 힘에 의해 수직 방향으로 변위가 발생하고, 이 정전용량 변화를 측정하여 각속도를 산출합니다.

#### 측정 범위

기본 설정(±250 °/s)에서 스케일 팩터는 `131 LSB/°/s`입니다.

```
각속도(°/s) = (raw_value - bias) / 131.0
```

#### 바이어스 보정

정지 상태에서도 자이로는 0이 아닌 DC 오프셋(바이어스)을 출력합니다. 이를 보정하기 위해 초기화 단계에서 64회 측정 평균을 바이어스로 저장하고, 이후 모든 자이로 읽기 값에서 차감합니다.

### 6.6 온도 센서

```
온도(°C) = (TEMP_OUT / 340) + 36.53
```

- TEMP_OUT은 부호 있는 16비트 정수 (단위: LSB)
- 1 LSB ≈ 0.00294°C (340 LSB/°C)
- 36.53°C에서 TEMP_OUT = 0

### 6.7 한계점

| 항목 | 설명 |
|---|---|
| **Yaw 드리프트** | 가속도만으로 Yaw를 계산할 수 없음. 자이로 적분 시 시간이 지남에 따라 드리프트 누적 |
| **가속도 기반 자세 제한** | 선형 가속이 있는 동적 상황에서는 Roll/Pitch가 부정확 |
| **센서 융합 부재** | 이 프로젝트는 가속도/자이로를 별도로 출력하지만, 보다 정확한 자세 추정을 위해 Complementary Filter나 Madgwick/Mahony 필터 권장 |

---

## 7. Python 3D 시각화

### 7.1 개요

`mpu6050_3d_viewer.py`는 STM32의 UART 출력을 시리얼로 수신하여 실시간 3D/2D 그래프로 표시합니다.

### 7.2 필수 라이브러리

```bash
pip install pyserial numpy matplotlib
```

### 7.3 실행

```bash
# 기본 포트 (COM15) 사용
python mpu6050_3d_viewer.py

# 포트 지정
python mpu6050_3d_viewer.py COM3
```

### 7.4 시각화 구성

| 영역 | 내용 |
|---|---|
| 3D 큐브 | Roll/Pitch/Yaw에 따라 회전하는 3차원 큐브 + 좌표축 |
| Roll/Pitch/Yaw | 시간에 따른 각도 변화 그래프 (2D) |
| Accelerometer | X/Y/Z 가속도 (g 단위, 2D) |
| Gyroscope | X/Y/Z 각속도 (°/s 단위, 2D) |
| Temperature | 온도 변화 (2D) |

### 7.5 Complementary Filter

Python 시각화에서는 Complementary Filter를 적용하여 Roll/Pitch를 보정합니다:

```
alpha = 0.96  (필터 계수, 자이로 신뢰도)
roll  = alpha * (roll  + gyro_x * dt) + (1 - alpha) * roll_accel
pitch = alpha * (pitch + gyro_y * dt) + (1 - alpha) * pitch_accel
```

- 자이로는 단기적으로 신뢰 (고주파 응답 우수)
- 가속도는 장기적으로 신뢰 (저주파 드리프트 없음)
- 동적 상황에서는 자이로 가중치 증가, 정적 상황(1g 감지)에서는 가속도 가중치 증가

### 7.6 테스트 내용

#### 7.6.1 테스트 환경

| 항목 | 내용 |
|---|---|
| PC | Windows (STM32CubeIDE 동작 환경) |
| 보드 | NUCLEO-F411RE (ST-Link VCP 내장) |
| 센서 | GY-521 (MPU6050 모듈) |
| 연결 | NUCLEO CN9 (ST-Link USB) → PC USB |
| 시리얼 포트 | STMicroelectronics STLink Virtual COM Port |

#### 7.6.2 포트 확인 방법

**Windows (장치 관리자):**
```
포트 (COM & LPT)
  └─ STMicroelectronics STLink Virtual COM Port (COM15)
```

**CLI로 확인:**
```bash
# Windows PowerShell
Get-WmiObject Win32_SerialPort | Select-Object DeviceID, Description
```

#### 7.6.3 실행 절차

```
1. STM32CubeIDE에서 MPU6050 프로젝트를 빌드 후 보드에 플래시/디버그
2. 시리얼 모니터(Tera Term 등)로 UART 출력 정상 확인
3. 시리얼 모니터 종료 (포트 점유 해제)
4. Python 스크립트 실행:
   python mpu6050_3d_viewer.py COM15

   (COM 포트 번호는 장치 관리자에서 확인)
```

> **주의:** 시리얼 모니터와 Python 스크립트가 동시에同一 포트를 열 수 없습니다. 반드시 시리얼 모니터를 먼저 종료한 후 Python 스크립트를 실행하세요.

#### 7.6.4 정상 실행 시 화면 구성

스크립트 실행 후 matplotlib 창이 열리며 5개 영역이 표시됩니다:

```
┌─────────────────┬──────────────────────┐
│                 │  Roll / Pitch / Yaw  │
│   3D Cube       │  (2D time-series)    │
│   (3D 회전)     ├──────────────────────┤
│                 │  Accelerometer       │
│                 │  (2D time-series)    │
├─────────────────┼──────────────────────┤
│  Gyroscope      │  Temperature         │
│  (2D time-series)│ (2D time-series)    │
└─────────────────┴──────────────────────┘
```

#### 7.6.5 정지 상태 관찰 결과

센서를 평평한 테이블에 고정하고 관찰한 결과:

| 그래프 | 관찰 결과 |
|---|---|
| **3D 큐브** | 거의 수평 유지. 작은 노이즈로 인해 미세하게 떨림 |
| **Roll** | 0° ± 1° 범위에서 안정 |
| **Pitch** | 0° ± 1° 범위에서 안정 |
| **Yaw** | Complementary Filter로 인해 천천히 변화 (저주파 드리프트 발생) |
| **Accel X/Y** | 0g 근처에서 ±0.02g 노이즈 |
| **Accel Z** | +1g (중력 방향) |
| **Gyro X/Y/Z** | 0°/s 근처 (±5 °/s 이내 노이즈, 바이어스 보정됨) |
| **Temperature** | 28 ~ 30°C (실온, 0.1°C 단위로 미세 변동) |

#### 7.6.6 동적 상태 관찰 결과

| 동작 | Roll | Pitch | 3D 큐브 |
|---|---|---|---|
| 좌우 기울이기 (Roll) | 즉시 반응 (±90°) | 거의 변화 없음 | 좌우 회전 |
| 앞뒤 기울이기 (Pitch) | 거의 변화 없음 | 즉시 반응 (±90°) | 앞뒤 회전 |
| 흔들기 | 모든 축에 노이즈 | 모든 축에 노이즈 | 불규칙 회전 |
| 180° 회전 | 가속도 중력 방향 반전 | 가속도 중력 방향 반전 | 큐브 뒤집힘 |

> **참고:** Yaw는 가속도만으로 절대 기준을 잡을 수 없어, Complementary Filter에서도 정지 시 업데이트를 차단하고 자이로 적분만 수행하므로 시간이 지날수록 드리프트가 누적됩니다. 정확한 Yaw를 얻으려면 지자기 센서(HMC5883L 등)를 추가하거나 MPU6050의 DMP를 활용해야 합니다.

#### 7.6.7 문제 해결 (Troubleshooting)

| 증상 | 원인 | 해결 |
|---|---|---|
| `Serial port error: could not open port` | 포트 번호 mismatch 또는 포트 사용 중 | 장치 관리자에서 COM 포트 확인, 시리얼 모니터 종료 |
| 그래프에 데이터 미표시 | UART 파싱 실패 | STM32 출력 포맷 확인 (`ACC: ... GYRO: ... RPY: ... TEMP: ... C`) |
| 3D 큐브 회전 안 됨 | 데이터 수신 안 됨 | I2C 연결 확인, WHO_AM_I=0x68 출력 확인 |
| Yaw가 계속 회전 | 자이로 바이어스 부정확 | 센서를 완전히 정지시키고 리셋 (자이로 캘리브레이션 재실행) |
| Temperature가 36.53°C 고정 | TEMP_OUT = 0 (센서 미동작) | PWR_MGMT_1 설정 확인 (Sleep 모드 해제) |
| `ImportError: No module named 'serial'` | pyserial 미설치 | `pip install pyserial` |
| matplotlib 창이 너무 느림 | CPU 부하 | MAX_SAMPLES 값을 줄이거나 `plt.pause` 간격 조정 |

## 8. 빌드 및 디버그

### 8.1 STM32CubeIDE에서 빌드

1. 프로젝트 우클릭 → `Build Project` (또는 `Ctrl+B`)
2. `Debug/MPU6050.elf` 생성 확인

### 8.2 디버그

1. USB 케이블로 NUCLEO-F411RE 연결 (ST-Link 내장)
2. 프로젝트 우클릭 → `Debug As` → `STM32 Cortex-M C/C++ Application`
3. 디버그 설정:
   - Debug probe: `ST-Link (OpenOCD)`
   - Interface: `SWD`
   - Target: `STM32F411RETx`

### 8.3 시리얼 모니터

- **Port**: ST-Link Virtual COM Port (보통 `COMx`)
- **Baud Rate**: `115200`
- **Data Bits**: `8`
- **Parity**: `None`
- **Stop Bits**: `1`
- **Line Ending**: `CR+LF` (\r\n)

PuTTY, Tera Term, CoolTerm, Arduino Serial Monitor 등 사용 가능.

---

## 9. 시리얼 출력 예시

```
I2C Scanning...
I2C device found at 0x68
Found 1 device(s)
WHO_AM_I = 0x68
MPU6050 initialized
Gyro calibrating... keep sensor still
Gyro bias:   12   -3    5
ACC:    384   152  16256  GYRO:    11    -2     6  RPY:   -1    1    0  TEMP: 28.45 C
ACC:    392   148  16224  GYRO:     8    -6     4  RPY:   -1    1    0  TEMP: 28.51 C
ACC:   4104 -3200  13248  GYRO:    -3   -11     8  RPY:  -14  -14    0  TEMP: 28.48 C
...
```

### 출력 형식

```
ACC:  <가속도X> <가속도Y> <가속도Z>  GYRO: <자이로X> <자이로Y> <자이로Z>  RPY: <Roll> <Pitch> <Yaw>  TEMP: <온도>°C
```

| 필드 | 범위 | 단위 |
|---|---|---|
| ACC (가속도 raw) | ±32768 | LSB (16384 = 1g) |
| GYRO (자이로 raw) | ±32768 (바이어스 보정됨) | LSB (131 = 1°/s) |
| Roll | -180 ~ 180 | degrees |
| Pitch | -90 ~ 90 | degrees |
| Yaw | 항상 0 | degrees |
| TEMP | 약 0 ~ 60 | °C |

---

## 참고 자료

- [MPU6050 Register Map (InvenSense)](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map-1.pdf)
- [STM32F411xC/E Datasheet (ST)](https://www.st.com/resource/en/datasheet/stm32f411re.pdf)
- [STM32CubeF4 HAL Manual](https://www.st.com/resource/en/user_manual/um1725-stm32cube-expansion-package-for-stm32cube-mcu-fw-pack-stmicroelectronics.pdf)
- [NUCLEO-F411RE User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo-64-board-mb1136-stmicroelectronics.pdf)
