# 불꽃 감지 센서 모듈 (Flame Sensor Module)

STM32F103 NUCLEO 보드를 이용한 불꽃 감지 센서 테스트 프로젝트

## 📋 개요

불꽃 감지 센서 모듈은 적외선(IR)을 감지하여 화염의 존재를 탐지합니다. 디지털 출력(DO)과 아날로그 출력(AO)을 모두 제공하여 간단한 화재 감지부터 화염 강도 측정까지 가능합니다.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| 불꽃 감지 센서 모듈 | 1 | IR 기반 화염 감지 센서 |
| 점퍼 와이어 | 4 | 연결용 |
| 브레드보드 | 1 | 선택사항 |

### 핀 연결

```
불꽃 감지 센서          NUCLEO-F103RB
┌─────────────┐        ┌─────────────┐
│     VCC     │───────▶│    3.3V     │
│     GND     │───────▶│    GND      │
│     DO      │───────▶│    PA0      │ (Digital Input)
│     AO      │───────▶│    PA1      │ (ADC1_IN1)
└─────────────┘        └─────────────┘
```

### 회로도

```
                    ┌─────────────────────┐
                    │   Flame Sensor      │
                    │     Module          │
    3.3V ──────────▶│ VCC                 │
                    │                     │
    GND  ──────────▶│ GND                 │
                    │                     │
    PA0  ◀──────────│ DO  (Digital Out)   │
                    │     Active LOW      │
    PA1  ◀──────────│ AO  (Analog Out)    │
                    │     0~3.3V          │
                    └─────────────────────┘
```

## 💻 소프트웨어 설명

### 주요 기능

1. **디지털 감지**: DO 핀을 통해 화염 감지 여부를 ON/OFF로 확인
2. **아날로그 측정**: AO 핀을 통해 화염 강도를 0~4095 값으로 측정
3. **실시간 모니터링**: UART를 통해 센서 상태 출력
4. **LED 알림**: 화염 감지 시 온보드 LED 점등

### 동작 원리

- **감지 원리**: 화염에서 방출되는 적외선(760nm~1100nm)을 포토다이오드로 감지
- **DO 출력**: 화염 감지 시 LOW(0), 미감지 시 HIGH(1)
- **AO 출력**: 화염 강도에 비례하여 전압 출력 (화염이 강할수록 값이 낮음)
- **감도 조절**: 모듈의 가변저항으로 감지 임계값 조절 가능

### 코드 구조

```
main.c
├── Flame_ReadDigital()   // 디지털 출력 읽기
├── Flame_ReadAnalog()    // 아날로그 값 읽기 (ADC)
├── MX_GPIO_Init()        // GPIO 초기화
├── MX_ADC1_Init()        // ADC 초기화
└── MX_USART2_UART_Init() // UART 초기화
```

## 📊 출력 예시

```
========================================
   Flame Sensor Module Test Program
   NUCLEO-F103RB Board
========================================

DO Pin: PA0 (Digital Input)
AO Pin: PA1 (ADC1_IN1)

ADC: 4012 | Intensity:   2% | Status: No flame
ADC: 3998 | Intensity:   2% | Status: No flame
ADC: 1523 | Intensity:  62% | Status: *** FLAME DETECTED! ***

!!! ALERT: Flame detected! !!!

ADC:  856 | Intensity:  79% | Status: *** FLAME DETECTED! ***
ADC: 3876 | Intensity:   5% | Status: No flame

--- Flame cleared ---
```

## ⚙️ 빌드 및 실행

### STM32CubeIDE 사용 시

1. 새 프로젝트 생성 (STM32F103RB 선택)
2. `main.c` 파일 내용으로 교체
3. 빌드 및 다운로드

### Makefile 사용 시

```bash
make clean
make all
make flash
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결방법 |
|------|------|----------|
| 항상 감지됨 | 감도 너무 높음 | 가변저항을 시계 반대방향으로 조절 |
| 감지 안됨 | 감도 너무 낮음 | 가변저항을 시계 방향으로 조절 |
| ADC 값 불안정 | 노이즈 | 전원에 100nF 캐패시터 추가 |
| UART 출력 없음 | 연결 오류 | ST-Link 가상 COM 포트 확인 |

## ⚠️ 주의사항

1. **감도 조절**: 환경에 맞게 가변저항으로 감도를 조절하세요
2. **오감지 주의**: 강한 햇빛, 전열기, 백열등도 감지될 수 있음
3. **감지 거리**: 일반적으로 80cm 이내에서 효과적
4. **감지 각도**: 약 60° 범위 내에서 감지
5. **전원**: 3.3V 또는 5V 모두 사용 가능

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

## 📜 라이선스

MIT License
