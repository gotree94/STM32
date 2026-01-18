# 03. 2색 LED 모듈

NUCLEO-F103RB에서 2색 LED 모듈을 제어하는 프로젝트입니다.

## 📋 목차

- [부품 정보](#부품-정보)
- [하드웨어 연결](#하드웨어-연결)
- [코드 설명](#코드-설명)
- [실행 결과](#실행-결과)
- [문제 해결](#문제-해결)

---

## 부품 정보

### 2색 LED 모듈 사양

| 항목 | 사양 |
|------|------|
| 동작 전압 | 3.3V ~ 5V |
| LED 색상 | 빨강 + 녹색 (또는 빨강 + 파랑) |
| 핀 구성 | R, G, GND (공통 캐소드) 또는 R, G, VCC (공통 애노드) |
| 제어 방식 | 디지털 또는 PWM |

### 2색 LED 타입

| 타입 | 공통 핀 | 빨강 ON | 녹색 ON | 노랑 (혼합) |
|------|---------|---------|---------|------------|
| 공통 캐소드 | GND (-) | R=HIGH | G=HIGH | R+G=HIGH |
| 공통 애노드 | VCC (+) | R=LOW | G=LOW | R+G=LOW |

### 색상 조합

| 빨강 | 녹색 | 결과 색상 |
|------|------|----------|
| OFF | OFF | 꺼짐 |
| ON | OFF | 빨강 |
| OFF | ON | 녹색 |
| ON | ON | 노랑 (주황) |

---

## 하드웨어 연결

### 회로도 (공통 캐소드)

```
2색 LED 모듈           NUCLEO-F103RB
┌──────────┐          ┌─────────────┐
│          │          │             │
│  R ──────┼──────────┤ D5  (PB4)   │  PWM
│          │          │             │
│  G ──────┼──────────┤ D6  (PB10)  │  PWM
│          │          │             │
│  - ──────┼──────────┤ GND         │
│          │          │             │
└──────────┘          └─────────────┘
```

### 연결 표

| 2색 LED 핀 | NUCLEO 핀 | Arduino 핀 | 기능 |
|-----------|-----------|------------|------|
| R (Red) | PB4 | D5 | PWM 출력 |
| G (Green) | PB10 | D6 | PWM 출력 |
| - (GND) | GND | GND | 접지 |

---

## 코드 설명

### 메인 코드 (Two_Color_LED.ino)

```cpp
/*
 * 2색 LED 모듈 테스트
 * 
 * 보드: NUCLEO-F103RB
 * 환경: Arduino IDE + STM32duino
 * 
 * 연결:
 *   R -> D5  (PB4,  PWM)
 *   G -> D6  (PB10, PWM)
 *   - -> GND
 */

// 핀 정의
#define PIN_RED     5    // D5 - Red LED
#define PIN_GREEN   6    // D6 - Green LED

// 공통 애노드인 경우 true로 변경
#define COMMON_ANODE false

// 색상 상수
enum Color {
  COLOR_OFF,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW  // 빨강 + 녹색 혼합
};

void setup() {
  // 시리얼 초기화
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("================================");
  Serial.println("  Two Color LED Module Test");
  Serial.println("  NUCLEO-F103RB + Arduino");
  Serial.println("================================");
  
  // PWM 핀 설정
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  
  // 초기 상태: OFF
  setColor(COLOR_OFF);
  
  Serial.println("Initialization complete!");
  Serial.println("Starting color demo...\n");
  
  delay(1000);
}

void loop() {
  // 1. 기본 색상 테스트
  Serial.println("=== Basic Colors ===");
  
  Serial.println("OFF");
  setColor(COLOR_OFF);
  delay(1000);
  
  Serial.println("RED");
  setColor(COLOR_RED);
  delay(1000);
  
  Serial.println("GREEN");
  setColor(COLOR_GREEN);
  delay(1000);
  
  Serial.println("YELLOW (Red + Green)");
  setColor(COLOR_YELLOW);
  delay(1000);
  
  // 2. PWM 혼합 (주황색 만들기)
  Serial.println("\n=== PWM Color Mixing ===");
  
  Serial.println("Orange (Red:255, Green:128)");
  setRGB(255, 128);
  delay(1000);
  
  Serial.println("Light Green (Red:64, Green:255)");
  setRGB(64, 255);
  delay(1000);
  
  Serial.println("Dark Yellow (Red:200, Green:200)");
  setRGB(200, 200);
  delay(1000);
  
  // 3. 색상 전환 효과
  Serial.println("\n=== Color Transition ===");
  transitionDemo();
  
  // 4. 교통 신호등 시뮬레이션
  Serial.println("\n=== Traffic Light Simulation ===");
  trafficLightDemo();
  
  // 5. 경고등 점멸
  Serial.println("\n=== Warning Light ===");
  warningDemo();
  
  Serial.println("\nDemo complete! Restarting in 2 seconds...\n");
  delay(2000);
}

// 색상 설정 (열거형)
void setColor(Color color) {
  switch (color) {
    case COLOR_OFF:
      setRGB(0, 0);
      break;
    case COLOR_RED:
      setRGB(255, 0);
      break;
    case COLOR_GREEN:
      setRGB(0, 255);
      break;
    case COLOR_YELLOW:
      setRGB(255, 255);
      break;
  }
}

// RGB 값으로 설정 (PWM)
void setRGB(uint8_t red, uint8_t green) {
  if (COMMON_ANODE) {
    red = 255 - red;
    green = 255 - green;
  }
  
  analogWrite(PIN_RED, red);
  analogWrite(PIN_GREEN, green);
}

// 색상 전환 효과
void transitionDemo() {
  // Red -> Yellow
  Serial.println("Red -> Yellow");
  for (int g = 0; g <= 255; g += 5) {
    setRGB(255, g);
    delay(30);
  }
  
  // Yellow -> Green
  Serial.println("Yellow -> Green");
  for (int r = 255; r >= 0; r -= 5) {
    setRGB(r, 255);
    delay(30);
  }
  
  // Green -> Off
  Serial.println("Green -> Off");
  for (int g = 255; g >= 0; g -= 5) {
    setRGB(0, g);
    delay(30);
  }
}

// 교통 신호등 시뮬레이션
void trafficLightDemo() {
  // 녹색 (통행)
  Serial.println("GREEN - Go!");
  setColor(COLOR_GREEN);
  delay(2000);
  
  // 노란색 (주의)
  Serial.println("YELLOW - Caution!");
  setColor(COLOR_YELLOW);
  delay(1000);
  
  // 빨간색 (정지)
  Serial.println("RED - Stop!");
  setColor(COLOR_RED);
  delay(2000);
}

// 경고등 점멸
void warningDemo() {
  for (int i = 0; i < 5; i++) {
    // 빨강 점멸
    setColor(COLOR_RED);
    delay(200);
    setColor(COLOR_OFF);
    delay(200);
  }
  
  for (int i = 0; i < 5; i++) {
    // 노랑 점멸
    setColor(COLOR_YELLOW);
    delay(200);
    setColor(COLOR_OFF);
    delay(200);
  }
}
```

---

## 실행 결과

### 시리얼 모니터 출력

```
================================
  Two Color LED Module Test
  NUCLEO-F103RB + Arduino
================================
Initialization complete!
Starting color demo...

=== Basic Colors ===
OFF
RED
GREEN
YELLOW (Red + Green)

=== PWM Color Mixing ===
Orange (Red:255, Green:128)
Light Green (Red:64, Green:255)
Dark Yellow (Red:200, Green:200)

=== Color Transition ===
Red -> Yellow
Yellow -> Green
Green -> Off

=== Traffic Light Simulation ===
GREEN - Go!
YELLOW - Caution!
RED - Stop!

=== Warning Light ===

Demo complete! Restarting in 2 seconds...
```

---

## 문제 해결

### LED가 켜지지 않음

| 원인 | 해결 방법 |
|------|----------|
| 공통 애노드 모듈 | `COMMON_ANODE`를 `true`로 변경 |
| 핀 연결 오류 | R, G 핀 위치 확인 |
| GND 미연결 | GND 배선 확인 |

### 노란색이 안 나옴

- PWM 동시 출력 확인
- 두 LED의 밝기 차이로 주황색으로 보일 수 있음
- `setRGB(255, 200)`으로 녹색 비율 조절

### 색상이 반대로 나옴

- R과 G 핀이 바뀌었을 가능성
- 배선 순서 재확인

---

## 관련 프로젝트

- [01. RGB LED 모듈](../01_RGB_LED/)
- [04. 소형 2색 LED 모듈](../04_Small_Two_Color_LED/)
- [05. 7색 LED 모듈](../05_Seven_Color_LED/)

---

## 라이선스

MIT License
