# 🔵 STM32 개발 보드 종합 가이드 (CPU Core별)

> STM32 Nucleo 및 Discovery 보드 Core별 완벽 정리

[![STM32](https://img.shields.io/badge/STM32-03234B?style=flat&logo=stmicroelectronics&logoColor=white)]()
[![ARM](https://img.shields.io/badge/ARM_Cortex--M-0091BD?style=flat&logo=arm&logoColor=white)]()

---

## 📑 목차

- [Cortex-M0/M0+](#cortex-m0m0)
- [Cortex-M3](#cortex-m3)
- [Cortex-M4/M4F](#cortex-m4m4f)
- [Cortex-M7](#cortex-m7)
- [Cortex-M33](#cortex-m33)
- [듀얼코어 (M33 + M0+)](#cortex-m33--cortex-m0-이종-듀얼코어)
- [요약 비교표](#stm32-요약-비교표)
- [용도별 추천](#stm32-추천-보드-용도별)

---

## Cortex-M0/M0+

### Nucleo 보드

|  | 보드명 | MCU | Core | Flash | RAM | 특징 |
|---|--------|-----|------|-------|-----|------|
|  | NUCLEO-F030R8 | STM32F030R8 | M0 | 64KB | 8KB | 입문용 |
|  | NUCLEO-F031K6 | STM32F031K6 | M0 | 32KB | 4KB | Nano |
|  | NUCLEO-F042K6 | STM32F042K6 | M0 | 32KB | 6KB | USB, CAN |
|  | NUCLEO-F070RB | STM32F070RB | M0 | 128KB | 16KB | USB |
|  | NUCLEO-F072RB | STM32F072RB | M0 | 128KB | 16KB | USB, CAN, Touch |
|  | NUCLEO-F091RC | STM32F091RC | M0 | 256KB | 32KB | 최대 사양 |
|  | NUCLEO-G031K8 | STM32G031K8 | M0+ | 64KB | 8KB | 저가 |
|  | NUCLEO-G041K8 | STM32G041K8 | M0+ | 64KB | 8KB | AES |
|  | NUCLEO-G070RB | STM32G070RB | M0+ | 128KB | 36KB | |
|  | **NUCLEO-G071RB** | **STM32G071RB** | **M0+** | **128KB** | **36KB** | **⭐ 추천** |
|  | NUCLEO-G0B1RE | STM32G0B1RE | M0+ | 512KB | 144KB | 대용량 |
|  | NUCLEO-L010RB | STM32L010RB | M0+ | 128KB | 20KB | 저전력 |
|  | NUCLEO-L011K4 | STM32L011K4 | M0+ | 16KB | 2KB | 초저전력 |
|  | NUCLEO-L031K6 | STM32L031K6 | M0+ | 32KB | 8KB | |
|  | NUCLEO-L053R8 | STM32L053R8 | M0+ | 64KB | 8KB | LCD |
|  | NUCLEO-L073RZ | STM32L073RZ | M0+ | 192KB | 20KB | USB, LCD |
|  | NUCLEO-C031C6 | STM32C031C6 | M0+ | 32KB | 12KB | 최저가 |

### Discovery 보드

| | 보드명 | MCU | Core | 특징 |
|---|--------|-----|------|------|
| | STM32F0DISCOVERY | STM32F051R8 | M0 | 기본 학습용 |
| | STM32F072B-DISCO | STM32F072RB | M0 | USB, Touch sensing |
| | STM32L0538-DISCO | STM32L053C8 | M0+ | E-paper 디스플레이 |
| | STM32L073Z-EVAL | STM32L073VZ | M0+ | 평가 보드 |
| | **B-L072Z-LRWAN1** | **STM32L072CZ** | **M0+** | **⭐ LoRa 무선** |

---

## Cortex-M3

### Nucleo 보드

|  | 보드명 | MCU | Flash | RAM | 특징 |
|-----|--------|-----|-------|-----|------|
| V | **NUCLEO-F103RB** | **STM32F103RB** | **128KB** | **20KB** | **⭐ 가장 대중적, 입문 필수** |
| | NUCLEO-F207ZG | STM32F207ZG | 1MB | 128KB | 이더넷, 144핀 |
| | NUCLEO-L152RE | STM32L152RE | 512KB | 80KB | 저전력, LCD |
| | NUCLEO-L100RB | STM32L100RB | 128KB | 16KB | 초저전력 |

### Discovery 보드

|  | 보드명 | MCU | 특징 |
|-----|--------|-----|------|
|  | STM32VLDISCOVERY | STM32F100RB | Value Line 입문용 |
|  | STM32F100C-EVAL | STM32F100VB | 평가 보드 |
|  | STM32L-DISCOVERY | STM32L152RB | LCD 세그먼트, 저전력 |
|  | 32L152CDISCOVERY | STM32L152RC | LCD 세그먼트 |

---

## Cortex-M4/M4F

### Nucleo 보드

|  | 보드명 | MCU | Flash | RAM | 특징 |
|----|--------|-----|-------|-----|------|
|  | NUCLEO-F302R8 | STM32F302R8 | 64KB | 16KB | Motor control |
|  | NUCLEO-F303K8 | STM32F303K8 | 64KB | 12KB | Nano 폼팩터 |
|  | NUCLEO-F303RE | STM32F303RE | 512KB | 64KB | 고성능 아날로그 |
|  | NUCLEO-F303ZE | STM32F303ZE | 512KB | 64KB | 144핀 |
|  | NUCLEO-F334R8 | STM32F334R8 | 64KB | 12KB | HRTIM |
|  | **NUCLEO-F401RE** | **STM32F401RE** | **512KB** | **96KB** | **⭐ 가장 대중적** |
|  | NUCLEO-F410RB | STM32F410RB | 128KB | 32KB | |
| V | **NUCLEO-F411RE** | **STM32F411RE** | **512KB** | **128KB** | **⭐ 고성능 추천** |
|  | NUCLEO-F412ZG | STM32F412ZG | 1MB | 256KB | |
|  | NUCLEO-F413ZH | STM32F413ZH | 1.5MB | 320KB | |
|  | NUCLEO-F446RE | STM32F446RE | 512KB | 128KB | 180MHz |
|  | NUCLEO-F446ZE | STM32F446ZE | 512KB | 128KB | 144핀 |
|  | NUCLEO-G431KB | STM32G431KB | 128KB | 32KB | Nano |
|  | NUCLEO-G431RB | STM32G431RB | 128KB | 32KB | |
|  | NUCLEO-G474RE | STM32G474RE | 512KB | 128KB | 고성능 아날로그 |
|  | NUCLEO-G491RE | STM32G491RE | 512KB | 112KB | |
|  | NUCLEO-L412KB | STM32L412KB | 128KB | 40KB | Ultra-low power |
|  | NUCLEO-L432KC | STM32L432KC | 256KB | 64KB | Nano |
|  | NUCLEO-L433RC-P | STM32L433RC | 256KB | 64KB | SMPS |
|  | NUCLEO-L452RE | STM32L452RE | 512KB | 160KB | |
|  | **NUCLEO-L476RG** | **STM32L476RG** | **1MB** | **128KB** | **⭐ 저전력 고성능** |
|  | NUCLEO-L496ZG | STM32L496ZG | 1MB | 320KB | |
|  | NUCLEO-L4A6ZG | STM32L4A6ZG | 1MB | 320KB | Crypto |
|  | NUCLEO-L4R5ZI | STM32L4R5ZI | 2MB | 640KB | |
|  | NUCLEO-L4P5ZG | STM32L4P5ZG | 1MB | 320KB | |
|  | NUCLEO-WB15CC | STM32WB15CC | 320KB | 48KB | BLE |
|  | **NUCLEO-WB55RG** | **STM32WB55RG** | **1MB** | **256KB** | **⭐ BLE, Zigbee** |
|  | **NUCLEO-WL55JC** | **STM32WL55JC** | **256KB** | **64KB** | **⭐ LoRa, Sub-GHz** |

### Discovery 보드

|  | 보드명 | MCU | 특징 |
|------|--------|-----|------|
|  | STM32F3DISCOVERY | STM32F303VC | 자이로스코프, 가속도계 |
|  | STM32F401C-DISCO | STM32F401VC | 오디오, 가속도계 |
|  | **STM32F407G-DISC1** | **STM32F407VG** | **⭐ 가장 인기, 오디오, 가속도계** |
|  | STM32F411E-DISCO | STM32F411VE | 자이로, 가속도계, 마그네토 |
|  | STM32F412G-DISCO | STM32F412ZG | TFT LCD, 오디오 |
|  | STM32F413H-DISCO | STM32F413ZH | LCD, 오디오, MEMS |
|  | **B-L475E-IOT01A** | **STM32L475VG** | **⭐ IoT Node, WiFi, BLE, 센서** |
|  | STM32L476G-DISCO | STM32L476VG | LCD, 오디오, 쿼드SPI |
|  | STM32L496G-DISCO | STM32L496AG | TFT LCD, 오디오 |
|  | STM32L4R9I-DISCO | STM32L4R9AI | AMOLED, DSI |
|  | B-L4S5I-IOT01A | STM32L4S5VI | IoT, WiFi, BLE |
|  | STM32G474E-EVAL | STM32G474QE | 평가 보드 |
|  | B-G474E-DPOW1 | STM32G474RE | Digital Power |
|  | P-NUCLEO-WB55 | STM32WB55RG | BLE 개발팩 |
|  | STM32WB5MM-DK | STM32WB5MMG | BLE 모듈 |

---

## Cortex-M7

### Nucleo 보드

|  | 보드명 | MCU | Flash | RAM | 특징 |
|----|--------|-----|-------|-----|------|
|  | NUCLEO-F722ZE | STM32F722ZE | 512KB | 256KB | |
|  | NUCLEO-F746ZG | STM32F746ZG | 1MB | 320KB | |
|  | NUCLEO-F756ZG | STM32F756ZG | 1MB | 320KB | Crypto |
|  | NUCLEO-F767ZI | STM32F767ZI | 2MB | 512KB | 고성능 |
|  | NUCLEO-H723ZG | STM32H723ZG | 1MB | 564KB | 550MHz |
|  | **NUCLEO-H743ZI** | **STM32H743ZI** | **2MB** | **1MB** | **⭐ 480MHz, 추천** |
|  | NUCLEO-H743ZI2 | STM32H743ZI | 2MB | 1MB | Rev.2 |
|  | **NUCLEO-H745ZI-Q** | **STM32H745ZI** | **2MB** | **1MB** | **⭐ 듀얼코어 M7+M4** |
|  | NUCLEO-H753ZI | STM32H753ZI | 2MB | 1MB | Crypto |
|  | NUCLEO-H755ZI-Q | STM32H755ZI | 2MB | 1MB | 듀얼코어 + Crypto |
|  | NUCLEO-H7A3ZI-Q | STM32H7A3ZI | 2MB | 1.4MB | |
|  | NUCLEO-H7S3L8 | STM32H7S3L8 | 64KB | 620KB | 외부 메모리 |

### Discovery 보드

|  | 보드명 | MCU | 특징 |
|----|--------|-----|------|
|  | **STM32F746G-DISCO** | **STM32F746NG** | **⭐ 4.3" LCD, 터치, 이더넷, USB** |
|  | STM32F769I-DISCO | STM32F769NI | 4" LCD, DSI, 오디오 |
|  | STM32F7508-DK | STM32F750N8 | LCD, 가성비 |
|  | STM32H735G-DK | STM32H735IG | 4" LCD, 이더넷, OSPI |
|  | STM32H743I-EVAL | STM32H743XI | 평가 보드 |
|  | STM32H745I-DISCO | STM32H745XI | 듀얼코어, LCD |
|  | **STM32H747I-DISCO** | **STM32H747XI** | **⭐ 듀얼코어, 4" LCD, 카메라** |
|  | STM32H750B-DK | STM32H750XB | LCD, 외부 플래시 |
|  | STM32H7B3I-DK | STM32H7B3LI | 4.3" LCD, OSPI |
|  | STM32H7B3I-EVAL | STM32H7B3II | 평가 보드 |
|  | STM32H7S78-DK | STM32H7S7L8 | 최신, 외부 메모리 |

---

## Cortex-M33

### Nucleo 보드

|  | 보드명 | MCU | Flash | RAM | 특징 |
|----|--------|-----|-------|-----|------|
|  | NUCLEO-L552ZE-Q | STM32L552ZE | 512KB | 256KB | TrustZone |
|  | NUCLEO-U545RE-Q | STM32U545RE | 512KB | 274KB | Ultra-low power |
|  | **NUCLEO-U575ZI-Q** | **STM32U575ZI** | **2MB** | **786KB** | **⭐ 추천** |
|  | NUCLEO-U5A5ZJ-Q | STM32U5A5ZJ | 4MB | 2.5MB | |
|  | NUCLEO-H503RB | STM32H503RB | 128KB | 32KB | 250MHz |
|  | NUCLEO-H533RE | STM32H533RE | 512KB | 272KB | |
|  | NUCLEO-H563ZI | STM32H563ZI | 2MB | 640KB | 250MHz |
|  | NUCLEO-WBA52CG | STM32WBA52CG | 1MB | 96KB | BLE 5.4 |
|  | NUCLEO-WBA55CG | STM32WBA55CG | 1MB | 128KB | BLE 5.4 |

### Discovery 보드

|  | 보드명 | MCU | 특징 |
|----|--------|-----|------|
|  | STM32L562E-DK | STM32L562QE | TrustZone, LCD, 터치 |
|  | **B-U585I-IOT02A** | **STM32U585AI** | **⭐ IoT, WiFi, BLE, 센서** |
|  | STM32U5A9J-DK | STM32U5A9NJ | LCD, 터치, OSPI |
|  | STM32H573I-DK | STM32H573II | LCD, 이더넷, USB-C |
|  | STM32WBA55G-DK1 | STM32WBA55CG | BLE 5.4, LCD |

---

## Cortex-M33 + Cortex-M0+ (이종 듀얼코어)

### Nucleo 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
| NUCLEO-WL33CC | STM32WL33CC | Sub-GHz 무선 |

### Discovery 보드

| 보드명 | MCU | 특징 |
|--------|-----|------|
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

---

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

## 📚 참고 자료

- [STM32 Nucleo Overview](https://www.st.com/en/evaluation-tools/stm32-nucleo-boards.html)
- [STM32 Discovery Kits](https://www.st.com/en/evaluation-tools/stm32-discovery-kits.html)
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)

---

> 📅 Last Updated: 2025-02
