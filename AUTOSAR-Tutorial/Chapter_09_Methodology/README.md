# Chapter 09: AUTOSAR 방법론

[← 이전 챕터](../Chapter_08_Diagnostic_Stack/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_10_Tools/README.md)

---

## 📋 목차

1. [학습 목표](#1-학습-목표)
2. [AUTOSAR 방법론 개요](#2-autosar-방법론-개요)
3. [개발 프로세스](#3-개발-프로세스)
4. [ARXML 파일](#4-arxml-파일)
5. [요약 및 연습 문제](#5-요약-및-연습-문제)

---

## 1. 학습 목표

```
✅ AUTOSAR 방법론의 개념과 목적 이해
✅ 개발 프로세스 단계별 이해
✅ ARXML 파일의 구조와 역할 파악
```

---

## 2. AUTOSAR 방법론 개요

### 2.1 방법론이란?

AUTOSAR 방법론은 **ECU 소프트웨어 개발을 위한 표준화된 프로세스**입니다.

- **표준화된 개발 프로세스**: OEM과 Supplier 간 협업
- **표준화된 교환 형식 (ARXML)**: 도구 간 데이터 교환
- **재사용 가능한 설계**: SWC 재사용, 설정 분리

---

## 3. 개발 프로세스

### 3.1 개발 5단계

```
┌─────────────────────────────────────────────────────────────────┐
│                  AUTOSAR 개발 프로세스                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   1. System Configuration Input                                 │
│      - 전체 시스템 요구사항 정의                                │
│      - SWC 식별, 통신 매트릭스                                  │
│                     │                                           │
│                     ▼                                           │
│   2. Configure System → System Description (.arxml)            │
│      - SWC를 ECU에 매핑                                         │
│      - 통신 토폴로지 정의                                       │
│                     │                                           │
│                     ▼                                           │
│   3. Extract ECU Info → ECU Extract (.arxml)                   │
│      - 각 ECU에 필요한 정보 추출                                │
│                     │                                           │
│                     ▼                                           │
│   4. Configure ECU → ECU Configuration (.arxml)                │
│      - BSW 모듈 설정 (OS, COM, NvM 등)                         │
│      - Task 스케줄링 설정                                       │
│                     │                                           │
│                     ▼                                           │
│   5. Generate Executable → .elf, .hex                          │
│      - RTE 코드 생성                                            │
│      - BSW 코드 생성                                            │
│      - 컴파일 및 링크                                           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 역할 분담

| 역할 | 담당 업무 | 산출물 |
|------|----------|--------|
| **OEM** | 시스템 설계, SWC-ECU 매핑 | System Description |
| **Tier-1** | ECU 설정, BSW 설정 | ECU Configuration |
| **BSW Supplier** | MCAL, BSW 모듈 제공 | BSW 코드, BSWMD |

---

## 4. ARXML 파일

### 4.1 ARXML이란?

**ARXML(AUTOSAR XML)** 은 AUTOSAR 정보 교환을 위한 표준 파일 형식입니다.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
  <AR-PACKAGES>
    <AR-PACKAGE>
      <SHORT-NAME>MyPackage</SHORT-NAME>
      <ELEMENTS>
        <!-- AUTOSAR 요소 정의 -->
      </ELEMENTS>
    </AR-PACKAGE>
  </AR-PACKAGES>
</AUTOSAR>
```

### 4.2 주요 ARXML 파일

| 파일 종류 | 내용 | 확장자 |
|----------|------|--------|
| **SWC Description** | SWC 정의 (Port, Runnable) | *_swc.arxml |
| **System Description** | 시스템 구성, ECU 매핑 | *_system.arxml |
| **ECU Extract** | ECU별 필요 정보 | *_ecuextract.arxml |
| **ECU Configuration** | BSW 설정값 | *_ecuc.arxml |
| **BSWMD** | BSW 모듈 설정 정의 | *_bswmd.arxml |

### 4.3 파일 흐름

```
  SWC Description + System Description
                │
                ▼
        System Configuration
                │
                ▼
           ECU Extract (ECU별)
                │
                ▼
        ECU Configuration + BSWMD
                │
                ▼
         Code Generation (RTE, BSW)
                │
                ▼
           ECU Executable
```

---

## 5. 요약 및 연습 문제

### 핵심 정리

```
┌─────────────────────────────────────────────────────────────────┐
│                    Chapter 09 핵심 정리                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📌 AUTOSAR 방법론:                                              │
│     - 표준화된 개발 프로세스                                    │
│     - ARXML 기반 정보 교환                                      │
│                                                                 │
│  📌 개발 프로세스 5단계:                                         │
│     1. System Configuration Input                              │
│     2. Configure System → System Description                   │
│     3. Extract ECU Info → ECU Extract                          │
│     4. Configure ECU → ECU Configuration                       │
│     5. Generate Executable                                     │
│                                                                 │
│  📌 주요 ARXML 파일:                                             │
│     ├── SWC Description: SWC 정의                              │
│     ├── System Description: 시스템 구성                        │
│     └── ECU Configuration: BSW 설정                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 연습 문제

**문제 1.** AUTOSAR 개발 프로세스 5단계를 순서대로 나열하세요.

<details>
<summary>정답</summary>

1. System Configuration Input
2. Configure System
3. Extract ECU-Specific Information
4. Configure ECU
5. Generate Executable
</details>

**문제 2.** System Description에 포함되는 정보는?

<details>
<summary>정답</summary>

- SWC-ECU 매핑 정보
- 통신 토폴로지 (버스 연결)
- 통신 매트릭스 (Signal-PDU-Frame 매핑)
</details>

---

[← 이전 챕터](../Chapter_08_Diagnostic_Stack/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_10_Tools/README.md)
