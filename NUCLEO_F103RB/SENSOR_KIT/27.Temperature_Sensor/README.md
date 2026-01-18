# Temperature Sensor Module Test

STM32F103 NUCLEO 보드를 이용한 아날로그 온도 센서 모듈 테스트 프로젝트

## 📌 개요

NTC 서미스터 또는 LM35 온도 센서를 이용하여 주변 온도를 측정하고 UART로 출력하는 프로젝트입니다.

## 🔧 하드웨어

### 필요 부품
| 부품 | 수량 | 비고 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 탑재 |
| Temperature Sensor Module | 1 | KY-013 (NTC) 또는 LM35 |
| 점퍼 와이어 | 3 | F-F 타입 |

### 핀 연결
```
Temperature Module          NUCLEO-F103RB
==================          ==============
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
 Temp AO ---------|PA0 (ADC)   |
                   |             |
                   +-------------+

KY-013 NTC Thermistor Module
+-------------------+
|     [10K NTC]     |
|        |          |
|     [10K R]       |
|                   |
|  VCC GND AO       |
+---+---+---+-------+
    |   |   |
   3.3V GND PA0
```

## 💻 소프트웨어

### 개발 환경
- STM32CubeIDE 또는 Keil MDK
- STM32F1 HAL Driver

### 주요 기능
1. **ADC 읽기**: 12-bit ADC (0-4095)
2. **온도 계산**: Steinhart-Hart 방정식 (NTC용)
3. **경고 LED**: 30°C 이상 시 LED 점등
4. **UART 출력**: 1초 간격 온도 데이터 전송

### 온도 계산 공식

#### NTC 서미스터 (KY-013)
```
Steinhart-Hart 방정식 (B-parameter form):
1/T = 1/T₀ + (1/B) × ln(R/R₀)

여기서:
- T: 측정 온도 (Kelvin)
- T₀: 기준 온도 (298.15K = 25°C)
- B: B 상수 (3950)
- R: 측정된 저항값
- R₀: 기준 저항값 (10kΩ @ 25°C)
```

#### LM35 센서
```
Temperature = Voltage × 100

LM35 출력: 10mV/°C
예: 250mV = 25°C
```

## 📝 코드 설명

### ADC 설정
```c
/* ADC1 Channel 0 (PA0) */
hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
hadc1.Init.ContinuousConvMode = DISABLE;
hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
hadc1.Init.NbrOfConversion = 1;

/* Sampling Time: 최대 정확도를 위해 길게 설정 */
sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
```

### NTC 온도 계산
```c
float Get_Temperature_NTC(uint16_t adc_value)
{
    float resistance;
    float steinhart;
    
    /* 저항값 계산 (전압 분배 공식) */
    resistance = SERIES_RESISTOR * ((float)adc_value / (ADC_RESOLUTION - adc_value));
    
    /* Steinhart-Hart 계산 */
    steinhart = resistance / THERMISTOR_NOMINAL;
    steinhart = log(steinhart);
    steinhart /= B_COEFFICIENT;
    steinhart += 1.0f / (TEMPERATURE_NOMINAL + 273.15f);
    steinhart = 1.0f / steinhart;
    steinhart -= 273.15f;
    
    return steinhart;
}
```

## 🚀 사용법

### 1. 빌드 및 업로드
```bash
# STM32CubeIDE 사용 시
1. 프로젝트 Import
2. Build Project (Ctrl+B)
3. Run As > STM32 Application
```

### 2. 시리얼 모니터 설정
- 포트: ST-Link Virtual COM Port
- 보드레이트: 115200
- 데이터 비트: 8N1

### 3. 출력 예시
```
========================================
  Temperature Sensor Test Program
  NUCLEO-F103RB
========================================
Sensor Type: NTC Thermistor (KY-013)

ADC: 2048 | Voltage: 1.65V | Temp: 25.3°C
ADC: 2035 | Voltage: 1.64V | Temp: 25.8°C
ADC: 2020 | Voltage: 1.63V | Temp: 26.4°C
ADC: 1890 | Voltage: 1.52V | Temp: 30.2°C   <- LED ON
```

## 📊 응용 예제

### 이동 평균 필터 적용
```c
#define SAMPLES 10

float Get_Filtered_Temperature(void)
{
    static float readings[SAMPLES];
    static uint8_t index = 0;
    float sum = 0;
    
    readings[index] = Get_Temperature_NTC(ADC_Read());
    index = (index + 1) % SAMPLES;
    
    for (int i = 0; i < SAMPLES; i++) {
        sum += readings[i];
    }
    
    return sum / SAMPLES;
}
```

### 온도 임계값 알람
```c
typedef enum {
    TEMP_NORMAL,
    TEMP_WARNING,
    TEMP_CRITICAL
} TempStatus;

TempStatus Check_Temperature(float temp)
{
    if (temp > 50.0f) return TEMP_CRITICAL;
    if (temp > 35.0f) return TEMP_WARNING;
    return TEMP_NORMAL;
}
```

### LM35 센서 사용 시
```c
/* main.c에서 Get_Temperature_LM35() 함수 사용 */
temperature = Get_Temperature_LM35(adc_value);

/* 또는 직접 계산 */
float voltage = (adc_value * 3.3f) / 4095.0f;
float temperature = voltage * 100.0f;  // 10mV/°C
```

## ⚠️ 주의사항

1. **전원 전압**: 3.3V 사용 시 ADC 기준 전압도 3.3V
2. **캘리브레이션**: 정확한 측정을 위해 ADC 캘리브레이션 실행
3. **샘플링 시간**: 노이즈 감소를 위해 긴 샘플링 시간 권장
4. **센서 특성**: NTC vs LM35 계산식 구분 필요

## 🔍 트러블슈팅

| 증상 | 원인 | 해결책 |
|------|------|--------|
| ADC 값이 0 | ADC 초기화 실패 | 클럭 설정 확인 |
| 온도 -273°C | ADC 값 0 (분모 0) | 배선 확인 |
| 부정확한 온도 | B 상수 또는 기준 저항 오류 | 데이터시트 확인 |
| 노이즈가 심함 | 샘플링 시간 부족 | 필터 적용 또는 평균화 |

## 📐 센서 비교

| 특성 | NTC (KY-013) | LM35 |
|------|-------------|------|
| 출력 타입 | 비선형 저항 | 선형 전압 |
| 측정 범위 | -55~125°C | -55~150°C |
| 정확도 | ±1°C (캘리브레이션 필요) | ±0.5°C |
| 응답 속도 | 빠름 | 보통 |
| 가격 | 저렴 | 보통 |

## 📚 참고자료

- [STM32F103 ADC Application Note](https://www.st.com/resource/en/application_note/an3116-stm32s-adc-modes-and-their-applications-stmicroelectronics.pdf)
- [NTC Thermistor Theory](https://www.electronics-tutorials.ws/io/thermistors.html)
- [LM35 Datasheet](https://www.ti.com/lit/ds/symlink/lm35.pdf)

## 📜 라이선스

MIT License
