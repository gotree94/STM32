# Analog Light Sensor Module - NUCLEO-F103RB

아날로그 조도(광) 센서 모듈을 NUCLEO-F103RB 보드에서 아두이노 환경으로 테스트하는 프로젝트입니다.

## 📋 개요

포토레지스터(LDR, Light Dependent Resistor)를 사용하여 주변 광량을 측정하는 센서 모듈입니다. 빛의 세기에 따라 아날로그 출력값이 변하고, 설정된 임계값을 기준으로 디지털 출력이 변화합니다.

## 🔧 하드웨어 요구사항

| 항목 | 사양 |
|------|------|
| 보드 | NUCLEO-F103RB |
| 센서 | Analog Light Sensor Module (Photoresistor/LDR) |
| 전압 | 3.3V ~ 5V |

## 📌 핀 연결

| 센서 모듈 | NUCLEO-F103RB | 설명 |
|-----------|---------------|------|
| VCC | 3.3V 또는 5V | 전원 |
| GND | GND | 접지 |
| DO | D2 (PA10) | 디지털 출력 (임계값) |
| AO | A0 (PA0) | 아날로그 출력 (광량) |

### 회로도

```
                    NUCLEO-F103RB
                   ┌─────────────┐
                   │             │
  Light Sensor     │             │
  ┌─────────┐      │             │
  │  VCC ───┼──────┤ 3.3V       │
  │  GND ───┼──────┤ GND        │
  │  DO  ───┼──────┤ D2 (PA10)  │
  │  AO  ───┼──────┤ A0 (PA0)   │
  └─────────┘      │             │
                   │    [POT]    │ ← 가변저항으로 임계값 조절
                   └─────────────┘
```

## 💻 소프트웨어 설정

### Arduino IDE 설정

1. **STM32duino 보드 매니저 설치**
   - File → Preferences → Additional Boards Manager URLs에 추가:
     ```
     https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
     ```
   - Tools → Board → Boards Manager → "STM32" 검색 후 설치

2. **보드 선택**
   - Tools → Board → STM32 Boards → Nucleo-64
   - Tools → Board part number → Nucleo F103RB

## 📝 사용 방법

1. 핀 연결을 완료합니다.
2. Arduino IDE에서 코드를 업로드합니다.
3. 시리얼 모니터를 열고 (115200 baud) 광량을 확인합니다.
4. 손으로 센서를 가리거나 빛을 비춰 테스트합니다.
5. 모듈의 가변저항으로 디지털 출력 임계값을 조절할 수 있습니다.

## 📊 시리얼 출력 예시

```
================================
Analog Light Sensor Module Test
Board: NUCLEO-F103RB
================================

ADC: 3245 | Light: 79% | Level: Bright | Digital: LIGHT
[███████████████░░░░░] 79%

ADC: 2156 | Light: 52% | Level: Normal | Digital: LIGHT
[██████████░░░░░░░░░░] 52%

ADC: 856 | Light: 20% | Level: Dim | Digital: DARK
[████░░░░░░░░░░░░░░░░] 20%

ADC: 234 | Light: 5% | Level: Dark | Digital: DARK
[█░░░░░░░░░░░░░░░░░░░] 5%
```

## 🔍 광량 레벨 분류

| ADC 값 | 레벨 | 설명 |
|--------|------|------|
| 0 - 499 | Dark | 어두움 (야간, 암실) |
| 500 - 1499 | Dim | 약간 어두움 (실내 조명 약함) |
| 1500 - 2499 | Normal | 보통 (일반 실내) |
| 2500 - 3499 | Bright | 밝음 (창가, 조명 밝음) |
| 3500 - 4095 | Very Bright | 매우 밝음 (직사광선) |

> ⚡ 임계값은 환경에 따라 코드 내에서 조정하세요.

## 🔍 동작 원리

### LDR (Light Dependent Resistor)

```
빛 → 저항 감소 → 전압 증가 → ADC 값 증가

밝음: 저항 낮음 (~1KΩ)  → 높은 ADC 값
어두움: 저항 높음 (~10MΩ) → 낮은 ADC 값
```

### 분압 회로

```
         VCC
          │
          ├─── AO (출력)
          │
         [R] 고정 저항 (10K)
          │
         [LDR] 포토레지스터
          │
         GND
```

## ⚙️ 임계값 조정

환경에 맞게 다음 상수를 조정하세요:

```cpp
#define DARK_THRESHOLD      500     // 어두움 판단
#define DIM_THRESHOLD       1500    // 약간 어두움 판단
#define NORMAL_THRESHOLD    2500    // 보통 판단
#define BRIGHT_THRESHOLD    3500    // 밝음 판단
```

## ⚠️ 주의사항

- LDR은 반응 속도가 느려 급격한 광량 변화에 지연이 있을 수 있습니다.
- 센서마다 특성이 다르므로 임계값 조정이 필요합니다.
- 직사광선에서는 포화될 수 있습니다.
- 온도에 따라 특성이 약간 변할 수 있습니다.

## 🎯 응용 예제

- 자동 조명 시스템
- 야간/주간 모드 전환
- 태양광 추적 시스템
- 조도 기반 알람
- 스마트 블라인드 제어

## 📁 파일 구조

```
03_Analog_Light_Sensor_Module/
├── Analog_Light_Sensor_Module.ino    # 메인 아두이노 코드
└── README.md                         # 문서
```

## 🔗 참고 자료

- [STM32duino GitHub](https://github.com/stm32duino)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [LDR/Photoresistor Application Guide](https://components101.com/resistors/ldr-datasheet)

## 📜 라이센스

MIT License
