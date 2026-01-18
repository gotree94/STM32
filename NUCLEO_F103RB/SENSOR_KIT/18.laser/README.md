# Laser Module - STM32F103 Test

## 📌 개요

레이저 모듈을 NUCLEO-F103RB 보드에 연결하여 다양한 동작 모드를 테스트하는 프로젝트입니다. PWM을 이용한 밝기 조절, 점멸, SOS 신호 등의 기능을 구현합니다.

⚠️ **경고**: 레이저를 절대 눈에 직접 비추지 마세요! 적절한 레이저 안전 수칙을 준수하세요.

## 🔧 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 설명 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 개발보드 |
| Laser Module | 1 | KY-008 또는 호환 레이저 모듈 |
| Jumper Wires | 3 | 연결용 점퍼선 |

### 핀 연결

```
┌─────────────────┐         ┌──────────────────┐
│  Laser Module   │         │  NUCLEO-F103RB   │
├─────────────────┤         ├──────────────────┤
│     VCC     ────┼─────────┼──── 5V           │
│     GND     ────┼─────────┼──── GND          │
│     S       ────┼─────────┼──── PA0 (PWM)    │
└─────────────────┘         │                  │
                            │     PC13 ────────│──── User Button
                            │     PA5 ─────────│──── Built-in LED
                            └──────────────────┘
```

### 회로도

```
       +5V
        │
   ┌────┴────┐
   │  Laser  │
   │ Module  │
   │  ┌───┐  │
   │  │LD │  │          ┌──────────┐
   │  └───┘  │          │          │
   │    S────┼──────────┤ PA0(PWM) │
   │         │          │          │
   │   GND───┼──────────┤ GND      │
   └─────────┘          │          │
                        │  PC13    │◄── User Button
                        └──────────┘
```

## 📂 프로젝트 구조

```
laser/
├── main.c          # 메인 소스 코드
├── README.md       # 프로젝트 문서
└── Makefile        # 빌드 설정 (선택)
```

## 💻 소프트웨어 동작

### 동작 모드

| 모드 | 번호 | 설명 |
|------|------|------|
| OFF | 0 | 레이저 완전 꺼짐 |
| ON | 1 | 레이저 연속 켜짐 |
| BLINK | 2 | 500ms 간격 점멸 |
| PWM | 3 | 부드러운 밝기 변화 (Breathing) |
| SOS | 4 | 모스 부호 SOS 신호 |

### 모드 전환
- **User Button (PC13)**: 버튼을 누를 때마다 모드 순환 (OFF → ON → BLINK → PWM → SOS → OFF...)

### SOS 모스 부호 패턴

```
S = ・・・ (짧음 3회)
O = ─ ─ ─ (길음 3회)  
S = ・・・ (짧음 3회)

타이밍:
・ (짧음) = 200ms
─ (길음) = 600ms
간격     = 200ms
단어 간격 = 1000ms
```

### PWM 설정
| 파라미터 | 값 | 설명 |
|----------|-----|------|
| PWM 주파수 | 1kHz | 부드러운 밝기 제어 |
| Duty Cycle 범위 | 0-100% | 0-1000 값 |
| Breathing 속도 | ~3초 주기 | 한 사이클 (밝아짐→어두워짐) |

## 🚀 빌드 및 실행

### STM32CubeIDE 사용

1. **새 프로젝트 생성**
   - File → New → STM32 Project
   - Board Selector에서 NUCLEO-F103RB 선택

2. **핀 설정 (CubeMX)**
   - PA0: TIM2_CH1 (PWM Output)
   - PA5: GPIO_Output (LED)
   - PC13: GPIO_Input (User Button)
   - USART2: Asynchronous, 115200 baud
   - TIM2: PWM Generation CH1

3. **타이머 설정**
   ```
   TIM2 Configuration:
   - Prescaler: 71 (72MHz / 72 = 1MHz)
   - Counter Period: 999 (1MHz / 1000 = 1kHz PWM)
   - PWM Mode: PWM Mode 1
   ```

4. **코드 추가**
   - `main.c` 내용을 생성된 프로젝트에 적용

5. **빌드 및 다운로드**
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
  Laser Module Test
  NUCLEO-F103RB
========================================
WARNING: Never point laser at eyes!

Modes (Press User Button to change):
  0: OFF
  1: ON (Continuous)
  2: BLINK
  3: PWM (Breathing)
  4: SOS

[MODE] Current: OFF
[MODE] Current: ON
[MODE] Current: BLINK
[MODE] Current: PWM
[MODE] Current: SOS
[MODE] Current: OFF
```

## 🔍 트러블슈팅

| 문제 | 원인 | 해결 방법 |
|------|------|----------|
| 레이저가 켜지지 않음 | 전원 불량 | 5V 전원 확인 |
| PWM이 동작하지 않음 | 타이머 설정 오류 | TIM2_CH1 설정 확인 |
| 버튼 반응 없음 | 디바운싱 문제 | 버튼 연결 및 딜레이 확인 |
| 밝기 조절 안됨 | 모듈 PWM 미지원 | 모듈 사양 확인 |

## ⚠️ 안전 주의사항

1. **눈 보호**: 레이저를 절대 눈에 직접 비추지 마세요
2. **반사 주의**: 거울 등 반사체 주변에서 주의
3. **어린이 주의**: 어린이의 손이 닿지 않는 곳에 보관
4. **클래스 확인**: 대부분의 모듈은 Class 3R 이하 (5mW 미만)

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [NUCLEO-F103RB User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [Laser Safety Standards (IEC 60825)](https://www.iec.ch/homepage)

## 📝 라이선스

MIT License

## ✍️ 작성자

Embedded Systems Lab - 2025
