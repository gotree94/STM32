# Mini Reed Switch Module - STM32F103 Test

## 📌 개요

미니 리드 스위치 모듈을 NUCLEO-F103RB 보드에 연결하여 자석 감지 기능을 테스트하는 프로젝트입니다. 리드 스위치는 자기장에 반응하여 회로를 열고 닫는 센서로, 문/창문 열림 감지, 위치 감지 등에 활용됩니다.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| Mini Reed Switch Module | 1 | 리드 스위치 모듈 |
| Jumper Wires | 3 | 연결용 점퍼선 |
| Magnet | 1 | 테스트용 자석 |

### 핀 연결

```
┌─────────────────┐         ┌──────────────────┐
│  Reed Module    │         │  NUCLEO-F103RB   │
├─────────────────┤         ├──────────────────┤
│     VCC     ────┼─────────┼──── 3.3V         │
│     GND     ────┼─────────┼──── GND          │
│     DO      ────┼─────────┼──── PA0          │
└─────────────────┘         │                  │
                            │     PA5 ─────────│──── Built-in LED
                            └──────────────────┘
```

### 회로도

```
       +3.3V
         │
    ┌────┴────┐
    │  Reed   │
    │ Module  │
    │         │
    │   DO────┼───────── PA0 (Input, Pull-up)
    │         │
    │   GND───┼───────── GND
    └─────────┘
```

## 📂 프로젝트 구조

```
mini_reed/
├── main.c          # 메인 소스 코드
├── README.md       # 프로젝트 문서
└── Makefile        # 빌드 설정 (선택)
```

## 💻 소프트웨어 동작

### 동작 원리
1. **초기화**: GPIO, UART 초기화 및 리드 스위치 초기 상태 확인
2. **폴링 감지**: 주기적으로 리드 스위치 상태 읽기
3. **디바운싱**: 50ms 지연으로 채터링 방지
4. **출력**: 상태 변화 시 UART로 메시지 출력 및 LED 제어

### 상태 표시
| 상태 | LED | UART 출력 |
|------|-----|----------|
| 자석 감지 (스위치 닫힘) | ON | `[EVENT] Magnet DETECTED - Reed Switch CLOSED` |
| 자석 미감지 (스위치 열림) | OFF | `[EVENT] Magnet REMOVED - Reed Switch OPEN` |

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
  Mini Reed Switch Module Test
  NUCLEO-F103RB
========================================
Bring a magnet close to the sensor...

[INIT] No magnet detected (Reed Switch OPEN)
[EVENT] Magnet DETECTED - Reed Switch CLOSED
[EVENT] Magnet REMOVED - Reed Switch OPEN
[EVENT] Magnet DETECTED - Reed Switch CLOSED
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결 방법 |
|------|------|----------|
| LED가 항상 켜져 있음 | Pull-up 설정 오류 | GPIO 설정 확인 |
| 반응이 없음 | 배선 오류 | VCC, GND, DO 연결 확인 |
| 불안정한 동작 | 디바운싱 부족 | 디바운스 딜레이 증가 |
| UART 출력 없음 | 시리얼 설정 오류 | Baud rate 115200 확인 |

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)

## 📝 라이선스

MIT License

## ✍️ 작성자

Embedded Systems Lab - 2025
