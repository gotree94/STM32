# STM32 Data Acquisition 장비 - NI LabVIEW 호환 시스템

STM32 기반 Data Acquisition(DAQ) 장비를 NI LabVIEW 및 표준 계측 소프트웨어와 호환되도록 개발하는 가이드입니다.

## 📋 개요

| 항목 | 내용 |
|------|------|
| 목표 | LabVIEW, MATLAB, Python에서 사용 가능한 DAQ 장비 |
| 프로토콜 | **USBTMC (USB Test & Measurement Class)** |
| 명령어 | **SCPI (Standard Commands for Programmable Instruments)** |
| 드라이버 | **NI-VISA** (National Instruments) |

## 🎯 시스템 구성

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    LabVIEW 호환 DAQ 시스템 구성                              │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   STM32F767 DAQ Device                     PC                               │
│   ┌─────────────────────────┐             ┌─────────────────────────┐      │
│   │                         │             │                         │      │
│   │  ┌─────────────────┐    │             │    NI-VISA Driver       │      │
│   │  │ Analog Inputs   │    │             │    (필수 설치)          │      │
│   │  │ ADC × 8ch       │    │             │         │               │      │
│   │  └─────────────────┘    │             │         ▼               │      │
│   │                         │   USB       │    ┌─────────────┐      │      │
│   │  ┌─────────────────┐    │   TMC       │    │  LabVIEW    │      │      │
│   │  │ Digital I/O     │    │◀──────────▶│    │  MATLAB     │      │      │
│   │  │ GPIO × 16ch     │    │   SCPI     │    │  Python     │      │      │
│   │  └─────────────────┘    │   명령어    │    │  LabWindows │      │      │
│   │                         │             │    └─────────────┘      │      │
│   │  ┌─────────────────┐    │             │                         │      │
│   │  │ USBTMC Stack    │    │             │  SCPI 명령 예:          │      │
│   │  │ SCPI Parser     │    │             │  *IDN? → 장치 정보      │      │
│   │  └─────────────────┘    │             │  MEAS:VOLT? → 전압 측정 │      │
│   │                         │             │  CONF:DIG:DIR OUT       │      │
│   └─────────────────────────┘             └─────────────────────────┘      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 🔧 Option 1: USBTMC 프로토콜 (표준 방식)

### USBTMC란?

**USB Test & Measurement Class**는 IEEE 488.2 (GPIB) 표준을 USB로 구현한 것입니다.

| 항목 | 내용 |
|------|------|
| USB Class | 0xFE (Application Specific) |
| USB Subclass | 0x03 (USBTMC) |
| 프로토콜 | SCPI 명령어 |
| 드라이버 | NI-VISA, Keysight IO Libraries |

### PC 측 필요 소프트웨어

| 소프트웨어 | 용도 | 다운로드 |
|-----------|------|----------|
| **NI-VISA** | USB TMC 드라이버 | [ni.com/visa](https://www.ni.com/visa) |
| **NI MAX** | 장치 관리/테스트 | NI-VISA와 함께 설치 |
| LabVIEW | 계측 프로그램 개발 | 유료 라이선스 |
| Python + pyvisa | 무료 대안 | `pip install pyvisa` |

### STM32 USBTMC 구현

#### usbd_usbtmc.h

```c
#ifndef __USBD_USBTMC_H
#define __USBD_USBTMC_H

#include "usbd_ioreq.h"

/* USBTMC Class Codes */
#define USB_USBTMC_CLASS            0xFE
#define USB_USBTMC_SUBCLASS         0x03
#define USB_USBTMC_PROTOCOL         0x00
#define USB_USBTMC_USB488_PROTOCOL  0x01

/* USBTMC Request Codes */
#define USBTMC_INITIATE_ABORT_BULK_OUT      1
#define USBTMC_CHECK_ABORT_BULK_OUT_STATUS  2
#define USBTMC_INITIATE_ABORT_BULK_IN       3
#define USBTMC_CHECK_ABORT_BULK_IN_STATUS   4
#define USBTMC_INITIATE_CLEAR               5
#define USBTMC_CHECK_CLEAR_STATUS           6
#define USBTMC_GET_CAPABILITIES             7
#define USBTMC_INDICATOR_PULSE              64

/* USBTMC Message Types */
#define USBTMC_MSGID_DEV_DEP_MSG_OUT        1
#define USBTMC_MSGID_REQUEST_DEV_DEP_MSG_IN 2
#define USBTMC_MSGID_DEV_DEP_MSG_IN         2
#define USBTMC_MSGID_VENDOR_SPECIFIC_OUT    126
#define USBTMC_MSGID_VENDOR_SPECIFIC_IN     127

/* USBTMC Bulk Header */
typedef struct {
    uint8_t  MsgID;
    uint8_t  bTag;
    uint8_t  bTagInverse;
    uint8_t  Reserved;
    uint32_t TransferSize;
    uint8_t  bmTransferAttributes;
    uint8_t  Reserved2[3];
} USBTMC_BulkHeader_t;

/* Capabilities Structure */
typedef struct {
    uint8_t  USBTMC_status;
    uint8_t  Reserved1;
    uint8_t  bcdUSBTMC[2];
    uint8_t  InterfaceCapabilities;
    uint8_t  DeviceCapabilities;
    uint8_t  Reserved2[6];
    uint8_t  bcdUSB488[2];
    uint8_t  USB488InterfaceCapabilities;
    uint8_t  USB488DeviceCapabilities;
    uint8_t  Reserved3[8];
} USBTMC_Capabilities_t;

/* Function Prototypes */
uint8_t USBD_USBTMC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
uint8_t USBD_USBTMC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
uint8_t USBD_USBTMC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
uint8_t USBD_USBTMC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
uint8_t USBD_USBTMC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);

/* Application Callbacks */
void USBTMC_ProcessCommand(uint8_t *cmd, uint16_t len);
uint16_t USBTMC_GetResponse(uint8_t *buf);

#endif /* __USBD_USBTMC_H */
```

#### SCPI 명령어 파서

```c
/* scpi_parser.c */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "scpi_parser.h"
#include "main.h"

/* 장치 정보 */
#define IDN_MANUFACTURER    "MyCompany"
#define IDN_MODEL          "STM32-DAQ"
#define IDN_SERIAL         "SN001"
#define IDN_FIRMWARE       "1.0.0"

/* ADC 데이터 (외부에서 업데이트) */
extern volatile uint16_t adc_values[8];
extern volatile uint32_t digital_inputs;
extern volatile uint32_t digital_outputs;

/* 응답 버퍼 */
static char response_buffer[1024];
static uint16_t response_length = 0;

/**
  * @brief  SCPI 명령어 처리
  * @param  cmd: 명령어 문자열
  * @param  len: 명령어 길이
  */
void SCPI_ProcessCommand(const char *cmd, uint16_t len)
{
    char cmd_upper[256];
    
    // 대문자로 변환
    for (int i = 0; i < len && i < 255; i++)
    {
        cmd_upper[i] = toupper(cmd[i]);
    }
    cmd_upper[len] = '\0';
    
    // 끝의 개행 제거
    char *newline = strchr(cmd_upper, '\n');
    if (newline) *newline = '\0';
    newline = strchr(cmd_upper, '\r');
    if (newline) *newline = '\0';
    
    response_length = 0;
    response_buffer[0] = '\0';
    
    /* ===== IEEE 488.2 공통 명령어 ===== */
    
    // *IDN? - 장치 식별
    if (strcmp(cmd_upper, "*IDN?") == 0)
    {
        response_length = sprintf(response_buffer, 
            "%s,%s,%s,%s\n",
            IDN_MANUFACTURER, IDN_MODEL, IDN_SERIAL, IDN_FIRMWARE);
    }
    // *RST - 장치 리셋
    else if (strcmp(cmd_upper, "*RST") == 0)
    {
        // 설정 초기화
        response_length = 0;
    }
    // *CLS - 상태 클리어
    else if (strcmp(cmd_upper, "*CLS") == 0)
    {
        response_length = 0;
    }
    // *OPC? - 동작 완료 확인
    else if (strcmp(cmd_upper, "*OPC?") == 0)
    {
        response_length = sprintf(response_buffer, "1\n");
    }
    
    /* ===== 측정 명령어 ===== */
    
    // MEASure:VOLTage? [channel] - 전압 측정
    else if (strncmp(cmd_upper, "MEAS:VOLT?", 10) == 0 ||
             strncmp(cmd_upper, "MEASURE:VOLTAGE?", 16) == 0)
    {
        int channel = 0;
        char *param = strchr(cmd_upper, '?');
        if (param && *(param + 1) != '\0')
        {
            channel = atoi(param + 1);
        }
        
        if (channel >= 0 && channel < 8)
        {
            float voltage = (float)adc_values[channel] * 3.3f / 4096.0f;
            response_length = sprintf(response_buffer, "%.6f\n", voltage);
        }
        else
        {
            response_length = sprintf(response_buffer, "ERROR: Invalid channel\n");
        }
    }
    // MEASure:VOLTage:ALL? - 모든 채널 전압 측정
    else if (strcmp(cmd_upper, "MEAS:VOLT:ALL?") == 0 ||
             strcmp(cmd_upper, "MEASURE:VOLTAGE:ALL?") == 0)
    {
        response_length = 0;
        for (int i = 0; i < 8; i++)
        {
            float voltage = (float)adc_values[i] * 3.3f / 4096.0f;
            response_length += sprintf(response_buffer + response_length, 
                "%.6f%s", voltage, (i < 7) ? "," : "\n");
        }
    }
    // MEASure:ADC? [channel] - ADC 원시값
    else if (strncmp(cmd_upper, "MEAS:ADC?", 9) == 0)
    {
        int channel = 0;
        char *param = strchr(cmd_upper, '?');
        if (param && *(param + 1) != '\0')
        {
            channel = atoi(param + 1);
        }
        
        if (channel >= 0 && channel < 8)
        {
            response_length = sprintf(response_buffer, "%u\n", adc_values[channel]);
        }
    }
    
    /* ===== 디지털 I/O 명령어 ===== */
    
    // DIGital:INput? - 디지털 입력 읽기
    else if (strcmp(cmd_upper, "DIG:INP?") == 0 ||
             strcmp(cmd_upper, "DIGITAL:INPUT?") == 0)
    {
        response_length = sprintf(response_buffer, "%lu\n", digital_inputs);
    }
    // DIGital:OUTput <value> - 디지털 출력 설정
    else if (strncmp(cmd_upper, "DIG:OUTP ", 9) == 0 ||
             strncmp(cmd_upper, "DIGITAL:OUTPUT ", 15) == 0)
    {
        char *param = strchr(cmd_upper, ' ');
        if (param)
        {
            digital_outputs = strtoul(param + 1, NULL, 0);
            // GPIO 업데이트 함수 호출
            Update_Digital_Outputs(digital_outputs);
        }
    }
    // DIGital:OUTput? - 디지털 출력 읽기
    else if (strcmp(cmd_upper, "DIG:OUTP?") == 0 ||
             strcmp(cmd_upper, "DIGITAL:OUTPUT?") == 0)
    {
        response_length = sprintf(response_buffer, "%lu\n", digital_outputs);
    }
    
    /* ===== 설정 명령어 ===== */
    
    // CONFigure:SAMPle:RATE <rate> - 샘플링 레이트 설정
    else if (strncmp(cmd_upper, "CONF:SAMP:RATE ", 15) == 0)
    {
        char *param = strchr(cmd_upper, ' ');
        if (param)
        {
            uint32_t rate = atoi(param + 1);
            // 샘플링 레이트 설정 함수 호출
            Set_Sample_Rate(rate);
        }
    }
    // CONFigure:SAMPle:RATE? - 샘플링 레이트 조회
    else if (strcmp(cmd_upper, "CONF:SAMP:RATE?") == 0)
    {
        response_length = sprintf(response_buffer, "%lu\n", Get_Sample_Rate());
    }
    
    /* ===== 데이터 수집 명령어 ===== */
    
    // ACQuire:STARt - 데이터 수집 시작
    else if (strcmp(cmd_upper, "ACQ:START") == 0 ||
             strcmp(cmd_upper, "ACQUIRE:START") == 0)
    {
        Start_Acquisition();
    }
    // ACQuire:STOP - 데이터 수집 중지
    else if (strcmp(cmd_upper, "ACQ:STOP") == 0 ||
             strcmp(cmd_upper, "ACQUIRE:STOP") == 0)
    {
        Stop_Acquisition();
    }
    // ACQuire:DATA? - 수집된 데이터 읽기
    else if (strcmp(cmd_upper, "ACQ:DATA?") == 0 ||
             strcmp(cmd_upper, "ACQUIRE:DATA?") == 0)
    {
        response_length = Get_Acquisition_Data(response_buffer, sizeof(response_buffer));
    }
    
    /* ===== 시스템 명령어 ===== */
    
    // SYSTem:ERRor? - 에러 조회
    else if (strcmp(cmd_upper, "SYST:ERR?") == 0 ||
             strcmp(cmd_upper, "SYSTEM:ERROR?") == 0)
    {
        response_length = sprintf(response_buffer, "0,\"No error\"\n");
    }
    // SYSTem:VERSion? - SCPI 버전
    else if (strcmp(cmd_upper, "SYST:VERS?") == 0 ||
             strcmp(cmd_upper, "SYSTEM:VERSION?") == 0)
    {
        response_length = sprintf(response_buffer, "1999.0\n");
    }
    
    /* ===== 알 수 없는 명령어 ===== */
    else
    {
        response_length = sprintf(response_buffer, "ERROR: Unknown command\n");
    }
}

/**
  * @brief  응답 데이터 가져오기
  * @param  buf: 출력 버퍼
  * @retval 응답 길이
  */
uint16_t SCPI_GetResponse(uint8_t *buf)
{
    if (response_length > 0)
    {
        memcpy(buf, response_buffer, response_length);
    }
    return response_length;
}
```

### LabVIEW에서 사용

#### VISA Resource Name

```
USB0::0x0483::0x5750::SN001::INSTR
     │       │       │
     │       │       └── Serial Number
     │       └── Product ID (PID)
     └── Vendor ID (VID, STMicroelectronics)
```

#### LabVIEW VI 예제

```
┌─────────────────────────────────────────────────────────────────┐
│                    LabVIEW Block Diagram                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐           │
│   │ VISA Open  │───▶│ VISA Write │───▶│ VISA Read  │           │
│   │            │    │            │    │            │           │
│   │ Resource:  │    │ "MEAS:VOLT │    │ Response   │           │
│   │ USB0::... │    │  ?0"       │    │ "1.234567" │           │
│   └────────────┘    └────────────┘    └────────────┘           │
│         │                                    │                  │
│         │           ┌────────────┐           │                  │
│         └──────────▶│ VISA Close │◀──────────┘                  │
│                     └────────────┘                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Python에서 사용 (pyvisa)

```python
import pyvisa

# VISA Resource Manager
rm = pyvisa.ResourceManager()

# 장치 열기
daq = rm.open_resource('USB0::0x0483::0x5750::SN001::INSTR')

# 장치 정보 조회
print(daq.query('*IDN?'))
# 출력: MyCompany,STM32-DAQ,SN001,1.0.0

# 전압 측정
voltage = float(daq.query('MEAS:VOLT?0'))
print(f"Channel 0 Voltage: {voltage:.6f} V")

# 모든 채널 측정
all_voltages = daq.query('MEAS:VOLT:ALL?')
print(f"All Channels: {all_voltages}")

# 디지털 출력 설정
daq.write('DIG:OUTP 0xFF')

# 디지털 입력 읽기
digital_in = int(daq.query('DIG:INP?'))
print(f"Digital Input: {digital_in:016b}")

# 장치 닫기
daq.close()
```

### MATLAB에서 사용

```matlab
% VISA 객체 생성
daq = visa('ni', 'USB0::0x0483::0x5750::SN001::INSTR');

% 연결
fopen(daq);

% 장치 정보 조회
fprintf(daq, '*IDN?');
idn = fscanf(daq);
disp(['Device: ' idn]);

% 전압 측정
fprintf(daq, 'MEAS:VOLT?0');
voltage = str2double(fscanf(daq));
disp(['Voltage: ' num2str(voltage) ' V']);

% 연결 종료
fclose(daq);
delete(daq);
```

---

## 🔧 Option 2: NI-DAQmx 호환 (고급)

더 높은 수준의 NI 호환성을 원한다면 **NI-DAQmx** 드라이버와 호환되는 방식도 있습니다.

### 구성

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    NI-DAQmx 호환 방식 (복잡)                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   이 방식은 NI의 독점 프로토콜을 구현해야 하므로                            │
│   일반적으로 권장하지 않습니다.                                             │
│                                                                             │
│   대안:                                                                     │
│   1. USBTMC + SCPI (권장) - 표준 방식                                      │
│   2. NI USB-6001 등 실제 NI 하드웨어 사용                                  │
│   3. LabVIEW LINX Toolkit 사용 (Arduino/Raspberry Pi 지원)                │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 📊 성능 비교

| 방식 | 전송 속도 | 호환성 | 개발 난이도 |
|------|----------|--------|------------|
| USBTMC + SCPI | ~1 MB/s | LabVIEW, MATLAB, Python | ⭐⭐⭐ |
| CDC + 커스텀 프로토콜 | ~1 MB/s | 커스텀 소프트웨어 필요 | ⭐⭐ |
| USB HS + Bulk | ~20 MB/s | 커스텀 드라이버/소프트웨어 | ⭐⭐⭐⭐ |

---

## 📁 SCPI 명령어 레퍼런스

### IEEE 488.2 공통 명령어

| 명령어 | 설명 |
|--------|------|
| `*IDN?` | 장치 식별 정보 |
| `*RST` | 장치 리셋 |
| `*CLS` | 상태 클리어 |
| `*OPC?` | 동작 완료 확인 |
| `*TST?` | 자가 테스트 |

### 측정 명령어

| 명령어 | 설명 | 응답 예시 |
|--------|------|----------|
| `MEAS:VOLT?0` | 채널 0 전압 측정 | `1.234567` |
| `MEAS:VOLT:ALL?` | 모든 채널 전압 | `1.23,2.34,3.45,...` |
| `MEAS:ADC?0` | 채널 0 ADC 원시값 | `1530` |
| `DIG:INP?` | 디지털 입력 읽기 | `255` |
| `DIG:OUTP 0xFF` | 디지털 출력 설정 | (응답 없음) |
| `DIG:OUTP?` | 디지털 출력 읽기 | `255` |

### 설정 명령어

| 명령어 | 설명 |
|--------|------|
| `CONF:SAMP:RATE 1000` | 샘플링 레이트 1kHz |
| `CONF:SAMP:RATE?` | 샘플링 레이트 조회 |
| `CONF:CHAN:ENAB 0xFF` | 채널 활성화 |

### 데이터 수집 명령어

| 명령어 | 설명 |
|--------|------|
| `ACQ:START` | 데이터 수집 시작 |
| `ACQ:STOP` | 데이터 수집 중지 |
| `ACQ:DATA?` | 수집된 데이터 읽기 |
| `ACQ:COUNT?` | 수집된 샘플 수 |

---

## 🔍 트러블슈팅

### NI MAX에서 장치가 안 보임

- [ ] NI-VISA 드라이버 설치 확인
- [ ] USB TMC Class 코드 (0xFE, 0x03) 확인
- [ ] USB Descriptor 확인

### LabVIEW에서 통신 에러

- [ ] VISA Resource Name 형식 확인
- [ ] bTag/bTagInverse 처리 확인
- [ ] 응답 끝에 `\n` 포함 확인

### 응답이 잘림

- [ ] TransferSize 필드 확인
- [ ] EOM (End of Message) 비트 설정 확인

---

## 📚 참고 자료

- [USBTMC Specification](https://www.usb.org/document-library/test-measurement-class-specification)
- [SCPI Standard](https://www.ivifoundation.org/scpi/)
- [NI-VISA Documentation](https://www.ni.com/docs/en-US/bundle/ni-visa/page/ni-visa-overview.html)
- [pyvisa Documentation](https://pyvisa.readthedocs.io/)

## 📝 라이선스

This document is licensed under the MIT License.

## ✍️ Author

Created for STM32 Data Acquisition system development.
