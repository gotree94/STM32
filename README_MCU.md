# 🚗 차량용(Automotive) MCU 종합 가이드

> 차량용 마이크로컨트롤러(MCU) 제조사별, 분야별 종합 정리

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Automotive](https://img.shields.io/badge/Automotive-AEC--Q100-blue.svg)]()
[![Safety](https://img.shields.io/badge/Safety-ISO%2026262-green.svg)]()

---

## 📑 목차

- [개요](#-개요)
- [제조사별 MCU](#-제조사별-mcu)
  - [NXP Semiconductors](#1-nxp-semiconductors)
  - [Infineon Technologies](#2-infineon-technologies)
  - [Renesas Electronics](#3-renesas-electronics)
  - [STMicroelectronics](#4-stmicroelectronics)
  - [Texas Instruments](#5-texas-instruments)
  - [Microchip](#6-microchip)
- [분야별 정리](#-분야별-정리)
- [안전등급 (ASIL)](#-안전등급-asil-지원-현황)
- [통신 인터페이스](#-통신-인터페이스-비교)
- [개발 환경](#-개발-환경)
- [추천 가이드](#-추천-가이드)

---

## 📋 개요

차량용 MCU는 일반 산업용/상용 MCU와 달리 다음 요구사항을 충족해야 합니다:

| 항목 | 요구사항 |
|------|----------|
| **온도 범위** | -40°C ~ +125°C (Grade 1) 또는 +150°C (Grade 0) |
| **품질 인증** | AEC-Q100 (IC), AEC-Q200 (수동소자) |
| **기능 안전** | ISO 26262 (ASIL-A ~ ASIL-D) |
| **수명** | 15년 이상 공급 보장 |
| **신뢰성** | Zero Defect 목표, FIT rate 관리 |

---

## 🏭 제조사별 MCU

### 1. NXP Semiconductors

#### S32K 시리즈 (Cortex-M 기반 범용)

##### S32K1 시리즈 (입문/범용)

| 모델 | Core | Flash | RAM | 클럭 | 특징 |
|------|------|-------|-----|------|------|
| S32K116 | M0+ | 128KB | 17KB | 48MHz | 저가 바디 제어 |
| S32K118 | M0+ | 256KB | 25KB | 48MHz | |
| S32K142 | M4F | 256KB | 32KB | 80MHz | CAN-FD |
| **S32K144** | **M4F** | **512KB** | **64KB** | **80MHz** | **⭐ 가장 대중적** |
| S32K146 | M4F | 1MB | 128KB | 112MHz | |
| S32K148 | M4F | 2MB | 256KB | 112MHz | 이더넷 |

##### S32K3 시리즈 (차세대, ASIL-D)

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

##### S32K 평가 보드

| 보드명 | MCU | 가격대 | 특징 |
|--------|-----|--------|------|
| S32K144EVB-Q100 | S32K144 | ~$50 | ⭐ 입문용, 저가 |
| S32K144-Q100 | S32K144 | ~$100 | 기본 평가 보드 |
| S32K344EVB-Q257 | S32K344 | ~$150 | ASIL-D, CAN-FD |
| S32K3X4EVB-Q257 | S32K3x4 | ~$200 | 확장 평가 보드 |

#### S32G/S32R 시리즈 (고성능 게이트웨이/레이더)

| 시리즈 | Core | 특징 | 용도 |
|--------|------|------|------|
| **S32G2** | 4x A53 + 3x M7 + 2x M33 | 네트워크 가속기 | 차량 게이트웨이, V2X |
| S32G274A | 쿼드 A53 @1GHz | PCIe, 이더넷 | 중앙 게이트웨이 |
| **S32G3** | 8x A53 + 4x M7 | 2.5Gbps 이더넷 | 차세대 게이트웨이 |
| **S32R294** | 3x M7 + DSP | 레이더 신호처리 | ADAS 레이더 |
| **S32R45** | 4x A53 + 2x M7 | 고성능 레이더 | 4D 이미징 레이더 |

#### MPC5xxx 시리즈 (Power Architecture)

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

### 2. Infineon Technologies

#### AURIX TC3xx 시리즈 (TriCore)

##### Entry Level

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| TC32x | 1x TriCore 1.6E | 1MB | 96KB | 200MHz | 입문 |
| TC33x | 2x TriCore 1.6E | 2MB | 192KB | 200MHz | 바디 |

##### Mid Level

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| TC35x | 2x TriCore 1.6E | 3MB | 440KB | 300MHz | 바디, 섀시 |
| TC36x | 3x TriCore 1.6E | 4MB | 720KB | 300MHz | |

##### High Level

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| TC37x | 2x TriCore 1.6P | 6.5MB | 1.5MB | 300MHz | 고성능 |
| TC38x | 4x TriCore 1.6P | 10MB | 2.5MB | 300MHz | 파워트레인 |
| **TC39x** | **6x TriCore 1.6P** | **16MB** | **3.5MB** | **300MHz** | **⭐ 최고 성능** |

##### TC4xx (차세대)

| 모델 | Core | 특징 |
|------|------|------|
| TC4xx | TriCore 1.8 | PPU (병렬처리), 400MHz+ |
| TC49x | 8x TriCore | 최고 성능, 자율주행 |

##### AURIX 평가 보드

| 보드명 | MCU | 가격대 | 특징 |
|--------|-----|--------|------|
| KIT_A2G_TC334_LITE | TC334 | ~$50 | 입문용 |
| KIT_A2G_TC375_LITE | TC375 | ~$100 | ⭐ 중급, 추천 |
| KIT_A2G_TC377_TFT | TC377 | ~$200 | TFT 디스플레이 |
| KIT_A2G_TC397_TFT | TC397 | ~$300 | 고성능 |
| KIT_A2G_TC399_TFT | TC399 | ~$400 | 6코어 최고사양 |

#### TRAVEO T2G 시리즈 (Cortex-M 기반)

| 시리즈 | Core | Flash | RAM | 용도 |
|--------|------|-------|-----|------|
| CYT2B7 | M4F | 1MB | 128KB | 클러스터 입문 |
| CYT2B9 | M4F | 2MB | 256KB | 바디, HMI |
| **CYT4BF** | **M7 듀얼** | **8MB** | **1MB** | **⭐ 클러스터, HMI** |
| CYT4DN | M7 쿼드 | 8MB | 2MB | 고급 클러스터 |

---

### 3. Renesas Electronics

#### RH850 시리즈 (Renesas 자체 Core)

##### F1x 시리즈 (범용)

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| RH850/F1KM | G3KH | 4MB | 256KB | 240MHz | 범용 |
| RH850/F1KH | G3KH 듀얼 | 6MB | 352KB | 240MHz | 고성능 |

##### C1x 시리즈 (섀시)

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| RH850/C1M | G3M | 2MB | 128KB | 160MHz | 모터 제어 |

##### P1x 시리즈 (파워트레인)

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| RH850/P1M | G3MH | 4MB | 512KB | 320MHz | 인버터 |
| RH850/P1H | G3MH 듀얼 | 8MB | 1MB | 400MHz | ECU |
| **RH850/P1H-C** | **4x G3MH** | **16MB** | **2MB** | **400MHz** | **⭐ 고성능 ECU** |

##### U2x 시리즈 (차세대)

| 모델 | Core | Flash | RAM | 클럭 | 용도 |
|------|------|-------|-----|------|------|
| RH850/U2A | G4MH 듀얼 | 16MB | 3.8MB | 400MHz | 통합 ECU |
| **RH850/U2B** | **4x G4MH** | **32MB** | **11.5MB** | **400MHz** | **⭐ 최신 최고사양** |

##### RH850 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| Y-RH850-F1KM-S1 | RH850/F1KM | 스타터 키트 |
| Y-RH850-F1KH-S4 | RH850/F1KH | 고성능 |
| Y-RH850-U2A8-176 | RH850/U2A | 차세대 통합 |
| Y-RH850-U2B-EVK | RH850/U2B | 최신 |

#### RL78 Automotive 시리즈 (16-bit)

| 시리즈 | Flash | RAM | 용도 |
|--------|-------|-----|------|
| RL78/F12 | 16~64KB | 2~4KB | 센서, 액추에이터 |
| RL78/F13 | 48~256KB | 4~20KB | 바디 서브 |
| RL78/F14 | 128~512KB | 16~32KB | LIN 노드 |
| RL78/F15 | 256~512KB | 24~32KB | 모터 제어 |

#### R-Car 시리즈 (Application Processor)

| 시리즈 | Core | 특징 | 용도 |
|--------|------|------|------|
| R-Car M3 | 2x A57 + 4x A53 | GPU, ISP | IVI 입문 |
| **R-Car H3** | **4x A57 + 4x A53** | **고성능 GPU** | **⭐ 프리미엄 IVI** |
| R-Car V3H | M3 기반 + CV | 컴퓨터 비전 | 서라운드 뷰 |
| R-Car V3M | CV 가속기 | NCAP | 카메라 ADAS |
| **R-Car V4H** | **A76 + R52 + CV-Engine** | **60 TOPS** | **⭐ L3+ 자율주행** |
| R-Car S4 | 8x A55 + 2x R52 | 보안, TSN | 게이트웨이 |

---

### 4. STMicroelectronics

#### Stellar 시리즈 (차량 전용)

| 시리즈 | Core | Flash | RAM | 용도 |
|--------|------|-------|-----|------|
| **Stellar-E** | 6x M7 락스텝 | 12MB | 2MB | 전동화(xEV) |
| SR6G7x | 6x M7 | 12MB | 2MB | 인버터, BMS |
| **Stellar-G** | 6x M7 락스텝 | 16MB | 3MB | 게이트웨이 |
| SR6P7x | 6x M7 | 16MB | 3MB | 도메인 컨트롤러 |
| **Stellar-P** | 8x M7 락스텝 | 20MB | 4MB | 파워트레인 |

#### SPC5 시리즈 (Power Architecture)

| 시리즈 | Core | Flash | 용도 |
|--------|------|-------|------|
| SPC560B | e200z0 | 256~768KB | 저가 바디 |
| SPC560P | e200z0 | 256~1MB | 파워트레인 입문 |
| SPC564A | e200z4 | 1~2MB | 파워트레인 |
| **SPC58xG** | 3x e200z4 | 6MB | 게이트웨이 |
| **SPC58xN** | 3x e200z4 | 8MB | 섀시 |

#### STM32 Automotive Grade

| 시리즈 | Core | 등급 | 용도 |
|--------|------|------|------|
| STM32F1x-A | M3 | AEC-Q100 | 레거시 |
| STM32F4x-A | M4F | AEC-Q100 | 범용 |
| STM32G4x-A | M4F | AEC-Q100 | 모터 |
| STM32H5x-A | M33 | AEC-Q100 | 최신 |
| **STM32A0** | M0+ | AEC-Q100 | 저가 차량용 |
| **STM32A3** | M33 | AEC-Q100 | 차량용 범용 |

---

### 5. Texas Instruments

#### Hercules 시리즈 (Safety MCU)

| 시리즈 | Core | Flash | RAM | 안전등급 | 용도 |
|--------|------|-------|-----|----------|------|
| TMS570LS0432 | R4F | 384KB | 32KB | ASIL-B | 입문 |
| **TMS570LS1227** | **R4F 듀얼** | **1.25MB** | **192KB** | **ASIL-D** | **범용** |
| TMS570LS3137 | R4F 듀얼 | 3MB | 256KB | ASIL-D | 고성능 |
| TMS570LC4357 | R5F 듀얼 | 4MB | 512KB | ASIL-D | 최고 성능 |
| RM46L852 | R4F 듀얼 | 1.25MB | 192KB | ASIL-D | 산업용 |
| RM48L952 | R4F 듀얼 | 3MB | 256KB | ASIL-D | |
| **RM57L843** | **R5F 듀얼** | **3MB** | **512KB** | **ASIL-D** | **고성능** |

##### Hercules 평가 보드

| 보드명 | MCU | 가격대 | 특징 |
|--------|-----|--------|------|
| LAUNCHXL2-TMS57012 | TMS570LS1227 | ~$30 | ⭐ 입문용, 저가 |
| TMDX570LS31HDK | TMS570LS3137 | ~$300 | 풀 평가 |
| LAUNCHXL2-RM46 | RM46L852 | ~$30 | 산업/차량 |
| LAUNCHXL2-RM57L | RM57L843 | ~$50 | 고성능 |

#### Jacinto 시리즈 (ADAS/IVI)

| 시리즈 | Core | AI 성능 | 용도 |
|--------|------|---------|------|
| **TDA4VM** | **2x A72 + C7x + 2x C66x** | **8 TOPS** | **⭐ 주력 ADAS** |
| TDA4AL | A72 + C7x | 2 TOPS | 저가 ADAS |
| TDA4VL | A72 + C7x | 4 TOPS | 중급 |
| DRA821 | 2x A72 + 2x R5F | GPU | 게이트웨이 |
| DRA829 | 4x A72 + R5F | 고성능 GPU | IVI |

##### Jacinto 평가 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| SK-TDA4VM | TDA4VM | ⭐ ADAS 스타터 |
| J721E-EVM | TDA4VM | 풀 평가 |

#### C2000 Automotive

| 모델 | Core | 용도 |
|------|------|------|
| TMS320F28035-Q1 | C28x | 모터 제어 |
| TMS320F28069-Q1 | C28x | HEV/EV 인버터 |
| **TMS320F28379D-Q1** | **듀얼 C28x** | **⭐ 고성능 드라이브** |
| TMS320F28388D-Q1 | 듀얼 C28x | 이더넷, 고급 |

---

### 6. Microchip

#### SAM Automotive 시리즈

| 시리즈 | Core | Flash | 용도 |
|--------|------|-------|------|
| SAMC21-A | M0+ | 256KB | CAN, 터치 |
| SAME5x-A | M4F | 1MB | 고성능 |
| SAME70-A | M7 | 2MB | 이더넷, USB |
| SAMV71-A | M7 | 2MB | 고성능 |

#### dsPIC / PIC32 Automotive

| 시리즈 | Core | 용도 |
|--------|------|------|
| dsPIC33CK-A | dsPIC | 모터 제어 |
| dsPIC33CH-A | 듀얼 dsPIC | 고성능 |
| PIC32MK-A | MIPS | 모터, CAN-FD |
| PIC32MZ-A | MIPS | 그래픽, 이더넷 |

---

## 🎯 분야별 정리

### 파워트레인 (엔진/변속기 제어)

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **Infineon** | TC39x | 6x TriCore | ⭐ 최고 성능, ASIL-D |
| Renesas | RH850/P1H-C | 4x G3MH | 400MHz |
| NXP | MPC5775K | 듀얼 e200z7 | Power Architecture |
| ST | Stellar-P | 8x M7 | 차세대 |

### 전동화 (EV/HEV 인버터, BMS)

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **Infineon** | TC38x | 4x TriCore | ⭐ 인버터 전용 |
| ST | Stellar-E (SR6G7) | 6x M7 | xEV 전용 |
| Renesas | RH850/C1M | G3M | 모터 제어 |
| TI | TMS320F28388D-Q1 | 듀얼 C28x | DSP 기반 |
| NXP | S32K344 | 듀얼 M7 | ASIL-D |

### 바디/컴포트 (도어, 시트, 조명)

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **NXP** | S32K144 | M4F | ⭐ 가성비, 대중적 |
| Infineon | TC33x | 2x TriCore | 저가 |
| Renesas | RH850/F1KM | G3KH | 범용 |
| ST | SPC560B | e200z0 | 저가 |

### 게이트웨이/도메인 컨트롤러

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **NXP** | S32G274A | 4x A53 + 3x M7 | ⭐ 중앙 게이트웨이 |
| Infineon | TC39x | 6x TriCore | 고성능 |
| Renesas | R-Car S4 | 8x A55 + R52 | 보안, TSN |
| ST | Stellar-G | 6x M7 | 도메인 컨트롤러 |

### ADAS (카메라, 레이더, 센서 퓨전)

| 제조사 | 추천 MCU | Core | AI 성능 | 용도 |
|--------|----------|------|---------|------|
| **TI** | TDA4VM | A72 + C7x | 8 TOPS | ⭐ 주력 ADAS |
| Renesas | R-Car V4H | A76 + CV | 60 TOPS | L3+ 자율주행 |
| NXP | S32R45 | A53 + M7 | - | 4D 레이더 |
| Mobileye | EyeQ | 전용 | ~176 TOPS | 완전 자율주행 |

### 클러스터/IVI (계기판, 인포테인먼트)

| 제조사 | 추천 MCU | Core | 특징 |
|--------|----------|------|------|
| **Renesas** | R-Car H3 | 4x A57 + 4x A53 | ⭐ 프리미엄 IVI |
| Infineon | TRAVEO T2G CYT4BF | 듀얼 M7 | 클러스터 |
| NXP | i.MX 8QuadMax | 2x A72 + 4x A53 | 멀티 디스플레이 |
| Qualcomm | SA8155P | Kryo | 안드로이드 IVI |

### Safety Critical (브레이크, 조향, 에어백)

| 제조사 | 추천 MCU | Core | 안전등급 |
|--------|----------|------|----------|
| **TI** | TMS570LS3137 | 듀얼 R4F 락스텝 | ⭐ ASIL-D |
| Infineon | TC39x | 6x TriCore 락스텝 | ASIL-D |
| Renesas | RH850/P1H-C | 4x G3MH | ASIL-D |
| NXP | S32K344 | 듀얼 M7 락스텝 | ASIL-D |

---

## 🛡️ 안전등급 (ASIL) 지원 현황

### ASIL 등급 설명

| 등급 | 위험도 | 적용 분야 | 대표 MCU |
|------|--------|-----------|----------|
| **ASIL-D** | 최고 | 브레이크, 조향, 에어백 | TC39x, TMS570, RH850/P1H, S32K344 |
| **ASIL-C** | 높음 | 램프, 와이퍼 | TC37x, S32K3 |
| **ASIL-B** | 중간 | ABS, ESC | TC33x, S32K144, RH850/F1K |
| **ASIL-A** | 낮음 | 리어램프 | TC32x |
| **QM** | 일반 | 인포테인먼트 | 일반 Automotive Grade |

### 락스텝(Lockstep) 지원 MCU

> 락스텝: 두 개의 동일한 코어가 같은 연산을 수행하고 결과를 비교하여 오류를 검출하는 방식

| 제조사 | MCU | 락스텝 방식 |
|--------|-----|-------------|
| TI | TMS570, RM4x, RM5x | ⭐ 완전 락스텝 |
| Infineon | TC3xx/TC4xx | 선택적 락스텝 |
| NXP | S32K3xx | 듀얼 M7 락스텝 |
| Renesas | RH850 P/E | 코어별 락스텝 |
| ST | Stellar | M7 락스텝 |

---

## 🔌 통신 인터페이스 비교

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

## 💻 개발 환경

| 제조사 | IDE | 컴파일러 | 디버거 |
|--------|-----|----------|--------|
| **NXP** | S32 Design Studio | GCC, GHS | P&E, Lauterbach |
| **Infineon** | AURIX Development Studio | Tasking, GHS, HighTec | DAP, Lauterbach |
| **Renesas** | e² studio, CS+ | CC-RH, GHS | E1/E2, Lauterbach |
| **ST** | SPC5 Studio, CubeIDE | GCC, GHS | ST-Link, Lauterbach |
| **TI** | Code Composer Studio | TI ARM, GCC | XDS, Lauterbach |

### AUTOSAR 지원

| 제조사 | MCAL 제공 | Classic Platform | Adaptive Platform |
|--------|-----------|------------------|-------------------|
| NXP | ✅ | ✅ | ✅ (S32G) |
| Infineon | ✅ | ✅ | ✅ |
| Renesas | ✅ | ✅ | ✅ (R-Car) |
| ST | ✅ | ✅ | ⚠️ 일부 |
| TI | ✅ | ✅ | ⚠️ 일부 |

---

## 🏆 추천 가이드

### 입문용 (학습/프로토타입)

| 용도 | 추천 보드 | 가격대 | 이유 |
|------|-----------|--------|------|
| 바디 제어 학습 | NXP S32K144EVB | ~$50 | 문서 풍부, 저가 |
| Safety MCU 학습 | TI LAUNCHXL2-TMS57012 | ~$30 | 최저가 ASIL-D |
| TriCore 학습 | Infineon TC375 Lite Kit | ~$100 | 업계 표준 |

### 프로덕션 (양산용)

| 분야 | 1순위 | 2순위 | 비고 |
|------|-------|-------|------|
| 파워트레인 | Infineon TC39x | Renesas RH850/U2B | 유럽 OEM 선호 |
| 전동화 | Infineon TC38x | ST Stellar-E | EV 전용 최적화 |
| 바디 | NXP S32K144 | Renesas RH850/F1K | 가성비 |
| 게이트웨이 | NXP S32G274A | Renesas R-Car S4 | 이더넷 성능 |
| ADAS | TI TDA4VM | Renesas R-Car V4H | AI 성능 |
| Safety | TI TMS570 | Infineon TC39x | 락스텝 |

### 지역별 선호도

| 지역 | 선호 제조사 | 대표 MCU |
|------|-------------|----------|
| 🇪🇺 유럽 | Infineon, NXP | TC3xx, S32K |
| 🇯🇵 일본 | Renesas | RH850, R-Car |
| 🇺🇸 북미 | TI, NXP | TMS570, S32K |
| 🇰🇷 한국 | Infineon, NXP, Renesas | 혼합 사용 |
| 🇨🇳 중국 | NXP, ST | S32K, SPC5 |

---

## 📚 참고 자료

### 공식 문서

- [NXP S32K Documentation](https://www.nxp.com/products/processors-and-microcontrollers/s32-automotive-platform:S32)
- [Infineon AURIX Documentation](https://www.infineon.com/cms/en/product/microcontroller/32-bit-tricore-microcontroller/)
- [Renesas RH850 Documentation](https://www.renesas.com/products/microcontrollers-microprocessors/rh850)
- [ST Stellar Documentation](https://www.st.com/en/automotive-microcontrollers.html)
- [TI Hercules Documentation](https://www.ti.com/microcontrollers-mcus-processors/arm-based-microcontrollers/overview.html)

### 표준 규격

- [ISO 26262 (기능 안전)](https://www.iso.org/standard/68383.html)
- [AEC-Q100 (IC 신뢰성)](http://www.aecouncil.com/AECDocuments.html)
- [AUTOSAR](https://www.autosar.org/)

---

## 📝 License

This document is licensed under the MIT License.

---

## 🤝 기여

오류 수정, 내용 추가, 최신 정보 업데이트 등의 기여를 환영합니다.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/update-mcu-info`)
3. Commit your changes (`git commit -m 'Add new MCU information'`)
4. Push to the branch (`git push origin feature/update-mcu-info`)
5. Open a Pull Request

---

> 📅 Last Updated: 2025-02  
> 📧 Contact: 문의사항이 있으시면 Issue를 생성해 주세요.
