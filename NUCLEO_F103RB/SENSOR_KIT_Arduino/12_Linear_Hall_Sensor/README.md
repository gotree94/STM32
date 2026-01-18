# 리니어 홀 센서 모듈 (Linear Hall Sensor Module)

NUCLEO-F103RB 보드를 이용한 리니어 홀 센서 테스트 프로젝트

## 📋 개요

리니어 홀 센서는 자기장의 세기를 선형적으로 측정하는 센서입니다. 일반 홀 센서(디지털)와 달리 자기장의 세기와 극성을 아날로그 값으로 출력하여 위치 감지, 전류 측정, 회전 감지 등에 활용됩니다.

## 🔧 하드웨어 구성

### 부품 목록
| 부품명 | 수량 | 비고 |
|--------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RBT6 |
| 리니어 홀 센서 모듈 | 1 | 49E/3144 계열 |
| 네오디뮴 자석 | 1~2 | 테스트용 |
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
  Linear Hall      │             │
  Sensor           │             │
  ┌─────────┐      │             │
  │ VCC ────┼──────┤ 3.3V       │
  │ GND ────┼──────┤ GND        │
  │ DO  ────┼──────┤ D2 (PA10)  │
  │ AO  ────┼──────┤ A0 (PA0)   │
  └─────────┘      │             │
                   │    LED(PA5) │ ← 내장 LED
                   └─────────────┘
       ↑
    [자석]
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

### 홀 효과 (Hall Effect)
도체에 전류가 흐를 때 수직 방향으로 자기장을 가하면 전류와 자기장 모두에 수직한 방향으로 전압이 발생하는 현상입니다.

### 센서 출력 특성
- **기준 전압**: 자기장이 없을 때 약 VCC/2 출력
- **N극 접근**: 기준값보다 높은 전압 출력
- **S극 접근**: 기준값보다 낮은 전압 출력

### 캘리브레이션
프로그램 시작 시 자석이 없는 상태에서 기준값을 측정하여 저장합니다.

```
Calibrated Value = Raw Value - Baseline Value
```

### 자기장 강도 판정
| 캘리브레이션 값 | 상태 |
|-----------------|------|
| > +200 | N극 (강함) |
| +100 ~ +200 | N극 (약함) |
| -100 ~ +100 | 자기장 없음 |
| -200 ~ -100 | S극 (약함) |
| < -200 | S극 (강함) |

## 📝 시리얼 출력 예시

```
========================================
  Linear Hall Sensor Module Test
  Board: NUCLEO-F103RB
========================================

Calibrating... Keep magnets away!
.....
Calibration complete!
Baseline value: 2048

Bring a magnet close to the sensor...

Raw: 2045 | Calibrated: -3 | Digital: Normal | Field: None/Minimal |          |          |
Raw: 2312 | Calibrated: 264 | Digital: TRIGGERED | Field: N-pole (Strong) |          |=====     |
Raw: 2489 | Calibrated: 441 | Digital: TRIGGERED | Field: N-pole (Strong) |          |========  |
Raw: 1756 | Calibrated: -292 | Digital: TRIGGERED | Field: S-pole (Strong) |    ======|          |
Raw: 1623 | Calibrated: -425 | Digital: TRIGGERED | Field: S-pole (Strong) |  ========|          |
```

## 🔬 응용 예제

### 1. 위치 감지
```cpp
// 자석의 위치를 mm 단위로 추정
float estimatePosition(int calibratedValue) {
  // 선형 관계 가정 (실제 캘리브레이션 필요)
  return map(calibratedValue, -500, 500, -50, 50);  // mm
}
```

### 2. 회전 감지
원형으로 배치된 자석과 함께 사용하여 회전 각도나 속도 측정

### 3. 전류 측정
도선 주변의 자기장을 측정하여 비접촉 전류 측정

## ⚠️ 주의사항

1. **캘리브레이션**: 사용 전 자석을 멀리한 상태에서 캘리브레이션 수행
2. **자석 극성**: N극과 S극에 따라 출력 방향이 반대
3. **온도 영향**: 홀 센서는 온도에 민감할 수 있음
4. **거리**: 감지 거리는 일반적으로 수 cm 이내
5. **강한 자석 주의**: 너무 강한 자석은 센서를 포화시킬 수 있음

## 📁 파일 구조

```
02_Linear_Hall_Sensor/
├── Linear_Hall_Sensor.ino    # 아두이노 소스 코드
└── README.md                 # 프로젝트 설명서
```

## 🔗 참고 자료

- [STM32F103RB Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)
- [Hall Effect Sensor Theory](https://www.electronics-tutorials.ws/electromagnetism/hall-effect.html)
- [49E Hall Sensor Datasheet](https://www.alldatasheet.com/datasheet-pdf/pdf/55092/ALLEGRO/A3144.html)

## 📜 라이선스

MIT License
