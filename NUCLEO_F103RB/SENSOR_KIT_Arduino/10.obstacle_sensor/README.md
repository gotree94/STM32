# 장애물 감지센서 모듈 (Obstacle Avoidance Sensor Module)

NUCLEO-F103RB 보드를 이용한 장애물 감지센서 모듈 테스트 예제입니다.

## 📋 개요

장애물 감지센서 모듈은 적외선(IR) 송수신을 이용하여 전방의 장애물을 감지합니다. 로봇의 충돌 방지, 자동문, 물체 감지 등에 활용됩니다.

## 🔧 하드웨어 요구사항

- **보드**: NUCLEO-F103RB (STM32F103RBT6)
- **센서**: 장애물 감지센서 모듈 (IR 기반)
- **점퍼 와이어**: 3개
- **(선택) 부저**: 경고음 출력용

## 📌 핀 연결

| 장애물 센서 모듈 | NUCLEO-F103RB | 설명 |
|---------------|---------------|------|
| VCC | 3.3V ~ 5V | 전원 공급 |
| GND | GND | 접지 |
| OUT | PA9 (D8) | 디지털 출력 |

### 회로도

```
NUCLEO-F103RB          장애물 감지센서 모듈
    5V   ─────────────── VCC
    GND  ─────────────── GND
    PA9  ─────────────── OUT
```

### 선택적 부저 연결

```
NUCLEO-F103RB          부저
    PA11 ─────────────── + (긴 다리)
    GND  ─────────────── - (짧은 다리)
```

## 💻 소프트웨어 설정

### Arduino IDE 설정

1. **보드 매니저에서 STM32 보드 패키지 설치**
   - File → Preferences → Additional Board Manager URLs에 추가:
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
   
2. **보드 선택**
   - Tools → Board → STM32 Boards → Nucleo-64
   - Board Part Number: Nucleo F103RB

3. **포트 선택**
   - Tools → Port → COMx (해당 포트 선택)

## 📝 코드 설명

### 동작 원리

```
IR LED 발광 → 물체에 반사 → 포토트랜지스터 수신 → LM393 비교 → OUT 출력
```

| 상황 | IR 반사 | OUT 출력 | LED |
|-----|--------|---------|-----|
| 장애물 있음 | 있음 | LOW | ON |
| 장애물 없음 | 없음 | HIGH | OFF |

### 주요 함수

```cpp
bool isObstacleDetected()              // 장애물 감지 여부 반환
bool waitForObstacle(timeout)          // 장애물 감지 대기
bool waitForClear(timeout)             // 장애물 제거 대기
void resetStatistics()                 // 통계 초기화
void continuousSensorTest(duration)    // 연속 센서 테스트
```

### 이벤트 콜백

```cpp
void onObstacleDetected()    // 장애물 감지 시 호출
void onObstacleCleared()     // 장애물 제거 시 호출
```

## 🖥️ 시리얼 출력 예시

```
================================================
  장애물 감지센서 테스트 (Obstacle Sensor)
================================================
보드: NUCLEO-F103RB
센서 핀: PA9 (D8)
감지 거리: 2cm ~ 30cm (가변저항 조절)
------------------------------------------------
장애물 감지 = LOW (LED ON)
장애물 없음 = HIGH (LED OFF)
------------------------------------------------
Tip: 모듈의 가변저항으로 감지 거리를 조절하세요.
================================================

상태: [□ 장애물 없음]  | 총 감지: 0회  | 누적 시간: 0.0초  --------

╔════════════════════════════════════════════╗
║     ⚠️  장애물 감지! (OBSTACLE DETECTED)     ║
╚════════════════════════════════════════════╝
감지 횟수: 1회
상태: [■ 장애물 있음]  지속: 0.5초  | 총 감지: 1회  | 누적 시간: 0.5초  ████████
상태: [■ 장애물 있음]  지속: 1.0초  | 총 감지: 1회  | 누적 시간: 1.0초  ████████

────────────────────────────────────────────
    ✓ 장애물 제거됨 (OBSTACLE CLEARED)
    감지 지속 시간: 1523ms
────────────────────────────────────────────
상태: [□ 장애물 없음]  | 총 감지: 1회  | 누적 시간: 1.5초  --------
```

## 🔧 감지 거리 조절 방법

1. **하드웨어 조절**: 모듈의 파란색 가변저항 조절
   - 시계 방향: 감지 거리 증가
   - 반시계 방향: 감지 거리 감소

2. **조절 순서**:
   - 원하는 감지 거리에 물체 배치
   - 가변저항을 천천히 돌려 LED가 켜지는 지점 찾기
   - 약간 더 돌려 안정적인 감지 확보

## 📊 응용 예제

### 자동 정지 로봇

```cpp
void loop() {
    if (isObstacleDetected()) {
        stopMotors();
        Serial.println("장애물 감지! 정지!");
    } else {
        moveForward();
    }
}
```

### 물체 카운터

```cpp
void onObstacleDetected() {
    static int objectCount = 0;
    objectCount++;
    Serial.print("물체 통과: ");
    Serial.println(objectCount);
}
```

### 근접 경고 시스템

```cpp
void loop() {
    if (isObstacleDetected()) {
        // 부저 경고음
        tone(BUZZER_PIN, 1000, 100);
        // LED 점멸
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
    }
}
```

## 📋 사양

| 항목 | 사양 |
|-----|------|
| 동작 전압 | 3.3V ~ 5V |
| 감지 거리 | 2cm ~ 30cm (조절 가능) |
| 감지 각도 | 약 35° |
| 출력 형식 | 디지털 (LOW/HIGH) |
| 응답 시간 | < 2ms |

## ⚠️ 주의사항

1. **검은색 물체**: 적외선을 흡수하여 감지가 어려울 수 있음
2. **투명한 물체**: 유리, 아크릴 등은 적외선이 투과하여 감지 불가
3. **주변 광원**: 강한 햇빛이나 IR 조명이 있으면 오동작 가능
4. **반사면 각도**: 경사진 표면은 감지 거리가 변할 수 있음

## 🔍 트러블슈팅

| 증상 | 원인 | 해결방법 |
|-----|------|---------|
| 항상 감지됨 | 감도 너무 높음 | 가변저항으로 감도 낮춤 |
| 감지 안됨 | 감도 너무 낮음 또는 배선 오류 | 감도 높이거나 배선 확인 |
| 불안정한 감지 | 주변 IR 간섭 | 차광 처리 또는 위치 변경 |
| 검은색만 감지 안됨 | IR 흡수 | 초음파 센서 병용 권장 |
| 특정 거리에서만 동작 | 가변저항 설정 | 원하는 거리에서 재조절 |

## 🔄 다른 센서와의 비교

| 특성 | IR 장애물 센서 | 초음파 센서 |
|-----|--------------|-----------|
| 감지 방식 | 적외선 반사 | 음파 반사 |
| 감지 거리 | 2-30cm | 2-400cm |
| 거리 측정 | 불가 (ON/OFF만) | 가능 |
| 검은색 감지 | 어려움 | 가능 |
| 투명체 감지 | 불가 | 가능 |
| 가격 | 저렴 | 보통 |
| 응답 속도 | 빠름 | 느림 |

## 📄 라이선스

MIT License

## 🔗 참고 자료

- [STM32duino 공식 문서](https://github.com/stm32duino/Arduino_Core_STM32)
- [LM393 데이터시트](https://www.ti.com/lit/ds/symlink/lm393.pdf)
- [NUCLEO-F103RB 핀맵](https://os.mbed.com/platforms/ST-Nucleo-F103RB/)
