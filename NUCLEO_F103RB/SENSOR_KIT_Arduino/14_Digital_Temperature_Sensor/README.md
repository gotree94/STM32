# 디지털 온도 센서 모듈 (Digital Temperature Sensor - DS18B20)

NUCLEO-F103RB 보드를 이용한 DS18B20 디지털 온도 센서 테스트 프로젝트

## 📋 개요

DS18B20은 Dallas/Maxim에서 제작한 1-Wire 디지털 온도 센서입니다. 단일 데이터 라인으로 통신하며, 고정밀 온도 측정이 가능합니다. 여러 센서를 같은 버스에 연결할 수 있어 다중 온도 측정에 적합합니다.

## 🔧 하드웨어 구성

### 부품 목록
| 부품명 | 수량 | 비고 |
|--------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RBT6 |
| DS18B20 모듈 | 1 | TO-92 또는 방수형 |
| 4.7kΩ 저항 | 1 | 풀업 저항 (모듈에 내장된 경우 생략) |
| 점퍼 와이어 | 3 | M-F 타입 |

### DS18B20 사양
| 항목 | 사양 |
|------|------|
| 전압 | 3.0V ~ 5.5V |
| 측정 범위 | -55°C ~ +125°C |
| 정확도 | ±0.5°C (-10°C ~ +85°C) |
| 분해능 | 9~12bit (설정 가능) |
| 변환 시간 | 93.75ms (9bit) ~ 750ms (12bit) |

### 핀 연결
| 센서 핀 | NUCLEO 핀 | 설명 |
|---------|-----------|------|
| VCC (빨강) | 3.3V / 5V | 전원 |
| GND (검정) | GND | 접지 |
| DATA (노랑) | D2 (PA10) | 데이터 (4.7kΩ 풀업) |

### 회로도
```
                    NUCLEO-F103RB
                   ┌─────────────┐
                   │             │
  DS18B20          │             │
  ┌─────────┐      │             │
  │ VCC ────┼──┬───┤ 3.3V       │
  │         │  │   │             │
  │         │ [4.7k]             │
  │         │  │   │             │
  │ DATA────┼──┴───┤ D2 (PA10)  │
  │ GND ────┼──────┤ GND        │
  └─────────┘      │             │
                   │    LED(PA5) │ ← 내장 LED (경보)
                   └─────────────┘
```

## 💻 소프트웨어 설정

### 필수 라이브러리 설치
Arduino IDE에서 다음 라이브러리를 설치하세요:
1. **OneWire** by Paul Stoffregen
   - Sketch → Include Library → Manage Libraries
   - "OneWire" 검색 후 설치
2. **DallasTemperature** by Miles Burton
   - "DallasTemperature" 검색 후 설치

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

### 1-Wire 프로토콜
- 단일 데이터 라인으로 양방향 통신
- 각 센서는 고유한 64-bit ROM 코드 보유
- 여러 센서를 같은 버스에 연결 가능 (병렬 연결)

### 분해능 설정
| 분해능 | 증분 | 변환 시간 |
|--------|------|-----------|
| 9-bit | 0.5°C | 93.75ms |
| 10-bit | 0.25°C | 187.5ms |
| 11-bit | 0.125°C | 375ms |
| 12-bit | 0.0625°C | 750ms |

### 온도 변환 공식
```
Temperature (°C) = Raw Data × 0.0625  (12-bit 분해능 기준)
Temperature (°F) = Temperature (°C) × 9/5 + 32
```

## 📝 시리얼 출력 예시

```
========================================
  Digital Temperature Sensor Test
  Sensor: DS18B20
  Board: NUCLEO-F103RB
========================================

Found 1 DS18B20 sensor(s)
Sensor Address: 28:FF:64:1E:5A:16:04:92
Resolution: 12 bits

Temperature alarm thresholds:
  High: 30.00°C
  Low: 10.00°C

Starting measurements...

[1] Temp: 24.56°C (76.21°F) | Min: 24.6°C Max: 24.6°C |=========           |
[2] Temp: 24.62°C (76.32°F) | Min: 24.6°C Max: 24.6°C |=========           |
[3] Temp: 25.81°C (78.46°F) | Min: 24.6°C Max: 25.8°C |==========          |
[4] Temp: 31.25°C (88.25°F) | Min: 24.6°C Max: 31.3°C | ⚠ ALARM! |!!!!!!!!!!!!        |
[5] Temp: 28.44°C (83.19°F) | Min: 24.6°C Max: 31.3°C |===========         |
```

## 🔬 응용 예제

### 1. 다중 센서 연결
```cpp
// 버스에 연결된 모든 센서 읽기
int deviceCount = sensors.getDeviceCount();
for (int i = 0; i < deviceCount; i++) {
  float temp = sensors.getTempCByIndex(i);
  Serial.print("Sensor ");
  Serial.print(i);
  Serial.print(": ");
  Serial.println(temp);
}
```

### 2. 비동기 온도 읽기
```cpp
// 비동기 변환 시작
sensors.setWaitForConversion(false);
sensors.requestTemperatures();

// 다른 작업 수행...

// 변환 완료 확인 후 읽기
if (sensors.isConversionComplete()) {
  float temp = sensors.getTempC(sensorAddress);
}
```

### 3. 파라사이트 전원 모드
```cpp
// VCC 없이 DATA 라인에서 전원 공급
// 2핀만 연결 (GND + DATA)
// 풀업 저항 필수
```

## ⚠️ 주의사항

1. **풀업 저항**: 4.7kΩ 풀업 저항 필수 (모듈에 없는 경우)
2. **케이블 길이**: 긴 케이블 사용 시 저항값 조정 필요 (2.2kΩ~4.7kΩ)
3. **변환 시간**: 12-bit 분해능 시 최대 750ms 소요
4. **다중 연결**: 많은 센서 연결 시 전원 용량 확인
5. **방수형**: 방수형 사용 시 침수 전 절연 확인

## 📁 파일 구조

```
04_Digital_Temperature_Sensor/
├── Digital_Temperature_Sensor.ino    # 아두이노 소스 코드
└── README.md                         # 프로젝트 설명서
```

## 🔗 참고 자료

- [STM32F103RB Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)
- [DS18B20 Datasheet](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf)
- [OneWire Library](https://www.pjrc.com/teensy/td_libs_OneWire.html)
- [DallasTemperature Library](https://github.com/milesburton/Arduino-Temperature-Control-Library)

## 📜 라이선스

MIT License
