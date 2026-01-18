# IR Receiver Module (IR 수신 모듈)

NUCLEO-F103RB 보드를 이용한 IR 수신 모듈 기본 테스트 프로젝트

## 개요

IR 수신 모듈을 이용하여 IR 신호를 감지하고, 펄스 폭과 개수를 측정하여 시리얼로 출력합니다.

## 하드웨어 연결

### 핀 배치

| 핀 | 기능 | 설명 |
|----|------|------|
| PA0 (A0) | IR RX | IR 수신 모듈 신호 입력 |
| PA5 (D13) | LED | 내장 LED (신호 감지 표시) |

### 회로도

```
IR Receiver Module (TSOP1838 / VS1838B)
┌──────────────────┐
│    ┌───────┐     │
│    │  ■■■  │     │      NUCLEO-F103RB
│    │  ■■■  │     │     ┌─────────────┐
│    └───────┘     │     │             │
│     │  │  │      │     │             │
│    OUT GND VCC   │     │             │
└─────┼──┼──┼──────┘     │             │
      │  │  │            │             │
      │  │  └──────────→ │ 3.3V        │
      │  └─────────────→ │ GND         │
      └────────────────→ │ PA0 (A0)    │
                         │             │
                         │ PA5 (LED)   │ ← 신호 감지 표시
                         └─────────────┘
```

### 부품 목록

- IR 수신 모듈 (TSOP1838, VS1838B, HX1838 등)
- IR 리모컨 (테스트용)

### IR 수신 모듈 핀 배치 (일반적)

```
정면에서 본 모습 (돔 부분이 위):

TSOP1838:          VS1838B:
  ┌───┐              ┌───┐
  │ ○ │              │ ○ │
  └┬┬┬┘              └┬┬┬┘
   │││                │││
  OUT GND VCC        OUT GND VCC
   1   2   3          1   2   3

* 모듈마다 핀 배치가 다를 수 있으니 데이터시트 확인 필요
```

## 소프트웨어 설정

### Arduino IDE 설정

1. **보드 매니저에서 STM32 패키지 설치**
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```

2. **보드 선택**
   - Tools → Board → Nucleo-64
   - Tools → Board part number → Nucleo F103RB

## 사용 방법

1. IR 수신 모듈 연결
2. 코드 업로드
3. 시리얼 모니터 열기 (115200 baud)
4. IR 리모컨으로 수신 모듈 방향으로 버튼 누르기
5. 수신된 신호 정보 확인

## 시리얼 출력 예시

```
========================================
  IR Receiver - NUCLEO-F103RB
  Signal Detection & Pulse Measurement
========================================

Hardware: TSOP1838, VS1838B, or compatible
Input Pin: PA0 (A0)

Waiting for IR signal...

========================================
[RX] Signal #1 - Pulses: 67
========================================
Pulse timings (us):
9012    4498    576     552     576     1672    576     552
576     1672    576     552     576     552     576     1672
...

--- Signal Analysis ---
Total duration: 67432 us
Pulse count: 67
Min pulse: 548 us
Max pulse: 9012 us
Average: 1006 us

Protocol guess: NEC (Leader ~9ms detected)
```

## 동작 원리

1. IR 수신 모듈은 38kHz 캐리어를 복조하여 디지털 신호 출력
2. 인터럽트로 신호 엣지 감지
3. 각 펄스의 시간 측정 및 저장
4. 신호 종료 후 분석 결과 출력

## 지원 프로토콜 감지

| 프로토콜 | Leader 펄스 특징 |
|----------|-----------------|
| NEC | ~9ms pulse + ~4.5ms space |
| RC5/RC6 | ~889us 기본 단위 |
| Sony SIRC | ~2.4ms header |

## 문제 해결

| 문제 | 해결 방법 |
|------|----------|
| 신호 감지 안됨 | VCC/GND 연결 확인, 리모컨 배터리 확인 |
| 노이즈 많음 | 형광등 피하기, 풀업 저항 추가 |
| 펄스 누락 | 인터럽트 우선순위 확인 |

## 라이선스

MIT License
