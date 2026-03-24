# STM32F411 Nucleo – LG TV IR 리모컨 제어

STM32F411RE Nucleo 보드로 **NEC 프로토콜** IR 신호를 생성하여 LG TV 전원을 켜고 끄는 프로젝트입니다.

---

## 📋 목차

- [개요](#개요)
- [하드웨어 구성](#하드웨어-구성)
- [회로 연결](#회로-연결)
- [클럭 트리 구성](#클럭-트리-구성)
- [타이머 설정](#타이머-설정)
- [NEC 프로토콜 설명](#nec-프로토콜-설명)
- [LG TV IR 코드](#lg-tv-ir-코드)
- [소프트웨어 구조](#소프트웨어-구조)
- [동작 방법](#동작-방법)
- [STM32CubeIDE 프로젝트 설정](#stm32cubeide-프로젝트-설정)
- [트러블슈팅](#트러블슈팅)

---

## 개요

| 항목 | 내용 |
|------|------|
| MCU | STM32F411RE (Nucleo-F411RE) |
| SYSCLK | 100MHz (HSI → PLL) |
| IR 반송파 | 38kHz (TIM1 CH1 PWM, PA8) |
| 딜레이 기준 | 1μs (TIM2 폴링) |
| 프로토콜 | NEC IR Protocol |
| 대상 | LG TV (대부분 모델 호환) |
| 트리거 | User Button (PC13, Active LOW) |

---

## 하드웨어 구성

### 필요 부품

| 부품 | 규격 | 수량 |
|------|------|------|
| IR LED | 940nm (예: TSAL6400, VSLB3940) | 1 |
| 저항 | 100Ω, 1/4W | 1 |
| 점퍼 와이어 | - | 2 |

> STM32 GPIO 출력 3.3V, IR LED Vf ≈ 1.2V  
> → (3.3 - 1.2) / 100Ω = **21mA** → 충분한 IR 출력

---

## 회로 연결

```
Nucleo-F411RE                    IR LED
┌─────────────────┐              ┌──────┐
│ PA8 (CN10-21)  ├──┤ 100Ω ├───┤ (+)  │
│                 │              │      │
│ GND (CN10-20)  ├──────────────┤ (-)  │
└─────────────────┘              └──────┘

PC13 = User Button (보드 내장, Active LOW)
PA5  = LD2 User LED (신호 전송 시 토글)
```

---

## 클럭 트리 구성

STM32F411RE Nucleo의 기본 클럭 설정 (CubeMX 자동 생성 기준)입니다.

```
HSI (16MHz)
    │
    ▼
  PLL
  ├─ PLLM = 8   → 16MHz / 8  =   2MHz
  ├─ PLLN = 200 → 2MHz × 200 = 400MHz (VCO)
  └─ PLLP = 4   → 400MHz / 4 = 100MHz
    │
    ▼
SYSCLK = 100MHz
    │
    ├─ AHB  (DIV1) → HCLK  = 84MHz
    │
    ├─ APB2 (DIV1) → PCLK2 = 84MHz
    │                   └─ TIM1 클럭 = 84MHz  ← 38kHz PWM 생성
    │
    └─ APB1 (DIV2) → PCLK1 =  84MHz
                        └─ TIM2 클럭 = 84MHz  ← 1μs 딜레이 기준
                           (APBx PSC > 1 → TIMx 클럭 = APBx × 2, RM0383 규칙)
```

> **TIM 클럭 배수 규칙 (RM0383 참조)**  
> APBx 프리스케일러 = 1 이면 → TIMx 클럭 = APBx 클럭  
> APBx 프리스케일러 > 1 이면 → TIMx 클럭 = APBx 클럭 × 2

---

## 타이머 설정

### TIM1 – 38kHz 반송파 PWM (PA8)

IR LED를 38kHz로 ON/OFF 변조하는 반송파를 생성합니다.

```
TIM1 클럭 = 84MHz

PSC  = 0
ARR  = 2631  →  84,000,000 / (0+1) / (2631+1) = 38,004 Hz ≈ 38kHz
CCR1 = 877   →  877 / 2632 ≈ 33% 듀티비 (IR 표준)

출력 핀: PA8 (AF1, TIM1_CH1)
```

> **주의:** TIM1은 Advanced Control Timer입니다.  
> BDTR 레지스터의 `MOE (Main Output Enable)` 비트가 반드시 활성화되어야 PA8 출력이 나옵니다.  
> 코드에서 `AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE` 로 설정합니다.

### TIM2 – 1μs 딜레이 기준 타이머

NEC 프로토콜의 마크/스페이스 타이밍(560μs, 4.5ms, 9ms 등)을 정확하게 생성합니다.

```
TIM2 클럭 = 100MHz  (APB1=50MHz, PSC=2 → TIM 배수 × 2)

PSC  = 99         →  100,000,000 / 100 = 1,000,000 Hz
ARR  = 0xFFFFFFFF →  32비트 최대
→ 1틱 = 1μs
```

> TIM1(반송파)과 TIM2(딜레이) 모두 TIM 클럭이 100MHz로 동일합니다.  
> SYSCLK가 변경되면 두 타이머의 PSC/ARR 값을 **함께** 재계산해야 합니다.

---

## NEC 프로토콜 설명

LG TV는 표준 **NEC IR 프로토콜**을 사용합니다.

### 프레임 구조

```
  9ms      4.5ms        32비트 (LSB first)          560μs
┌───────┐ ┌─────┐  ┌─────────────────────────┐  ┌───┐
│ MARK  │ │ SP  │  │ ADDR │~ADDR│ CMD │~CMD  │  │ M │
└───────┘ └─────┘  └─────────────────────────┘  └───┘
 리더마크  스페이스       데이터 32비트               종료
```

### 비트 인코딩

```
비트 '0' :  마크 560μs  +  스페이스  560μs  =  총 1,120μs
비트 '1' :  마크 560μs  +  스페이스 1,690μs  =  총 2,250μs
```

### 32비트 데이터 구성

```
bits [7 : 0]  = Address    (기기 주소)
bits [15: 8]  = ~Address   (주소 반전, 오류 검출)
bits [23:16]  = Command    (명령)
bits [31:24]  = ~Command   (명령 반전, 오류 검출)
```

---

## LG TV IR 코드

| 기능 | 32비트 코드 | Address | Command |
|------|------------|---------|---------|
| **Power Toggle** | `0x20DF10EF` | 0x04 | 0x08 |
| Power On | `0x20DFB34C` | 0x04 | 0xCD |
| Power Off | `0x20DFA35C` | 0x04 | 0xC5 |
| Volume Up | `0x20DF40BF` | 0x04 | 0x02 |
| Volume Down | `0x20DFC03F` | 0x04 | 0x03 |
| Mute | `0x20DF906F` | 0x04 | 0x09 |
| CH Up | `0x20DF00FF` | 0x04 | 0x00 |
| CH Down | `0x20DF807F` | 0x04 | 0x01 |

> **일반적으로 `0x20DF10EF` (Power Toggle)을 사용합니다.**  
> 동작하지 않을 경우 `0x20DFB34C` (Power On 전용)을 시도해보세요.

---

## 소프트웨어 구조

```
LG_IR_Remote/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c
└── README.md
```

### 주요 함수

| 함수 | 설명 |
|------|------|
| `IR_CarrierOn()` | TIM1 PWM 시작 → 38kHz 반송파 ON |
| `IR_CarrierOff()` | TIM1 PWM 정지 → 반송파 OFF |
| `IR_DelayUs(us)` | TIM2 카운터 폴링으로 μs 대기 |
| `IR_SendMark(us)` | 반송파 ON 상태로 지정 시간 유지 |
| `IR_SendSpace(us)` | 반송파 OFF 상태로 지정 시간 유지 |
| `NEC_SendFrame(code)` | NEC 32비트 프레임 완전 전송 |
| `NEC_SendRepeat()` | NEC 반복 프레임 전송 |

### 신호 흐름

```
User Button 누름 (PC13 LOW)
    │
    ▼
NEC_SendFrame(0x20DF10EF)
    │
    ├─ IR_SendMark(9000)   ← 리더 마크  9ms   [TIM1 ON  + TIM2 딜레이]
    ├─ IR_SendSpace(4500)  ← 리더 스페이스 4.5ms [TIM1 OFF + TIM2 딜레이]
    │
    ├─ (32회 반복)
    │   ├─ IR_SendMark(560)
    │   └─ IR_SendSpace(560 or 1690)  ← 비트 '0' or '1'
    │
    ├─ IR_SendMark(560)    ← 종료 마크
    └─ HAL_Delay(40)       ← 프레임 후 간격
```

---

## 동작 방법

1. 회로를 연결합니다 (`PA8 → 100Ω → IR LED(+) → IR LED(-) → GND`)
2. STM32CubeIDE에서 프로젝트를 빌드하고 플래시합니다
3. IR LED를 LG TV 수신부 방향으로 향하게 합니다
4. **User Button (PC13)** 을 누릅니다
5. LG TV 전원이 켜지거나 꺼집니다
6. **LD2 (PA5)** LED가 신호 전송마다 토글됩니다

---

## STM32CubeIDE 프로젝트 설정

### 새 프로젝트 생성

1. `File > New > STM32 Project`
2. Board: `NUCLEO-F411RE` 선택
3. 생성된 `main.c` 내용을 본 코드로 교체

### Clock Configuration (CubeMX)

```
Input frequency : 16 MHz (HSI)
PLL Source Mux  : HSI
PLLM = 8 / PLLN = 200 / PLLP = 4
System Clock Mux: PLLCLK
HCLK            : 100 MHz
APB1 Prescaler  : /2  (PCLK1 = 50MHz)
APB2 Prescaler  : /1  (PCLK2 = 100MHz)
```

### Pinout (CubeMX)

```
PA8  : TIM1_CH1
PA5  : GPIO_Output (LD2)
PC13 : GPIO_Input  (User Button)
```

---

## 트러블슈팅

| 증상 | 원인 | 해결책 |
|------|------|--------|
| TV 반응 없음 | IR 코드 모델 불일치 | `0x20DFB34C` 등 다른 코드 시도 |
| TV 반응 없음 | IR LED 방향 오류 | LED 극성 및 TV 수신부 방향 확인 |
| PA8 출력 없음 | TIM1 MOE 미설정 | `AutomaticOutput = ENABLE` 확인 |
| 타이밍 틀어짐 | SYSCLK 변경 후 PSC 미수정 | TIM1 ARR, TIM2 PSC 재계산 |
| IR 거리 짧음 | LED 전류 부족 | 저항을 47Ω으로 낮춤 |
| 버튼 오동작 | 채터링 | `HAL_Delay(20)` 값을 50으로 증가 |

### 클럭별 TIM 설정값 참고

| SYSCLK | APB2 | TIM1 클럭 | ARR (38kHz) | CCR (33%) | TIM2 PSC (1μs) |
|--------|------|-----------|-------------|-----------|----------------|
| 100MHz | 100MHz | 100MHz | 2631 | 877 | 99 |
| 84MHz  |  84MHz |  84MHz | 2209 | 736 | 83 |
| 16MHz  |  16MHz |  16MHz |  421 | 140 | 15 |

---

## 참고 자료

- [NEC IR Protocol – SB-Projects](https://www.sbprojects.net/knowledge/ir/nec.php)
- [LG IR Codes – LIRC Database](http://lirc.sourceforge.net/remotes/lg/)
- [STM32F411xC/E Reference Manual (RM0383)](https://www.st.com/resource/en/reference_manual/rm0383-stm32f411xce-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [Nucleo-F411RE User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

---

> 작성: STM32F411 Nucleo IR Remote Control Project
