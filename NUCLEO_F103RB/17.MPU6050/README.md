# MPU6050

<img width="382" height="289" alt="124" src="https://github.com/user-attachments/assets/66e7c924-b961-4ac9-84db-a953738b1a70" />

![](Fig.NUCLEO-F103RB.png)

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


