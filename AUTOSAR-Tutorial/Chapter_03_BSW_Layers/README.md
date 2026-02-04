# Chapter 03: Basic Software (BSW) 계층

[← 이전 챕터](../Chapter_02_Classic_Architecture/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_04_Application_Software/README.md)

---

## 📋 목차

1. [학습 목표](#1-학습-목표)
2. [BSW 개요](#2-bsw-개요)
3. [MCAL](#3-mcal-microcontroller-abstraction-layer)
4. [ECU Abstraction Layer](#4-ecu-abstraction-layer)
5. [Services Layer](#5-services-layer)
6. [Complex Device Drivers](#6-complex-device-drivers-cdd)
7. [요약 및 연습 문제](#7-요약-및-연습-문제)

---

## 1. 학습 목표

```
✅ BSW의 역할과 계층 구조 이해
✅ MCAL의 구성 모듈과 역할 파악
✅ ECU Abstraction Layer의 기능 이해
✅ Services Layer의 주요 서비스 모듈 학습
✅ Complex Device Drivers의 필요성과 사용 사례 이해
```

---

## 2. BSW 개요

### 2.1 BSW란?

**BSW(Basic Software)** 는 하드웨어를 추상화하고 표준 서비스를 제공하는 AUTOSAR의 핵심 계층입니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                      BSW 계층 상세 구조                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│                          RTE                                    │
│  ───────────────────────────────────────────────────────────── │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                   Services Layer                         │   │
│  │  ┌─────────┬─────────┬─────────┬─────────┬─────────┐   │   │
│  │  │   OS    │  COM    │  NvM    │   Dem   │  BswM   │   │   │
│  │  └─────────┴─────────┴─────────┴─────────┴─────────┘   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │               ECU Abstraction Layer                      │   │
│  │  ┌─────────┬─────────┬─────────┬─────────┬─────────┐   │   │
│  │  │ IoHwAb  │ ComM    │ EcuM    │ CanIf   │  MemIf  │   │   │
│  │  └─────────┴─────────┴─────────┴─────────┴─────────┘   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │        Microcontroller Abstraction Layer (MCAL)          │   │
│  │  ┌─────────┬─────────┬─────────┬─────────┬─────────┐   │   │
│  │  │   Dio   │   Adc   │   Pwm   │   Can   │   Fls   │   │   │
│  │  └─────────┴─────────┴─────────┴─────────┴─────────┘   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ───────────────────────────────────────────────────────────── │
│                       Microcontroller                           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 BSW 기능 그룹

| 스택 | 주요 모듈 | 기능 |
|------|----------|------|
| **System** | OS, EcuM, BswM, Det | 시스템 관리 |
| **Communication** | COM, PduR, CanIf, CanSM | 통신 처리 |
| **Memory** | NvM, MemIf, Fee, Fls | 비휘발성 메모리 관리 |
| **Diagnostic** | Dcm, Dem, FiM | 진단 처리 |
| **I/O** | IoHwAb, Port, Dio, Adc | I/O 처리 |

---

## 3. MCAL (Microcontroller Abstraction Layer)

### 3.1 MCAL 개요

**MCAL**은 마이크로컨트롤러 하드웨어에 직접 접근하는 최하위 계층입니다.

- **하드웨어 종속적**: MCU에 직접 종속 (MCU 벤더가 제공)
- **레지스터 직접 접근**: MCU 레지스터를 직접 제어
- **표준화된 API**: 상위 계층에 표준 인터페이스 제공

### 3.2 주요 MCAL 모듈

| 모듈 | 전체 이름 | 기능 |
|------|----------|------|
| **Mcu** | Microcontroller Unit | 클럭, 리셋, 저전력 모드 |
| **Port** | Port Driver | GPIO 핀 방향/모드 설정 |
| **Dio** | Digital I/O | GPIO 읽기/쓰기 |
| **Adc** | Analog to Digital | 아날로그 입력 변환 |
| **Pwm** | Pulse Width Modulation | PWM 출력 생성 |
| **Can** | CAN Driver | CAN 통신 |
| **Spi** | SPI Driver | SPI 통신 |
| **Fls** | Flash Driver | 내장 플래시 R/W |

### 3.3 MCAL API 예시

```c
/* Dio (Digital I/O) Driver API */

/* 채널 읽기 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId);

/* 채널 쓰기 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/* 사용 예시: LED 토글 */
void ToggleLED(void)
{
    Dio_LevelType currentLevel;
    currentLevel = Dio_ReadChannel(DioConf_DioChannel_LED1);
    
    if (currentLevel == STD_HIGH) {
        Dio_WriteChannel(DioConf_DioChannel_LED1, STD_LOW);
    } else {
        Dio_WriteChannel(DioConf_DioChannel_LED1, STD_HIGH);
    }
}
```

```c
/* ADC Driver API */

/* ADC 그룹 변환 시작 */
void Adc_StartGroupConversion(Adc_GroupType Group);

/* 사용 예시: 온도 센서 읽기 */
Adc_ValueGroupType adcBuffer[4];

void ReadTemperature(void)
{
    Adc_SetupResultBuffer(AdcConf_AdcGroup_TempSensor, adcBuffer);
    Adc_StartGroupConversion(AdcConf_AdcGroup_TempSensor);
    
    /* 변환 완료 후 adcBuffer에서 결과 읽기 */
}
```

---

## 4. ECU Abstraction Layer

### 4.1 개요

ECU Abstraction Layer는 **MCU와 ECU 하드웨어의 차이를 추상화**합니다.

- **MCAL**: MCU 내부 주변장치 추상화 (예: 내장 CAN)
- **ECU Abstraction**: ECU 보드 레벨 하드웨어 추상화 (예: 외부 EEPROM)

### 4.2 주요 모듈

| 모듈 | 전체 이름 | 기능 |
|------|----------|------|
| **CanIf** | CAN Interface | CAN 드라이버 추상화 |
| **MemIf** | Memory Interface | 메모리 드라이버 추상화 |
| **Fee** | Flash EEPROM Emulation | Flash를 EEPROM처럼 사용 |
| **IoHwAb** | I/O HW Abstraction | 센서/액추에이터 추상화 |
| **WdgIf** | Watchdog Interface | Watchdog 추상화 |

### 4.3 IoHwAb 예시

```c
/* IoHwAb - 온도 센서 추상화 */

FUNC(Std_ReturnType, IOHWAB_CODE) IoHwAb_GetTemperature(
    P2VAR(float32, AUTOMATIC, IOHWAB_APPL_DATA) Temperature
)
{
    Adc_ValueGroupType adcValue;
    float32 voltage, tempCelsius;
    
    /* ADC 값 읽기 (MCAL 호출) */
    Adc_ReadGroup(AdcConf_AdcGroup_TempSensor, &adcValue);
    
    /* ADC → 전압 → 온도 변환 */
    voltage = ((float32)adcValue / 4095.0f) * 5.0f;
    tempCelsius = (voltage * 100.0f) - 40.0f;
    
    *Temperature = tempCelsius;
    return E_OK;
}
```

---

## 5. Services Layer

### 5.1 개요

Services Layer는 **운영체제, 통신, 메모리, 진단 등의 고수준 서비스**를 제공합니다.

### 5.2 주요 서비스 모듈

#### OS (Operating System)

- **기반**: OSEK/VDX 표준
- **기능**: Task 스케줄링, Interrupt 관리, Event/Alarm

```c
/* Task 정의 */
TASK(Task_10ms)
{
    RE_CalculateTorque();
    TerminateTask();
}

/* Alarm Callback */
ALARMCALLBACK(Alarm_10ms)
{
    ActivateTask(Task_10ms);
}
```

#### EcuM (ECU State Manager)

```
     STARTUP → UP ←→ SLEEP → SHUTDOWN
                 └──────────────┘
```

- **기능**: ECU 상태 관리 (시작, 실행, 슬립, 종료)

#### NvM (Non-Volatile Memory Manager)

```c
/* NvM 사용 예시 */

/* 비동기 읽기 */
NvM_ReadBlock(NvMConf_NvMBlockDescriptor_CalibData, &calibData);

/* 비동기 쓰기 */
NvM_WriteBlock(NvMConf_NvMBlockDescriptor_CalibData, &calibData);

/* 완료 확인 */
NvM_RequestResultType result;
NvM_GetErrorStatus(NvMConf_NvMBlockDescriptor_CalibData, &result);
```

#### COM (Communication)

```c
/* Signal 송신 */
Com_SendSignal(ComConf_ComSignal_VehicleSpeed, &speed);

/* Signal 수신 */
Com_ReceiveSignal(ComConf_ComSignal_VehicleSpeed, &speed);
```

---

## 6. Complex Device Drivers (CDD)

### 6.1 CDD란?

**CDD**는 AUTOSAR 표준 BSW로 지원되지 않는 복잡한 하드웨어를 위한 드라이버입니다.

### 6.2 CDD 사용 사례

| 사례 | 설명 |
|------|------|
| **특수 센서** | 레이더, 라이다, 카메라 |
| **성능 최적화** | BSW 오버헤드 제거 필요 시 |
| **레거시 통합** | 기존 Non-AUTOSAR 드라이버 |
| **안전 기능** | ASIL-D 특수 구현 |

### 6.3 CDD 아키텍처 위치

```
    RTE
     │
     ▼
┌─────────┐     ┌─────────┐
│Services │     │   CDD   │ ← 모든 계층 우회 가능
│ Layer   │     │         │
└────┬────┘     └────┬────┘
     │               │
     ▼               │
┌─────────┐          │
│ECU Abs. │          │ Direct
│ Layer   │          │ Access
└────┬────┘          │
     │               │
     ▼               │
┌─────────┐          │
│  MCAL   │◄─────────┘
└────┬────┘
     │
     ▼
 Hardware
```

---

## 7. 요약 및 연습 문제

### 핵심 정리

```
┌─────────────────────────────────────────────────────────────────┐
│                    Chapter 03 핵심 정리                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📌 BSW 3계층 구조:                                              │
│     ├── Services Layer (OS, COM, NvM, Dem 등)                  │
│     ├── ECU Abstraction Layer (CanIf, MemIf, IoHwAb 등)        │
│     └── MCAL (Dio, Adc, Can, Fls 등)                           │
│                                                                 │
│  📌 MCAL 특징:                                                   │
│     ├── MCU 하드웨어에 직접 종속                                │
│     └── 표준화된 API로 상위 계층에 서비스 제공                  │
│                                                                 │
│  📌 주요 통신 스택:                                              │
│     └── COM → PduR → CanIf → Can                               │
│                                                                 │
│  📌 주요 메모리 스택:                                            │
│     └── NvM → MemIf → Fee/Ea → Fls/Eep                         │
│                                                                 │
│  📌 CDD: 표준 BSW로 지원되지 않는 하드웨어용                     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 연습 문제

**문제 1.** BSW의 3개 계층을 하위에서 상위 순서로 나열하세요.

<details>
<summary>정답</summary>

1. MCAL (Microcontroller Abstraction Layer)
2. ECU Abstraction Layer
3. Services Layer
</details>

**문제 2.** CAN 메시지 송신 시 데이터가 거치는 BSW 모듈 순서는?

<details>
<summary>정답</summary>

COM → PduR → CanIf → Can (MCAL) → CAN Bus
</details>

**문제 3.** CDD가 필요한 경우를 2가지 이상 쓰세요.

<details>
<summary>정답</summary>

1. 표준에 없는 하드웨어 (레이더, 라이다)
2. 성능 최적화 (BSW 오버헤드 제거)
3. 레거시 소프트웨어 통합
4. ASIL-D 안전 기능 특수 구현
</details>

---

[← 이전 챕터](../Chapter_02_Classic_Architecture/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_04_Application_Software/README.md)
