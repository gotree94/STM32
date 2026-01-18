# IR Receiver Module Test

STM32F103 NUCLEO 보드를 이용한 IR 수신 모듈 테스트 프로젝트

## 개요

이 프로젝트는 IR(적외선) 수신 모듈의 기본 동작을 테스트합니다. IR 신호가 감지되면 LED가 토글되고 펄스 정보가 시리얼로 출력됩니다.

## 하드웨어 구성

### 필요 부품

| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103 개발보드 |
| IR Receiver Module | 1 | TSOP1838, VS1838B 등 |

### 핀 연결

```
STM32F103 NUCLEO          IR Receiver Module
-----------------          ------------------
PA0           <--------- OUT (Signal)
3.3V          ---------> VCC
GND           ---------> GND

PA5 (LED)    : 내장 LED (신호 감지 표시)
PA2 (USART2_TX): USB Virtual COM (디버그 출력)
PA3 (USART2_RX): USB Virtual COM
```

### IR 수신 모듈 핀 배치

일반적인 IR 수신 모듈 (TSOP1838/VS1838B):

```
       +-----+
       |     |
       | ○○○ |  <- IR 수신부
       +-----+
        | | |
        G V S
        N C I
        D C G
            N
            A
            L

핀 순서 (모듈에 따라 다를 수 있음):
- TSOP1838: OUT - GND - VCC
- VS1838B:  OUT - VCC - GND
```

### 회로도

```
              +3.3V
                |
                |
         [10kΩ] (선택사항, 풀업)
                |
    PA0 <-------+----[OUT]
                      [IR Receiver]
    GND --------+----[GND]
                |
    3.3V -------+----[VCC]
```

## 소프트웨어 설명

### 동작 원리

1. IR 수신 모듈은 38kHz 변조된 IR 신호를 감지
2. 신호가 있으면 OUT 핀이 LOW
3. 신호가 없으면 OUT 핀이 HIGH (Active Low)
4. MCU는 펄스 수와 폭을 측정하여 출력

### 주요 변수

```c
#define SIGNAL_TIMEOUT_MS   100    // 신호 버스트 종료 감지 시간
#define NOISE_FILTER_US     100    // 노이즈 필터링 최소 펄스 폭
```

### 프로그램 흐름

```
시작
  ↓
GPIO/UART/TIM 초기화
  ↓
타이머 시작 (마이크로초 측정용)
  ↓
┌──────────────────────────┐
│  메인 루프               │
│  ├─ IR 핀 상태 읽기      │
│  ├─ 하강 에지 감지?      │
│  │   └─ 펄스 시작 기록   │
│  ├─ 상승 에지 감지?      │
│  │   └─ 펄스 폭 계산     │
│  └─ 타임아웃?            │
│      └─ 결과 출력        │
└──────────────────────────┘
```

## 빌드 및 실행

### STM32CubeIDE 사용 시

1. 새 STM32 프로젝트 생성 (STM32F103RB 선택)
2. `main.c` 파일을 생성된 프로젝트에 복사
3. STM32F1xx HAL 드라이버 포함 확인
4. 빌드 후 플래시

## 사용 방법

1. 보드에 프로그램 업로드
2. 시리얼 터미널 연결 (115200 baud)
3. IR 리모컨으로 수신 모듈을 향해 버튼 누르기
4. 터미널에서 수신 결과 확인

### 시리얼 출력 예시

```
========================================
   IR Receiver Module Test - STM32F103
========================================
IR Receiver Pin: PA0
Status LED: PA5 (On-board)
----------------------------------------
Waiting for IR signals...

[1] IR Signal Detected!
       Total Pulses: 34
       Pulse Width: 560 ~ 9000 us
       Signal burst ended

[2] IR Signal Detected!
       Total Pulses: 34
       Pulse Width: 560 ~ 9000 us
       Signal burst ended

[3] IR Signal Detected!
       Total Pulses: 2
       Pulse Width: 560 ~ 560 us
       Signal burst ended
```

## 출력 해석

| 항목 | 설명 |
|------|------|
| Total Pulses | 수신된 IR 펄스 총 개수 |
| Pulse Width Min | 가장 짧은 펄스 폭 (보통 비트 펄스) |
| Pulse Width Max | 가장 긴 펄스 폭 (보통 리더 펄스) |

### NEC 프로토콜 참고

```
NEC 프레임 = 34 펄스
- Leader: 1 pulse (9000us)
- Address: 8 bits = 8 pulses
- Address Inv: 8 bits = 8 pulses  
- Command: 8 bits = 8 pulses
- Command Inv: 8 bits = 8 pulses
- Stop: 1 pulse

Repeat 프레임 = 2 펄스
- Leader: 1 pulse (9000us pulse + 2250us space)
- Stop: 1 pulse
```

## 문제 해결

| 증상 | 원인 | 해결 방법 |
|------|------|----------|
| 신호 미감지 | 핀 연결 오류 | VCC/GND/OUT 핀 순서 확인 |
| 노이즈 검출 | 환경광 영향 | 수신부를 가리거나 방향 조정 |
| 펄스 수 불일치 | 거리 너무 멈 | 리모컨을 가까이 |
| LED 안 깜빡임 | 코드 오류 | GPIO 초기화 확인 |

## 지원 IR 수신 모듈

| 모듈 | 캐리어 주파수 | 전압 |
|------|--------------|------|
| TSOP1838 | 38kHz | 2.5-5.5V |
| VS1838B | 38kHz | 2.7-5.5V |
| TSOP1736 | 36kHz | 2.5-5.5V |
| TSOP1740 | 40kHz | 2.5-5.5V |

## 확장 아이디어

- 인터럽트 기반 펄스 측정으로 정밀도 향상
- DMA를 이용한 타이밍 캡처
- 여러 IR 프로토콜 자동 감지

## 참고 자료

- [TSOP1838 Datasheet](https://www.vishay.com/docs/82491/tsop18xx.pdf)
- [VS1838B Datasheet](https://www.alldatasheet.com/datasheet-pdf/pdf/1132466/ETC2/VS1838B.html)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

## 라이선스

MIT License
