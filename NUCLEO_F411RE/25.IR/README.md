# STM32F411 Nucleo – LG TV IR 리모컨 제어

STM32F411 Nucleo 보드로 **NEC 프로토콜** IR 신호를 생성하여 LG TV 전원을 켜고 끄는 프로젝트입니다.

---

## 📋 목차

- [개요](#개요)
- [하드웨어 구성](#하드웨어-구성)
- [회로 연결](#회로-연결)
- [NEC 프로토콜 설명](#nec-프로토콜-설명)
- [LG TV IR 코드](#lg-tv-ir-코드)
- [타이머 설정](#타이머-설정)
- [소프트웨어 구조](#소프트웨어-구조)
- [동작 방법](#동작-방법)
- [STM32CubeIDE 프로젝트 설정](#stm32cubeide-프로젝트-설정)
- [트러블슈팅](#트러블슈팅)

---

## 개요

| 항목 | 내용 |
|------|------|
| MCU | STM32F411RE (Nucleo-F411RE) |
| 클럭 | HSI 16MHz |
| IR 반송파 | 38kHz (TIM1 CH1 PWM) |
| 프로토콜 | NEC IR Protocol |
| 대상 | LG TV (대부분 모델 호환) |
| 트리거 | User Button (PC13) |

---

## 하드웨어 구성

```
[STM32F411 Nucleo]
     PA8 (TIM1_CH1)
          │
        100Ω
          │
        IR LED 애노드(+)
        IR LED 캐소드(-) ── GND
```

### 필요 부품

| 부품 | 규격 | 수량 |
|------|------|------|
| IR LED | 940nm, 예: TSAL6400 | 1 |
| 저항 | 100Ω, 1/4W | 1 |
| 점퍼 와이어 | - | 2 |

> **참고:** IR LED 순방향 전압 약 1.2V, STM32 GPIO 출력 3.3V  
> → (3.3V - 1.2V) / 100Ω = 21mA → 충분한 출력

---

## 회로 연결

```
Nucleo-F411RE               IR LED
┌────────────────┐           ┌──────┐
│ PA8 (CN10-21) ├──┤100Ω├──┤ (+)  │
│               │            │      │
│ GND (CN10-20) ├────────────┤ (-) │
└────────────────┘           └──────┘

PC13 = User Button (보드 내장, Active LOW)
PA5  = LD2 User LED (시각적 피드백)
```

---

## NEC 프로토콜 설명

LG TV는 표준 **NEC IR 프로토콜**을 사용합니다.

### 프레임 구조

```
9ms      4.5ms   LSB ─────────────── MSB   560μs
┌────────┐ ┌──┐  bit0 bit1 ... bit31  ┌─┐
│ MARK   │ │SP│  [ADDR][~ADDR][CMD][~CMD] │M│
└────────┘ └──┘                        └─┘
 리더 마크  리더   32비트 데이터 (LSB first)  종료
           스페이스
```

### 비트 인코딩

```
비트 '0':  마크 560μs  + 스페이스  560μs  (총 1120μs)
비트 '1':  마크 560μs  + 스페이스 1690μs  (총 2250μs)

  ┌──┐    ┌──┐
  │  │    │  │
──┘  └────┘  └────  →  '0' (스페이스 짧음)

  ┌──┐         ┌──┐
  │  │         │  │
──┘  └─────────┘  └──  →  '1' (스페이스 김)
```

### 32비트 데이터 구성

```
비트 [31:24] = Address     (기기 주소)
비트 [23:16] = ~Address    (주소 반전, 오류 검출)
비트 [15: 8] = Command     (명령)
비트  [7: 0] = ~Command    (명령 반전, 오류 검출)
```

---

## LG TV IR 코드

| 기능 | 32비트 코드 | Address | Command |
|------|------------|---------|---------|
| Power Toggle | `0x20DF10EF` | 0x04 | 0x08 |
| Power On | `0x20DFB34C` | 0x04 | 0xCD |
| Power Off | `0x20DFA35C` | 0x04 | 0xC5 |
| Volume Up | `0x20DF40BF` | 0x04 | 0x02 |
| Volume Down | `0x20DFC03F` | 0x04 | 0x03 |
| Mute | `0x20DF906F` | 0x04 | 0x09 |
| CH Up | `0x20DF00FF` | 0x04 | 0x00 |
| CH Down | `0x20DF807F` | 0x04 | 0x01 |

> **일반적으로 `0x20DF10EF` (Power Toggle)이 가장 많이 사용됩니다.**  
> 모델에 따라 코드가 다를 수 있으니 동작하지 않을 경우 `0x20DFB34C`도 시도해보세요.

---

## 타이머 설정

### TIM1 – 38kHz 반송파 PWM (PA8)

```
SYSCLK = 16MHz (HSI)
APB2   = 16MHz

PSC  = 0    → TIM1 클럭 = 16,000,000 Hz
ARR  = 421  → 주파수 = 16,000,000 / 422 ≈ 37,914 Hz ≈ 38kHz
CCR1 = 140  → 듀티비 = 140/422 ≈ 33%  (IR LED 표준 듀티)

CH1 출력: PA8 (AF1, TIM1_CH1)
```

> TIM1은 **Advanced Timer**이므로 `TIM_BDTR` 레지스터의  
> `MOE` 비트가 활성화되어야 합니다. (`AutomaticOutput = ENABLE`)

### TIM2 – 1μs 딜레이 기준 타이머

```
SYSCLK = 16MHz
APB1   = 16MHz

PSC  = 15   → TIM2 클럭 = 16,000,000 / 16 = 1,000,000 Hz
ARR  = 0xFFFFFFFF (32비트 최대)
→ 1틱 = 1μs
```

---

## 소프트웨어 구조

```
LG_IR_Remote/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c          ← 전체 구현
└── README.md
```

### 주요 함수

| 함수 | 설명 |
|------|------|
| `IR_CarrierOn()` | TIM1 PWM 시작 (38kHz 반송파 ON) |
| `IR_CarrierOff()` | TIM1 PWM 정지 (반송파 OFF) |
| `IR_DelayUs(us)` | TIM2 기반 마이크로초 딜레이 |
| `IR_SendMark(us)` | 반송파 ON 상태로 지정 시간 유지 |
| `IR_SendSpace(us)` | 반송파 OFF 상태로 지정 시간 유지 |
| `NEC_SendFrame(code)` | NEC 32비트 프레임 전송 |
| `NEC_SendRepeat()` | NEC 반복 프레임 전송 |

---

## 동작 방법

1. 회로를 연결합니다 (PA8 → 100Ω → IR LED → GND)
2. STM32CubeIDE에서 프로젝트를 빌드하고 플래시합니다
3. IR LED를 LG TV 수신부 쪽으로 향합니다
4. **User Button (PC13)** 을 누릅니다
5. LG TV 전원이 켜지거나 꺼집니다
6. **LD2 (PA5)** LED가 신호 전송마다 토글됩니다

---

## STM32CubeIDE 프로젝트 설정

### 새 프로젝트 생성

1. `File > New > STM32 Project`
2. Board: `NUCLEO-F411RE`
3. `main.c` 내용을 위 코드로 교체

### 클럭 설정 (없어도 동작 – HSI 기본값)

- System Core Clock: 16MHz (HSI)
- No PLL required

### TIM1 CubeMX 설정 (직접 코드 초기화로 대체 가능)

```
TIM1 > Channel 1: PWM Generation CH1
  Prescaler: 0
  Counter Period: 421
  Pulse: 140
  Auto-reload preload: Disable

TIM1 BDTR:
  Automatic Output: Enable
```

### TIM2 CubeMX 설정

```
TIM2 > Activated
  Prescaler: 15
  Counter Period: 4294967295 (0xFFFFFFFF)
```

---

## 트러블슈팅

| 증상 | 원인 | 해결책 |
|------|------|--------|
| TV 반응 없음 | IR LED 방향 오류 | LED 극성 및 방향 확인 |
| TV 반응 없음 | 코드 모델 불일치 | `0x20DFB34C` 또는 다른 코드 시도 |
| PA8 출력 없음 | TIM1 MOE 미설정 | `AutomaticOutput = ENABLE` 확인 |
| 신호 왜곡 | 38kHz 부정확 | PSC/ARR 값 재계산 |
| 버튼 채터링 | 디바운싱 부족 | `HAL_Delay(20)` 값 증가 |
| IR 거리 짧음 | 전류 부족 | 저항값을 47Ω으로 낮춤 |

### 38kHz 정확도 확인

오실로스코프 또는 로직 애널라이저로 PA8 출력을 측정하여 38kHz 확인:

```
측정값: 37,914 Hz ≈ 38kHz (오차 0.2%)  → 정상
허용 범위: 36kHz ~ 40kHz
```

---

## 참고 자료

- [NEC IR Protocol - SB-Projects](https://www.sbprojects.net/knowledge/ir/nec.php)
- [LG IR Codes - GitHub Gist](https://gist.github.com/francis2110/8f69843dd57ae07dce80)
- [STM32F411 Reference Manual (RM0383)](https://www.st.com/resource/en/reference_manual/rm0383-stm32f411xce-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

---

> 작성자: 나무 | STM32F411 Nucleo IR Remote Control Project
