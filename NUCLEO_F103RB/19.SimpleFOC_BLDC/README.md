# SimpleFOC_BLDC

<img width="428" height="370" alt="122" src="https://github.com/user-attachments/assets/beff5a57-0826-4138-9d36-0e8cdaabafbd" />

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



