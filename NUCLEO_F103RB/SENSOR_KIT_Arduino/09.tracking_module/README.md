# 트래킹 모듈 (Line Tracking Sensor Module)

NUCLEO-F103RB 보드를 이용한 트래킹(라인 감지) 센서 모듈 테스트 예제입니다.

## 📋 개요

트래킹 모듈은 TCRT5000 적외선 센서를 기반으로 바닥의 검은색 라인을 감지합니다. 라인 트레이서 로봇, 자동 주행 시스템 등에 활용됩니다.

## 🔧 하드웨어 요구사항

- **보드**: NUCLEO-F103RB (STM32F103RBT6)
- **센서**: 3채널 트래킹 센서 모듈 (TCRT5000 기반)
- **점퍼 와이어**: 5개

## 📌 핀 연결

| 트래킹 모듈 | NUCLEO-F103RB | 설명 |
|-----------|---------------|------|
| VCC | 5V | 전원 공급 |
| GND | GND | 접지 |
| L (왼쪽) | PB5 (D4) | 왼쪽 센서 출력 |
| C (중앙) | PB4 (D5) | 중앙 센서 출력 |
| R (오른쪽) | PB10 (D6) | 오른쪽 센서 출력 |

### 회로도

```
NUCLEO-F103RB          3채널 트래킹 모듈
    5V   ─────────────── VCC
    GND  ─────────────── GND
    PB5  ─────────────── L (왼쪽)
    PB4  ─────────────── C (중앙)
    PB10 ─────────────── R (오른쪽)
```

### 센서 배치도

```
       진행 방향
          ↑
    ┌─────────────┐
    │  L   C   R  │
    │  ●   ●   ●  │  <- IR 센서
    └─────────────┘
         로봇
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

| 바닥 색상 | IR 반사 | 센서 출력 | 감지 상태 |
|---------|--------|----------|----------|
| 검은색 (라인) | 흡수 | LOW | 감지됨 |
| 흰색 (바닥) | 반사 | HIGH | 미감지 |

### 센서 조합 및 방향 결정

| L | C | R | 이진값 | 동작 | 설명 |
|---|---|---|-------|------|------|
| 0 | 0 | 0 | 0b000 | LINE_LOST | 라인 이탈 |
| 0 | 0 | 1 | 0b001 | SHARP_RIGHT | 급우회전 |
| 0 | 1 | 0 | 0b010 | FORWARD | 직진 |
| 0 | 1 | 1 | 0b011 | TURN_RIGHT | 우회전 |
| 1 | 0 | 0 | 0b100 | SHARP_LEFT | 급좌회전 |
| 1 | 0 | 1 | 0b101 | FORWARD | 교차로 직진 |
| 1 | 1 | 0 | 0b110 | TURN_LEFT | 좌회전 |
| 1 | 1 | 1 | 0b111 | FORWARD | 넓은 라인 |

*1 = 라인 감지, 0 = 라인 미감지*

### 주요 함수

```cpp
void readSensors()                    // 센서 값 읽기
Direction determineDirection()        // 방향 결정
void calibrateSensors()               // 센서 보정
void testSingleSensor(pin, name)      // 단일 센서 테스트
```

## 🖥️ 시리얼 출력 예시

```
================================================
  트래킹 모듈 테스트 (Line Tracking Sensor)
================================================
보드: NUCLEO-F103RB
왼쪽 센서: PB5 (D4)
중앙 센서: PB4 (D5)
오른쪽 센서: PB10 (D6)
------------------------------------------------
검은색 라인 = 감지 (LOW)
흰색 바닥   = 미감지 (HIGH)
================================================

센서: [□|■|□]  L:0 C:1 R:0  -> 직진        ↑    
센서: [□|■|■]  L:0 C:1 R:1  -> 우회전          ↗  
센서: [□|□|■]  L:0 C:0 R:1  -> 급우회전          →
센서: [■|■|□]  L:1 C:1 R:0  -> 좌회전      ↖      
센서: [■|□|□]  L:1 C:0 R:0  -> 급좌회전  ←        
센서: [□|□|□]  L:0 C:0 R:0  -> 라인이탈      ?    
```

## 🔧 감도 조절 방법

1. **하드웨어 조절**: 각 센서 옆의 가변저항 조절
   - 시계 방향: 감도 증가
   - 반시계 방향: 감도 감소

2. **최적 설정 방법**:
   - 센서를 흰색 바닥에 놓고 LED가 꺼지도록 조절
   - 검은색 라인에 놓고 LED가 켜지는지 확인
   - 경계에서 확실한 전환이 되도록 미세 조정

## 📊 응용 예제

### 모터 제어 연동

```cpp
void controlMotors(Direction dir) {
    switch (dir) {
        case FORWARD:
            setMotorSpeed(LEFT_MOTOR, 200);
            setMotorSpeed(RIGHT_MOTOR, 200);
            break;
        case TURN_LEFT:
            setMotorSpeed(LEFT_MOTOR, 100);
            setMotorSpeed(RIGHT_MOTOR, 200);
            break;
        case TURN_RIGHT:
            setMotorSpeed(LEFT_MOTOR, 200);
            setMotorSpeed(RIGHT_MOTOR, 100);
            break;
        // ... 기타 방향
    }
}
```

### 교차로 감지

```cpp
bool isIntersection() {
    return (sensorState.left && sensorState.center && sensorState.right);
}
```

## ⚠️ 주의사항

1. **감지 거리**: 센서와 바닥 사이 거리 1~25mm 유지
2. **조명 조건**: 강한 직사광선 아래에서는 오동작 가능
3. **바닥 재질**: 반사율이 너무 높거나 낮은 바닥 주의
4. **라인 폭**: 센서 간격에 맞는 적절한 라인 폭 사용 (보통 2~3cm)

## 🔍 트러블슈팅

| 증상 | 원인 | 해결방법 |
|-----|------|---------|
| 모든 센서 항상 HIGH | 감도 너무 낮음 | 가변저항으로 감도 증가 |
| 모든 센서 항상 LOW | 감도 너무 높음 | 가변저항으로 감도 감소 |
| 불안정한 감지 | 거리 또는 조명 문제 | 센서 높이 조절, 차광 처리 |
| 특정 센서만 동작 안함 | 배선 또는 센서 불량 | 배선 확인, 센서 교체 |

## 📄 라이선스

MIT License

## 🔗 참고 자료

- [STM32duino 공식 문서](https://github.com/stm32duino/Arduino_Core_STM32)
- [TCRT5000 데이터시트](https://www.vishay.com/docs/83760/tcrt5000.pdf)
- [라인 트레이서 로봇 설계 가이드](https://www.instructables.com/Line-Follower-Robot/)
