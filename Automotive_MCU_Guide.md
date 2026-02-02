# 🚗 차량용(Automotive) MCU 및 개발 보드 종합 가이드

> 차량용 마이크로컨트롤러(MCU) 제조사별, 분야별 종합 정리 + STM32/TI 개발 보드 완벽 가이드

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Automotive](https://img.shields.io/badge/Automotive-AEC--Q100-blue.svg)]()
[![Safety](https://img.shields.io/badge/Safety-ISO%2026262-green.svg)]()

---

## 📑 목차

- [개요](#-개요)
- [STM32 개발 보드 (Core별)](#-stm32-개발-보드-core별)
- [Texas Instruments 개발 보드 (Core별)](#-texas-instruments-개발-보드-core별)
- [차량용 MCU 제조사별](#-차량용-mcu-제조사별)
- [분야별 정리](#-분야별-정리)
- [안전등급 (ASIL)](#-안전등급-asil-지원-현황)
- [통신 인터페이스](#-통신-인터페이스-비교)
- [개발 환경](#-개발-환경)
- [추천 가이드](#-추천-가이드)

---

## 📋 개요

본 문서는 임베디드 시스템 개발자를 위한 MCU 및 개발 보드 종합 가이드입니다.

### 문서 구성

| 섹션 | 내용 |
|------|------|
| **STM32 개발 보드** | Nucleo, Discovery 보드 Core별 완벽 정리 |
| **TI 개발 보드** | LaunchPad, 평가 보드 Core별 완벽 정리 |
| **차량용 MCU** | Automotive Grade MCU 제조사별 정리 |

### 차량용 MCU 요구사항

| 항목 | 요구사항 |
|------|----------|
| **온도 범위** | -40°C ~ +125°C (Grade 1) 또는 +150°C (Grade 0) |
| **품질 인증** | AEC-Q100 (IC), AEC-Q200 (수동소자) |
| **기능 안전** | ISO 26262 (ASIL-A ~ ASIL-D) |
| **수명** | 15년 이상 공급 보장 |
| **신뢰성** | Zero Defect 목표, FIT rate 관리 |

---

# 🔵 STM32 개발 보드 (Core별)

## Cortex-M0/M0+

### Nucleo 보드

| 보드명 | MCU | Core | Flash | RAM | 특징 |
|--------|-----|------|-------|-----|------|
| NUCLEO-F030R8 | STM32F030R8 | M0 | 64KB | 8KB | 입문용 |
| NUCLEO-F031K6 | STM32F031K6 | M0 | 32KB | 4KB | Nano |
| NUCLEO-F042K6 | STM32F042K6 | M0 | 32KB | 6KB | USB, CAN |
| NUCLEO-F070RB | STM32F070RB | M0 | 128KB | 16KB | USB |
| NUCLEO-F072RB | STM32F072RB | M0 | 128KB | 16KB | USB, CAN, Touch |
| NUCLEO-F091RC | STM32F091RC | M0 | 256KB | 32KB | 최대 사양 |
| NUCLEO-G031K8 | STM32G031K8 | M0+ | 64KB | 8KB | 저가 |
| NUCLEO-G041K8 | STM32G041K8 | M0+ | 64KB | 8KB | AES |
| NUCLEO-G070RB | STM32G070RB | M0+ | 128KB | 36KB | |
| NUCLEO-G071RB | STM32G071RB | M0+ | 128KB | 36KB | ⭐ 추천 |
| NUCLEO-G0B1RE | STM32G0B1RE | M0+ | 512KB | 144KB | 대용량 |
| NUCLEO-L010RB | STM32L010RB | M0+ | 128KB | 20KB | 저전력 |
| NUCLEO-L011K4 | STM32L011K4 | M0+ | 16KB | 2KB | 초저전력 |
| NUCLEO-L031K6 | STM32L031K6 | M0+ | 32KB | 8KB | |
| NUCLEO-L053R8 | STM32L053R8 | M0+ | 64KB | 8KB | LCD |
| NUCLEO-L073RZ | STM32L073RZ | M0+ | 192KB | 20KB | USB, LCD |
| NUCLEO-C031C6 | STM32C031C6 | M0+ | 32KB | 12KB | 최저가 |

### Discovery 보드

| 보드명 | MCU | Core | 특징 |
|--------|-----|------|------|
| STM32F0DISCOVERY | STM32F051R8 | M0 | 기본 학습용 |
| STM32F072B-DISCO | STM32F072RB | M0 | USB, Touch sensing |
| STM32L0538-DISCO | STM32L053C8 | M0+ | E-paper 디스플레이 |
| STM32L073Z-EVAL | STM32L073VZ | M0+ | 평가 보드 |
| B-L072Z-LRWAN1 | STM32L072CZ | M0+ | ⭐ LoRa 무선 |

---

## Cortex-M3

### Nucleo 보드

| 보드명 | MCU | Flash | RAM | 특징 |
|--------|-----|-------|-----|------|
| **NUCLEO-F103RB** | **STM32F103RB** | **128KB** | **20KB** | **⭐ 가장 대중적, 입문 필수** |
| NUCLEO-F207ZG | STM32F207ZG | 1MB | 128KB | 이더넷, 144핀 |
| NUCLEO-L152RE | STM32L152RE | 512KB | 80KB | 저전력, LCD |
| NUCLEO-L100RB | STM32L100RB | 128KB | 16KB | 초저전력 |

### Discovery 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| STM32VLDISCOVERY | STM32F100RB | Value Line 입문용 |
| STM32F100C-EVAL | STM32F100VB | 평가 보드 |
| STM32L-DISCOVERY | STM32L152RB | LCD 세그먼트, 저전력 |
| 32L152CDISCOVERY | STM32L152RC | LCD 세그먼트 |

---

## Cortex-M4/M4F

### Nucleo 보드

| 보드명 | MCU | Flash | RAM | 특징 |
|--------|-----|-------|-----|------|
| NUCLEO-F302R8 | STM32F302R8 | 64KB | 16KB | Motor control |
| NUCLEO-F303K8 | STM32F303K8 | 64KB | 12KB | Nano 폼팩터 |
| NUCLEO-F303RE | STM32F303RE | 512KB | 64KB | 고성능 아날로그 |
| NUCLEO-F303ZE | STM32F303ZE | 512KB | 64KB | 144핀 |
| NUCLEO-F334R8 | STM32F334R8 | 64KB | 12KB | HRTIM |
| **NUCLEO-F401RE** | **STM32F401RE** | **512KB** | **96KB** | **⭐ 가장 대중적** |
| NUCLEO-F410RB | STM32F410RB | 128KB | 32KB | |
| **NUCLEO-F411RE** | **STM32F411RE** | **512KB** | **128KB** | **⭐ 고성능 추천** |
| NUCLEO-F412ZG | STM32F412ZG | 1MB | 256KB | |
| NUCLEO-F413ZH | STM32F413ZH | 1.5MB | 320KB | |
| NUCLEO-F446RE | STM32F446RE | 512KB | 128KB | 180MHz |
| NUCLEO-F446ZE | STM32F446ZE | 512KB | 128KB | 144핀 |
| NUCLEO-G431KB | STM32G431KB | 128KB | 32KB | Nano |
| NUCLEO-G431RB | STM32G431RB | 128KB | 32KB | |
| NUCLEO-G474RE | STM32G474RE | 512KB | 128KB | 고성능 아날로그 |
| NUCLEO-G491RE | STM32G491RE | 512KB | 112KB | |
| NUCLEO-L412KB | STM32L412KB | 128KB | 40KB | Ultra-low power |
| NUCLEO-L432KC | STM32L432KC | 256KB | 64KB | Nano |
| NUCLEO-L433RC-P | STM32L433RC | 256KB | 64KB | SMPS |
| NUCLEO-L452RE | STM32L452RE | 512KB | 160KB | |
| **NUCLEO-L476RG** | **STM32L476RG** | **1MB** | **128KB** | **⭐ 저전력 고성능** |
| NUCLEO-L496ZG | STM32L496ZG | 1MB | 320KB | |
| NUCLEO-L4A6ZG | STM32L4A6ZG | 1MB | 320KB | Crypto |
| NUCLEO-L4R5ZI | STM32L4R5ZI | 2MB | 640KB | |
| NUCLEO-L4P5ZG | STM32L4P5ZG | 1MB | 320KB | |
| NUCLEO-WB15CC | STM32WB15CC | 320KB | 48KB | BLE |
| **NUCLEO-WB55RG** | **STM32WB55RG** | **1MB** | **256KB** | **⭐ BLE, Zigbee** |
| NUCLEO-WL55JC | STM32WL55JC | 256KB | 64KB | ⭐ LoRa, Sub-GHz |

### Discovery 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| STM32F3DISCOVERY | STM32F303VC | 자이로스코프, 가속도계 |
| STM32F401C-DISCO | STM32F401VC | 오디오, 가속도계 |
| **STM32F407G-DISC1** | **STM32F407VG** | **⭐ 가장 인기, 오디오, 가속도계** |
| STM32F411E-DISCO | STM32F411VE | 자이로, 가속도계, 마그네토 |
| STM32F412G-DISCO | STM32F412ZG | TFT LCD, 오디오 |
| STM32F413H-DISCO | STM32F413ZH | LCD, 오디오, MEMS |
| **B-L475E-IOT01A** | **STM32L475VG** | **⭐ IoT Node, WiFi, BLE, 센서** |
| STM32L476G-DISCO | STM32L476VG | LCD, 오디오, 쿼드SPI |
| STM32L496G-DISCO | STM32L496AG | TFT LCD, 오디오 |
| STM32L4R9I-DISCO | STM32L4R9AI | AMOLED, DSI |
| B-L4S5I-IOT01A | STM32L4S5VI | IoT, WiFi, BLE |
| STM32G474E-EVAL | STM32G474QE | 평가 보드 |
| B-G474E-DPOW1 | STM32G474RE | Digital Power |
| P-NUCLEO-WB55 | STM32WB55RG | BLE 개발팩 |
| STM32WB5MM-DK | STM32WB5MMG | BLE 모듈 |

---

## Cortex-M7

### Nucleo 보드

| 보드명 | MCU | Flash | RAM | 특징 |
|--------|-----|-------|-----|------|
| NUCLEO-F722ZE | STM32F722ZE | 512KB | 256KB | |
| NUCLEO-F746ZG | STM32F746ZG | 1MB | 320KB | |
| NUCLEO-F756ZG | STM32F756ZG | 1MB | 320KB | Crypto |
| NUCLEO-F767ZI | STM32F767ZI | 2MB | 512KB | 고성능 |
| NUCLEO-H723ZG | STM32H723ZG | 1MB | 564KB | 550MHz |
| **NUCLEO-H743ZI** | **STM32H743ZI** | **2MB** | **1MB** | **⭐ 480MHz, 추천** |
| NUCLEO-H743ZI2 | STM32H743ZI | 2MB | 1MB | Rev.2 |
| **NUCLEO-H745ZI-Q** | **STM32H745ZI** | **2MB** | **1MB** | **⭐ 듀얼코어 M7+M4** |
| NUCLEO-H753ZI | STM32H753ZI | 2MB | 1MB | Crypto |
| NUCLEO-H755ZI-Q | STM32H755ZI | 2MB | 1MB | 듀얼코어 + Crypto |
| NUCLEO-H7A3ZI-Q | STM32H7A3ZI | 2MB | 1.4MB | |
| NUCLEO-H7S3L8 | STM32H7S3L8 | 64KB | 620KB | 외부 메모리 |

### Discovery 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| **STM32F746G-DISCO** | **STM32F746NG** | **⭐ 4.3" LCD, 터치, 이더넷, USB** |
| STM32F769I-DISCO | STM32F769NI | 4" LCD, DSI, 오디오 |
| STM32F7508-DK | STM32F750N8 | LCD, 가성비 |
| STM32H735G-DK | STM32H735IG | 4" LCD, 이더넷, OSPI |
| STM32H743I-EVAL | STM32H743XI | 평가 보드 |
| STM32H745I-DISCO | STM32H745XI | 듀얼코어, LCD |
| **STM32H747I-DISCO** | **STM32H747XI** | **⭐ 듀얼코어, 4" LCD, 카메라** |
| STM32H750B-DK | STM32H750XB | LCD, 외부 플래시 |
| STM32H7B3I-DK | STM32H7B3LI | 4.3" LCD, OSPI |
| STM32H7B3I-EVAL | STM32H7B3II | 평가 보드 |
| STM32H7S78-DK | STM32H7S7L8 | 최신, 외부 메모리 |

---

## Cortex-M33

### Nucleo 보드

| 보드명 | MCU | Flash | RAM | 특징 |
|--------|-----|-------|-----|------|
| NUCLEO-L552ZE-Q | STM32L552ZE | 512KB | 256KB | TrustZone |
| NUCLEO-U545RE-Q | STM32U545RE | 512KB | 274KB | Ultra-low power |
| **NUCLEO-U575ZI-Q** | **STM32U575ZI** | **2MB** | **786KB** | **⭐ 추천** |
| NUCLEO-U5A5ZJ-Q | STM32U5A5ZJ | 4MB | 2.5MB | |
| NUCLEO-H503RB | STM32H503RB | 128KB | 32KB | 250MHz |
| NUCLEO-H533RE | STM32H533RE | 512KB | 272KB | |
| NUCLEO-H563ZI | STM32H563ZI | 2MB | 640KB | 250MHz |
| NUCLEO-WBA52CG | STM32WBA52CG | 1MB | 96KB | BLE 5.4 |
| NUCLEO-WBA55CG | STM32WBA55CG | 1MB | 128KB | BLE 5.4 |

### Discovery 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| STM32L562E-DK | STM32L562QE | TrustZone, LCD, 터치 |
| **B-U585I-IOT02A** | **STM32U585AI** | **⭐ IoT, WiFi, BLE, 센서** |
| STM32U5A9J-DK | STM32U5A9NJ | LCD, 터치, OSPI |
| STM32H573I-DK | STM32H573II | LCD, 이더넷, USB-C |
| STM32WBA55G-DK1 | STM32WBA55CG | BLE 5.4, LCD |

---

## Cortex-M33 + Cortex-M0+ (이종 듀얼코어)

| 보드명 | MCU | 특징 |
|--------|-----|------|
| NUCLEO-WL33CC | STM32WL33CC | Sub-GHz 무선 |
| STM32N6570-DK | STM32N657X0 | Neural-ART 가속기, AI |

---

## STM32 요약 비교표

| Core | 주요 시리즈 | 최대 클럭 | 특징 | 대표 보드 |
|------|-------------|-----------|------|-----------|
| **M0/M0+** | F0, G0, L0, C0 | 48-64MHz | 저전력, 저가, 입문용 | NUCLEO-G071RB |
| **M3** | F1, F2, L1 | 72-120MHz | 범용, 레거시 | **NUCLEO-F103RB** |
| **M4F** | F3, F4, G4, L4, WB, WL | 80-180MHz | DSP, FPU, 범용 고성능 | **NUCLEO-F411RE** |
| **M7** | F7, H7 | 216-550MHz | 고성능, 그래픽, DSP | **NUCLEO-H743ZI** |
| **M33** | L5, U5, H5, WBA | 110-250MHz | TrustZone 보안, 최신 | NUCLEO-U575ZI-Q |

## STM32 추천 보드 (용도별)

| 용도 | 추천 Nucleo | 추천 Discovery |
|------|-------------|----------------|
| **입문/교육** | NUCLEO-F103RB, NUCLEO-F411RE | STM32F407G-DISC1 |
| **저전력 IoT** | NUCLEO-L476RG, NUCLEO-WB55RG | B-L475E-IOT01A |
| **고성능 그래픽** | NUCLEO-H743ZI | STM32F746G-DISCO, STM32H747I-DISCO |
| **모터 제어** | NUCLEO-G474RE, NUCLEO-F334R8 | B-G474E-DPOW1 |
| **무선 통신** | NUCLEO-WB55RG (BLE), NUCLEO-WL55JC (LoRa) | P-NUCLEO-WB55 |
| **보안 (TrustZone)** | NUCLEO-L552ZE-Q, NUCLEO-U575ZI-Q | STM32L562E-DK |

---

# 🔴 Texas Instruments 개발 보드 (Core별)

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
| LAUNCHXL2-TMS57012 | TMS570LS1227 | R4F 듀얼 | 1.25MB | 192KB | ⭐ 안전 (Safety) |
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

## TI 무선 연결 요약 (Wireless Connectivity)

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

# 🚗 차량용 MCU 제조사별

## 1. NXP Semiconductors

### S32K 시리즈 (Cortex-M 기반 범용)

#### S32K1 시리즈 (입문/범용)

| 모델 | Core | Flash | RAM | 클럭 | 특징 |
|------|------|-------|-----|------|------|
| S32K116 | M0+ | 128KB | 17KB | 48MHz | 저가 바디 제어 |
| S32K118 | M0+ | 256KB | 25KB | 48MHz | |
| S32K142 | M4F | 256KB | 32KB | 80MHz | CAN-FD |
| **S32K144** | **M4F** | **512KB** | **64KB** | **80MHz** | **⭐ 가장 대중적** |
| S32K146 | M4F | 1MB | 128KB | 112MHz | |
| S32K148 | M4F | 2MB | 256KB | 112MHz | 이더넷 |

#### S32K3 시리즈 (차세대, ASIL-D)

| 모델 | Core | Flash | RAM | 클럭 | 특징 |
|------|------|-------|-----|------|------|
| S32K311 | M7 | 1MB | 128KB | 120MHz | 단일코어 |
| S32K312 | M7 | 1MB | 192KB | 120MHz | |
| S32K314 | M7 | 2MB | 384KB | 160MHz | |
| S32K322 | M7 듀얼 | 2MB | 384KB | 160MHz | 락스텝 |
| S32K324 | M7 듀얼 | 2MB | 384KB | 160MHz | |
| S32K341 | M7 듀얼 | 4MB | 512KB | 160MHz | |
| S32K342 | M7 듀얼 | 4MB | 512KB | 172MHz | |
| **S32K344** | **M7 듀얼** | **4MB** | **512KB** | **160MHz** | **⭐ 고성능 ASIL-D** |
| S32K358 | M7 트리플 | 8MB | 1MB | 240MHz | 최고 사양 |

#### S32K 평가 보드

| 보드명 | MCU | 가격대 | 특징 |
|--------|-----|--------|------|
| S32K144EVB-Q100 | S32K144 | ~$50 | ⭐ 입문용, 저가 |
| S32K144-Q100 | S32K144 | ~$100 | 기본 평가 보드 |
| S32K344EVB-Q257 | S32K344 | ~$150 | ASIL-D, CAN-FD |
| S32K3X4EVB-Q257 | S32K3x4 | ~$200 | 확장 평가 보드 |

### S32G/S32R 시리즈 (고성능 게이트웨이/레이더)

| 시리즈 | Core | 특징 | 용도 |
|--------|------|------|------|
| **S32G2** | 4x A53 + 3x M7 + 2x M33 | 네트워크 가속기 | 차량 게이트웨이, V2X |
| S32G274A | 쿼드 A53 @1GHz | PCIe, 이더넷 | 중앙 게이트웨이 |
| **S32G3** | 8x A53 + 4x M7 | 2.5Gbps 이더넷 | 차세대 게이트웨이 |
| **S32R294** | 3x M7 + DSP | 레이더 신호처리 | ADAS 레이더 |
| **S32R45** | 4x A53 + 2x M7 | 고성능 레이더 | 4D 이미징 레이더 |

### MPC5xxx 시리즈 (Power Architecture)

| 시리즈 | Core | Flash | 특징 |
|--------|------|-------|------|
| MPC5602P | e200z0 | 256KB | 저가 파워트레인 |
| MPC5604B | e200z0 | 512KB | 바디 제어 |
| MPC5643L | 듀얼 e200z4 | 1MB | ASIL-D 락스텝 |
| MPC5644A | e200z4 | 2MB | 엔진 제어 |
| MPC5674F | e200z7 | 4MB | 고성능 파워트레인 |
| MPC5775K | 듀얼 e200z7 | 6MB | ASIL-D |
| **MPC5748G** | 3x e200 | 6MB | ⭐ 게이트웨이 |

---

## 2. Infineon Technologies

### AURIX TC3xx 시리즈 (TriCore)

#### Entry Level

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| TC32x | 1x TriCore 1.6E | 1MB | 96KB | 200MHz | 입문 |
| TC33x | 2x TriCore 1.6E | 2MB | 192KB | 200MHz | 바디 |

#### Mid Level

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| TC35x | 2x TriCore 1.6E | 3MB | 440KB | 300MHz | 바디, 섀시 |
| TC36x | 3x TriCore 1.6E | 4MB | 720KB | 300MHz | |

#### High Level

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| TC37x | 2x TriCore 1.6P | 6.5MB | 1.5MB | 300MHz | 고성능 |
| TC38x | 4x TriCore 1.6P | 10MB | 2.5MB | 300MHz | 파워트레인 |
| **TC39x** | **6x TriCore 1.6P** | **16MB** | **3.5MB** | **300MHz** | **⭐ 최고 성능** |

#### TC4xx (차세대)

| 모델 | Core | 특징 |
|------|------|------|
| TC4xx | TriCore 1.8 | PPU (병렬처리), 400MHz+ |
| TC49x | 8x TriCore | 최고 성능, 자율주행 |

#### AURIX 평가 보드

| 보드명 | MCU | 가격대 | 특징 |
|--------|-----|--------|------|
| KIT_A2G_TC334_LITE | TC334 | ~$50 | 입문용 |
| KIT_A2G_TC375_LITE | TC375 | ~$100 | ⭐ 중급, 추천 |
| KIT_A2G_TC377_TFT | TC377 | ~$200 | TFT 디스플레이 |
| KIT_A2G_TC397_TFT | TC397 | ~$300 | 고성능 |
| KIT_A2G_TC399_TFT | TC399 | ~$400 | 6코어 최고사양 |

### TRAVEO T2G 시리즈 (Cortex-M 기반)

| 시리즈 | Core | Flash | RAM | 용도 |
|--------|------|-------|-----|------|
| CYT2B7 | M4F | 1MB | 128KB | 클러스터 입문 |
| CYT2B9 | M4F | 2MB | 256KB | 바디, HMI |
| **CYT4BF** | **M7 듀얼** | **8MB** | **1MB** | **⭐ 클러스터, HMI** |
| CYT4DN | M7 쿼드 | 8MB | 2MB | 고급 클러스터 |

---

## 3. Renesas Electronics

### RH850 시리즈 (Renesas 자체 Core)

#### F1x 시리즈 (범용)

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| RH850/F1KM | G3KH | 4MB | 256KB | 240MHz | 범용 |
| RH850/F1KH | G3KH 듀얼 | 6MB | 352KB | 240MHz | 고성능 |

#### P1x 시리즈 (파워트레인)

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| RH850/P1M | G3MH | 4MB | 512KB | 320MHz | 인버터 |
| RH850/P1H | G3MH 듀얼 | 8MB | 1MB | 400MHz | ECU |
| **RH850/P1H-C** | **4x G3MH** | **16MB** | **2MB** | **400MHz** | **⭐ 고성능 ECU** |

#### U2x 시리즈 (차세대)

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| RH850/U2A | G4MH 듀얼 | 16MB | 3.8MB | 400MHz | 통합 ECU |
| **RH850/U2B** | **4x G4MH** | **32MB** | **11.5MB** | **400MHz** | **⭐ 최신 최고사양** |

### R-Car 시리즈 (Application Processor)

| 시리즈 | Core | 특징 | 용도 |
|--------|------|------|------|
| R-Car M3 | 2x A57 + 4x A53 | GPU, ISP | IVI 입문 |
| **R-Car H3** | **4x A57 + 4x A53** | **고성능 GPU** | **⭐ 프리미엄 IVI** |
| R-Car V3H | M3 기반 + CV | 컴퓨터 비전 | 서라운드 뷰 |
| **R-Car V4H** | **A76 + R52 + CV-Engine** | **60 TOPS** | **⭐ L3+ 자율주행** |
| R-Car S4 | 8x A55 + 2x R52 | 보안, TSN | 게이트웨이 |

---

## 4. STMicroelectronics

### Stellar 시리즈 (차량 전용)

| 시리즈 | Core | Flash | RAM | 용도 |
|--------|------|-------|-----|------|
| **Stellar-E** | 6x M7 락스텝 | 12MB | 2MB | 전동화(xEV) |
| **Stellar-G** | 6x M7 락스텝 | 16MB | 3MB | 게이트웨이 |
| **Stellar-P** | 8x M7 락스텝 | 20MB | 4MB | 파워트레인 |

### SPC5 시리즈 (Power Architecture)

| 시리즈 | Core | Flash | 용도 |
|--------|------|-------|------|
| SPC560B | e200z0 | 256~768KB | 저가 바디 |
| SPC560P | e200z0 | 256~1MB | 파워트레인 입문 |
| **SPC58xG** | 3x e200z4 | 6MB | 게이트웨이 |
| **SPC58xN** | 3x e200z4 | 8MB | 섀시 |

### STM32 Automotive Grade

| 시리즈 | Core | 등급 | 용도 |
|--------|------|------|------|
| STM32F1x-A | M3 | AEC-Q100 | 레거시 |
| STM32F4x-A | M4F | AEC-Q100 | 범용 |
| STM32G4x-A | M4F | AEC-Q100 | 모터 |
| **STM32A0** | M0+ | AEC-Q100 | 저가 차량용 |
| **STM32A3** | M33 | AEC-Q100 | 차량용 범용 |

---

## 5. Texas Instruments

### Hercules 시리즈 (Safety MCU)

| 시리즈 | Core | Flash | RAM | 안전등급 | 용도 |
|--------|------|-------|-----|----------|------|
| TMS570LS0432 | R4F | 384KB | 32KB | ASIL-B | 입문 |
| **TMS570LS1227** | **R4F 듀얼** | **1.25MB** | **192KB** | **ASIL-D** | **범용** |
| TMS570LS3137 | R4F 듀얼 | 3MB | 256KB | ASIL-D | 고성능 |
| TMS570LC4357 | R5F 듀얼 | 4MB | 512KB | ASIL-D | 최고 성능 |
| **RM57L843** | **R5F 듀얼** | **3MB** | **512KB** | **ASIL-D** | **고성능** |

### Jacinto 시리즈 (ADAS/IVI)

| 시리즈 | Core | AI 성능 | 용도 |
|--------|------|---------|------|
| **TDA4VM** | **2x A72 + C7x + 2x C66x** | **8 TOPS** | **⭐ 주력 ADAS** |
| TDA4AL | A72 + C7x | 2 TOPS | 저가 ADAS |
| DRA829 | 4x A72 + R5F | 고성능 GPU | IVI |

### C2000 Automotive

| 모델 | Core | 용도 |
|------|------|------|
| TMS320F28069-Q1 | C28x | HEV/EV 인버터 |
| **TMS320F28379D-Q1** | **듀얼 C28x** | **⭐ 고성능 드라이브** |

---

## 6. Microchip

### SAM Automotive 시리즈

| 시리즈 | Core | Flash | 용도 |
|--------|------|-------|------|
| SAMC21-A | M0+ | 256KB | CAN, 터치 |
| SAME5x-A | M4F | 1MB | 고성능 |
| SAMV71-A | M7 | 2MB | 고성능 |

### dsPIC / PIC32 Automotive

| 시리즈 | Core | 용도 |
|--------|------|------|
| dsPIC33CK-A | dsPIC | 모터 제어 |
| PIC32MK-A | MIPS | 모터, CAN-FD |

---

# 🎯 분야별 정리

## 파워트레인 (엔진/변속기 제어)

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **Infineon** | TC39x | 6x TriCore | ⭐ 최고 성능, ASIL-D |
| Renesas | RH850/P1H-C | 4x G3MH | 400MHz |
| NXP | MPC5775K | 듀얼 e200z7 | Power Architecture |
| ST | Stellar-P | 8x M7 | 차세대 |

## 전동화 (EV/HEV 인버터, BMS)

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **Infineon** | TC38x | 4x TriCore | ⭐ 인버터 전용 |
| ST | Stellar-E | 6x M7 | xEV 전용 |
| Renesas | RH850/C1M | G3M | 모터 제어 |
| TI | TMS320F28388D-Q1 | 듀얼 C28x | DSP 기반 |
| NXP | S32K344 | 듀얼 M7 | ASIL-D |

## 바디/컴포트 (도어, 시트, 조명)

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **NXP** | S32K144 | M4F | ⭐ 가성비, 대중적 |
| Infineon | TC33x | 2x TriCore | 저가 |
| Renesas | RH850/F1KM | G3KH | 범용 |

## 게이트웨이/도메인 컨트롤러

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **NXP** | S32G274A | 4x A53 + 3x M7 | ⭐ 중앙 게이트웨이 |
| Infineon | TC39x | 6x TriCore | 고성능 |
| Renesas | R-Car S4 | 8x A55 + R52 | 보안, TSN |
| ST | Stellar-G | 6x M7 | 도메인 컨트롤러 |

## ADAS (카메라, 레이더, 센서 퓨전)

| 제조사 | 추천 MCU | Core | AI 성능 | 용도 |
|--------|----------|------|---------|------|
| **TI** | TDA4VM | A72 + C7x | 8 TOPS | ⭐ 주력 ADAS |
| Renesas | R-Car V4H | A76 + CV | 60 TOPS | L3+ 자율주행 |
| NXP | S32R45 | A53 + M7 | - | 4D 레이더 |

## 클러스터/IVI (계기판, 인포테인먼트)

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **Renesas** | R-Car H3 | 4x A57 + 4x A53 | ⭐ 프리미엄 IVI |
| Infineon | TRAVEO T2G CYT4BF | 듀얼 M7 | 클러스터 |
| Qualcomm | SA8155P | Kryo | 안드로이드 IVI |

## Safety Critical (브레이크, 조향, 에어백)

| 제조사 | 추천 MCU | Core | 안전등급 |
|--------|----------|------|----------|
| **TI** | TMS570LS3137 | 듀얼 R4F 락스텝 | ⭐ ASIL-D |
| Infineon | TC39x | 6x TriCore 락스텝 | ASIL-D |
| Renesas | RH850/P1H-C | 4x G3MH | ASIL-D |
| NXP | S32K344 | 듀얼 M7 락스텝 | ASIL-D |

---

# 🛡️ 안전등급 (ASIL) 지원 현황

## ASIL 등급 설명

| 등급 | 위험도 | 적용 분야 | 대표 MCU |
|------|--------|-----------|----------|
| **ASIL-D** | 최고 | 브레이크, 조향, 에어백 | TC39x, TMS570, RH850/P1H, S32K344 |
| **ASIL-C** | 높음 | 램프, 와이퍼 | TC37x, S32K3 |
| **ASIL-B** | 중간 | ABS, ESC | TC33x, S32K144, RH850/F1K |
| **ASIL-A** | 낮음 | 리어램프 | TC32x |
| **QM** | 일반 | 인포테인먼트 | 일반 Automotive Grade |

## 락스텝(Lockstep) 지원 MCU

> 락스텝: 두 개의 동일한 코어가 같은 연산을 수행하고 결과를 비교하여 오류를 검출하는 방식

| 제조사 | MCU | 락스텝 방식 |
|--------|-----|-------------|
| TI | TMS570, RM4x, RM5x | ⭐ 완전 락스텝 |
| Infineon | TC3xx/TC4xx | 선택적 락스텝 |
| NXP | S32K3xx | 듀얼 M7 락스텝 |
| Renesas | RH850 P/E | 코어별 락스텝 |
| ST | Stellar | M7 락스텝 |

---

# 🔌 통신 인터페이스 비교

| 인터페이스 | 속도 | 용도 | 지원 MCU |
|------------|------|------|----------|
| **CAN 2.0** | 1 Mbps | 레거시 차량 네트워크 | 모든 차량용 MCU |
| **CAN-FD** | 5-8 Mbps | 고속 차량 네트워크 | S32K, TC3xx, RH850 |
| **LIN** | 20 Kbps | 저속 서브 네트워크 | 대부분 |
| **FlexRay** | 10 Mbps | 고속 Safety 네트워크 | TC3xx, MPC5xxx, RH850 |
| **100BASE-T1** | 100 Mbps | 차량용 이더넷 | S32G, TC39x, R-Car |
| **1000BASE-T1** | 1 Gbps | 고속 이더넷 | S32G, R-Car, TDA4 |
| **TSN** | 1+ Gbps | 시간 동기화 이더넷 | S32G3, R-Car S4 |
| **PCIe** | 수 Gbps | 프로세서 연결 | S32G, R-Car, TDA4 |

---

# 💻 개발 환경

| 제조사 | IDE | 컴파일러 | 디버거 |
|--------|-----|----------|--------|
| **NXP** | S32 Design Studio | GCC, GHS | P&E, Lauterbach |
| **Infineon** | AURIX Development Studio | Tasking, GHS, HighTec | DAP, Lauterbach |
| **Renesas** | e² studio, CS+ | CC-RH, GHS | E1/E2, Lauterbach |
| **ST** | SPC5 Studio, CubeIDE | GCC, GHS | ST-Link, Lauterbach |
| **TI** | Code Composer Studio | TI ARM, GCC | XDS, Lauterbach |

## AUTOSAR 지원

| 제조사 | MCAL 제공 | Classic Platform | Adaptive Platform |
|--------|-----------|------------------|-------------------|
| NXP | ✅ | ✅ | ✅ (S32G) |
| Infineon | ✅ | ✅ | ✅ |
| Renesas | ✅ | ✅ | ✅ (R-Car) |
| ST | ✅ | ✅ | ⚠️ 일부 |
| TI | ✅ | ✅ | ⚠️ 일부 |

---

# 🏆 추천 가이드

## 입문용 (학습/프로토타입)

| 용도 | 추천 보드 | 가격대 | 이유 |
|------|-----------|--------|------|
| **STM32 입문** | NUCLEO-F103RB | ~$15 | 가장 많은 자료 |
| **STM32 고성능** | NUCLEO-F411RE | ~$15 | M4F 입문 최적 |
| **TI 입문** | EK-TM4C123GXL | ~$15 | 풍부한 예제 |
| **TI 무선** | LP-CC2340R5 | ~$30 | BLE 5.3 최신 |
| **바디 제어** | NXP S32K144EVB | ~$50 | 차량용 입문 |
| **Safety MCU** | TI LAUNCHXL2-TMS57012 | ~$30 | 최저가 ASIL-D |
| **TriCore** | Infineon TC375 Lite | ~$100 | 업계 표준 |

## 프로덕션 (양산용)

| 분야 | 1순위 | 2순위 | 비고 |
|------|-------|-------|------|
| 파워트레인 | Infineon TC39x | Renesas RH850/U2B | 유럽 OEM 선호 |
| 전동화 | Infineon TC38x | ST Stellar-E | EV 전용 최적화 |
| 바디 | NXP S32K144 | Renesas RH850/F1K | 가성비 |
| 게이트웨이 | NXP S32G274A | Renesas R-Car S4 | 이더넷 성능 |
| ADAS | TI TDA4VM | Renesas R-Car V4H | AI 성능 |
| Safety | TI TMS570 | Infineon TC39x | 락스텝 |

## 지역별 선호도

| 지역 | 선호 제조사 | 대표 MCU |
|------|-------------|----------|
| 🇪🇺 유럽 | Infineon, NXP | TC3xx, S32K |
| 🇯🇵 일본 | Renesas | RH850, R-Car |
| 🇺🇸 북미 | TI, NXP | TMS570, S32K |
| 🇰🇷 한국 | Infineon, NXP, Renesas | 혼합 사용 |
| 🇨🇳 중국 | NXP, ST | S32K, SPC5 |

---

# 📚 참고 자료

## 공식 문서

### STM32
- [STM32 Nucleo Overview](https://www.st.com/en/evaluation-tools/stm32-nucleo-boards.html)
- [STM32 Discovery Kits](https://www.st.com/en/evaluation-tools/stm32-discovery-kits.html)
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)

### Texas Instruments
- [TI LaunchPad Development Kits](https://www.ti.com/design-resources/embedded-development/hardware-kits-boards.html)
- [Code Composer Studio](https://www.ti.com/tool/CCSTUDIO)
- [TI SimpleLink SDK](https://www.ti.com/tool/SIMPLELINK-CC13XX-CC26XX-SDK)

### Automotive
- [NXP S32K Documentation](https://www.nxp.com/products/processors-and-microcontrollers/s32-automotive-platform:S32)
- [Infineon AURIX Documentation](https://www.infineon.com/cms/en/product/microcontroller/32-bit-tricore-microcontroller/)
- [Renesas RH850 Documentation](https://www.renesas.com/products/microcontrollers-microprocessors/rh850)

## 표준 규격

- [ISO 26262 (기능 안전)](https://www.iso.org/standard/68383.html)
- [AEC-Q100 (IC 신뢰성)](http://www.aecouncil.com/AECDocuments.html)
- [AUTOSAR](https://www.autosar.org/)

---

# 📝 License

This document is licensed under the MIT License.

---

# 🤝 기여

오류 수정, 내용 추가, 최신 정보 업데이트 등의 기여를 환영합니다.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/update-mcu-info`)
3. Commit your changes (`git commit -m 'Add new MCU information'`)
4. Push to the branch (`git push origin feature/update-mcu-info`)
5. Open a Pull Request

---

> 📅 Last Updated: 2025-02  
> 📧 Contact: 문의사항이 있으시면 Issue를 생성해 주세요.
