# IR Remote Control Decoder

STM32F103 NUCLEO 보드를 이용한 IR 리모컨 신호 디코더 프로젝트

## 개요

이 프로젝트는 IR 리모컨에서 송신된 신호를 수신하여 NEC 프로토콜로 디코딩하고, 주소(Address)와 명령(Command) 코드를 시리얼 통신으로 PC에 전송합니다.

## 주요 기능

- NEC / NEC Extended 프로토콜 디코딩
- 타이머 Input Capture를 이용한 정밀한 타이밍 측정
- 리피트 코드 감지
- 체크섬 검증
- 일반 리모컨 키 이름 자동 인식

## 하드웨어 구성

### 필요 부품

| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103 개발보드 |
| IR Receiver Module | 1 | TSOP1838, VS1838B 등 |
| 점퍼 와이어 | 3 | 연결용 |

### 핀 연결

```
STM32F103 NUCLEO          IR Receiver Module
-----------------          ------------------
PA6 (TIM3_CH1) <--------- OUT (Signal)  ← Input Capture용
PA0           <--------- OUT (Signal)  ← 상태 확인용 (점퍼로 연결)
3.3V          ---------> VCC
GND           ---------> GND

PA5 (LED)    : 내장 LED (수신 표시)
PA2 (USART2_TX): USB Virtual COM (디버그 출력)
PA3 (USART2_RX): USB Virtual COM
```

> **참고:** TIM3_CH1은 PA6에 할당되어 있습니다. IR 수신 모듈의 OUT 핀을 PA6에 연결하거나, PA0과 PA6를 점퍼로 연결하세요.

### 회로도

```
              +3.3V
                |
         [10kΩ] (내장 풀업 사용)
                |
    PA6 <-------+----[OUT]
                |     [IR Receiver]
    PA0 <-------+    
                |
    GND --------+----[GND]
                |
    3.3V -------+----[VCC]
```

## NEC 프로토콜 상세

### 프레임 구조

```
┌──────────┬──────────┬─────────┬─────────────┬─────────┬─────────────┬──────┐
│  Leader  │  Leader  │ Address │ Address Inv │ Command │ Command Inv │ Stop │
│  Pulse   │  Space   │ (8-bit) │   (8-bit)   │ (8-bit) │   (8-bit)   │ Bit  │
│   9ms    │  4.5ms   │         │             │         │             │560us │
└──────────┴──────────┴─────────┴─────────────┴─────────┴─────────────┴──────┘

총 프레임 길이: 약 67.5ms
```

### 비트 인코딩

```
Logic 0:
    ┌───┐
    │   │         560us pulse + 560us space = 1.12ms
────┘   └─────

Logic 1:
    ┌───┐
    │   │             560us pulse + 1690us space = 2.25ms
────┘   └─────────
```

### 리피트 프레임

버튼을 계속 누르고 있으면 리피트 프레임이 전송됩니다:

```
┌──────────┬──────────┬──────┐
│  Leader  │  Repeat  │ Stop │
│  Pulse   │  Space   │ Bit  │
│   9ms    │  2.25ms  │560us │
└──────────┴──────────┴──────┘

리피트 간격: 108ms
```

## 소프트웨어 설명

### 상태 머신

```
                    ┌─────────────┐
                    │   IDLE      │
                    │  대기 상태   │
                    └──────┬──────┘
                           │ Falling Edge
                           ▼
                    ┌─────────────┐
                    │LEADER_PULSE │
                    │  9ms 펄스   │
                    └──────┬──────┘
                           │ Rising Edge (9ms 확인)
                           ▼
                    ┌─────────────┐
                    │LEADER_SPACE │◄──────────────┐
                    │ 4.5ms/2.25ms│               │
                    └──────┬──────┘               │
           ┌───────────────┼───────────────┐     │
           │               │               │     │
        4.5ms           2.25ms          Error   │
           ▼               ▼               │     │
    ┌─────────────┐ ┌─────────────┐        │     │
    │ DATA_PULSE  │ │   REPEAT    │        │     │
    │  비트 펄스   │ │  리피트 코드 │        │     │
    └──────┬──────┘ └─────────────┘        │     │
           │                               │     │
           ▼ (32비트 수신)                  │     │
    ┌─────────────┐                        │     │
    │  COMPLETE   │                        │     │
    │  디코딩 완료 │                        │     │
    └──────┬──────┘                        │     │
           │                               │     │
           └───────────────────────────────┴─────┘
                       Reset
```

### 주요 함수

| 함수 | 설명 |
|------|------|
| `HAL_TIM_IC_CaptureCallback()` | Input Capture 인터럽트 콜백 |
| `IR_Reset()` | 디코더 상태 초기화 |
| `IR_ProcessData()` | 원시 데이터 처리 및 구조체 저장 |
| `IR_GetKeyName()` | 키 코드를 이름으로 변환 |

### 타이밍 허용 오차

```c
#define TOLERANCE 25  // ±25%

// 예: NEC_LEADER_PULSE = 9000us
// 허용 범위: 6750us ~ 11250us
```

## 빌드 및 실행

### STM32CubeIDE 사용 시

1. 새 STM32 프로젝트 생성 (STM32F103RB 선택)
2. `main.c` 파일을 생성된 프로젝트에 복사
3. STM32F1xx HAL 드라이버 포함 확인
4. 빌드 후 플래시

### NVIC 설정 확인

```c
HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(TIM3_IRQn);
```

## 사용 방법

1. 보드에 프로그램 업로드
2. 시리얼 터미널 연결 (115200 baud)
3. IR 리모컨을 수신 모듈 방향으로 향하게 함
4. 리모컨 버튼 누르기
5. 터미널에서 디코딩된 값 확인

### 시리얼 출력 예시

```
================================================
   IR Remote Control Decoder - STM32F103
================================================
Protocol: NEC / NEC Extended
IR Receiver Pin: PA0
------------------------------------------------
Point your IR remote at the receiver and
press any button to see the decoded values.
------------------------------------------------

[1] Received IR Code:
       Address:     0x00 (inv: 0xFF)
       Command:     0x16 (inv: 0xE9)
       Status:      Valid NEC Frame
       Key:         0

[2] REPEAT

[3] REPEAT

[4] Received IR Code:
       Address:     0x00 (inv: 0xFF)
       Command:     0x0C (inv: 0xF3)
       Status:      Valid NEC Frame
       Key:         1

[5] Received IR Code:
       Address:     0x00 (inv: 0xBF)
       Command:     0x45 (inv: 0xBA)
       Status:      Valid NEC Extended (16-bit addr)
       Full Addr:   0xBF00
```

## 출력 해석

| 필드 | 설명 |
|------|------|
| Address | 디바이스 주소 (8-bit) |
| Address Inv | 주소의 비트 반전 (검증용) |
| Command | 명령 코드 (8-bit) |
| Command Inv | 명령의 비트 반전 (검증용) |
| Status | NEC / NEC Extended / Error |
| Full Addr | 16비트 확장 주소 (NEC Extended) |
| Key | 인식된 키 이름 (일반 리모컨용) |

## 일반 리모컨 키 매핑

Address 0x00 (일반 IR 리모컨):

| Command | 키 이름 |
|---------|---------|
| 0x45 | CH- |
| 0x46 | CH |
| 0x47 | CH+ |
| 0x44 | PREV |
| 0x40 | NEXT |
| 0x43 | PLAY/PAUSE |
| 0x07 | VOL- |
| 0x15 | VOL+ |
| 0x09 | EQ |
| 0x16 | 0 |
| 0x0C | 1 |
| 0x18 | 2 |
| ... | ... |

## 커스터마이징

### 새 리모컨 추가

`IR_GetKeyName()` 함수에 새 리모컨의 주소와 명령 코드를 추가하세요:

```c
const char* IR_GetKeyName(uint8_t address, uint8_t command) {
    /* Samsung TV 리모컨 예시 */
    if (address == 0x07) {
        switch (command) {
            case 0x02: return "POWER";
            case 0x1A: return "VOL+";
            case 0x1E: return "VOL-";
            // ... 추가 키
        }
    }
    return NULL;
}
```

### 타이밍 조정

특정 리모컨이 잘 인식되지 않으면 타이밍 허용 오차를 조정하세요:

```c
#define TOLERANCE 30  // 25% -> 30%로 증가
```

## 문제 해결

| 증상 | 원인 | 해결 방법 |
|------|------|----------|
| 신호 미감지 | 핀 연결 오류 | PA6에 IR OUT 연결 확인 |
| 디코딩 실패 | 타이밍 오류 | TOLERANCE 값 증가 |
| Checksum Error | 신호 왜곡 | 거리 조정, 노이즈 확인 |
| 리피트만 출력 | 버튼 짧게 누름 | 버튼 더 오래 누르기 |
| 값이 이상함 | 다른 프로토콜 | RC5, Sony 등은 별도 구현 필요 |

## 지원 프로토콜

| 프로토콜 | 지원 여부 | 비고 |
|----------|----------|------|
| NEC | ✅ | 완전 지원 |
| NEC Extended | ✅ | 16비트 주소 |
| RC5 | ❌ | 별도 구현 필요 |
| RC6 | ❌ | 별도 구현 필요 |
| Sony SIRC | ❌ | 별도 구현 필요 |

## 확장 아이디어

1. **다중 프로토콜 지원**: RC5, Sony SIRC 추가
2. **학습 리모컨**: EEPROM에 코드 저장
3. **IR 블래스터**: 디코딩된 코드 재전송
4. **스마트홈 연동**: WiFi 모듈 추가하여 MQTT 전송

## 참고 자료

- [NEC Protocol Specification](https://www.sbprojects.net/knowledge/ir/nec.php)
- [IR Remote Control Theory](https://www.vishay.com/docs/80071/dataform.pdf)
- [STM32F103 Timer Input Capture](https://www.st.com/resource/en/application_note/an4013-stm32-crossseries-timer-overview-stmicroelectronics.pdf)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

## 라이선스

MIT License
