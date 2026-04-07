# NUCLEO STM32F103 전원부 회로 분석

> **NUCLEO-F103RB** 보드의 전원 공급 회로(Power Supply Section)에 대한 상세 분석 문서입니다.  
> 외부 전원(VIN)이 MCU용 3.3V(VDD)로 변환되기까지의 전 과정과 P-ch MOSFET 스위치 동작을 다룹니다.

---

## 목차

- [전체 신호 흐름](#전체-신호-흐름)
- [회로 구성 요소 상세](#회로-구성-요소-상세)
  - [PWR_ENn — 전원 인에이블 신호](#1-pwr_enn--전원-인에이블-신호)
  - [T2 (STS7PF30L) — P-ch MOSFET 전원 스위치](#2-t2-sts7pf30l--p-ch-mosfet-전원-스위치)
  - [LD1117S50TR — 1차 LDO (5V)](#3-ld1117s50tr--1차-ldo-5v)
  - [JP5 — 전원 경로 선택 점퍼](#4-jp5--전원-경로-선택-점퍼)
  - [LD39050PU33R — 2차 LDO (3.3V)](#5-ld39050pu33r--2차-ldo-33v)
- [T2 MOSFET 동작 원리](#t2-mosfet-동작-원리)
  - [P-ch MOSFET 기초](#p-ch-mosfet-기초)
  - [SB1 열림 (Default OFF)](#sb1-열림--default-off)
  - [SB1 단락 (ON)](#sb1-단락--on)
  - [R28의 역할](#r28의-역할)
- [전원 경로 다이어그램](#전원-경로-다이어그램)
- [설계 의도 요약](#설계-의도-요약)
- [주요 부품 스펙 요약](#주요-부품-스펙-요약)

---

<img src="power_001" width="200" height="100"><br>
<img src="power_001" width="200" height="100">


## 전체 신호 흐름

```
VIN (외부 전원)
  │
  ├──[T2: P-ch MOSFET 스위치]── (SB1 단락 시 활성화)
  │
  ▼
LD1117S50TR (LDO)
  │  VIN → 5V 강압
  ▼
D4 (STPS2L30A 쇼트키 다이오드)  ←←← JP5 ←←← USB 5V (대체 입력)
  │  역전류 방지
  ▼
JP5 분기점 (E5V)
  │
  ▼
LD39050PU33R (LDO)
  │  5V → 3.3V 강압
  ▼
VDD (STM32F103 MCU 공급)
```

---

## 회로 구성 요소 상세

### 1. PWR_ENn — 전원 인에이블 신호

| 항목 | 내용 |
|------|------|
| 신호명 | `PWR_ENn` |
| 논리 | Active-Low (`n` 접미사) |
| 동작 | Low → 전원 활성화 / High(또는 오픈) → 전원 차단 |

이 신호는 외부 제어기(디버거, 호스트 PC 등)가 보드의 전원을 소프트웨어적으로 제어할 수 있게 합니다.

---

### 2. T2 (STS7PF30L) — P-ch MOSFET 전원 스위치

VIN 경로의 물리적 온/오프를 담당하는 핵심 스위치 소자입니다.  
**SB1(점퍼)의 상태**에 따라 Gate 전위가 결정되어 도통 여부가 결정됩니다.

> SB1은 기본값이 **Open(열림)** 이므로, 별도 설정 없이는 VIN 경로가 차단됩니다.

---

### 3. LD1117S50TR — 1차 LDO (5V)

| 항목 | 내용 |
|------|------|
| 출력 전압 | **5V** |
| 패키지 | SOT-223 |
| 입력 캐패시터 | C16 — 10μF / 25V |
| 출력 캐패시터 | C17 — 10μF |
| 후단 보호 | D4 (STPS2L30A 쇼트키, ~0.3V 강하) |

LDO(Low Dropout Regulator) 특성상 입출력 전압차가 최소 1.2V 이상 필요합니다.  
D4는 역전류 방지 역할과 함께, USB 5V와 LD1117 출력 5V가 JP5에서 합류할 때 **OR-ing 다이오드** 기능을 수행합니다.

---

### 4. JP5 — 전원 경로 선택 점퍼

| JP5 위치 | 전원 입력 |
|----------|----------|
| Pin 1-2 | USB VBUS (5V) 직접 사용 |
| Pin 2-3 | VIN → LD1117 경유 5V 사용 |

이 점퍼로 인해 ST-LINK USB 전원 또는 외부 어댑터 전원 중 하나를 유연하게 선택할 수 있습니다.

---

### 5. LD39050PU33R — 2차 LDO (3.3V)

| 항목 | 내용 |
|------|------|
| 입력 전압 | 5V (JP5 출력) |
| 출력 전압 | **3.3V (VDD)** |
| EN 핀 | R32(1KΩ) 풀업 + C21(1μF) 필터링 |
| 전원 표시 | LD3 (RED LED) |
| 입력 바이패스 | C20 — 100nF |
| 출력 디커플링 | C18(1μF) + C19(100nF) |
| PG 핀 | 3.3V 정상 출력 시 High → MCU Ready 신호 |

출력 디커플링을 1μF + 100nF 병렬로 구성한 것은 넓은 주파수 대역의 노이즈를 제거하기 위함입니다(대용량: 저주파, 소용량: 고주파).

---

## T2 MOSFET 동작 원리

### P-ch MOSFET 기초

P-channel MOSFET은 N-channel과 반대로, **Vgs(게이트-소스 전압)가 충분히 음수(−)** 일 때 채널이 형성되어 전류가 흐릅니다.

```
P-ch 도통 조건:  Vgs < Vth  (Vth는 음수, 예: −1 ~ −2V)
```

STS7PF30L의 Vth(게이트 임계전압)는 일반적으로 −1V ~ −2.5V입니다.

---

### SB1 열림 — Default OFF

```
U5V ──[R28: 4.7KΩ]──┬── Gate (G)
                    │
                    ○  (SB1 열림: GND 연결 없음)

Source (S) = VIN (예: 7V)
Gate   (G) = U5V (5V) via R28 풀업

Vgs = Gate − Source = 5V − 7V = −2V  →  도통 가능?
```

> **주의**: VIN이 U5V보다 높을 때는 Vgs가 음수가 되어 의도치 않게 도통될 수 있습니다.  
> 이 회로에서 SB1은 Gate를 GND로 강하게 당겨 Vgs를 충분히 음수로 만드는 역할을 합니다.  
> SB1이 열린 상태에서는 R28이 Gate를 U5V로 당겨 Vgs ≈ 0에 가깝게 유지, **채널 차단**이 기본값입니다.

---

### SB1 단락 — ON

```
U5V ──[R28: 4.7KΩ]──┬── Gate (G)
                    │
                   [SB1 단락]
                    │
                   GND

Source (S) = VIN (예: 7V)
Gate   (G) = 0V (GND로 강제)

Vgs = 0V − 7V = −7V  →  Vth 초과 → 채널 ON
```

채널이 형성되면 `Drain → Source` 방향으로 전류가 흘러 **VIN이 LD1117로 공급**됩니다.

---

### R28의 역할

R28(4.7KΩ)은 두 가지 역할을 동시에 수행합니다.

1. **풀업 저항**: SB1이 열릴 때 Gate를 U5V로 당겨 FET를 OFF 상태로 유지
2. **전류 제한 저항**: SB1이 GND에 연결될 때 U5V → GND 간 단락 전류를 제한

```
SB1 단락 시 R28을 통한 전류: I = U5V / R28 = 5V / 4.7KΩ ≈ 1.06mA  (안전 범위)
```

---

## 전원 경로 다이어그램

```
                    SB1
                 ┌──○ ○──┐
U5V ─[R28:4.7K]─┤        ├─ GND
                 └── G ───┘
                     │
         VIN ────── S│
         (Source)    │T2 (P-ch)
                     │D
                     ├─── → LD1117S50TR ─→ 5V
                                              │
                              USB 5V ─[D4]──┤ JP5
                                              │
                                          LD39050PU33R
                                              │
                                           3.3V = VDD
                                           (STM32F103)
```

---

## 설계 의도 요약

| 설계 요소 | 목적 |
|-----------|------|
| T2 MOSFET + SB1 | VIN 경로의 물리적 차단/활성화. 기본값 OFF로 안전한 개발 환경 제공 |
| JP5 점퍼 | USB ↔ 외부 VIN 전원 경로 이중화 및 유연한 선택 |
| D4 쇼트키 다이오드 | USB와 VIN 경로 합류 시 역전류 방지 (OR-ing 다이오드) |
| 2단 LDO 구조 | 효율보다 안정성 우선. 노이즈 리젝션 향상 |
| C18 + C19 병렬 | 저주파(1μF) + 고주파(100nF) 노이즈 동시 제거 |
| PG 핀 | 3.3V 안정화 확인 후 MCU에 Ready 신호 제공 |

---

## 주요 부품 스펙 요약

| 부품 | 파트 넘버 | 역할 | 주요 스펙 |
|------|-----------|------|-----------|
| T2 | STS7PF30L | P-ch MOSFET 스위치 | Vds: −30V, Id: −7A, Vth: −1~−2.5V |
| U3 | LD1117S50TR | 1차 LDO | Vin max: 15V, Vout: 5V, Iout: 800mA |
| U4 | LD39050PU33R | 2차 LDO | Vin: 2.5~5.5V, Vout: 3.3V, Iout: 500mA |
| D4 | STPS2L30A | 쇼트키 다이오드 | Vf: ~0.3V, If: 2A, Vr: 30V |
| R28 | — | 풀업 / 전류제한 | 4.7KΩ |
| R32 | — | EN 핀 풀업 | 1KΩ |

---

> **참고 자료**
> - [LD1117 Datasheet (ST)](https://www.st.com/resource/en/datasheet/ld1117.pdf)
> - [LD39050 Datasheet (ST)](https://www.st.com/resource/en/datasheet/ld39050.pdf)
> - [STS7PF30L Datasheet (ST)](https://www.st.com/resource/en/datasheet/sts7pf30l.pdf)
> - [NUCLEO-F103RB User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
