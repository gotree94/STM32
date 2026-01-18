# 디지털 온도 센서 모듈 (Digital Temperature Sensor - DS18B20)

STM32F103 NUCLEO 보드를 이용한 DS18B20 디지털 온도 센서 테스트 프로젝트

## 📋 개요

DS18B20은 1-Wire 프로토콜을 사용하는 디지털 온도 센서입니다. 단일 데이터 라인으로 통신하며, -55°C에서 +125°C까지 측정 가능하고 ±0.5°C의 높은 정확도를 제공합니다. 각 센서는 고유한 64비트 주소를 가져 여러 센서를 하나의 버스에 연결할 수 있습니다.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| DS18B20 모듈 | 1 | 디지털 온도 센서 |
| 4.7KΩ 저항 | 1 | 풀업 저항 (모듈에 내장된 경우 불필요) |
| 점퍼 와이어 | 3 | 연결용 |

### 핀 연결

```
DS18B20 모듈            NUCLEO-F103RB
┌─────────────┐        ┌─────────────┐
│     VCC     │───────▶│    3.3V     │
│     GND     │───────▶│    GND      │
│    DATA     │───────▶│    PA0      │ (1-Wire)
└─────────────┘        └─────────────┘
                              │
                         4.7KΩ 풀업
                              │
                            3.3V
```

### 회로도

```
                    ┌─────────────────────┐
                    │     DS18B20         │
    3.3V ──────────▶│ VCC (Pin 3)         │
                    │                     │
    GND  ──────────▶│ GND (Pin 1)         │
                    │                     │
    PA0  ◀──────────│ DATA (Pin 2)        │
         │          └─────────────────────┘
         │
        ┌┴┐
        │ │ 4.7KΩ (Pull-up)
        └┬┘
         │
    3.3V ┘

    DS18B20 핀 배열 (바닥면에서 볼 때):
    ┌─────────────────┐
    │    DS18B20      │
    │   ┌───────┐     │
    │   │   ○   │     │ ← 둥근 부분
    │   └───────┘     │
    │  1   2   3      │
    │ GND DATA VCC    │
    └─────────────────┘
```

## 💻 소프트웨어 설명

### 주요 기능

1. **1-Wire 프로토콜**: 소프트웨어 구현 1-Wire 통신
2. **온도 측정**: 12비트 해상도 (0.0625°C)
3. **ROM 코드 읽기**: 센서 고유 ID 확인
4. **Min/Max 추적**: 최저/최고 온도 기록
5. **온도 알람**: 설정 온도 초과 시 경고

### 1-Wire 프로토콜 타이밍

```
초기화 시퀀스:
MCU          DS18B20
 │   Reset Pulse (480µs)
 ├────────────────────────────►
 │                    │
 │   Presence Pulse   │
 ◄────────────────────┤
 │     (60-240µs)     │

비트 쓰기 (Write '1'):
 │ __
 │   |___________________|████████████████
     2µs                 60µs (recovery)

비트 쓰기 (Write '0'):
 │ ________________________________
 │                                 |████
     60µs                          2µs

비트 읽기:
 │ __                     Sample Point
 │   |_____________________|▼______|████
     2µs        10µs              50µs
```

### 온도 데이터 형식

```
12비트 해상도 (기본값):
┌────────────────────────────────┐
│ S  S  S  S  S  2⁶ 2⁵ 2⁴ 2³ 2² 2¹ 2⁰ 2⁻¹ 2⁻² 2⁻³ 2⁻⁴ │
│ MSB                                            LSB │
└────────────────────────────────┘
 S = Sign bit (양수: 0, 음수: 1)

예시:
 +25.0625°C = 0x0191 = 0000 0001 1001 0001
 -25.0625°C = 0xFE6F = 1111 1110 0110 1111
```

### 코드 구조

```
main.c
├── DWT_Init()                // 마이크로초 타이머 초기화
├── delay_us()                // 마이크로초 딜레이
├── DS18B20_Reset()           // 리셋 및 Presence 확인
├── DS18B20_WriteByte()       // 바이트 전송
├── DS18B20_ReadByte()        // 바이트 수신
├── DS18B20_StartConversion() // 온도 변환 시작
├── DS18B20_ReadTemperatureRaw() // Raw 온도 읽기
├── DS18B20_GetTemperature()  // 온도 읽기 (°C)
├── DS18B20_ReadROM()         // ROM 코드 읽기
├── Print_TempBar()           // 온도 바 그래프
├── MX_GPIO_Init()            // GPIO 초기화
└── MX_USART2_UART_Init()     // UART 초기화
```

## 📊 출력 예시

```
========================================
  Digital Temperature Sensor Test
  DS18B20 on NUCLEO-F103RB
========================================

DATA Pin: PA0 (1-Wire)

Checking for DS18B20...
DS18B20 detected!

ROM Code: 28-FF-64-1E-83-16-03-52
Family Code: 0x28 (DS18B20 = 0x28)

Starting temperature monitoring...
Scale: -10°C to +50°C

Temp: +24.56°C [█████████████████░░░░░░░░░░░░░] Min:+24.5 Max:+24.6
Temp: +24.62°C [█████████████████░░░░░░░░░░░░░] Min:+24.5 Max:+24.6
Temp: +25.00°C [█████████████████░░░░░░░░░░░░░] Min:+24.5 Max:+25.0
Temp: +36.25°C [███████████████████████░░░░░░░] Min:+24.5 Max:+36.3 [HIGH!]
Temp: + 8.50°C [█████████▓░░░░░░░░░░░░░░░░░░░░] Min:+ 8.5 Max:+36.3 [LOW!]
```

## ⚙️ 빌드 및 실행

### STM32CubeIDE 사용 시

1. 새 프로젝트 생성 (STM32F103RB 선택)
2. `main.c` 파일 내용으로 교체
3. 빌드 및 다운로드

### 해상도 설정

| 해상도 | 변환 시간 | 정밀도 |
|--------|----------|--------|
| 9-bit  | 93.75ms  | 0.5°C  |
| 10-bit | 187.5ms  | 0.25°C |
| 11-bit | 375ms    | 0.125°C|
| 12-bit | 750ms    | 0.0625°C|

## 🔬 응용 예제

### 1. 다중 센서 연결
```
하나의 1-Wire 버스에 여러 DS18B20 연결 가능

    3.3V ────┬────┬────┬────┐
             │    │    │    │
           [DS1][DS2][DS3] 4.7K
             │    │    │    │
    PA0  ────┴────┴────┴────┘
             │    │    │
    GND  ────┴────┴────┘

각 센서는 고유 ROM 코드로 식별
```

### 2. 온도 로깅
```c
// RTC와 함께 사용하여 시간별 온도 기록
typedef struct {
    uint32_t timestamp;
    float temperature;
} TempLog_t;

TempLog_t log[100];
```

### 3. 온도 알람 시스템
```c
// 임계값 설정 (DS18B20 내부 알람 기능)
void DS18B20_SetAlarm(int8_t th, int8_t tl) {
    DS18B20_Reset();
    DS18B20_WriteByte(0xCC);  // Skip ROM
    DS18B20_WriteByte(0x4E);  // Write Scratchpad
    DS18B20_WriteByte(th);    // TH register
    DS18B20_WriteByte(tl);    // TL register
    DS18B20_WriteByte(0x7F);  // Config (12-bit)
}
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결방법 |
|------|------|----------|
| 센서 감지 안됨 | 풀업 저항 없음 | 4.7KΩ 풀업 저항 추가 |
| 온도값 85°C | 변환 미완료 | 변환 대기 시간 확인 |
| 온도값 -127°C | 통신 오류 | 배선 및 타이밍 확인 |
| 불안정한 값 | 노이즈 | 전원 안정화, 케이블 단축 |

## ⚠️ 주의사항

1. **풀업 저항**: 4.7KΩ 필수 (긴 케이블: 1.5~4.7KΩ)
2. **기생 전원 모드**: 이 예제는 외부 전원 모드 사용
3. **변환 시간**: 12비트 해상도는 750ms 필요
4. **케이블 길이**: 최대 100m까지 가능 (풀업 저항 조정 필요)
5. **센서 방수**: TO-92 패키지는 방수 아님 (방수 모듈 별도)

### ROM 코드 구조

```
┌────┬─────────────────────────────────────────────┬────┐
│ FC │              Serial Number                  │CRC │
│ 1B │                   6 Bytes                   │ 1B │
└────┴─────────────────────────────────────────────┴────┘
 FC: Family Code (0x28 = DS18B20)
 CRC: CRC-8 검증 코드
```

## 📚 참고 자료

- [DS18B20 Datasheet](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf)
- [1-Wire Protocol Guide](https://www.maximintegrated.com/en/design/technical-documents/tutorials/1/126.html)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

## 📜 라이선스

MIT License
