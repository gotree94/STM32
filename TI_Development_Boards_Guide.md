# 🔴 Texas Instruments 개발 보드 종합 가이드 (CPU Core별)

> TI LaunchPad 및 평가 보드 Core별 완벽 정리

[![Texas Instruments](https://img.shields.io/badge/Texas_Instruments-CC0000?style=flat&logo=ti&logoColor=white)]()
[![ARM](https://img.shields.io/badge/ARM_Cortex-0091BD?style=flat&logo=arm&logoColor=white)]()

---

## 📑 목차

- [Cortex-M0+](#cortex-m0)
- [Cortex-M3](#cortex-m3)
- [Cortex-M4/M4F](#cortex-m4m4f)
- [Cortex-M33](#cortex-m33)
- [Cortex-R4/R5 (Real-Time)](#cortex-r4r5-real-time-core)
- [C2000 DSP](#c2000-dsp-실시간-제어)
- [MSP430 (16-bit)](#msp430-16-bit-ultra-low-power)
- [Sitara (Application Processor)](#sitara-application-processor)
- [무선 연결 요약](#ti-무선-연결-요약)
- [요약 비교표](#ti-요약-비교표)
- [용도별 추천](#ti-추천-보드-용도별)
- [STM32 vs TI 비교](#stm32-vs-ti-비교)

---

## Cortex-M0+

### LaunchPad 보드

| 보드명 | MCU | Core | Flash | RAM | 특징 |
|--------|-----|------|-------|-----|------|
| LP-MSPM0L1306 | MSPM0L1306 | M0+ | 64KB | 4KB | 저가 입문용 |
| LP-MSPM0L2228 | MSPM0L2228 | M0+ | 128KB | 16KB | LCD 컨트롤러 |
| **LP-MSPM0G3507** | **MSPM0G3507** | **M0+** | **128KB** | **32KB** | **⭐ 고성능 아날로그** |
| LP-MSPM0C1104 | MSPM0C1104 | M0+ | 16KB | 1KB | 초저가 |

### 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| MSPM0L1306-Q1EVM | MSPM0L1306-Q1 | Automotive Grade |
| MSPM0G3507-Q1EVM | MSPM0G3507-Q1 | Automotive Grade |

---

## Cortex-M3

### LaunchPad 보드

| 보드명 | MCU | Flash | RAM | 특징 |
|--------|-----|-------|-----|------|
| **EK-TM4C123GXL** | **TM4C123GH6PM** | **256KB** | **32KB** | **⭐ 가장 인기, 80MHz** |
| EK-TM4C1294XL | TM4C1294NCPDT | 1MB | 256KB | 이더넷, 120MHz |
| EK-TM4C129EXL | TM4C129ENCPDT | 1MB | 256KB | 이더넷, 암호화 |

### 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| DK-TM4C123G | TM4C123GH6PGE | LCD, USB OTG |
| DK-TM4C129X | TM4C129XNCZAD | LCD, 이더넷, USB |

---

## Cortex-M4/M4F

### LaunchPad 보드

| 보드명 | MCU | Flash | RAM | 특징 |
|--------|-----|-------|-----|------|
| LAUNCHXL-CC1310 | CC1310 | 128KB | 20KB | Sub-1GHz 무선 |
| LAUNCHXL-CC1312R1 | CC1312R1 | 352KB | 80KB | Sub-1GHz, 저전력 |
| LAUNCHXL-CC1350 | CC1350 | 128KB | 20KB | 듀얼밴드 (Sub-1G + BLE) |
| **LAUNCHXL-CC1352R1** | **CC1352R1** | **352KB** | **80KB** | **듀얼밴드, 저전력** |
| LAUNCHXL-CC1352P | CC1352P | 352KB | 80KB | PA 내장, 장거리 |
| LAUNCHXL-CC2640R2 | CC2640R2F | 128KB | 20KB | BLE 5.0 |
| LAUNCHXL-CC2650 | CC2650 | 128KB | 20KB | BLE, Zigbee, 6LoWPAN |
| **LAUNCHXL-CC2652R1** | **CC2652R1** | **352KB** | **80KB** | **⭐ 멀티프로토콜** |
| LAUNCHXL-CC2652RB | CC2652RB | 352KB | 80KB | 크리스탈리스 |
| LP-CC2652R7 | CC2652R7 | 704KB | 144KB | 확장 메모리 |
| LP-CC2651P3 | CC2651P3 | 352KB | 80KB | PA 내장, BLE |
| **LP-CC1352P7** | **CC1352P7** | **704KB** | **144KB** | **⭐ 확장, PA 내장** |
| MSP-EXP432P401R | MSP432P401R | 256KB | 64KB | 저전력, 48MHz |
| MSP-EXP432P4111 | MSP432P4111 | 2MB | 256KB | LCD, 고성능 |

### 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| CC1310DK | CC1310 | Sub-1GHz 개발 키트 |
| CC1350STK | CC1350 | SensorTag (다양한 센서) |
| CC2650STK | CC2650 | SensorTag BLE |
| CC26X2R1-LAUNCHXL | CC2652R1 | 멀티프로토콜 |
| TIDM-LOWCOST-CANFD | MSPM0G3507 | CAN-FD 레퍼런스 |

---

## Cortex-M33

### LaunchPad 보드

| 보드명 | MCU | Flash | RAM | 특징 |
|--------|-----|-------|-----|------|
| **LP-CC2340R5** | **CC2340R5** | **512KB** | **36KB** | **⭐ BLE 5.3, 저전력, 최신** |
| LP-CC2340R53 | CC2340R53 | 512KB | 36KB | BLE 5.3, 확장 |
| LP-EM-CC1354P10 | CC1354P10 | 1MB | 256KB | Sub-1GHz + BLE, PA |
| LP-CC2745R10-Q1 | CC2745R10-Q1 | 1MB | 256KB | Automotive BLE |
| LP-AM2434 | AM2434 | - | - | 듀얼 M4F + M33 PRU |

### 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| CC2340R5-Q1EVM | CC2340R5-Q1 | Automotive Grade |
| LP-EM-CC2340R5 | CC2340R5 | BoosterPack 확장 |

---

## Cortex-R4/R5 (Real-Time Core)

### LaunchPad 보드

| 보드명 | MCU | Core | Flash | RAM | 특징 |
|--------|-----|------|-------|-----|------|
| **LAUNCHXL2-TMS57012** | **TMS570LS1227** | **R4F 듀얼** | **1.25MB** | **192KB** | **⭐ 안전 (Safety)** |
| LAUNCHXL2-RM46 | RM46L852 | R4F | 1.25MB | 192KB | 산업용 안전 |
| **LAUNCHXL2-RM57L** | **RM57L843** | **R5F 듀얼** | **3MB** | **512KB** | **⭐ 고성능 안전** |

### 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| TMDX570LS31HDK | TMS570LS3137 | Hercules Safety MCU |
| TMDXRM48HDK | RM48L952 | 고성능 Safety |

---

## C2000 DSP (실시간 제어)

### LaunchPad 보드

| 보드명 | MCU | Core | Flash | RAM | 특징 |
|--------|-----|------|-------|-----|------|
| LAUNCHXL-F28027F | TMS320F28027F | C28x | 64KB | 12KB | 입문용 |
| LAUNCHXL-F28069M | TMS320F28069M | C28x | 256KB | 100KB | 모터 제어 |
| **LAUNCHXL-F28379D** | **TMS320F28379D** | **듀얼 C28x + CLA** | **1MB** | **204KB** | **⭐ 고성능 모터** |
| LAUNCHXL-F280025C | TMS320F280025C | C28x + CLA | 128KB | 52KB | 저가 실시간 |
| LAUNCHXL-F280039C | TMS320F280039C | C28x + CLA | 384KB | 164KB | CAN-FD |
| LAUNCHXL-F280049C | TMS320F280049C | C28x + CLA | 256KB | 100KB | InstaSPIN |
| LAUNCHXL-F2800137 | TMS320F2800137 | C28x + CLA | 256KB | 52KB | 신형 |
| LP-F280015 | TMS320F280015 | C28x | 128KB | 24KB | 저가 |

### 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| TMDSCNCD28379D | TMS320F28379D | controlCARD 모듈 |
| TMDSCNCD28388D | TMS320F28388D | 듀얼코어, 이더넷 |
| TMDSHVMTRINSPIN | TMS320F28069M | 모터 제어 키트 |
| TIDM-02010 | TMS320F280039C | Vienna 정류기 |

---

## MSP430 (16-bit Ultra-Low Power)

### LaunchPad 보드

| 보드명 | MCU | Flash | RAM | 특징 |
|--------|-----|-------|-----|------|
| **MSP-EXP430G2ET** | **MSP430G2xx** | **16KB** | **512B** | **⭐ 초입문, 저가** |
| MSP-EXP430FR2355 | MSP430FR2355 | 32KB FRAM | 4KB | FRAM, 저전력 |
| MSP-EXP430FR2433 | MSP430FR2433 | 16KB FRAM | 4KB | FRAM |
| MSP-EXP430FR4133 | MSP430FR4133 | 16KB FRAM | 2KB | LCD 컨트롤러 |
| MSP-EXP430FR5969 | MSP430FR5969 | 64KB FRAM | 2KB | FRAM 대용량 |
| **MSP-EXP430FR5994** | **MSP430FR5994** | **256KB FRAM** | **8KB** | **⭐ LEA (저전력 가속기)** |
| MSP-EXP430FR6989 | MSP430FR6989 | 128KB FRAM | 2KB | LCD, 저전력 |
| MSP-EXP430F5529LP | MSP430F5529 | 128KB | 8KB | USB, 고성능 |

### 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| EVM430-FR6043 | MSP430FR6043 | 초음파 센싱 |
| TIDM-ULTRASONIC-FLOW | MSP430FR6047 | 유량 측정 |

---

## Sitara (Application Processor)

### LaunchPad / 개발 보드

| 보드명 | MCU | Core | 특징 |
|--------|-----|------|------|
| LAUNCHXL-AM2434 | AM2434 | 듀얼 A53 + M4F | 산업용 이더넷 |
| SK-AM62 | AM625 | 쿼드 A53 + M4F | Linux, 범용 |
| **SK-AM62A** | **AM62A7** | **쿼드 A53 + M4F + AI** | **⭐ AI 가속, 비전** |
| SK-AM62B | AM625B | 쿼드 A53 + M4F | B variant |
| SK-AM64B | AM6442 | 듀얼 A53 + 듀얼 M4F + PRU | 산업용 통신 |
| SK-AM68 | AM6802 | 듀얼 A72 + 12x C7x | 고성능 비전 AI |
| SK-AM69 | AM6821 | 옥타 A72 + C7x | 최고성능 |
| BBONEAI-64 | AM5729 | 듀얼 A15 + DSP + PRU | BeagleBone AI-64 |

### 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| TMDS64EVM | AM6442 | AM64x 풀 평가 보드 |
| SK-TDA4VM | TDA4VM | 자율주행 AI |
| J721E-EVM | DRA829V | ADAS 고성능 |

---

## TI 무선 연결 요약

| 프로토콜 | LaunchPad | MCU | Core |
|----------|-----------|-----|------|
| **BLE 5.0** | LAUNCHXL-CC2640R2 | CC2640R2F | M4F |
| **BLE 5.3** | LP-CC2340R5 | CC2340R5 | **M33** |
| **Sub-1GHz** | LAUNCHXL-CC1312R1 | CC1312R1 | M4F |
| **듀얼밴드** | LP-CC1352P7 | CC1352P7 | M4F |
| **Zigbee/Thread** | LAUNCHXL-CC2652R1 | CC2652R1 | M4F |
| **Wi-Fi** | CC3220SF-LAUNCHXL | CC3220SF | M4 |
| **Wi-Fi 6** | LP-CC3235SF | CC3235SF | M4 |

---

## TI 요약 비교표

| Core | 주요 시리즈 | 최대 클럭 | 특징 | 대표 보드 |
|------|-------------|-----------|------|-----------|
| **M0+** | MSPM0 | 32-80MHz | 초저가, 저전력, 최신 | LP-MSPM0G3507 |
| **M3** | Tiva-C (TM4C) | 80-120MHz | 범용, 이더넷 | **EK-TM4C123GXL** |
| **M4F** | CC13xx/CC26xx, MSP432 | 48-80MHz | 무선, 저전력 | LAUNCHXL-CC2652R1 |
| **M33** | CC23xx/CC27xx | 48-96MHz | 최신 BLE, TrustZone | **LP-CC2340R5** |
| **R4/R5** | Hercules (TMS570, RM) | 160-300MHz | Safety Critical | LAUNCHXL2-RM57L |
| **C28x** | C2000 | 100-200MHz | 실시간 모터 제어 | **LAUNCHXL-F28379D** |
| **MSP430** | MSP430 | 8-25MHz | 16-bit 초저전력 | MSP-EXP430FR5994 |
| **A53/A72** | Sitara AM6x | 1-2GHz | Linux, AI, 고성능 | SK-AM62A |

---

## TI 추천 보드 (용도별)

| 용도 | 추천 LaunchPad | 특징 |
|------|----------------|------|
| **입문/교육** | EK-TM4C123GXL | 풍부한 자료, 저가 |
| **초저가** | LP-MSPM0C1104, MSP-EXP430G2ET | $5 이하 |
| **초저전력 IoT** | MSP-EXP430FR5994 | FRAM, nA급 대기전류 |
| **BLE** | LP-CC2340R5 | 최신 BLE 5.3, 저전력 |
| **Sub-1GHz/LoRa** | LP-CC1352P7 | 장거리, PA 내장 |
| **멀티프로토콜** | LAUNCHXL-CC2652R1 | BLE + Zigbee + Thread |
| **Wi-Fi** | CC3220SF-LAUNCHXL | 내장 Wi-Fi |
| **모터 제어** | LAUNCHXL-F28379D | 듀얼코어 DSP |
| **산업용 Safety** | LAUNCHXL2-RM57L | ISO 26262, IEC 61508 |
| **Linux/AI** | SK-AM62A | AI 가속기, 비전 |
| **이더넷** | EK-TM4C1294XL | 내장 MAC/PHY |

---

## STM32 vs TI 비교

| 항목 | STM32 | TI |
|------|-------|-----|
| **Core 범위** | M0 ~ M33, M7 | M0+ ~ M33, R4/R5, C28x, A53/A72 |
| **강점** | 범용 MCU, 다양한 선택지 | 무선(CC), 모터(C2000), Safety(Hercules) |
| **무선** | STM32WB/WL (제한적) | CC13xx/CC26xx (풍부) |
| **저전력** | STM32L 시리즈 | MSP430, CC23xx |
| **실시간 제어** | STM32G4 (HRTIM) | C2000 (전문) |
| **Safety** | STM32L5 (TrustZone) | Hercules (전문 Safety) |
| **에코시스템** | CubeIDE, HAL, 풍부한 커뮤니티 | CCS, SDK, 산업 특화 |

---

## 📚 참고 자료

- [TI LaunchPad Development Kits](https://www.ti.com/design-resources/embedded-development/hardware-kits-boards.html)
- [Code Composer Studio](https://www.ti.com/tool/CCSTUDIO)
- [TI SimpleLink SDK](https://www.ti.com/tool/SIMPLELINK-CC13XX-CC26XX-SDK)
- [C2000 MotorControl SDK](https://www.ti.com/tool/C2000WARE-MOTORCONTROL-SDK)
- [MSP430 SDK](https://www.ti.com/tool/MSP430-SDK)

---

> 📅 Last Updated: 2025-02
