# 7-Color LED Module Test - NUCLEO-F103RB

7색 자동 변환 LED 모듈을 STM32F103 NUCLEO 보드에서 제어하는 프로젝트입니다.

## 📌 개요

7색 LED 모듈(KY-034)은 내장된 IC가 자동으로 Red, Orange, Yellow, Green, Cyan, Blue, Purple 색상을 순환합니다. 외부에서는 전원 ON/OFF와 PWM을 통한 밝기 조절만 가능하며, 색상 변화는 내장 IC가 자동으로 처리합니다.

## 🛠 하드웨어 구성

### 필요 부품
| 부품 | 수량 | 비고 |
|------|------|------|
| NUCLEO-F103RB | 1 | STM32F103RB 탑재 |
| 7색 LED 모듈 | 1 | KY-034 또는 유사 모듈 |
| 점퍼 와이어 | 3 | Female-Female |

### 핀 연결

```
7-Color LED Module      NUCLEO-F103RB
┌─────────────┐        ┌─────────────┐
│     S  ─────┼────────┤ PC8 (TIM8_CH3)
│   VCC  ─────┼────────┤ 3.3V
│   GND  ─────┼────────┤ GND
└─────────────┘        └─────────────┘
```

### 회로도

```
        3.3V
         │
    ┌────┴────────────────┐
    │   7-Color LED       │
    │   ┌─────────────┐   │
    │   │ Auto Color  │   │
    │   │   Cycle IC  │   │
    │   └──────┬──────┘   │
    │          │          │
    │   ┌──────┴──────┐   │
    │   │  RGB LED    │   │
    │   └──────┬──────┘   │
    │          │          │
    └──────────┼──────────┘
               │
               S ──────── PC8
               │
              GND
```

## 💻 소프트웨어

### 모듈 특성

7색 LED 모듈은 내장 IC에 의해 자동으로 색상이 변환됩니다:

| 순서 | 색상 | 영문 |
|------|------|------|
| 1 | 빨강 | Red |
| 2 | 주황 | Orange |
| 3 | 노랑 | Yellow |
| 4 | 초록 | Green |
| 5 | 청록 | Cyan |
| 6 | 파랑 | Blue |
| 7 | 보라 | Purple |

### 제어 방법

1. **ON/OFF 제어**: GPIO로 전원 공급 제어
2. **밝기 조절**: PWM 듀티비로 전체 밝기 조절
3. **색상 선택**: 불가능 (내장 IC가 자동 순환)

### 주요 함수

```c
// 기본 제어
void SevenColorLED_On(void);              // LED 켜기
void SevenColorLED_Off(void);             // LED 끄기
void SevenColorLED_SetBrightness(uint8_t brightness);  // 밝기 (0~255)
void SevenColorLED_Toggle(void);          // ON/OFF 토글

// 효과 데모
void SevenColorLED_AutoCycleDemo(void);   // 자동 색상 순환 관찰
void SevenColorLED_StrobeDemo(void);      // 스트로브 효과
void SevenColorLED_PatternDemo(void);     // 다양한 패턴
void SevenColorLED_BrightnessDemo(void);  // 밝기 조절
void SevenColorLED_PartyMode(void);       // 파티 모드
void SevenColorLED_NotificationDemo(void); // 알림 패턴
```

### PWM 설정

```c
Timer: TIM8
Channel: CH3 (PC8)
Prescaler: 63 (64MHz / 64 = 1MHz)
Period: 999 (1kHz PWM)
```

## 📂 프로젝트 구조

```
05_Seven_Color_LED/
├── main.c          # 메인 소스 코드
└── README.md       # 프로젝트 설명서
```

## 🔧 빌드 및 실행

### STM32CubeIDE 사용 시
1. 새 STM32 프로젝트 생성 (NUCLEO-F103RB 선택)
2. `main.c` 내용을 프로젝트에 복사
3. 빌드 후 보드에 플래시

## 📊 시리얼 출력 예시

```
=============================================
  7-Color LED Module Test - NUCLEO-F103RB
=============================================
  Module Type: Auto Color Cycling (KY-034)

[Test 1] Basic ON/OFF Control
  LED ON (5 seconds - watch color changes)...
  LED OFF...

[Test 2] Brightness Control (PWM)
  Brightness levels: 25% 50% 75% 100%
  Fade in/out...

[Test 3] Auto Color Cycle Observation
  Observing auto color cycle for 10 seconds...
  Colors: Red -> Orange -> Yellow -> Green -> Cyan -> Blue -> Purple
  [1 sec]
  [2 sec]
  ...

[Test 4] Strobe Effect
  Slow strobe...
  Fast strobe...
  Accelerating strobe...

[Test 5] Pattern Demo
  Heartbeat pattern...
  Candle flicker effect...
  Pulse effect...

[Test 6] Party Mode
  Party mode active for 10 seconds...

[Test 7] Notification Patterns
  New message notification...
  Warning notification...
  Charging notification...
  Completion notification...

--- Cycle Complete ---
```

## 📝 효과 패턴 상세

### 심박 효과 (Heartbeat)
```
첫 번째 박동: 밝기 0 → 255 → 0 (빠르게)
짧은 휴식: 80ms
두 번째 박동: 밝기 0 → 180 → 0 (약하게)
긴 휴식: 350ms
```

### 캔들 효과 (Candle Flicker)
```
불규칙한 밝기 변화 (150~250)
불규칙한 시간 간격 (50~130ms)
```

### 파티 모드
```
1. 빠른 점멸 (30ms ON/OFF)
2. 호흡 효과 (부드러운 밝기 변화)
3. 스트로브 (20ms ON/OFF)
4. 랜덤 밝기 변화
→ 4가지 패턴 순환
```

### 알림 패턴
| 패턴 | 설명 |
|------|------|
| 새 메시지 | 부드러운 펄스 3회 |
| 경고 | 빠른 점멸 6회 |
| 충전 중 | 느린 호흡 효과 |
| 완료 | 긴 점등 후 유지 |

## 🔍 트러블슈팅

| 증상 | 원인 | 해결 방법 |
|------|------|----------|
| LED가 켜지지 않음 | 배선 오류 | VCC/GND 확인 |
| 색상이 안 바뀜 | 모듈 불량 | 다른 모듈로 테스트 |
| 밝기 조절 안됨 | PWM 설정 오류 | TIM8 설정 확인 |
| 깜빡임이 심함 | PWM 주파수 낮음 | Period 값 감소 |

## 💡 응용 예제

### 분위기 조명
```c
void MoodLight(void) {
    SevenColorLED_On();  // 자동 색상 변환
    
    // 호흡 효과로 분위기 연출
    while (1) {
        for (int b = 100; b <= 255; b += 2) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(20);
        }
        for (int b = 255; b >= 100; b -= 2) {
            SevenColorLED_SetBrightness(b);
            HAL_Delay(20);
        }
    }
}
```

### 이벤트 알림기
```c
void NotifyEvent(uint8_t event_type) {
    switch (event_type) {
        case EVENT_INFO:
            SevenColorLED_SetBrightness(150);
            HAL_Delay(1000);
            SevenColorLED_Off();
            break;
        case EVENT_WARNING:
            for (int i = 0; i < 5; i++) {
                SevenColorLED_On();
                HAL_Delay(200);
                SevenColorLED_Off();
                HAL_Delay(200);
            }
            break;
        case EVENT_ERROR:
            for (int i = 0; i < 10; i++) {
                SevenColorLED_On();
                HAL_Delay(100);
                SevenColorLED_Off();
                HAL_Delay(100);
            }
            break;
    }
}
```

## ⚠️ 주의사항

1. **색상 제어 불가**: 내장 IC가 자동으로 색상을 변환하므로 특정 색상 선택 불가
2. **변환 속도 고정**: 색상 변환 속도는 내장 IC에 의해 고정됨
3. **수동 RGB 모듈**: 개별 색상 제어가 필요하면 RGB LED 모듈(KY-016) 사용

## 📚 참고 자료

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [KY-034 7-Color Flash LED Module](https://arduinomodules.info/ky-034-7-color-auto-flash-led-module/)

## 📜 라이선스

MIT License
