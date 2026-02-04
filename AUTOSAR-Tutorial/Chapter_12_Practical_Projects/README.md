# Chapter 12: 실습 프로젝트

[← 이전 챕터](../Chapter_11_Adaptive_AUTOSAR/README.md) | [메인으로 돌아가기](../README.md)

---

## 📋 목차

1. [학습 목표](#1-학습-목표)
2. [프로젝트 1: LED 제어 SWC](#2-프로젝트-1-led-제어-swc)
3. [프로젝트 2: CAN 통신 기반 속도 표시](#3-프로젝트-2-can-통신-기반-속도-표시)
4. [프로젝트 3: 진단 서비스 구현](#4-프로젝트-3-진단-서비스-구현)
5. [추가 학습 자료](#5-추가-학습-자료)

---

## 1. 학습 목표

```
✅ AUTOSAR 개념을 실제 코드로 구현
✅ SWC, RTE, BSW 연동 이해
✅ 실제 프로젝트 구조 경험
```

---

## 2. 프로젝트 1: LED 제어 SWC

### 2.1 프로젝트 개요

간단한 LED 토글 기능을 AUTOSAR SWC로 구현합니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                    LED 제어 시스템 구조                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌─────────────────┐                 ┌─────────────────┐      │
│   │  LedControl     │                 │   IoHwAb        │      │
│   │     SWC         │                 │                 │      │
│   │                 │                 │                 │      │
│   │ [P] LedCommand ─┼─────────────────┼─► [R] LedCmd    │      │
│   │                 │    RTE          │                 │      │
│   └─────────────────┘                 └────────┬────────┘      │
│                                                │                │
│                                                ▼                │
│                                         ┌───────────┐          │
│                                         │    Dio    │          │
│                                         │   (MCAL)  │          │
│                                         └─────┬─────┘          │
│                                               │                │
│                                               ▼                │
│                                            [ LED ]             │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 SWC 코드

```c
/* LedControl.c */
#include "Rte_LedControl.h"

static uint8 ledState = STD_LOW;

/* 100ms 주기 Runnable */
FUNC(void, LedControl_CODE) RE_LedToggle(void)
{
    /* LED 상태 토글 */
    if (ledState == STD_LOW) {
        ledState = STD_HIGH;
    } else {
        ledState = STD_LOW;
    }
    
    /* P-Port로 LED 명령 전송 */
    (void)Rte_Write_PP_LedCommand_value(ledState);
}
```

### 2.3 Interface 정의 (ARXML 개념)

```xml
<!-- Interface 정의 -->
<SENDER-RECEIVER-INTERFACE>
  <SHORT-NAME>IF_LedCommand</SHORT-NAME>
  <DATA-ELEMENTS>
    <VARIABLE-DATA-PROTOTYPE>
      <SHORT-NAME>value</SHORT-NAME>
      <TYPE-TREF>uint8</TYPE-TREF>
    </VARIABLE-DATA-PROTOTYPE>
  </DATA-ELEMENTS>
</SENDER-RECEIVER-INTERFACE>
```

---

## 3. 프로젝트 2: CAN 통신 기반 속도 표시

### 3.1 프로젝트 개요

CAN으로 차량 속도를 수신하여 처리하는 SWC를 구현합니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                    차량 속도 시스템 구조                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│                        CAN Bus                                  │
│                          │                                      │
│                          ▼                                      │
│   ┌───────────────────────────────────────────────────────┐    │
│   │                     BSW                                │    │
│   │   Can → CanIf → PduR → COM                            │    │
│   └───────────────────────────────────────────────────────┘    │
│                          │ Signal: VehicleSpeed                 │
│                          ▼                                      │
│                        RTE                                      │
│                          │                                      │
│          ┌───────────────┴───────────────┐                     │
│          ▼                               ▼                     │
│   ┌─────────────────┐          ┌─────────────────┐             │
│   │ SpeedProcessor  │          │  SpeedDisplay   │             │
│   │      SWC        │          │      SWC        │             │
│   │                 │          │                 │             │
│   │[R] RawSpeed     │          │[R] FilteredSpeed│             │
│   │[P] FilteredSpeed├──────────┤                 │             │
│   └─────────────────┘          └─────────────────┘             │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 SpeedProcessor SWC

```c
/* SpeedProcessor.c */
#include "Rte_SpeedProcessor.h"

#define FILTER_COEFFICIENT  0.2f

static float32 filteredSpeed = 0.0f;

/* 10ms 주기 Runnable */
FUNC(void, SpeedProcessor_CODE) RE_ProcessSpeed(void)
{
    uint16 rawSpeed;
    Std_ReturnType status;
    
    /* CAN에서 수신된 속도 읽기 */
    status = Rte_Read_RP_RawSpeed_value(&rawSpeed);
    
    if (status == RTE_E_OK) {
        /* 1차 저역통과 필터 적용 */
        filteredSpeed = (FILTER_COEFFICIENT * (float32)rawSpeed) 
                      + ((1.0f - FILTER_COEFFICIENT) * filteredSpeed);
    }
    
    /* 필터링된 속도 출력 */
    (void)Rte_Write_PP_FilteredSpeed_value((uint16)filteredSpeed);
}
```

### 3.3 SpeedDisplay SWC

```c
/* SpeedDisplay.c */
#include "Rte_SpeedDisplay.h"

/* 100ms 주기 Runnable */
FUNC(void, SpeedDisplay_CODE) RE_DisplaySpeed(void)
{
    uint16 speed;
    
    /* 필터링된 속도 읽기 */
    (void)Rte_Read_RP_FilteredSpeed_value(&speed);
    
    /* 속도에 따른 처리 */
    if (speed > 120) {
        /* 과속 경고 */
        SetWarningLight(TRUE);
    } else {
        SetWarningLight(FALSE);
    }
    
    /* 디스플레이 업데이트 */
    UpdateSpeedDisplay(speed);
}
```

---

## 4. 프로젝트 3: 진단 서비스 구현

### 4.1 프로젝트 개요

UDS 진단 서비스(Read Data By Identifier)를 구현합니다.

### 4.2 Dcm Callout 구현

```c
/* Dcm_Callout.c */
#include "Dcm.h"

/* DID 정의 */
#define DID_VEHICLE_SPEED     0xF190
#define DID_ENGINE_TEMP       0xF191
#define DID_SOFTWARE_VERSION  0xF195

/* Read Data By Identifier (0x22) Callout */
FUNC(Std_ReturnType, DCM_CODE) Dcm_ReadDid_VehicleSpeed(
    Dcm_OpStatusType OpStatus,
    P2VAR(uint8, AUTOMATIC, DCM_VAR) Data,
    P2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_VAR) ErrorCode
)
{
    uint16 speed;
    
    /* 현재 차량 속도 읽기 */
    speed = GetCurrentVehicleSpeed();
    
    /* 데이터 포맷팅 (Big Endian) */
    Data[0] = (uint8)(speed >> 8);
    Data[1] = (uint8)(speed & 0xFF);
    
    return E_OK;
}

/* Read Data By Identifier - Software Version */
FUNC(Std_ReturnType, DCM_CODE) Dcm_ReadDid_SoftwareVersion(
    Dcm_OpStatusType OpStatus,
    P2VAR(uint8, AUTOMATIC, DCM_VAR) Data,
    P2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_VAR) ErrorCode
)
{
    /* SW 버전: "V1.0.0" */
    Data[0] = 'V';
    Data[1] = '1';
    Data[2] = '.';
    Data[3] = '0';
    Data[4] = '.';
    Data[5] = '0';
    
    return E_OK;
}
```

### 4.3 Dem 연동

```c
/* 센서 모니터링 및 DTC 리포트 */
FUNC(void, SWC_CODE) RE_MonitorSensor(void)
{
    uint16 sensorValue = ReadSensorValue();
    
    /* 센서 값 범위 검사 */
    if (sensorValue > SENSOR_HIGH_THRESHOLD) {
        /* 고장 발생 - Failed 리포트 */
        Dem_SetEventStatus(
            DemConf_DemEventParameter_SensorOutOfRange,
            DEM_EVENT_STATUS_FAILED
        );
    }
    else if (sensorValue < SENSOR_LOW_THRESHOLD) {
        /* 저전압 고장 */
        Dem_SetEventStatus(
            DemConf_DemEventParameter_SensorLowVoltage,
            DEM_EVENT_STATUS_FAILED
        );
    }
    else {
        /* 정상 - Passed 리포트 */
        Dem_SetEventStatus(
            DemConf_DemEventParameter_SensorOutOfRange,
            DEM_EVENT_STATUS_PASSED
        );
        Dem_SetEventStatus(
            DemConf_DemEventParameter_SensorLowVoltage,
            DEM_EVENT_STATUS_PASSED
        );
    }
}
```

---

## 5. 추가 학습 자료

### 5.1 공식 리소스

| 리소스 | URL | 설명 |
|--------|-----|------|
| AUTOSAR 공식 | https://www.autosar.org | 표준 문서 |
| Vector Academy | https://elearning.vector.com | E-Learning |
| Elektrobit | https://www.elektrobit.com | 기술 문서 |

### 5.2 추천 학습 경로

```
┌─────────────────────────────────────────────────────────────────┐
│                    추천 학습 경로                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1️⃣ 기초 (2-4주)                                                │
│     └── AUTOSAR 개념, 아키텍처 이해                            │
│                                                                 │
│  2️⃣ 도구 학습 (2-4주)                                           │
│     └── DaVinci Configurator 또는 EB tresos 사용법             │
│                                                                 │
│  3️⃣ SWC 개발 (2-4주)                                            │
│     └── 간단한 SWC 작성, RTE API 사용                          │
│                                                                 │
│  4️⃣ BSW 설정 (4-8주)                                            │
│     └── COM, NvM, OS 등 BSW 모듈 설정                          │
│                                                                 │
│  5️⃣ 통합/테스트 (4-8주)                                         │
│     └── 전체 ECU 통합, CAN 통신, 진단                          │
│                                                                 │
│  6️⃣ 심화 (지속)                                                 │
│     └── Adaptive AUTOSAR, 기능 안전                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 5.3 인증 및 자격

| 인증 | 발급 기관 | 내용 |
|------|----------|------|
| AUTOSAR Certified | AUTOSAR | 공식 인증 |
| Vector Academy | Vector | 도구 활용 |
| TÜV Certified | TÜV | 기능 안전 |

---

## 📝 마무리

이 강의를 통해 AUTOSAR의 기본 개념부터 실무 적용까지 학습하셨습니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                    학습 완료 체크리스트                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ☑️ AUTOSAR의 목적과 역사 이해                                   │
│  ☑️ Classic Platform 아키텍처 (ASW, RTE, BSW)                   │
│  ☑️ BSW 계층 구조 (MCAL, ECU Abs, Services)                    │
│  ☑️ SWC, Port, Interface 개념                                   │
│  ☑️ RTE 역할과 통신 메커니즘                                    │
│  ☑️ Communication Stack (CAN, COM)                             │
│  ☑️ Memory Stack (NvM, Fee)                                    │
│  ☑️ Diagnostic Stack (Dcm, Dem)                                │
│  ☑️ AUTOSAR 방법론과 ARXML                                     │
│  ☑️ 개발 도구 이해                                              │
│  ☑️ Adaptive Platform 개요                                     │
│  ☑️ 실습 프로젝트 수행                                          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**다음 단계:**
- 실제 도구로 프로젝트 실습
- 더 깊은 BSW 모듈 학습
- Adaptive AUTOSAR 심화 학습
- ISO 26262 기능 안전 학습

---

[← 이전 챕터](../Chapter_11_Adaptive_AUTOSAR/README.md) | [메인으로 돌아가기](../README.md)

---

<div align="center">

**🎉 AUTOSAR 강의를 완료하셨습니다! 🎉**

Made with ❤️ for Automotive Software Engineers

</div>
