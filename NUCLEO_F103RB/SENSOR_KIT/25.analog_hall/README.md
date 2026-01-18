# 아날로그 홀센서 모듈 (Analog Hall Effect Sensor Module) - STM32F103

## 📋 개요

아날로그 홀센서 모듈(KY-035)은 49E 선형 홀효과 센서를 사용하여 자기장의 세기와 극성을 아날로그 전압으로 출력합니다. 자석이 없을 때 Vcc/2 전압을 출력하고, N극이 가까워지면 전압이 낮아지고, S극이 가까워지면 전압이 높아집니다.

## 🔧 하드웨어 구성

### 센서 모듈 사양

| 항목 | 사양 |
|------|------|
| 동작 전압 | 3.3V ~ 5V |
| 출력 타입 | 아날로그 (비례 출력) |
| 센서 IC | 49E (A3144와 다름) |
| 감도 | 약 1.4mV/Gauss |
| 무자기장 출력 | Vcc / 2 (약 1.65V @ 3.3V) |
| 동작 온도 | -40°C ~ +85°C |

### 핀 연결

```
Hall Sensor Module          NUCLEO-F103RB
==================          ===============
    S (Signal)  ---------->  PA0 (ADC1_CH0)
    + (VCC)     ---------->  3.3V
    - (GND)     ---------->  GND
    
    (선택) 외부 LED/부저:
    PA6 (PWM) ---------->  LED/Buzzer
```

### 센서 동작 원리

```
           자기장 방향
              ↓
    ┌─────────────────┐
    │     49E IC      │
    │  ┌───────────┐  │
    │  │ Hall      │  │
    │  │ Element   │  │
    │  └───────────┘  │
    │                 │
    │ V+ GND OUT      │
    └──┬───┬───┬──────┘
       │   │   │
       │   │   └──────> Analog Output
       │   └──────────> GND
       └──────────────> VCC
       
    출력 전압:
    - N극 접근: Vout < Vcc/2 (전압 감소)
    - S극 접근: Vout > Vcc/2 (전압 증가)
    - 무자기장: Vout ≈ Vcc/2
```

## 💻 소프트웨어 구성

### 주요 기능

1. **양극성 자기장 측정**: N극/S극 구분 감지
2. **자동 영점 보정**: 시작 시 무자기장 상태 캘리브레이션
3. **DMA 기반 ADC**: 32샘플 평균으로 노이즈 감소
4. **시각적 피드백**: 바이폴라 바 그래프 출력
5. **PWM 강도 표시**: 자기장 세기에 비례한 LED 밝기

### 출력 해석

| ADC 값 | 자기장 | 극성 |
|--------|--------|------|
| 0 ~ 1800 | 강한 N극 | NORTH ↓ |
| 1800 ~ 2000 | 약한 N극 | NORTH |
| 2000 ~ 2100 | 무자기장 | - |
| 2100 ~ 2300 | 약한 S극 | SOUTH |
| 2300 ~ 4095 | 강한 S극 | SOUTH ↑ |

### 핵심 코드 설명

```c
/* 영점 보정 (자석 없는 상태에서 호출) */
void Calibrate_Zero_Point(void) {
    uint32_t sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += Get_ADC_Value();
        HAL_Delay(10);
    }
    zero_point = sum / 100;  /* 약 2048 예상 */
}

/* 자기장 값 계산 (기준점 대비 차이) */
int16_t magnetic_value = (int16_t)adc_value - (int16_t)zero_point;

/* 극성 판별 */
if (magnetic_value < -NOISE_MARGIN) {
    /* N극 감지 */
} else if (magnetic_value > NOISE_MARGIN) {
    /* S극 감지 */
}
```

### LED 피드백 로직

| 상태 | LED 동작 |
|------|----------|
| N극 감지 | 빠른 깜빡임 |
| S극 감지 | 켜짐 |
| 무자기장 | 꺼짐 |
| PWM LED (PA6) | 자기장 강도에 비례한 밝기 |

## 📁 프로젝트 구조

```
05_analog_hall/
├── main.c              # 메인 애플리케이션
├── README.md           # 프로젝트 문서
└── STM32CubeIDE/       # (선택) IDE 프로젝트 파일
```

## 🚀 빌드 및 실행

### STM32CubeIDE 사용

1. 새 STM32 프로젝트 생성 (NUCLEO-F103RB 선택)
2. `main.c` 내용을 프로젝트에 복사
3. 빌드 및 다운로드

### 예상 출력

```
========================================
  Analog Hall Sensor Module Test
  Board: NUCLEO-F103RB
  Sensor: 49E Linear Hall Effect
========================================

Calibrating... Keep magnets away!
Zero point calibrated: 2051 (1.65V)

Bring a magnet close to the sensor...

>>> Magnet Detected! <<<
[Hall Sensor Reading]
  ADC Raw    : 1423
  Voltage    : 1.147 V
  Magnetic   : -628 (relative to zero)
  Polarity   : Strong NORTH (N) ↓
  Strength   : N [████████░░|░░░░░░░░░░] S

<<< Magnet Removed >>>

>>> Magnet Detected! <<<
[Hall Sensor Reading]
  ADC Raw    : 2856
  Voltage    : 2.302 V
  Magnetic   : +805 (relative to zero)
  Polarity   : Strong SOUTH (S) ↑
  Strength   : N [░░░░░░░░░░|░░░░████████] S
```

## 🎯 동작 원리

### 홀 효과 (Hall Effect)

```
         전류 방향 (I)
              │
              ▼
    ┌─────────────────┐
    │   · · · · · ·   │ ← 전자 흐름
    │                 │
    │ ──────────────▶ │ ← 로렌츠 힘으로 전자 편향
    │                 │
    │   · · · · · ·   │
    └────┬───────┬────┘
         │       │
         V-      V+      ← 홀 전압 발생
         
    자기장 (B) ⊙ 또는 ⊗ (지면 방향)
```

### 전압 출력 특성

```
출력 전압 (V)
    │
3.3V├─────────────────────○ S극 최대
    │                   /
2.5V├─────────────────/
    │               /
1.65├─────────────○───── 무자기장 (Vcc/2)
    │           /
0.8V├─────────/
    │       /
   0├─────○─────────────── N극 최대
    └────┬────┬────┬────> 자기장 (Gauss)
       -500   0   +500
         N극      S극
```

## 📌 응용 아이디어

1. **비접촉 위치 센서**: 자석 부착 물체의 위치 감지
2. **회전 속도 측정**: 자석 부착 휠의 RPM 측정
3. **전류 측정**: 홀 효과 전류 센서
4. **리드 스위치 대체**: 무접점 스위치
5. **자기장 매핑**: 자석/코일 주변 자기장 분포 측정

### 회전 속도 측정 예제

```c
volatile uint32_t pulse_count = 0;
volatile uint32_t last_pulse_time = 0;
uint32_t rpm = 0;

/* 자석 통과 감지 (임계값 기반) */
void Check_Rotation(int16_t magnetic_value) {
    static uint8_t magnet_present = 0;
    
    if (!magnet_present && abs(magnetic_value) > 300) {
        /* 자석 진입 */
        magnet_present = 1;
        pulse_count++;
        
        uint32_t current_time = HAL_GetTick();
        uint32_t period = current_time - last_pulse_time;
        
        if (period > 0) {
            rpm = 60000 / period;  /* RPM 계산 */
        }
        last_pulse_time = current_time;
        
    } else if (magnet_present && abs(magnetic_value) < 100) {
        /* 자석 이탈 */
        magnet_present = 0;
    }
}
```

### 자기장 강도를 Gauss로 변환

```c
/* 49E 센서 감도: 약 1.4mV/Gauss (@ 5V 공급 시) */
/* 3.3V 공급 시 비례 감소: 약 0.92mV/Gauss */

#define SENSITIVITY_MV_PER_GAUSS    0.92f
#define ADC_MV_PER_LSB              (3300.0f / 4096.0f)  /* 0.806mV/LSB */

float ADC_to_Gauss(int16_t adc_offset) {
    float mv_offset = adc_offset * ADC_MV_PER_LSB;
    return mv_offset / SENSITIVITY_MV_PER_GAUSS;
}
```

## ⚠️ 주의사항

- 강한 자석은 센서를 영구적으로 손상시킬 수 있음
- 온도 변화에 따른 드리프트 발생 가능 (주기적 재보정 권장)
- 전원 노이즈가 출력에 직접 영향을 줌 (디커플링 커패시터 권장)
- 49E는 선형 센서로, A3144(디지털 래치)와 다름

## 🔍 디지털 홀센서와의 비교

| 특성 | 아날로그 (49E) | 디지털 (A3144) |
|------|---------------|----------------|
| 출력 | 연속 전압 | ON/OFF |
| 극성 감지 | N/S 구분 가능 | 단일 극성만 |
| 거리 측정 | 상대 거리 추정 가능 | 불가 |
| 노이즈 | 필터링 필요 | 히스테리시스 내장 |
| 용도 | 정밀 측정 | 스위칭 |

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [49E Hall Sensor Datasheet](https://www.allegromicro.com/en/products/sense/linear-and-angular-position/linear-position-sensor-ics/a1324-5-6)
- [KY-035 Module Info](https://arduinomodules.info/ky-035-analog-hall-magnetic-sensor-module/)

## 📝 라이선스

MIT License - 자유롭게 사용, 수정, 배포 가능
