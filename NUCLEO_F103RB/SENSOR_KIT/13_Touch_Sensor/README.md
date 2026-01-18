# 터치 센서 모듈 (Touch Sensor Module)

STM32F103 NUCLEO 보드를 이용한 정전식 터치 센서 테스트 프로젝트

## 📋 개요

정전식 터치 센서 모듈(TTP223 기반)은 손가락의 접촉을 감지하여 디지털 신호를 출력합니다. 기계적 버튼 없이 터치만으로 입력을 받을 수 있어 다양한 사용자 인터페이스에 활용됩니다.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| 터치 센서 모듈 | 1 | TTP223 기반 |
| 점퍼 와이어 | 3 | 연결용 |

### 핀 연결

```
터치 센서 모듈          NUCLEO-F103RB
┌─────────────┐        ┌─────────────┐
│     VCC     │───────▶│    3.3V     │
│     GND     │───────▶│    GND      │
│     SIG     │───────▶│    PA0      │ (Digital Input)
└─────────────┘        └─────────────┘
```

### 회로도

```
                    ┌─────────────────────┐
                    │   TTP223 Touch      │
                    │   Sensor Module     │
    3.3V ──────────▶│ VCC                 │
                    │     ┌───────┐       │
    GND  ──────────▶│ GND │ Touch │       │
                    │     │ Pad   │       │
    PA0  ◀──────────│ SIG └───────┘       │
                    │     (Capacitive)    │
                    └─────────────────────┘
                              │
                              ▼
                         손가락 터치
```

## 💻 소프트웨어 설명

### 주요 기능

1. **디바운싱**: 안정적인 터치 감지를 위한 소프트웨어 디바운싱
2. **제스처 인식**:
   - TAP: 짧은 터치
   - DOUBLE TAP: 빠른 연속 터치
   - LONG PRESS: 길게 누르기
3. **터치 통계**: 터치 횟수 및 지속 시간 추적
4. **실시간 표시**: 프로그레스 바로 터치 상태 시각화

### 동작 원리

```
     터치 없음                터치 중
         │                      │
         ▼                      ▼
  ┌────────────┐         ┌────────────┐
  │  SIG = LOW │         │ SIG = HIGH │
  │    (0V)    │         │   (3.3V)   │
  └────────────┘         └────────────┘
```

- **정전 용량 방식**: 인체의 정전 용량을 이용하여 터치 감지
- **TTP223 IC**: 내부에서 터치 감지 및 디지털 출력 생성
- **동작 모드**: 순간(Momentary) 또는 토글(Toggle) 모드 선택 가능

### 제스처 타이밍

```
TAP:
   ┌──┐
───┘  └──────────────────
   <50ms

DOUBLE TAP:
   ┌──┐    ┌──┐
───┘  └────┘  └──────────
   |<--300ms-->|

LONG PRESS:
   ┌──────────────────┐
───┘                  └───
   |<----1000ms----->|
```

### 코드 구조

```
main.c
├── Touch_Read()          // 디바운싱된 터치 읽기
├── Touch_Update()        // 상태 업데이트 및 제스처 감지
├── Touch_JustPressed()   // 터치 시작 감지
├── Touch_JustReleased()  // 터치 종료 감지
├── Touch_IsPressed()     // 현재 터치 상태
├── Print_ProgressBar()   // 진행 바 출력
├── MX_GPIO_Init()        // GPIO 초기화
└── MX_USART2_UART_Init() // UART 초기화
```

## 📊 출력 예시

```
========================================
   Touch Sensor Module Test Program
   NUCLEO-F103RB Board
========================================

SIG Pin: PA0 (Digital Input)

Gesture Recognition:
  - TAP: Quick touch and release
  - DOUBLE TAP: Two taps within 300 ms
  - LONG PRESS: Hold for 1000 ms

State: RELEASED | Count:   0 | [░░░░░░░░░░░░░░░░░░░░]    0 ms
State: TOUCHED  | Count:   0 | [██░░░░░░░░░░░░░░░░░░]  200 ms
State: TOUCHED  | Count:   0 | [████████░░░░░░░░░░░░]  800 ms
>>> LONG PRESS detected! <<<
State: TOUCHED  | Count:   0 | [████████████████████] 1600 ms
State: RELEASED | Count:   1 | [░░░░░░░░░░░░░░░░░░░░]    0 ms
>>> TAP detected! (duration: 120 ms) <<<
>>> DOUBLE TAP detected! <<<
State: RELEASED | Count:   3 | [░░░░░░░░░░░░░░░░░░░░]    0 ms
```

## ⚙️ 빌드 및 실행

### STM32CubeIDE 사용 시

1. 새 프로젝트 생성 (STM32F103RB 선택)
2. `main.c` 파일 내용으로 교체
3. 빌드 및 다운로드

### 설정 변경

```c
// main.c 상단의 설정값 수정
#define DEBOUNCE_TIME_MS        50      // 디바운스 시간
#define LONG_PRESS_TIME_MS      1000    // 길게 누르기 임계값
#define DOUBLE_TAP_TIME_MS      300     // 더블탭 인식 시간
```

## 🎮 응용 예제

### 1. LED 밝기 조절
```c
// 터치 시간에 따라 PWM 듀티 조절
if (Touch_IsPressed()) {
    uint16_t duty = (touch.touch_duration > 1000) ? 
                    1000 : touch.touch_duration;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
}
```

### 2. 메뉴 네비게이션
```c
// 제스처별 다른 기능
if (double_tap_detected) {
    menu_select();
} else if (long_press_detected) {
    menu_back();
} else if (tap_detected) {
    menu_next();
}
```

### 3. 터치 버튼 매트릭스
```
┌───┬───┬───┐
│ 1 │ 2 │ 3 │  PA0, PA1, PA4
├───┼───┼───┤
│ 4 │ 5 │ 6 │  PB0, PB1, PB2
└───┴───┴───┘
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결방법 |
|------|------|----------|
| 터치 감지 안됨 | 전원 문제 | VCC/GND 연결 확인 |
| 항상 터치됨 | 모듈 고장/노이즈 | 접지 상태 확인, 모듈 교체 |
| 불안정한 감지 | 디바운스 부족 | DEBOUNCE_TIME_MS 증가 |
| 더블탭 인식 안됨 | 타이밍 문제 | DOUBLE_TAP_TIME_MS 조정 |

## ⚠️ 주의사항

1. **전원**: 반드시 3.3V 사용 (5V 사용 시 센서 손상 가능)
2. **접지**: 안정적인 접지가 중요 (플로팅 접지 주의)
3. **터치 패드**: 금속 물체로 터치 시 오작동 가능
4. **모드 설정**: 모듈의 A/B 점퍼로 순간/토글 모드 선택
5. **감도**: 일부 모듈은 감도 조절용 트리머 보유

### TTP223 모드 설정

```
A점퍼  B점퍼  모드
───────────────────
Open   Open   순간모드, Active High
Open   Close  순간모드, Active Low  
Close  Open   토글모드, Active High
Close  Close  토글모드, Active Low
```

## 📚 참고 자료

- [TTP223 Datasheet](http://www.datasheetcafe.com/ttp223-datasheet-touch-detector-ic/)
- [Capacitive Touch Sensing](https://www.analog.com/en/analog-dialogue/articles/capacitive-touch-sensing.html)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

## 📜 라이선스

MIT License
