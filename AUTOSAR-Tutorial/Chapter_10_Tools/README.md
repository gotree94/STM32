# Chapter 10: 개발 도구 및 환경

[← 이전 챕터](../Chapter_09_Methodology/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_11_Adaptive_AUTOSAR/README.md)

---

## 📋 목차

1. [학습 목표](#1-학습-목표)
2. [AUTOSAR 개발 도구 개요](#2-autosar-개발-도구-개요)
3. [주요 상용 도구](#3-주요-상용-도구)
4. [오픈소스 대안](#4-오픈소스-대안)
5. [개발 환경 구축](#5-개발-환경-구축)
6. [요약 및 연습 문제](#6-요약-및-연습-문제)

---

## 1. 학습 목표

```
✅ AUTOSAR 개발에 필요한 도구 종류 파악
✅ 주요 상용 도구의 기능과 특징 이해
✅ 오픈소스 대안 탐색
✅ 개발 환경 구축 방법 학습
```

---

## 2. AUTOSAR 개발 도구 개요

### 2.1 도구 종류

```
┌─────────────────────────────────────────────────────────────────┐
│                   AUTOSAR 개발 도구 분류                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📐 Authoring Tool (설계 도구)                                   │
│     - SWC 설계, 인터페이스 정의                                 │
│     - System 설계, ECU 매핑                                     │
│     - 예: DaVinci Developer, SystemDesk                        │
│                                                                 │
│  ⚙️ Configuration Tool (설정 도구)                               │
│     - BSW 모듈 설정                                             │
│     - MCAL 설정                                                 │
│     - 예: DaVinci Configurator, EB tresos                      │
│                                                                 │
│  🔧 Code Generator (코드 생성기)                                 │
│     - RTE 코드 생성                                             │
│     - BSW 코드 생성                                             │
│                                                                 │
│  🧪 Validation Tool (검증 도구)                                  │
│     - ARXML 검증                                                │
│     - 일관성 검사                                               │
│                                                                 │
│  📊 Model-Based Design (모델 기반 설계)                          │
│     - SWC 동작 모델링                                           │
│     - 코드 자동 생성                                            │
│     - 예: MATLAB/Simulink                                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 3. 주요 상용 도구

### 3.1 Vector 도구

| 도구 | 용도 |
|------|------|
| **DaVinci Developer** | SWC 설계, 인터페이스 정의 |
| **DaVinci Configurator** | BSW/MCAL 설정, RTE 생성 |
| **CANoe** | 통신 시뮬레이션, 테스트 |
| **CANape** | 캘리브레이션, 측정 |

### 3.2 Elektrobit 도구

| 도구 | 용도 |
|------|------|
| **EB tresos Studio** | BSW/MCAL 설정 |
| **EB tresos Safety** | ISO 26262 지원 |
| **EB corbos** | Adaptive AUTOSAR |

### 3.3 기타 도구

| 벤더 | 도구 | 용도 |
|------|------|------|
| **dSPACE** | SystemDesk | 시스템 설계 |
| **MathWorks** | Simulink | 모델 기반 개발 |
| **ETAS** | ISOLAR | BSW 설정 |
| **KPIT** | K-SAR | BSW 스택 |

---

## 4. 오픈소스 대안

### 4.1 오픈소스 프로젝트

```
┌─────────────────────────────────────────────────────────────────┐
│                   오픈소스 AUTOSAR 프로젝트                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  🌐 Arctic Core (ARCCORE)                                       │
│     - 오픈소스 AUTOSAR 스택                                     │
│     - BSW, MCAL 제공                                            │
│     - GPL 라이선스                                              │
│     - https://github.com/openAUTOSAR/classic-platform          │
│                                                                 │
│  🌐 AUTOSAR.io                                                  │
│     - 학습용 AUTOSAR 리소스                                     │
│     - 샘플 ARXML 제공                                           │
│                                                                 │
│  🌐 OpenSynergy                                                 │
│     - COQOS Hypervisor                                         │
│     - Adaptive AUTOSAR용                                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 학습용 환경

| 방법 | 설명 |
|------|------|
| **평가판 사용** | Vector, EB 등 30일 평가판 |
| **학교 라이선스** | 대학 프로그램 활용 |
| **오픈소스** | Arctic Core 등 활용 |
| **시뮬레이션** | QEMU + 오픈소스 BSW |

---

## 5. 개발 환경 구축

### 5.1 일반적인 개발 환경

```
┌─────────────────────────────────────────────────────────────────┐
│                   AUTOSAR 개발 환경 구성                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   개발 PC                                                       │
│   ┌─────────────────────────────────────────────────────────┐  │
│   │                                                          │  │
│   │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │  │
│   │  │ DaVinci     │  │ Simulink    │  │ IDE         │     │  │
│   │  │ Configurator│  │             │  │ (Eclipse)   │     │  │
│   │  └─────────────┘  └─────────────┘  └─────────────┘     │  │
│   │                                                          │  │
│   │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │  │
│   │  │ Compiler    │  │ CANoe       │  │ Debugger    │     │  │
│   │  │ (GCC/IAR)   │  │             │  │ (Lauterbach)│     │  │
│   │  └─────────────┘  └─────────────┘  └─────────────┘     │  │
│   │                                                          │  │
│   └─────────────────────────────────────────────────────────┘  │
│                              │                                  │
│                         USB/JTAG                                │
│                              │                                  │
│                              ▼                                  │
│   ┌─────────────────────────────────────────────────────────┐  │
│   │                    Target ECU                            │  │
│   │               (개발 보드 또는 실제 ECU)                   │  │
│   └─────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 5.2 필수 도구

| 카테고리 | 도구 | 비고 |
|----------|------|------|
| **BSW 설정** | DaVinci Configurator / EB tresos | 필수 |
| **컴파일러** | GCC / IAR / GHS | MCU 맞춤 |
| **디버거** | Lauterbach / J-Link | HW 디버깅 |
| **버전 관리** | Git | ARXML 관리 |

---

## 6. 요약 및 연습 문제

### 핵심 정리

```
┌─────────────────────────────────────────────────────────────────┐
│                    Chapter 10 핵심 정리                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📌 AUTOSAR 도구 종류:                                           │
│     ├── Authoring Tool: SWC/System 설계                        │
│     ├── Configuration Tool: BSW/MCAL 설정                      │
│     ├── Code Generator: RTE/BSW 코드 생성                      │
│     └── MBD Tool: Simulink 모델 기반 개발                      │
│                                                                 │
│  📌 주요 상용 도구:                                              │
│     ├── Vector: DaVinci Developer/Configurator                 │
│     ├── Elektrobit: EB tresos                                  │
│     └── MathWorks: MATLAB/Simulink                             │
│                                                                 │
│  📌 오픈소스 대안:                                               │
│     └── Arctic Core, AUTOSAR.io                                │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 연습 문제

**문제 1.** DaVinci Developer와 DaVinci Configurator의 차이점은?

<details>
<summary>정답</summary>

- **DaVinci Developer**: SWC 설계, 인터페이스 정의, 시스템 아키텍처 설계
- **DaVinci Configurator**: BSW/MCAL 설정, RTE 코드 생성, ECU Configuration
</details>

**문제 2.** 학습용으로 AUTOSAR를 접해볼 수 있는 방법은?

<details>
<summary>정답</summary>

- 상용 도구 평가판 (30일)
- 대학 라이선스 프로그램
- 오픈소스 프로젝트 (Arctic Core)
- AUTOSAR.io 샘플 ARXML
</details>

---

[← 이전 챕터](../Chapter_09_Methodology/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_11_Adaptive_AUTOSAR/README.md)
