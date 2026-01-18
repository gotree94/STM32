# 불꽃감지 센서 모듈 (Flame Sensor Module)

NUCLEO-F103RB 보드를 이용한 불꽃감지 센서 테스트 프로젝트

## 📋 개요

불꽃감지 센서는 불꽃에서 방출되는 적외선(IR) 파장(760nm~1100nm)을 감지하는 센서입니다. 화재 감지 시스템, 안전 장치 등에 활용됩니다.

## 🔧 하드웨어 구성

### 부품 목록
| 부품명 | 수량 | 비고 |
|--------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RBT6 |
| 불꽃감지 센서 모듈 | 1 | IR Flame Sensor |
| 점퍼 와이어 | 4 | M-F 타입 |

### 핀 연결
| 센서 핀 | NUCLEO 핀 | 설명 |
|---------|-----------|------|
| VCC | 3.3V / 5V | 전원 |
| GND | GND | 접지 |
| DO | D2 (PA10) | 디지털 출력 |
| AO | A0 (PA0) | 아날로그 출력 |

### 회로도
```
                    NUCLEO-F103RB
                   ┌─────────────┐
                   │             │
  Flame Sensor     │             │
  ┌─────────┐      │             │
  │ VCC ────┼──────┤ 3.3V       │
  │ GND ────┼──────┤ GND        │
  │ DO  ────┼──────┤ D2 (PA10)  │
  │ AO  ────┼──────┤ A0 (PA0)   │
  └─────────┘      │             │
                   │    LED(PA5) │ ← 내장 LED
                   └─────────────┘
```

## 💻 소프트웨어 설정

### Arduino IDE 설정
1. **보드 매니저에서 STM32 설치**
   - File → Preferences → Additional Boards Manager URLs에 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
2. **보드 선택**
   - Tools → Board → STM32 boards groups → Nucleo-64
   - Board part number: Nucleo F103RB
3. **포트 선택**
   - Tools → Port → COMx (STM32 STLink)

## 📊 동작 원리

### 센서 출력
- **디지털 출력 (DO)**: 
  - LOW (0): 불꽃 감지됨
  - HIGH (1): 불꽃 없음
  - 가변저항으로 감도 조절 가능
  
- **아날로그 출력 (AO)**:
  - 0 ~ 4095 (12-bit ADC)
  - 값이 낮을수록 강한 불꽃 감지

### 불꽃 강도 판정
| 아날로그 값 | 상태 |
|-------------|------|
| 0 ~ 500 | 강한 불꽃 (근거리) |
| 500 ~ 2000 | 보통 강도 |
| 2000 ~ 3500 | 약한 불꽃 (원거리) |
| 3500 ~ 4095 | 불꽃 없음 |

## 📝 시리얼 출력 예시

```
========================================
  Flame Sensor Module Test
  Board: NUCLEO-F103RB
========================================

Sensor initialized. Monitoring...
DO: LOW = Flame detected, HIGH = No flame

Analog Value: 4012 (98%) | Digital: No flame | Intensity: No flame
Analog Value: 3856 (94%) | Digital: No flame | Intensity: No flame
Analog Value: 1245 (30%) | Digital: FLAME DETECTED! | Intensity: Moderate flame
Analog Value: 356 (8%) | Digital: FLAME DETECTED! | Intensity: Strong flame nearby!
```

## ⚠️ 주의사항

1. **화재 위험**: 테스트 시 라이터나 성냥 사용에 주의하세요
2. **감도 조절**: 모듈의 가변저항으로 DO 출력 임계값 조절 가능
3. **감지 각도**: 센서의 감지 각도는 약 60도입니다
4. **거리**: 일반적으로 80cm 이내에서 효과적으로 동작

## 📁 파일 구조

```
01_Flame_Sensor/
├── Flame_Sensor.ino    # 아두이노 소스 코드
└── README.md           # 프로젝트 설명서
```

## 🔗 참고 자료

- [STM32F103RB Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

## 📜 라이선스

MIT License
