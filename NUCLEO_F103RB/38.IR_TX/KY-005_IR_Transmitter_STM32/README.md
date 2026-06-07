# KY-005 IR Transmitter — STM32F103 NUCLEO (STM32CubeIDE / HAL)

KY-005 적외선 송신기 모듈을 STM32F103RB NUCLEO 보드에서 구동하는 HAL 기반 프로젝트입니다.

## 하드웨어 연결

```
KY-005          NUCLEO-F103RB
─────           ──────────────
S   (Signal) ──→ PA0  (TIM2_CH1, CN7 pin 30)
VCC (중간)   ──→ 5V   (CN7 pin 18)
GND (−)      ──→ GND  (CN7 pin 20)
```

### 보드 내장 자원
| 기능 | 핀 | 비고 |
|------|-----|------|
| LD2 (LED) | PA5 | 출력, 전송 표시 |
| B1 (버튼) | PC13 | 입력, IR 송신 트리거 |
| VCP (UART) | PA2/PA3 | ST-Link 가상 COM 포트 |

## STM32CubeMX 설정 방법

이 프로젝트는 STM32CubeMX .ioc 파일에서 다음 설정이 필요합니다:

### 1. 새 프로젝트 생성
1. STM32CubeIDE 실행 → File → New → STM32 Project
2. Board Selector 탭 → "NUCLEO-F103RB" 검색/선택
3. 프로젝트 이름: `KY-005_IR_Transmitter_STM32`
4. Target Language: C, Binary Type: Executable, Project Type: STM32Cube
5. Finish → "Initialize all peripherals with their default mode?" → Yes

### 2. RCC (System Clock) 설정
- Pinout & Configuration → System Core → RCC
- High Speed Clock (HSE): **Crystal/Ceramic Resonator**
- Clock Configuration 탭:
  - HSE: 8MHz
  - PLLMUL: x9 → PLLCLK = 72MHz
  - HCLK: 72MHz
  - APB1 Prescaler: /2 → 36MHz
  - APB2 Prescaler: /1 → 72MHz

### 3. TIM2 설정 (38kHz PWM — IR 반송파)

**Pinout:**
- Pinout & Configuration → Timers → TIM2
- Channel1: **PWM Generation CH1**
- PA0이 자동으로 TIM2_CH1로 할당됩니다

**Parameter Settings:**
| 항목 | 값 | 설명 |
|------|-----|------|
| Prescaler (PSC) | 0 | 분주 없음 |
| Counter Mode | Up | 상향 카운트 |
| Counter Period (ARR) | 1894 | 72MHz / 1895 ≈ 38.0kHz |
| Auto-reload preload | Disable | |
| CH1 Pulse | 947 | 50% 듀티 사이클 |
| CH1 Polarity | High | |

### 4. TIM3 설정 (1us 타이머 — IR 타이밍)

**Pinout:**
- Pinout & Configuration → Timers → TIM3
- 모든 Channel: **Disabled** (내부 타이머로만 사용)

**Parameter Settings:**
| 항목 | 값 | 설명 |
|------|-----|------|
| Prescaler (PSC) | 71 | 72MHz / 72 = 1MHz → 1us/tick |
| Counter Mode | Up | |
| Counter Period (ARR) | 65535 | 최대값 (16비트) |
| Auto-reload preload | Disable | |

### 5. USART2 설정 (Virtual COM Port)

**Pinout:**
- Pinout & Configuration → Connectivity → USART2
- Mode: **Asynchronous**
- PA2(TX)와 PA3(RX)가 자동 할당됨

**Parameter Settings:**
| 항목 | 값 |
|------|-----|
| Baud Rate | 115200 |
| Word Length | 8 Bit |
| Parity | None |
| Stop Bits | 1 |

NVIC Settings:
- USART2 global interrupt: **Enabled**

### 6. GPIO 설정

기본 GPIO는 보드 초기화 시 자동 설정됩니다:
- PA5: LD2 (Output, Push-Pull)
- PC13: B1 (Input, IT Falling Edge)

### 7. 코드 생성

1. Project Manager → Code Generator
   - "Copy only the necessary library files" 체크
   - "Generate peripheral initialization as a pair of .c/.h files" 체크
2. 저장 (Ctrl+S) → "Generate Code" 클릭

## 프로젝트에 커스텀 파일 추가

코드 생성 후, 이 패키지의 다음 파일들을 프로젝트에 추가하세요:

### 추가할 파일
```
Core/Inc/ir_transmitter.h    → KY-005 IR 드라이버 헤더
Core/Src/ir_transmitter.c    → KY-005 IR 드라이버 구현
```

### main.c 수정 사항

CubeMX가 생성한 `main.c`에서:
1. Includes 섹션에 `#include "ir_transmitter.h"` 추가
2. `/* USER CODE BEGIN 2 */` 에 IR 트랜스미터 초기화 코드 추가:
   ```c
   IR_Transmitter_Init(&htim2, TIM_CHANNEL_1, &htim3);
   HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
   Print_Menu();
   ```
3. `/* USER CODE BEGIN PV */` 에 UART 수신 버퍼 변수 추가:
   ```c
   uint8_t uart_rx_byte;
   ```
4. UART Callback 함수와 메뉴 함수를 `/* USER CODE BEGIN 4 */` 에 추가

(자세한 코드는 이 패키지의 `Core/Src/main.c` 참조)

## 사용법

### 시리얼 모니터 (115200 baud) 명령어
| 명령 | 동작 |
|------|------|
| `1` | Samsung TV Power (NEC 0x07, 0x40) |
| `2` | Samsung Vol+ (NEC 0x07, 0x10) |
| `3` | Samsung Vol- (NEC 0x07, 0x11) |
| `4` | Samsung Mute (NEC 0x07, 0x0D) |
| `5` | NEC Repeat 코드 |
| `6` | Sony SIRC Power (12-bit) |
| `b` | 버튼 모드 전환 |
| `h` | 도움말 출력 |

### 하드웨어 버튼
NUCLEO 보드의 **B1 (User Button)**을 누르면 Samsung Vol+ IR 코드가 전송됩니다.

### LED 표시
- LD2가 켜져 있는 동안 IR 신호 전송 중
- 전송 완료 후 LD2 소등

## NEC 프로토콜 상세

### 프레임 구조
```
Leader:  9.0ms ON  +  4.5ms OFF
Address: 8bit ADDR + 8bit ~ADDR (LSB First)
Command: 8bit CMD  + 8bit ~CMD  (LSB First)
Stop:    562us ON
```

### 비트 인코딩
```
Logic 0: 562us ON  +  562us OFF  = 1.125ms
Logic 1: 562us ON  + 1687us OFF  = 2.250ms
```

### 38kHz 반송파 계산
```
TIM2 Clock = APB1 × 2 = 36MHz × 2 = 72MHz
PSC = 0
ARR = (72,000,000 / 38,000) - 1 = 1894
실제 주파수 = 72,000,000 / 1895 ≈ 38,015 Hz (0.04% 오차)
```

## 통신 거리 및 주의사항

- KY-005는 트랜지스터 드라이버가 내장되어 있지 않습니다
- STM32 GPIO 핀 전류(약 20mA)로 1~3m 거리에서 안정적 동작
- 더 긴 거리가 필요하면 2N2222/BC547 NPN 트랜지스터 드라이버 추가
- IR LED는 좁은 빔(±30°)을 가지므로 수신기를 직접 조준해야 함
- 38kHz 반송파는 IRremote 라이브러리와 호환됨
