# Chapter 04: Application Software Layer

[← 이전 챕터](../Chapter_03_BSW_Layers/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_05_RTE/README.md)

---

## 📋 목차

1. [학습 목표](#1-학습-목표)
2. [Application Layer 개요](#2-application-layer-개요)
3. [Software Component 상세](#3-software-component-상세)
4. [Internal Behavior](#4-internal-behavior)
5. [SWC 개발 실습](#5-swc-개발-실습)
6. [Composition과 시스템 설계](#6-composition과-시스템-설계)
7. [요약 및 연습 문제](#7-요약-및-연습-문제)

---

## 1. 학습 목표

```
✅ Application Layer의 역할과 구조 이해
✅ Software Component의 상세 구성 요소 학습
✅ Internal Behavior와 Runnable 개념 이해
✅ 실제 SWC 코드 작성 방법 습득
✅ Composition을 통한 시스템 설계 이해
```

---

## 2. Application Layer 개요

### 2.1 Application Layer란?

Application Layer는 차량의 **실제 기능을 구현**하는 소프트웨어 컴포넌트들의 집합입니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                   Application Layer 특징                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  🎯 하드웨어 독립성                                              │
│     └── RTE를 통해서만 하위 계층과 통신                         │
│                                                                 │
│  🔄 재사용성                                                     │
│     └── 동일 SWC를 다른 ECU/차량에 재사용 가능                  │
│                                                                 │
│  📦 컴포넌트 기반                                                │
│     └── 모듈화된 SW Component 단위로 개발                       │
│                                                                 │
│  📝 ARXML로 기술                                                 │
│     └── SWC 정의는 ARXML 파일로 기술됨                          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Application Layer 구성

```
┌─────────────────────────────────────────────────────────────────┐
│                   Application Layer 구성                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              Application Layer (ASW)                     │   │
│  │                                                          │   │
│  │   ┌─────────────────────────────────────────────────┐   │   │
│  │   │            Composition (Top-Level)               │   │   │
│  │   │                                                  │   │   │
│  │   │  ┌──────────┐  ┌──────────┐  ┌──────────┐      │   │   │
│  │   │  │   SWC    │  │   SWC    │  │   SWC    │      │   │   │
│  │   │  │ Sensor   │──│ Control  │──│ Actuator │      │   │   │
│  │   │  │          │  │          │  │          │      │   │   │
│  │   │  └──────────┘  └──────────┘  └──────────┘      │   │   │
│  │   │                                                  │   │   │
│  │   └─────────────────────────────────────────────────┘   │   │
│  │                                                          │   │
│  └─────────────────────────────────────────────────────────┘   │
│                              │                                  │
│                              │ RTE API                          │
│                              ▼                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                         RTE                              │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 3. Software Component 상세

### 3.1 SWC 종류

```
┌─────────────────────────────────────────────────────────────────┐
│                     SWC 종류 및 용도                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Application SW Component                                │   │
│  │  - 일반적인 애플리케이션 로직 구현                       │   │
│  │  - 예: 토크 계산, 속도 제어, 상태 머신                   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Sensor/Actuator SW Component                            │   │
│  │  - 센서 입력 처리 또는 액추에이터 출력 제어              │   │
│  │  - IoHwAb와 직접 연결                                    │   │
│  │  - 예: 온도 센서 SWC, 모터 제어 SWC                      │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Service SW Component                                    │   │
│  │  - BSW 서비스를 Application에 제공                       │   │
│  │  - NvM, Dem 등 BSW 모듈의 래퍼 역할                      │   │
│  │  - 예: NvMProxy, DemProxy                                │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Parameter SW Component                                  │   │
│  │  - 캘리브레이션 파라미터 제공                            │   │
│  │  - 읽기 전용 데이터 제공                                 │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 SWC 구성 요소

```
┌─────────────────────────────────────────────────────────────────┐
│                    SWC 구성 요소                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│            ┌───────────────────────────────────┐               │
│            │      Software Component           │               │
│  ┌─────────┼───────────────────────────────────┼─────────┐    │
│  │         │                                   │         │    │
│  │ P-Port  │                                   │ R-Port  │    │
│  │         │                                   │         │    │
│  └─────────┼───────────────────────────────────┼─────────┘    │
│            │                                   │               │
│            │  ┌─────────────────────────────┐ │               │
│            │  │     Internal Behavior       │ │               │
│            │  │                             │ │               │
│            │  │  ┌────────────────────┐    │ │               │
│            │  │  │ Runnable Entities  │    │ │               │
│            │  │  │  - RE_Init         │    │ │               │
│            │  │  │  - RE_Cyclic       │    │ │               │
│            │  │  │  - RE_Event        │    │ │               │
│            │  │  └────────────────────┘    │ │               │
│            │  │                             │ │               │
│            │  │  ┌────────────────────┐    │ │               │
│            │  │  │ Inter-Runnable     │    │ │               │
│            │  │  │ Variables (IRV)    │    │ │               │
│            │  │  └────────────────────┘    │ │               │
│            │  │                             │ │               │
│            │  │  ┌────────────────────┐    │ │               │
│            │  │  │ Per-Instance       │    │ │               │
│            │  │  │ Memory             │    │ │               │
│            │  │  └────────────────────┘    │ │               │
│            │  │                             │ │               │
│            │  └─────────────────────────────┘ │               │
│            │                                   │               │
│            └───────────────────────────────────┘               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.3 Port 종류

| Port 종류 | 방향 | 설명 |
|----------|------|------|
| **P-Port** (Provide) | 출력 | 데이터/서비스 제공 |
| **R-Port** (Require) | 입력 | 데이터/서비스 요청 |
| **PR-Port** | 양방향 | 제공과 요청 동시 |

### 3.4 Interface 종류

| Interface | 용도 | 통신 방식 |
|-----------|------|----------|
| **Sender-Receiver** | 데이터 전송 | 비동기 (Signal) |
| **Client-Server** | 서비스 호출 | 동기/비동기 (Operation) |
| **Mode-Switch** | 모드 전환 알림 | 이벤트 기반 |
| **Parameter** | 파라미터 접근 | 읽기 전용 |
| **Trigger** | 트리거 전달 | 이벤트 |

---

## 4. Internal Behavior

### 4.1 Runnable Entity

**Runnable Entity**는 SWC 내에서 스케줄링 가능한 실행 단위입니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                    Runnable Entity 구성                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                   Runnable Entity                        │   │
│  │                                                          │   │
│  │   ┌────────────────────────────────────────────────┐    │   │
│  │   │  Symbol (Entry Point)                           │    │   │
│  │   │  - C 함수명                                     │    │   │
│  │   │  - 예: RE_CalculateTorque                       │    │   │
│  │   └────────────────────────────────────────────────┘    │   │
│  │                                                          │   │
│  │   ┌────────────────────────────────────────────────┐    │   │
│  │   │  RTE Event (Trigger)                            │    │   │
│  │   │  - TimingEvent: 주기적 실행                     │    │   │
│  │   │  - DataReceivedEvent: 데이터 수신 시            │    │   │
│  │   │  - OperationInvokedEvent: 서비스 호출 시        │    │   │
│  │   │  - ModeSwitchEvent: 모드 변경 시                │    │   │
│  │   └────────────────────────────────────────────────┘    │   │
│  │                                                          │   │
│  │   ┌────────────────────────────────────────────────┐    │   │
│  │   │  Data Access                                    │    │   │
│  │   │  - 읽기/쓰기할 Port Data Element                │    │   │
│  │   │  - Inter-Runnable Variable 접근                 │    │   │
│  │   └────────────────────────────────────────────────┘    │   │
│  │                                                          │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 RTE Event 종류

| Event | 트리거 조건 | 사용 사례 |
|-------|------------|----------|
| **TimingEvent** | 주기적 (예: 10ms) | 센서 읽기, 제어 루프 |
| **DataReceivedEvent** | 데이터 수신 | CAN 메시지 처리 |
| **DataReceiveErrorEvent** | 수신 오류/타임아웃 | 오류 처리 |
| **OperationInvokedEvent** | C/S 호출 | 진단 서비스 |
| **ModeSwitchEvent** | 모드 전환 | 시동 ON/OFF |
| **InitEvent** | 초기화 시 | 초기 설정 |

### 4.3 Data Access 종류

```
┌─────────────────────────────────────────────────────────────────┐
│                   Data Access 종류                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Explicit Data Access                                        │
│     - Runnable 내에서 명시적으로 API 호출                       │
│     - Rte_Read_xxx(), Rte_Write_xxx()                          │
│                                                                 │
│  2. Implicit Data Access                                        │
│     - RTE가 Task 시작/종료 시 자동으로 읽기/쓰기               │
│     - Runnable 코드에서는 전역 변수처럼 접근                    │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │           Explicit vs Implicit 비교                      │   │
│  ├─────────────────────────────────────────────────────────┤   │
│  │  Explicit:                                               │   │
│  │  - 유연한 타이밍 제어                                    │   │
│  │  - 여러 번 읽기/쓰기 가능                                │   │
│  │  - 코드 복잡도 증가                                      │   │
│  │                                                          │   │
│  │  Implicit:                                               │   │
│  │  - 간단한 코드                                           │   │
│  │  - 데이터 일관성 보장                                    │   │
│  │  - Task 실행 중 값 불변                                  │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 5. SWC 개발 실습

### 5.1 예제: 엔진 토크 제어 SWC

#### ARXML 정의 (간략화)

```xml
<APPLICATION-SW-COMPONENT-TYPE>
  <SHORT-NAME>TorqueControl</SHORT-NAME>
  
  <!-- Ports -->
  <PORTS>
    <!-- 입력 포트: 스로틀 위치 -->
    <R-PORT-PROTOTYPE>
      <SHORT-NAME>RP_ThrottlePosition</SHORT-NAME>
      <REQUIRED-INTERFACE-TREF DEST="SENDER-RECEIVER-INTERFACE">
        /Interfaces/IF_ThrottlePosition
      </REQUIRED-INTERFACE-TREF>
    </R-PORT-PROTOTYPE>
    
    <!-- 입력 포트: 엔진 속도 -->
    <R-PORT-PROTOTYPE>
      <SHORT-NAME>RP_EngineSpeed</SHORT-NAME>
      <REQUIRED-INTERFACE-TREF DEST="SENDER-RECEIVER-INTERFACE">
        /Interfaces/IF_EngineSpeed
      </REQUIRED-INTERFACE-TREF>
    </R-PORT-PROTOTYPE>
    
    <!-- 출력 포트: 목표 토크 -->
    <P-PORT-PROTOTYPE>
      <SHORT-NAME>PP_TargetTorque</SHORT-NAME>
      <PROVIDED-INTERFACE-TREF DEST="SENDER-RECEIVER-INTERFACE">
        /Interfaces/IF_TargetTorque
      </PROVIDED-INTERFACE-TREF>
    </P-PORT-PROTOTYPE>
  </PORTS>
  
  <!-- Internal Behavior -->
  <INTERNAL-BEHAVIORS>
    <SWC-INTERNAL-BEHAVIOR>
      <SHORT-NAME>IB_TorqueControl</SHORT-NAME>
      
      <!-- Runnables -->
      <RUNNABLES>
        <RUNNABLE-ENTITY>
          <SHORT-NAME>RE_Init</SHORT-NAME>
          <SYMBOL>TorqueControl_Init</SYMBOL>
        </RUNNABLE-ENTITY>
        
        <RUNNABLE-ENTITY>
          <SHORT-NAME>RE_CalculateTorque</SHORT-NAME>
          <SYMBOL>TorqueControl_CalculateTorque</SYMBOL>
          <MINIMUM-START-INTERVAL>0.01</MINIMUM-START-INTERVAL>
        </RUNNABLE-ENTITY>
      </RUNNABLES>
      
      <!-- Events -->
      <EVENTS>
        <INIT-EVENT>
          <SHORT-NAME>InitEvent</SHORT-NAME>
          <START-ON-EVENT-REF DEST="RUNNABLE-ENTITY">
            RE_Init
          </START-ON-EVENT-REF>
        </INIT-EVENT>
        
        <TIMING-EVENT>
          <SHORT-NAME>TE_10ms</SHORT-NAME>
          <PERIOD>0.01</PERIOD>
          <START-ON-EVENT-REF DEST="RUNNABLE-ENTITY">
            RE_CalculateTorque
          </START-ON-EVENT-REF>
        </TIMING-EVENT>
      </EVENTS>
      
    </SWC-INTERNAL-BEHAVIOR>
  </INTERNAL-BEHAVIORS>
</APPLICATION-SW-COMPONENT-TYPE>
```

#### C 코드 구현

```c
/* TorqueControl.c */

#include "Rte_TorqueControl.h"

/* 상수 정의 */
#define TORQUE_COEFFICIENT  0.5f
#define MAX_TORQUE          500.0f
#define MIN_TORQUE          -100.0f

/* 내부 변수 */
static float32 lastTorque = 0.0f;

/**
 * @brief 초기화 Runnable
 */
FUNC(void, TorqueControl_CODE) TorqueControl_Init(void)
{
    /* 내부 변수 초기화 */
    lastTorque = 0.0f;
}

/**
 * @brief 토크 계산 Runnable (10ms 주기)
 */
FUNC(void, TorqueControl_CODE) TorqueControl_CalculateTorque(void)
{
    /* 지역 변수 선언 */
    float32 throttlePosition = 0.0f;
    uint16 engineSpeed = 0U;
    float32 targetTorque = 0.0f;
    Std_ReturnType status;
    
    /* ========================================
     * 1. 입력 데이터 읽기 (R-Port)
     * ======================================== */
    
    /* 스로틀 위치 읽기 */
    status = Rte_Read_RP_ThrottlePosition_value(&throttlePosition);
    if (status != RTE_E_OK) {
        /* 읽기 실패 시 기본값 사용 */
        throttlePosition = 0.0f;
    }
    
    /* 엔진 속도 읽기 */
    status = Rte_Read_RP_EngineSpeed_value(&engineSpeed);
    if (status != RTE_E_OK) {
        engineSpeed = 0U;
    }
    
    /* ========================================
     * 2. 토크 계산 알고리즘
     * ======================================== */
    
    /* 기본 토크 계산 */
    targetTorque = throttlePosition * (float32)engineSpeed * TORQUE_COEFFICIENT;
    
    /* 토크 제한 (Saturation) */
    if (targetTorque > MAX_TORQUE) {
        targetTorque = MAX_TORQUE;
    } else if (targetTorque < MIN_TORQUE) {
        targetTorque = MIN_TORQUE;
    }
    
    /* 급격한 변화 방지 (Rate Limiter) */
    if ((targetTorque - lastTorque) > 50.0f) {
        targetTorque = lastTorque + 50.0f;
    } else if ((targetTorque - lastTorque) < -50.0f) {
        targetTorque = lastTorque - 50.0f;
    }
    
    /* 이전 값 저장 */
    lastTorque = targetTorque;
    
    /* ========================================
     * 3. 출력 데이터 쓰기 (P-Port)
     * ======================================== */
    
    (void)Rte_Write_PP_TargetTorque_value(targetTorque);
}
```

#### 헤더 파일 (RTE 생성)

```c
/* Rte_TorqueControl.h (RTE Generator가 자동 생성) */

#ifndef RTE_TORQUECONTROL_H
#define RTE_TORQUECONTROL_H

#include "Rte_Type.h"

/* ========================================
 * RTE Read API (R-Port)
 * ======================================== */
 
extern FUNC(Std_ReturnType, RTE_CODE) 
    Rte_Read_TorqueControl_RP_ThrottlePosition_value(
        P2VAR(float32, AUTOMATIC, RTE_APPL_DATA) data
    );

extern FUNC(Std_ReturnType, RTE_CODE) 
    Rte_Read_TorqueControl_RP_EngineSpeed_value(
        P2VAR(uint16, AUTOMATIC, RTE_APPL_DATA) data
    );

/* Macro 정의 */
#define Rte_Read_RP_ThrottlePosition_value(data) \
    Rte_Read_TorqueControl_RP_ThrottlePosition_value(data)

#define Rte_Read_RP_EngineSpeed_value(data) \
    Rte_Read_TorqueControl_RP_EngineSpeed_value(data)

/* ========================================
 * RTE Write API (P-Port)
 * ======================================== */

extern FUNC(Std_ReturnType, RTE_CODE) 
    Rte_Write_TorqueControl_PP_TargetTorque_value(
        float32 data
    );

#define Rte_Write_PP_TargetTorque_value(data) \
    Rte_Write_TorqueControl_PP_TargetTorque_value(data)

#endif /* RTE_TORQUECONTROL_H */
```

---

## 6. Composition과 시스템 설계

### 6.1 Composition 개념

**Composition**은 여러 SWC를 논리적으로 그룹화하는 컨테이너입니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                    Composition 개념                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │         Composition: EngineControlSystem                 │   │
│  │                                                          │   │
│  │   ┌──────────────────────────────────────────────────┐  │   │
│  │   │  Delegation Connector (외부 포트 연결)            │  │   │
│  │   └──────────────────────────────────────────────────┘  │   │
│  │              │                              │            │   │
│  │   ┌──────────▼──────────┐        ┌─────────▼─────────┐  │   │
│  │   │  ThrottleSensor     │        │  SpeedSensor      │  │   │
│  │   │       SWC           │        │       SWC         │  │   │
│  │   │                     │        │                   │  │   │
│  │   │  [P] ThrottlePos    │        │  [P] EngineSpeed  │  │   │
│  │   └──────────┬──────────┘        └─────────┬─────────┘  │   │
│  │              │                              │            │   │
│  │              │    Assembly Connector       │            │   │
│  │              │    (SWC 간 연결)            │            │   │
│  │              │                              │            │   │
│  │              ▼                              ▼            │   │
│  │   ┌─────────────────────────────────────────────────┐   │   │
│  │   │              TorqueControl SWC                   │   │   │
│  │   │                                                  │   │   │
│  │   │  [R] ThrottlePos  [R] EngineSpeed  [P] Torque   │   │   │
│  │   └──────────────────────────────────┬──────────────┘   │   │
│  │                                      │                   │   │
│  │                                      ▼                   │   │
│  │   ┌──────────────────────────────────────────────────┐  │   │
│  │   │  Delegation Connector (외부 포트 연결)            │  │   │
│  │   └──────────────────────────────────────────────────┘  │   │
│  │                                                          │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 6.2 Connector 종류

| Connector | 용도 | 연결 대상 |
|-----------|------|----------|
| **Assembly Connector** | SWC 간 연결 | SWC ↔ SWC |
| **Delegation Connector** | 내외부 포트 연결 | Composition Port ↔ SWC Port |

### 6.3 ECU 매핑

```
┌─────────────────────────────────────────────────────────────────┐
│                    ECU 매핑 개념                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│     System Design (VFB Level)                                   │
│     ══════════════════════════                                  │
│                                                                 │
│     ┌─────────┐     ┌─────────┐     ┌─────────┐               │
│     │  SWC A  │─────│  SWC B  │─────│  SWC C  │               │
│     └─────────┘     └─────────┘     └─────────┘               │
│                                                                 │
│                          │                                      │
│                    ECU Mapping                                  │
│                          │                                      │
│                          ▼                                      │
│                                                                 │
│     ECU Configuration                                           │
│     ═════════════════════                                       │
│                                                                 │
│     ┌─────────────────┐       ┌─────────────────┐              │
│     │     ECU 1       │       │     ECU 2       │              │
│     │                 │       │                 │              │
│     │  ┌───────────┐  │  CAN  │  ┌───────────┐  │              │
│     │  │  SWC A    │  │◄─────►│  │  SWC C    │  │              │
│     │  │  SWC B    │  │       │  │           │  │              │
│     │  └───────────┘  │       │  └───────────┘  │              │
│     │                 │       │                 │              │
│     └─────────────────┘       └─────────────────┘              │
│                                                                 │
│  - SWC는 VFB 레벨에서 설계                                      │
│  - ECU 매핑은 배치 단계에서 결정                                │
│  - 동일 SWC를 다른 ECU에 재배치 가능                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 7. 요약 및 연습 문제

### 핵심 정리

```
┌─────────────────────────────────────────────────────────────────┐
│                    Chapter 04 핵심 정리                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📌 Application Layer:                                          │
│     ├── 차량의 실제 기능을 구현하는 SWC들의 집합               │
│     └── RTE를 통해서만 하위 계층과 통신 (하드웨어 독립)        │
│                                                                 │
│  📌 SWC 구성 요소:                                               │
│     ├── Port (P-Port, R-Port): 통신 접점                       │
│     ├── Internal Behavior: 내부 동작 정의                      │
│     └── Runnable Entity: 실행 가능한 코드 단위                 │
│                                                                 │
│  📌 RTE Event 종류:                                              │
│     ├── TimingEvent: 주기적 실행                               │
│     ├── DataReceivedEvent: 데이터 수신 시                      │
│     └── OperationInvokedEvent: C/S 호출 시                     │
│                                                                 │
│  📌 Composition:                                                 │
│     ├── 여러 SWC를 논리적으로 그룹화                           │
│     ├── Assembly Connector: SWC 간 연결                        │
│     └── Delegation Connector: 외부 포트 연결                   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 연습 문제

**문제 1.** Application SWC와 Sensor/Actuator SWC의 차이점은?

<details>
<summary>정답</summary>

- **Application SWC**: 일반적인 애플리케이션 로직 구현 (토크 계산, 상태 머신 등)
- **Sensor/Actuator SWC**: 센서 입력 처리 또는 액추에이터 출력 제어, IoHwAb와 직접 연결됨
</details>

**문제 2.** Runnable이 10ms 주기로 실행되도록 하려면 어떤 Event를 사용해야 하나요?

<details>
<summary>정답</summary>

**TimingEvent**를 사용하고 PERIOD를 0.01 (10ms)로 설정합니다.
</details>

**문제 3.** Explicit Data Access와 Implicit Data Access의 차이점은?

<details>
<summary>정답</summary>

- **Explicit**: Runnable 내에서 Rte_Read/Write API를 명시적으로 호출
- **Implicit**: RTE가 Task 시작/종료 시 자동으로 데이터를 읽고/쓰며, Runnable에서는 전역 변수처럼 접근
</details>

---

[← 이전 챕터](../Chapter_03_BSW_Layers/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_05_RTE/README.md)
