# Chapter 08: Diagnostic Stack

[← 이전 챕터](../Chapter_07_Memory_Stack/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_09_Methodology/README.md)

---

## 📋 목차

1. [학습 목표](#1-학습-목표)
2. [Diagnostic Stack 개요](#2-diagnostic-stack-개요)
3. [Dcm (Diagnostic Communication Manager)](#3-dcm-diagnostic-communication-manager)
4. [Dem (Diagnostic Event Manager)](#4-dem-diagnostic-event-manager)
5. [FiM (Function Inhibition Manager)](#5-fim-function-inhibition-manager)
6. [요약 및 연습 문제](#6-요약-및-연습-문제)

---

## 1. 학습 목표

```
✅ Diagnostic Stack의 구조와 역할 이해
✅ UDS (ISO 14229) 프로토콜 기본 개념 학습
✅ Dcm의 서비스 처리 방식 이해
✅ Dem의 DTC 관리 기능 파악
✅ FiM의 기능 억제 메커니즘 이해
```

---

## 2. Diagnostic Stack 개요

### 2.1 Diagnostic Stack이란?

Diagnostic Stack은 **차량 진단 통신 및 고장 관리**를 담당합니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                  Diagnostic Stack 구조                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│            Diagnostic Tester (외부 진단기)                      │
│                         │                                       │
│                    CAN / Ethernet                               │
│                         │                                       │
│  ───────────────────────┼─────────────────────────────────────  │
│                         ▼                                       │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                       Dcm                                │   │
│  │         Diagnostic Communication Manager                 │   │
│  │              (UDS 프로토콜 처리)                          │   │
│  └──────────┬─────────────────────────────┬────────────────┘   │
│             │                             │                     │
│             ▼                             ▼                     │
│  ┌─────────────────────┐       ┌─────────────────────┐         │
│  │        Dem          │       │        FiM          │         │
│  │  Diagnostic Event   │◄─────►│ Function Inhibition │         │
│  │     Manager         │       │      Manager        │         │
│  │   (DTC 관리)        │       │   (기능 억제)       │         │
│  └──────────┬──────────┘       └─────────────────────┘         │
│             │                                                   │
│             ▼                                                   │
│  ┌─────────────────────┐                                       │
│  │        NvM          │                                       │
│  │   (DTC 저장)        │                                       │
│  └─────────────────────┘                                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 UDS 프로토콜 (ISO 14229)

**UDS(Unified Diagnostic Services)** 는 자동차 진단의 국제 표준입니다.

| 서비스 ID | 서비스 이름 | 용도 |
|-----------|------------|------|
| 0x10 | Diagnostic Session Control | 세션 전환 |
| 0x11 | ECU Reset | ECU 리셋 |
| 0x14 | Clear DTC | DTC 삭제 |
| 0x19 | Read DTC Information | DTC 읽기 |
| 0x22 | Read Data By Identifier | 데이터 읽기 |
| 0x27 | Security Access | 보안 인증 |
| 0x2E | Write Data By Identifier | 데이터 쓰기 |
| 0x31 | Routine Control | 루틴 실행 |
| 0x34 | Request Download | 플래싱 시작 |
| 0x36 | Transfer Data | 데이터 전송 |
| 0x37 | Request Transfer Exit | 플래싱 종료 |
| 0x3E | Tester Present | 연결 유지 |

---

## 3. Dcm (Diagnostic Communication Manager)

### 3.1 Dcm 역할

```
┌─────────────────────────────────────────────────────────────────┐
│                       Dcm 주요 기능                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📨 메시지 처리                                                  │
│     - UDS 요청 수신 및 파싱                                     │
│     - UDS 응답 생성 및 송신                                     │
│                                                                 │
│  🔐 세션 관리                                                    │
│     - Default Session                                           │
│     - Programming Session                                       │
│     - Extended Diagnostic Session                               │
│                                                                 │
│  🔒 보안 관리                                                    │
│     - Security Access (Seed & Key)                              │
│     - 서비스별 접근 제어                                        │
│                                                                 │
│  📊 서비스 라우팅                                                │
│     - 서비스별 핸들러 호출                                      │
│     - Application/Dem/NvM 연동                                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 Dcm 세션

```
┌─────────────────────────────────────────────────────────────────┐
│                    Dcm Session 종류                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Default Session (0x01)                                  │   │
│  │  - 기본 상태                                             │   │
│  │  - 제한된 서비스만 허용                                  │   │
│  │  - 예: 0x22 (Read), 0x19 (Read DTC)                     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                         │                                       │
│                    0x10 02 (Session Control)                    │
│                         ▼                                       │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Programming Session (0x02)                              │   │
│  │  - ECU 플래싱용                                          │   │
│  │  - 0x34, 0x36, 0x37 허용                                │   │
│  │  - 보안 잠금 해제 필요                                   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Extended Diagnostic Session (0x03)                      │   │
│  │  - 확장 진단용                                           │   │
│  │  - 0x2E (Write), 0x31 (Routine) 허용                    │   │
│  │  - 캘리브레이션, 테스트                                  │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.3 Dcm API 예시

```c
/* ═══════════════════════════════════════════════════════════════
 * Dcm Callout 함수 (Application에서 구현)
 * ═══════════════════════════════════════════════════════════════ */

/* Read Data By Identifier (0x22) */
Std_ReturnType Dcm_ReadDataByIdentifier_VehicleSpeed(
    Dcm_OpStatusType OpStatus,
    uint8* Data,
    Dcm_NegativeResponseCodeType* ErrorCode
)
{
    uint16 speed;
    
    /* 차량 속도 읽기 */
    speed = GetVehicleSpeed();
    
    /* 데이터 포맷팅 (Big Endian) */
    Data[0] = (uint8)(speed >> 8);
    Data[1] = (uint8)(speed & 0xFF);
    
    return E_OK;
}

/* Write Data By Identifier (0x2E) */
Std_ReturnType Dcm_WriteDataByIdentifier_CalibValue(
    const uint8* Data,
    Dcm_OpStatusType OpStatus,
    Dcm_NegativeResponseCodeType* ErrorCode
)
{
    CalibDataType calibData;
    
    /* 보안 레벨 확인 */
    if (!IsSecurityUnlocked()) {
        *ErrorCode = DCM_E_SECURITYACCESSDENIED;
        return E_NOT_OK;
    }
    
    /* 데이터 파싱 및 저장 */
    calibData.value = (Data[0] << 8) | Data[1];
    SaveCalibration(&calibData);
    
    return E_OK;
}
```

---

## 4. Dem (Diagnostic Event Manager)

### 4.1 Dem 역할

```
┌─────────────────────────────────────────────────────────────────┐
│                       Dem 주요 기능                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  🚨 이벤트 관리                                                  │
│     - Diagnostic Event 상태 관리                                │
│     - Passed/Failed/Prefailed 상태                              │
│                                                                 │
│  📋 DTC (Diagnostic Trouble Code) 관리                          │
│     - DTC 저장/삭제                                             │
│     - DTC 상태 (Active, Passive, Confirmed)                    │
│                                                                 │
│  📊 Freeze Frame                                                 │
│     - 고장 발생 시점의 데이터 스냅샷                            │
│     - 차량 속도, 엔진 RPM, 온도 등                              │
│                                                                 │
│  🔧 Extended Data                                                │
│     - 추가 진단 정보                                            │
│     - 고장 발생 횟수, 주행 거리 등                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 DTC 구조

```
┌─────────────────────────────────────────────────────────────────┐
│                    DTC 포맷 (3 Bytes)                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   DTC: 0x P0 01 23                                              │
│         │  │  │  │                                              │
│         │  │  │  └── Failure Type (고장 유형)                   │
│         │  │  └───── Sub-System (하위 시스템)                   │
│         │  └──────── System (시스템)                            │
│         └─────────── Category (P=Powertrain, B=Body 등)         │
│                                                                 │
│   예시:                                                         │
│   - P0123: Throttle Position Sensor - Circuit High              │
│   - P0300: Engine Misfire Detected                              │
│   - U0100: Lost Communication with ECM                          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 4.3 Dem API 예시

```c
/* ═══════════════════════════════════════════════════════════════
 * Dem API 사용 예시
 * ═══════════════════════════════════════════════════════════════ */

/* 이벤트 리포트 - SWC에서 호출 */
void MonitorThrottleSensor(void)
{
    uint16 throttleVoltage = ReadThrottleVoltage();
    
    if (throttleVoltage > VOLTAGE_HIGH_THRESHOLD) {
        /* 고장 감지 - Failed 리포트 */
        Dem_SetEventStatus(
            DemConf_DemEventParameter_ThrottleSensorHigh,
            DEM_EVENT_STATUS_FAILED
        );
    }
    else if (throttleVoltage < VOLTAGE_LOW_THRESHOLD) {
        /* 전압 낮음 고장 */
        Dem_SetEventStatus(
            DemConf_DemEventParameter_ThrottleSensorLow,
            DEM_EVENT_STATUS_FAILED
        );
    }
    else {
        /* 정상 - Passed 리포트 */
        Dem_SetEventStatus(
            DemConf_DemEventParameter_ThrottleSensorHigh,
            DEM_EVENT_STATUS_PASSED
        );
        Dem_SetEventStatus(
            DemConf_DemEventParameter_ThrottleSensorLow,
            DEM_EVENT_STATUS_PASSED
        );
    }
}

/* Freeze Frame 데이터 제공 - Dem Callback */
Std_ReturnType Dem_GetFreezeFrameData_VehicleSpeed(
    uint8* Buffer
)
{
    uint16 speed = GetVehicleSpeed();
    
    Buffer[0] = (uint8)(speed >> 8);
    Buffer[1] = (uint8)(speed & 0xFF);
    
    return E_OK;
}
```

---

## 5. FiM (Function Inhibition Manager)

### 5.1 FiM 역할

FiM은 **DTC 상태에 따라 기능을 억제**합니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                       FiM 동작 원리                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌───────────────┐        ┌───────────────┐                   │
│   │    센서 고장   │        │   기능 억제    │                   │
│   │    (DTC)      │───────►│   (FID)       │                   │
│   └───────────────┘        └───────────────┘                   │
│                                                                 │
│   예시:                                                         │
│   - 스로틀 센서 고장 → 토크 제어 기능 억제                      │
│   - 차속 센서 고장 → 크루즈 컨트롤 비활성화                     │
│                                                                 │
│   ┌─────────────────────────────────────────────────────────┐  │
│   │                      FiM 설정                            │  │
│   │                                                          │  │
│   │   DTC: ThrottleSensorHigh                                │  │
│   │        ↓                                                 │  │
│   │   FID: FiM_TorqueControl                                 │  │
│   │        ↓                                                 │  │
│   │   결과: 토크 계산 대신 안전 값 사용                       │  │
│   └─────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 5.2 FiM API 예시

```c
/* FiM 사용 예시 */
void TorqueControl_Runnable(void)
{
    boolean permissionGranted;
    
    /* FiM에 기능 허용 여부 확인 */
    FiM_GetFunctionPermission(
        FiMConf_FiMFunctionId_TorqueControl,
        &permissionGranted
    );
    
    if (permissionGranted == TRUE) {
        /* 정상 토크 계산 */
        CalculateNormalTorque();
    }
    else {
        /* 안전 모드 - 제한된 토크 */
        ApplySafeTorque();
    }
}
```

---

## 6. 요약 및 연습 문제

### 핵심 정리

```
┌─────────────────────────────────────────────────────────────────┐
│                    Chapter 08 핵심 정리                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📌 Diagnostic Stack 구성:                                       │
│     ├── Dcm: UDS 통신 처리                                     │
│     ├── Dem: DTC/Event 관리                                    │
│     └── FiM: 기능 억제                                         │
│                                                                 │
│  📌 Dcm 주요 기능:                                               │
│     ├── UDS 서비스 처리 (0x10, 0x22, 0x2E 등)                  │
│     ├── 세션 관리 (Default, Programming, Extended)             │
│     └── 보안 관리 (Seed & Key)                                 │
│                                                                 │
│  📌 Dem 주요 기능:                                               │
│     ├── DTC 저장/삭제                                          │
│     ├── Event 상태 관리 (Passed/Failed)                        │
│     └── Freeze Frame (고장 시점 데이터 스냅샷)                 │
│                                                                 │
│  📌 FiM 역할:                                                    │
│     └── DTC 상태에 따른 기능 억제                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 연습 문제

**문제 1.** UDS 서비스 ID 0x22와 0x2E의 용도는?

<details>
<summary>정답</summary>

- **0x22 (Read Data By Identifier)**: 데이터 읽기
- **0x2E (Write Data By Identifier)**: 데이터 쓰기
</details>

**문제 2.** Dem에서 Freeze Frame의 역할은?

<details>
<summary>정답</summary>

고장 발생 시점의 차량 데이터(차량 속도, 엔진 RPM, 온도 등)를 스냅샷으로 저장하여 나중에 고장 원인 분석에 사용합니다.
</details>

**문제 3.** FiM의 용도와 예시를 설명하세요.

<details>
<summary>정답</summary>

FiM은 DTC 상태에 따라 특정 기능을 억제합니다. 예: 스로틀 센서 고장(DTC) 발생 시 토크 제어 기능(FID)을 억제하고 안전 값을 사용합니다.
</details>

---

[← 이전 챕터](../Chapter_07_Memory_Stack/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_09_Methodology/README.md)
