# 리드 스위치 모듈 (Reed Switch Module)

NUCLEO-F103RB 보드를 이용한 리드 스위치 테스트 프로젝트

## 📋 개요

리드 스위치는 자기장에 의해 동작하는 전자기계식 스위치입니다. 유리관 안에 두 개의 강자성체 리드가 들어있으며, 자석이 접근하면 리드가 서로 접촉하여 회로가 연결됩니다. 문/창문 열림 감지, 보안 시스템, 위치 감지 등에 널리 사용됩니다.

## 🔧 하드웨어 구성

### 부품 목록
| 부품명 | 수량 | 비고 |
|--------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RBT6 |
| 리드 스위치 모듈 | 1 | DO 출력 |
| 네오디뮴 자석 | 1 | 테스트용 |
| 점퍼 와이어 | 3~4 | M-F 타입 |

### 핀 연결
| 센서 핀 | NUCLEO 핀 | 설명 |
|---------|-----------|------|
| VCC | 3.3V / 5V | 전원 |
| GND | GND | 접지 |
| DO | D2 (PA10) | 디지털 출력 |
| AO | A0 (PA0) | 아날로그 출력 (선택) |

### 회로도
```
                    NUCLEO-F103RB
                   ┌─────────────┐
                   │             │
  Reed Switch      │             │
  Module           │             │
  ┌─────────┐      │             │
  │ VCC ────┼──────┤ 3.3V       │
  │ GND ────┼──────┤ GND        │
  │ DO  ────┼──────┤ D2 (PA10)  │
  │ AO  ────┼──────┤ A0 (PA0)   │ (선택)
  └─────────┘      │             │
                   │    LED(PA5) │ ← 내장 LED
       [자석]      └─────────────┘
         ↓
    ┌─────────┐
    │ Reed SW │ ← 자석 접근 시 스위치 동작
    └─────────┘
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

### 리드 스위치 구조
```
    유리관
  ┌──────────────────┐
  │  ┌──┐      ┌──┐  │
  │  │  │ →  ← │  │  │  ← 강자성체 리드
  │  └──┘      └──┘  │
  └──────────────────┘
       평상시 (Open)

  ┌──────────────────┐
  │  ┌──┐┌────┐┌──┐  │
  │  │  ││    ││  │  │  ← 자석 접근 시 접촉
  │  └──┘└────┘└──┘  │
  └──────────────────┘
       자석 접근 (Closed)
```

### 출력 특성 (모듈에 따라 다를 수 있음)
| 상태 | DO 출력 | 설명 |
|------|---------|------|
| 자석 접근 (문 닫힘) | LOW | 스위치 ON |
| 자석 이탈 (문 열림) | HIGH | 스위치 OFF |

### 모듈 설정
- **Active LOW (기본)**: 자석 감지 시 LOW 출력
- **Active HIGH**: 자석 감지 시 HIGH 출력
- 코드에서 `ACTIVE_LOW` 변수로 설정 가능

## 📝 시리얼 출력 예시

```
========================================
  Reed Switch Module Test
  Board: NUCLEO-F103RB
========================================

Reed switch initialized.
Switch type: Active LOW (normally open)

Initial state: CLOSED (magnet detected)

Commands via Serial:
  's' - Show statistics
  'r' - Reset statistics

Monitoring for state changes...

========================================
STATE CHANGE at 00:00:05
>>> OPENED (Magnet removed)
Open count: 1
========================================

========================================
STATE CHANGE at 00:00:08
<<< CLOSED (Magnet detected)
Close count: 1
Was open for: 3s
========================================

=========== REED SWITCH STATISTICS ===========
Current state: CLOSED

Open events: 3
Close events: 3

Last opened: 45s ago
Last closed: 12s ago

Total open time: 1m 23s
==============================================
```

## 🎮 시리얼 명령어

| 명령 | 기능 |
|------|------|
| `s` / `S` | 통계 출력 |
| `r` / `R` | 통계 초기화 |

## 🔬 응용 예제

### 1. 문/창문 알람 시스템
```cpp
// 문이 일정 시간 이상 열려있으면 경보
const unsigned long ALARM_THRESHOLD = 60000;  // 60초

void checkDoorAlarm() {
  if (isDoorOpen && (millis() - doorOpenStartTime) > ALARM_THRESHOLD) {
    soundAlarm();
  }
}
```

### 2. 열림 횟수 기반 알림
```cpp
// 냉장고 문 열림 횟수 모니터링
const int MAX_OPENS_PER_HOUR = 20;

void checkOpenFrequency() {
  if (openCount > MAX_OPENS_PER_HOUR) {
    sendNotification("냉장고 문이 너무 자주 열렸습니다!");
  }
}
```

### 3. 회전 감지 (자석 + 리드 스위치)
```cpp
// 휠에 자석 부착하여 회전수 카운트
volatile unsigned long rotationCount = 0;

void countRotation() {
  rotationCount++;
}

float calculateRPM() {
  // 1분간 회전수 계산
  return rotationCount * 60.0 / (millis() / 1000.0);
}
```

## ⚠️ 주의사항

1. **자석 극성**: 자석의 극성에 따라 감지 거리가 달라질 수 있음
2. **자석 강도**: 적절한 강도의 자석 사용 (너무 약하면 감지 안됨)
3. **설치 거리**: 자석과 리드 스위치 사이 거리 5~15mm 권장
4. **디바운싱**: 기계식 스위치이므로 디바운싱 처리 필요
5. **진동 주의**: 강한 진동 환경에서 오동작 가능성
6. **수명**: 기계식이므로 수명 제한 있음 (일반적으로 수백만 회)

## 📁 파일 구조

```
05_Reed_Switch/
├── Reed_Switch.ino    # 아두이노 소스 코드
└── README.md          # 프로젝트 설명서
```

## 🔗 참고 자료

- [STM32F103RB Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)
- [Reed Switch Working Principle](https://www.electronics-tutorials.ws/io/io_6.html)
- [Debouncing Techniques](https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce)

## 📜 라이선스

MIT License
