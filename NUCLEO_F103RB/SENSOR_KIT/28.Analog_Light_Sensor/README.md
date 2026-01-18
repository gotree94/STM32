# Analog Light Sensor Module Test

STM32F103 NUCLEO 보드를 이용한 아날로그 광센서(조도센서) 모듈 테스트 프로젝트

## 📌 개요

광저항(LDR/CdS)을 이용하여 주변 밝기를 측정하고, 조도 레벨에 따라 LED를 제어하는 프로젝트입니다.

## 🔧 하드웨어

### 필요 부품
| 부품 | 수량 | 비고 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 탑재 |
| Light Sensor Module | 1 | KY-018 또는 호환 모듈 |
| 점퍼 와이어 | 3 | F-F 타입 |

### 핀 연결
```
Light Sensor Module         NUCLEO-F103RB
===================         ==============
VCC  ------------------>    3.3V
GND  ------------------>    GND
AO   ------------------>    PA0 (A0, CN8-1)
```

### 회로도
```
                    NUCLEO-F103RB
                   +-------------+
                   |             |
    +3.3V ---------|3.3V     PA5|----[LD2]
                   |             |
     GND ---------|GND      PA2|-----> UART TX
                   |             |
Light AO ---------|PA0 (ADC)   |
                   |             |
                   +-------------+

KY-018 Photoresistor Module
+-------------------+
|     [LDR/CdS]     |
|        |          |
|     [10K R]       |
|                   |
|  VCC GND AO       |
+---+---+---+-------+
    |   |   |
   3.3V GND PA0
```

## 💻 소프트웨어

### 주요 기능
1. **밝기 측정**: 8샘플 평균 ADC 값 읽기
2. **조도 레벨 분류**: 5단계 (DARK, DIM, NORMAL, BRIGHT, VERY BRIGHT)
3. **프로그레스 바**: 실시간 밝기 시각화
4. **자동 조명**: 어두울 때 LED 자동 점등

### 조도 레벨 임계값
```
ADC Value       Level           Description
---------       -----           -----------
0    ~ 499      DARK            매우 어두움 (야간)
500  ~ 1499     DIM             어두움 (실내 조명 약함)
1500 ~ 2499     NORMAL          보통 (일반 실내)
2500 ~ 3499     BRIGHT          밝음 (창가 근처)
3500 ~ 4095     VERY BRIGHT     매우 밝음 (직사광선)
```

## 📝 코드 설명

### 평균 필터
```c
uint16_t ADC_Read_Average(uint8_t samples)
{
    uint32_t sum = 0;
    
    for (uint8_t i = 0; i < samples; i++)
    {
        sum += ADC_Read();
    }
    
    return (uint16_t)(sum / samples);
}
```

### 조도 레벨 판정
```c
LightLevel Get_Light_Level(uint16_t adc_value)
{
    if (adc_value < LIGHT_DARK)      return LEVEL_DARK;
    if (adc_value < LIGHT_DIM)       return LEVEL_DIM;
    if (adc_value < LIGHT_NORMAL)    return LEVEL_NORMAL;
    if (adc_value < LIGHT_BRIGHT)    return LEVEL_BRIGHT;
    return LEVEL_VERY_BRIGHT;
}
```

### 프로그레스 바 생성
```c
uint8_t filled = percentage / 5;  /* 20 chars = 100% */
for (int i = 0; i < 20; i++) {
    bar[i] = (i < filled) ? '#' : '-';
}
```

## 🚀 사용법

### 1. 빌드 및 업로드
```bash
1. STM32CubeIDE에서 프로젝트 Import
2. Build Project (Ctrl+B)
3. Run As > STM32 Application
```

### 2. 시리얼 모니터 설정
- 포트: ST-Link Virtual COM Port
- 보드레이트: 115200

### 3. 출력 예시
```
========================================
  Analog Light Sensor Test Program
  NUCLEO-F103RB
========================================
Sensor: Photoresistor (LDR/CdS)

ADC:2048 | V:1.65V | [##########----------]  50% | NORMAL
ADC: 512 | V:0.41V | [##------------------]  12% | DIM
>> Light Level Changed: NORMAL -> DIM
ADC: 256 | V:0.21V | [#-------------------]   6% | DARK
>> Light Level Changed: DIM -> DARK
```

## 📊 응용 예제

### PWM LED 밝기 제어
```c
/* TIM3 CH1 (PA6) PWM 출력 */
uint16_t pwm_value = 4095 - adc_value;  /* 어두우면 밝게 */
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_value);
```

### 히스테리시스 적용
```c
#define HYSTERESIS 100

LightLevel Get_Light_Level_Hysteresis(uint16_t adc_value, LightLevel current)
{
    uint16_t threshold_up, threshold_down;
    
    switch (current) {
        case LEVEL_DARK:
            threshold_up = LIGHT_DARK + HYSTERESIS;
            if (adc_value > threshold_up) return LEVEL_DIM;
            break;
        case LEVEL_DIM:
            threshold_down = LIGHT_DARK - HYSTERESIS;
            threshold_up = LIGHT_DIM + HYSTERESIS;
            if (adc_value < threshold_down) return LEVEL_DARK;
            if (adc_value > threshold_up) return LEVEL_NORMAL;
            break;
        /* ... 나머지 레벨 처리 ... */
    }
    return current;
}
```

### 일출/일몰 감지
```c
typedef struct {
    uint16_t min_value;
    uint16_t max_value;
    uint8_t rising;  /* 1: 밝아지는 중, 0: 어두워지는 중 */
} DaylightTracker;

void Track_Daylight(DaylightTracker *tracker, uint16_t adc_value)
{
    static uint16_t prev_avg = 0;
    static uint32_t avg_count = 0;
    
    /* 1분 평균으로 천천히 변하는 추세 감지 */
    if (++avg_count >= 300) {  /* 200ms * 300 = 60초 */
        if (adc_value > prev_avg + 50) {
            tracker->rising = 1;
            printf("Sunrise detected!\r\n");
        } else if (adc_value < prev_avg - 50) {
            tracker->rising = 0;
            printf("Sunset detected!\r\n");
        }
        prev_avg = adc_value;
        avg_count = 0;
    }
}
```

## ⚠️ 주의사항

1. **광센서 방향**: LDR 표면이 빛을 받도록 배치
2. **응답 속도**: LDR은 응답이 느림 (수백 ms)
3. **온도 영향**: 극한 온도에서 특성 변화
4. **전압 범위**: 모듈에 따라 5V 필요할 수 있음

## 🔍 트러블슈팅

| 증상 | 원인 | 해결책 |
|------|------|--------|
| 항상 최대값 | VCC 미연결 또는 쇼트 | 배선 확인 |
| 항상 최소값 | GND 문제 또는 센서 불량 | 모듈 교체 |
| 값이 불안정 | 노이즈 또는 평균화 부족 | 샘플 수 증가 |
| 레벨 빈번 전환 | 히스테리시스 없음 | 히스테리시스 추가 |

## 📐 센서 특성

### LDR (Light Dependent Resistor)
```
조도 (lux)     저항 (Ω)
----------    ----------
1             >1MΩ
10            ~100kΩ
100           ~10kΩ
1000          ~1kΩ
10000         <100Ω
```

### 전압 분배 회로
```
Vout = VCC × R_fixed / (R_LDR + R_fixed)

밝을 때: R_LDR ↓ → Vout ↑
어두울 때: R_LDR ↑ → Vout ↓
```

## 📚 참고자료

- [LDR/CdS Sensor Characteristics](https://www.electronics-tutorials.ws/io/io_4.html)
- [STM32 ADC Application Note](https://www.st.com/resource/en/application_note/an3116-stm32s-adc-modes-and-their-applications-stmicroelectronics.pdf)

## 📜 라이선스

MIT License
