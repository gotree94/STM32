# 터치 센서 모듈 (Touch Sensor Module)

NUCLEO-F103RB 보드를 이용한 정전식 터치 센서 테스트 프로젝트

## 📋 개요

터치 센서 모듈은 정전용량 방식으로 터치를 감지하는 센서입니다. 일반적으로 TTP223 칩을 사용하며, 버튼 대용, UI 인터페이스, 근접 감지 등에 활용됩니다.

## 🔧 하드웨어 구성

### 부품 목록
| 부품명 | 수량 | 비고 |
|--------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RBT6 |
| 터치 센서 모듈 | 1 | TTP223 기반 |
| 점퍼 와이어 | 3 | M-F 타입 |

### 핀 연결
| 센서 핀 | NUCLEO 핀 | 설명 |
|---------|-----------|------|
| VCC | 3.3V / 5V | 전원 |
| GND | GND | 접지 |
| SIG/OUT | D2 (PA10) | 신호 출력 |

### 회로도
```
                    NUCLEO-F103RB
                   ┌─────────────┐
                   │             │
  Touch Sensor     │             │
  ┌─────────┐      │             │
  │ VCC ────┼──────┤ 3.3V       │
  │ GND ────┼──────┤ GND        │
  │ SIG ────┼──────┤ D2 (PA10)  │
  └─────────┘      │             │
       ↑           │             │
    [터치]         │    LED(PA5) │ ← 내장 LED
                   └─────────────┘
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

### 정전용량 방식 (Capacitive Touch)
터치 패드와 손가락 사이에 형성되는 정전용량의 변화를 감지합니다. 손가락이 접근하면 정전용량이 증가하고, 이를 TTP223 칩이 감지하여 디지털 신호로 출력합니다.

### TTP223 동작 모드
| 점퍼 설정 | 모드 | 설명 |
|----------|------|------|
| A 오픈 | 순간(Momentary) | 터치 중에만 출력 |
| A 브릿지 | 토글(Toggle) | 터치할 때마다 상태 반전 |
| B 오픈 | Active High | 터치 시 HIGH 출력 |
| B 브릿지 | Active Low | 터치 시 LOW 출력 |

### 출력 특성 (기본 설정)
- **대기 상태**: LOW (0)
- **터치 상태**: HIGH (1)

## 📝 시리얼 출력 예시

```
========================================
  Touch Sensor Module Test
  Board: NUCLEO-F103RB
========================================

Touch sensor initialized.
Touch the sensor pad...

Commands via Serial:
  'r' - Reset statistics
  's' - Show statistics

----------------------------------------
TOUCH #1 - Started
Time: 00:05
TOUCH #1 - Released (Short tap)
Duration: 156 ms
----------------------------------------

----------------------------------------
TOUCH #2 - Started
Time: 00:08
TOUCH #2 - Released (Long press)
Duration: 1234 ms
----------------------------------------

========== TOUCH STATISTICS ==========
Total touches: 2
Short taps (<500ms): 1
Long presses (>=500ms): 1
Total touch time: 1390 ms
Average touch duration: 695 ms
Current state: Released
=======================================
```

## 🎮 시리얼 명령어

| 명령 | 기능 |
|------|------|
| `r` / `R` | 통계 초기화 |
| `s` / `S` | 현재 통계 출력 |

## 🔬 응용 예제

### 1. 터치 버튼
```cpp
// 간단한 터치 버튼으로 LED 제어
if (touchState == HIGH) {
  // 버튼 눌림 동작
  toggleLED();
}
```

### 2. 롱 프레스 감지
```cpp
// 긴 터치와 짧은 터치 구분
if (touchDuration >= 1000) {
  // 1초 이상: 설정 모드 진입
  enterSettingMode();
} else if (touchDuration >= 50) {
  // 50ms~1초: 일반 선택
  selectItem();
}
```

### 3. 더블 탭 감지
```cpp
unsigned long lastTapTime = 0;
const unsigned long doubleTapInterval = 300;

void handleTap() {
  unsigned long currentTime = millis();
  if (currentTime - lastTapTime < doubleTapInterval) {
    // 더블 탭 감지됨
    handleDoubleTap();
  }
  lastTapTime = currentTime;
}
```

## ⚠️ 주의사항

1. **감도 조절**: 일부 모듈은 가변저항으로 감도 조절 가능
2. **절연 두께**: 터치 패드 위에 덮개를 씌울 경우 감도 조정 필요
3. **접지 연결**: 안정적인 GND 연결이 중요
4. **전원 노이즈**: 전원 노이즈가 오동작을 유발할 수 있음
5. **습기 영향**: 습한 환경에서 오감지 가능성 있음

## 📁 파일 구조

```
03_Touch_Sensor/
├── Touch_Sensor.ino    # 아두이노 소스 코드
└── README.md           # 프로젝트 설명서
```

## 🔗 참고 자료

- [STM32F103RB Datasheet](https://www.st.com/resource/en/datasheet/stm32f103rb.pdf)
- [TTP223 Datasheet](https://www.alldatasheet.com/datasheet-pdf/pdf/795463/ETC2/TTP223.html)
- [Capacitive Touch Sensing Theory](https://www.analog.com/en/analog-dialogue/articles/capacitive-touch-sensing.html)

## 📜 라이선스

MIT License
