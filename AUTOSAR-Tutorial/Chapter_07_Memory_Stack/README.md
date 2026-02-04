# Chapter 07: Memory Stack

[← 이전 챕터](../Chapter_06_Communication_Stack/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_08_Diagnostic_Stack/README.md)

---

## 📋 목차

1. [학습 목표](#1-학습-목표)
2. [Memory Stack 개요](#2-memory-stack-개요)
3. [NvM (NVRAM Manager)](#3-nvm-nvram-manager)
4. [Fee와 Ea](#4-fee와-ea)
5. [Memory 드라이버](#5-memory-드라이버)
6. [요약 및 연습 문제](#6-요약-및-연습-문제)

---

## 1. 학습 목표

```
✅ Memory Stack의 계층 구조 이해
✅ NvM의 역할과 Block 개념 학습
✅ Fee (Flash EEPROM Emulation) 동작 원리 이해
✅ 비휘발성 메모리 관리 방법 파악
```

---

## 2. Memory Stack 개요

### 2.1 Memory Stack이란?

Memory Stack은 **비휘발성 메모리(NVM) 관리**를 담당하는 BSW 모듈입니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                    Memory Stack 구조                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│                         RTE                                     │
│  ───────────────────────────────────────────────────────────── │
│                          │                                      │
│                          ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                       NvM                                │   │
│  │          NVRAM Manager (블록 관리, Job 큐)               │   │
│  └────────────────────────┬────────────────────────────────┘   │
│                           │                                     │
│                           ▼                                     │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                      MemIf                               │   │
│  │           Memory Abstraction Interface                   │   │
│  └──────────┬─────────────────────────────┬────────────────┘   │
│             │                             │                     │
│             ▼                             ▼                     │
│  ┌─────────────────────┐       ┌─────────────────────┐         │
│  │        Fee          │       │         Ea          │         │
│  │  Flash EEPROM       │       │     EEPROM          │         │
│  │  Emulation          │       │    Abstraction      │         │
│  └──────────┬──────────┘       └──────────┬──────────┘         │
│             │                             │                     │
│             ▼                             ▼                     │
│  ┌─────────────────────┐       ┌─────────────────────┐         │
│  │        Fls          │       │        Eep          │         │
│  │   Flash Driver      │       │   EEPROM Driver     │         │
│  │      (MCAL)         │       │      (MCAL)         │         │
│  └─────────────────────┘       └─────────────────────┘         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Memory 종류

| 메모리 | 특징 | 용도 |
|--------|------|------|
| **Internal Flash** | MCU 내장, 대용량 | 코드, 대용량 데이터 |
| **Internal EEPROM** | MCU 내장, 소용량 | 설정값, 카운터 |
| **External EEPROM** | SPI/I2C 연결 | 추가 저장 공간 |

---

## 3. NvM (NVRAM Manager)

### 3.1 NvM 역할

```
┌─────────────────────────────────────────────────────────────────┐
│                       NvM 주요 기능                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📦 Block 관리                                                   │
│     - 데이터를 Block 단위로 관리                                │
│     - Block ID로 식별                                           │
│                                                                 │
│  📋 Job Queue                                                    │
│     - 읽기/쓰기 요청을 큐에 저장                                │
│     - 비동기 처리 (Main Function에서 처리)                      │
│                                                                 │
│  🔒 데이터 무결성                                                │
│     - CRC 검사                                                  │
│     - Redundant Block (이중화)                                  │
│                                                                 │
│  💾 기본값 처리                                                  │
│     - ROM Default 값 제공                                       │
│     - 읽기 실패 시 기본값 사용                                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 NvM Block 종류

| Block 종류 | 설명 |
|------------|------|
| **Native** | 기본 블록 (단일 복사본) |
| **Redundant** | 이중화 블록 (2개 복사본) |
| **Dataset** | 다중 데이터셋 (같은 구조의 여러 인스턴스) |

### 3.3 NvM API

```c
/* ═══════════════════════════════════════════════════════════════
 * NvM API 예시
 * ═══════════════════════════════════════════════════════════════ */

/* 블록 읽기 (비동기) */
Std_ReturnType NvM_ReadBlock(
    NvM_BlockIdType BlockId,
    void* NvM_DstPtr
);

/* 블록 쓰기 (비동기) */
Std_ReturnType NvM_WriteBlock(
    NvM_BlockIdType BlockId,
    const void* NvM_SrcPtr
);

/* 작업 상태 확인 */
Std_ReturnType NvM_GetErrorStatus(
    NvM_BlockIdType BlockId,
    NvM_RequestResultType* RequestResultPtr
);

/* 모든 블록 읽기 (초기화 시) */
void NvM_ReadAll(void);

/* 모든 블록 쓰기 (종료 시) */
void NvM_WriteAll(void);
```

### 3.4 NvM 사용 예시

```c
/* 캘리브레이션 데이터 저장/읽기 예시 */

typedef struct {
    float32 throttleGain;
    float32 speedFactor;
    uint16 maxRpm;
} CalibrationDataType;

CalibrationDataType calibData;

/* ---- 초기화 시 읽기 ---- */
void Init_ReadCalibration(void)
{
    /* 비동기 읽기 요청 */
    NvM_ReadBlock(NvMConf_NvMBlockDescriptor_CalibData, &calibData);
}

/* ---- 주기적으로 완료 확인 ---- */
void CheckReadComplete(void)
{
    NvM_RequestResultType status;
    
    NvM_GetErrorStatus(NvMConf_NvMBlockDescriptor_CalibData, &status);
    
    switch (status) {
        case NVM_REQ_OK:
            /* 읽기 성공 - calibData 사용 가능 */
            break;
        case NVM_REQ_NOT_OK:
            /* 읽기 실패 - 기본값 사용 */
            UseDefaultCalibration(&calibData);
            break;
        case NVM_REQ_PENDING:
            /* 아직 처리 중 */
            break;
    }
}

/* ---- 데이터 저장 ---- */
void SaveCalibration(void)
{
    calibData.throttleGain = 1.5f;
    calibData.maxRpm = 7000;
    
    /* 비동기 쓰기 요청 */
    NvM_WriteBlock(NvMConf_NvMBlockDescriptor_CalibData, &calibData);
}
```

---

## 4. Fee와 Ea

### 4.1 Fee (Flash EEPROM Emulation)

Fee는 **Flash 메모리를 EEPROM처럼 사용**할 수 있게 합니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                    Fee 동작 원리                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  문제: Flash는 Byte 단위 쓰기 불가, Sector 단위 삭제만 가능     │
│                                                                 │
│  해결: Fee가 Wear Leveling과 Garbage Collection 수행            │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              Flash Memory Layout (Fee 사용)              │   │
│  │                                                          │   │
│  │  Sector 0          Sector 1          Sector 2           │   │
│  │  ┌──────────┐     ┌──────────┐     ┌──────────┐        │   │
│  │  │ Block A  │     │ Block A' │     │          │        │   │
│  │  │ (old)    │     │ (new)    │     │  Empty   │        │   │
│  │  ├──────────┤     ├──────────┤     │          │        │   │
│  │  │ Block B  │     │ Block C  │     │          │        │   │
│  │  │ (old)    │     │          │     │          │        │   │
│  │  ├──────────┤     ├──────────┤     │          │        │   │
│  │  │ Block C  │     │  Empty   │     │          │        │   │
│  │  │ (old)    │     │          │     │          │        │   │
│  │  └──────────┘     └──────────┘     └──────────┘        │   │
│  │                                                          │   │
│  │  → Block A 업데이트 시 Sector 1에 새로 기록              │   │
│  │  → Sector 0이 가득 차면 Garbage Collection               │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 Ea (EEPROM Abstraction)

Ea는 **외부 EEPROM 접근을 추상화**합니다.

| 특징 | Fee | Ea |
|------|-----|-----|
| 대상 | Internal Flash | External EEPROM |
| 드라이버 | Fls (Flash Driver) | Eep (EEPROM Driver) |
| 인터페이스 | SPI (내부) | SPI/I2C (외부) |
| Wear Leveling | 필요 (Fee가 수행) | 불필요 (EEPROM 자체 지원) |

---

## 5. Memory 드라이버

### 5.1 Fls (Flash Driver)

```c
/* ═══════════════════════════════════════════════════════════════
 * Fls API (MCAL)
 * ═══════════════════════════════════════════════════════════════ */

/* Flash 읽기 */
Std_ReturnType Fls_Read(
    Fls_AddressType SourceAddress,
    uint8* TargetAddressPtr,
    Fls_LengthType Length
);

/* Flash 쓰기 */
Std_ReturnType Fls_Write(
    Fls_AddressType TargetAddress,
    const uint8* SourceAddressPtr,
    Fls_LengthType Length
);

/* Flash Sector 삭제 */
Std_ReturnType Fls_Erase(
    Fls_AddressType TargetAddress,
    Fls_LengthType Length
);

/* 작업 상태 확인 */
MemIf_StatusType Fls_GetStatus(void);
```

### 5.2 데이터 흐름

```
  NvM_WriteBlock()
       │
       ▼
  ┌─────────┐
  │   NvM   │  Block → 물리 주소 변환
  └────┬────┘
       │
       ▼
  ┌─────────┐
  │  MemIf  │  Fee/Ea 선택
  └────┬────┘
       │
       ▼
  ┌─────────┐
  │   Fee   │  Wear Leveling, 빈 공간 찾기
  └────┬────┘
       │
       ▼
  ┌─────────┐
  │   Fls   │  Flash 레지스터 접근
  └────┬────┘
       │
       ▼
  [ Flash Memory ]
```

---

## 6. 요약 및 연습 문제

### 핵심 정리

```
┌─────────────────────────────────────────────────────────────────┐
│                    Chapter 07 핵심 정리                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📌 Memory Stack 계층:                                           │
│     NvM → MemIf → Fee/Ea → Fls/Eep                             │
│                                                                 │
│  📌 NvM 기능:                                                    │
│     ├── Block 단위 데이터 관리                                  │
│     ├── 비동기 읽기/쓰기 (Job Queue)                           │
│     ├── CRC 검사, Redundant Block                              │
│     └── ROM Default 값 제공                                    │
│                                                                 │
│  📌 Fee vs Ea:                                                   │
│     ├── Fee: Flash를 EEPROM처럼 사용 (Wear Leveling)           │
│     └── Ea: 외부 EEPROM 추상화                                 │
│                                                                 │
│  📌 주요 API:                                                    │
│     ├── NvM_ReadBlock / NvM_WriteBlock                         │
│     ├── NvM_ReadAll / NvM_WriteAll                             │
│     └── NvM_GetErrorStatus                                     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 연습 문제

**문제 1.** NvM API가 비동기인 이유는?

<details>
<summary>정답</summary>

Flash/EEPROM 쓰기는 수~수십 ms가 소요되어 동기 호출 시 CPU가 대기해야 합니다. 비동기 처리로 다른 작업을 계속 수행하고, NvM_MainFunction()에서 백그라운드로 처리합니다.
</details>

**문제 2.** Fee가 필요한 이유는?

<details>
<summary>정답</summary>

Flash는 Byte 단위 쓰기가 불가능하고 Sector 단위로만 삭제할 수 있습니다. Fee는 Wear Leveling과 Garbage Collection을 통해 Flash를 EEPROM처럼 Byte 단위로 업데이트할 수 있게 합니다.
</details>

**문제 3.** NvM Block 종류 3가지와 각각의 용도는?

<details>
<summary>정답</summary>

- **Native**: 기본 블록, 일반 데이터 저장
- **Redundant**: 이중화 블록, 안전 필수 데이터
- **Dataset**: 다중 인스턴스, 같은 구조의 여러 데이터셋
</details>

---

[← 이전 챕터](../Chapter_06_Communication_Stack/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_08_Diagnostic_Stack/README.md)
