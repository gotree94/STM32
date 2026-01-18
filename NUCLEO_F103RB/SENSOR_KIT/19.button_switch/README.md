# Button Switch Module - STM32F103 Test

## 📌 개요

버튼 스위치 모듈을 NUCLEO-F103RB 보드에 연결하여 다양한 버튼 이벤트를 감지하는 프로젝트입니다. 짧은 누름, 긴 누름, 더블 클릭을 구분하여 감지하며, 소프트웨어 디바운싱을 구현합니다.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| Button Switch Module | 1 | 버튼 스위치 모듈 (KY-004 등) |
| Jumper Wires | 3 | 연결용 점퍼선 |

### 핀 연결

```
┌─────────────────┐         ┌──────────────────┐
│  Button Module  │         │  NUCLEO-F103RB   │
├─────────────────┤         ├──────────────────┤
│     VCC     ────┼─────────┼──── 3.3V         │
│     GND     ────┼─────────┼──── GND          │
│     S       ────┼─────────┼──── PA0          │
└─────────────────┘         │                  │
                            │     PA5 ─────────│──── Built-in LED
                            └──────────────────┘
```

### 회로도

```
       +3.3V
         │
    ┌────┴────┐
    │ Button  │
    │ Module  │        Internal Pull-up
    │  ┌───┐  │              │
    │  │ SW│──┼──────────────┼──── PA0
    │  └───┘  │              │
    │   GND───┼──────────────┴──── GND
    └─────────┘

    동작: 버튼 미누름 = HIGH (Pull-up)
          버튼 누름   = LOW (GND 연결)
```

## 📂 프로젝트 구조

```
button_switch/
├── main.c          # 메인 소스 코드
├── README.md       # 프로젝트 문서
└── Makefile        # 빌드 설정 (선택)
```

## 💻 소프트웨어 동작

### 감지 이벤트

| 이벤트 | 조건 | LED 피드백 |
|--------|------|-----------|
| Short Press | 누름 < 1초 | 누르는 동안 켜짐 |
| Long Press | 누름 >= 1초 | 3회 깜빡임 |
| Double Click | 2회 클릭 간격 < 300ms | 5회 빠른 깜빡임 |

### 상태 다이어그램

```
                    ┌────────────────┐
                    │     IDLE       │
                    │  (Button HIGH) │
                    └───────┬────────┘
                            │ Button Pressed
                            ▼
                    ┌────────────────┐
                    │   PRESSED      │
                    │  Start Timer   │
                    └───────┬────────┘
                            │
              ┌─────────────┼─────────────┐
              │             │             │
              ▼             ▼             ▼
     ┌────────────┐  ┌────────────┐  ┌────────────┐
     │ < 1 sec    │  │ >= 1 sec   │  │ 2nd Click  │
     │            │  │            │  │ < 300ms    │
     └─────┬──────┘  └─────┬──────┘  └─────┬──────┘
           │               │               │
           ▼               ▼               ▼
     ┌──────────┐    ┌──────────┐    ┌──────────┐
     │  SHORT   │    │   LONG   │    │  DOUBLE  │
     │  PRESS   │    │  PRESS   │    │  CLICK   │
     └──────────┘    └──────────┘    └──────────┘
```

### 디바운싱 알고리즘

```c
/* 타이밍 파라미터 */
#define DEBOUNCE_MS         50      /* 채터링 방지 */
#define LONG_PRESS_MS       1000    /* 긴 누름 기준 */
#define DOUBLE_CLICK_MS     300     /* 더블 클릭 창 */
```

## 🚀 빌드 및 실행

### STM32CubeIDE 사용

1. **새 프로젝트 생성**
   - File → New → STM32 Project
   - Board Selector에서 NUCLEO-F103RB 선택

2. **핀 설정 (CubeMX)**
   - PA0: GPIO_Input (Pull-up)
   - PA5: GPIO_Output (LED)
   - USART2: Asynchronous, 115200 baud

3. **코드 추가**
   - `main.c` 내용을 생성된 프로젝트에 적용

4. **빌드 및 다운로드**
   - Project → Build All
   - Run → Debug

### 시리얼 모니터 설정
- Baud Rate: 115200
- Data Bits: 8
- Stop Bits: 1
- Parity: None

## 📊 출력 예시

```
========================================
  Button Switch Module Test
  NUCLEO-F103RB
========================================
Button Events Detected:
  - Short Press (< 1 sec)
  - Long Press (>= 1 sec)
  - Double Click (< 300ms)

Press the button to start...

[PRESS] Button pressed (#1)
[RELEASE] Duration: 150 ms
>>> EVENT: SHORT PRESS <<<
    Action: Toggle something

[PRESS] Button pressed (#2)
[RELEASE] Duration: 1250 ms
>>> EVENT: LONG PRESS <<<
    Action: Enter settings mode

[PRESS] Button pressed (#3)
[RELEASE] Duration: 80 ms
[PRESS] Button pressed (#4)
[RELEASE] Duration: 90 ms
>>> EVENT: DOUBLE CLICK <<<
    Action: Special function
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결 방법 |
|------|------|----------|
| 버튼 반응 없음 | 배선 오류 | S핀 연결 확인 |
| 채터링 발생 | 디바운스 부족 | DEBOUNCE_MS 값 증가 |
| 더블클릭 감지 안됨 | 클릭 간격 너무 김 | DOUBLE_CLICK_MS 값 증가 |
| 긴 누름 감지 안됨 | 임계값 문제 | LONG_PRESS_MS 값 조정 |
| 항상 눌림 상태 | Pull-up 설정 오류 | GPIO_PULLUP 설정 확인 |

### 파라미터 튜닝 가이드

사용자 특성에 따라 조정:

```c
/* 노인 또는 느린 반응 */
#define DEBOUNCE_MS         100
#define LONG_PRESS_MS       1500
#define DOUBLE_CLICK_MS     500

/* 게이머 또는 빠른 반응 */
#define DEBOUNCE_MS         30
#define LONG_PRESS_MS       800
#define DOUBLE_CLICK_MS     200
```

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [Debouncing Techniques](https://www.embedded.com/my-favorite-software-debouncers/)

## 📝 라이선스

MIT License

## ✍️ 작성자

Embedded Systems Lab - 2025
