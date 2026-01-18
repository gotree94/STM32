# 05. 7색 LED 모듈

NUCLEO-F103RB에서 7색 자동 변환 LED 모듈을 제어하는 프로젝트입니다.

## 📋 목차

- [부품 정보](#부품-정보)
- [하드웨어 연결](#하드웨어-연결)
- [코드 설명](#코드-설명)
- [실행 결과](#실행-결과)
- [문제 해결](#문제-해결)

---

## 부품 정보

### 7색 LED 모듈 사양

| 항목 | 사양 |
|------|------|
| 동작 전압 | 3.3V ~ 5V |
| LED 타입 | 자동 색상 변환 (내장 IC) |
| 핀 구성 | S (Signal), V (VCC), G (GND) |
| 색상 | 빨강, 주황, 노랑, 녹색, 청색, 남색, 보라 |
| 제어 방식 | 디지털 ON/OFF |

### 7색 LED 특징

- **내장 IC**: 전원 공급만으로 자동 색상 변환
- **변환 주기**: 약 0.5~1초마다 색상 변경
- **제어 방식**: ON/OFF만 가능 (개별 색상 제어 불가)
- **용도**: 장식용, 시각적 효과, 어린이 교육용

### RGB LED vs 7색 LED 비교

| 항목 | RGB LED | 7색 LED |
|------|---------|---------|
| 핀 수 | 4핀 (R,G,B,GND) | 3핀 (S,V,G) |
| 색상 제어 | PWM으로 자유롭게 | 불가 (자동 변환) |
| 복잡도 | 높음 | 낮음 |
| 용도 | 정밀 제어 | 장식, 효과 |

---

## 하드웨어 연결

### 회로도

```
7색 LED 모듈           NUCLEO-F103RB
┌──────────┐          ┌─────────────┐
│          │          │             │
│  S ──────┼──────────┤ D4  (PB5)   │  디지털 출력
│          │          │             │
│  V ──────┼──────────┤ 3.3V        │  (또는 5V)
│          │          │             │
│  G ──────┼──────────┤ GND         │
│          │          │             │
└──────────┘          └─────────────┘
```

### 연결 표

| 7색 LED 핀 | NUCLEO 핀 | Arduino 핀 | 기능 |
|-----------|-----------|------------|------|
| S (Signal) | PB5 | D4 | 디지털 출력 |
| V (VCC) | 3.3V | 3.3V | 전원 |
| G (GND) | GND | GND | 접지 |

> ⚠️ **참고**: 일부 모듈은 S핀 없이 VCC, GND만 있습니다. 이 경우 VCC를 GPIO에 연결하여 제어합니다.

---

## 코드 설명

### 메인 코드 (Seven_Color_LED.ino)

```cpp
/*
 * 7색 LED 모듈 테스트
 * 
 * 보드: NUCLEO-F103RB
 * 환경: Arduino IDE + STM32duino
 * 
 * 연결:
 *   S -> D4  (PB5)
 *   V -> 3.3V
 *   G -> GND
 * 
 * 특징:
 *   - 내장 IC로 자동 색상 변환
 *   - ON/OFF 제어만 가능
 */

// 핀 정의
#define LED_PIN     4    // D4 - 7색 LED Signal

// Active High인 경우 true
#define ACTIVE_HIGH true

// 내장 LED
#define BUILTIN_LED PA5

void setup() {
  // 시리얼 초기화
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("================================");
  Serial.println("  7-Color Auto LED Module Test");
  Serial.println("  NUCLEO-F103RB + Arduino");
  Serial.println("================================");
  Serial.println("Note: Colors change automatically");
  Serial.println("      Only ON/OFF control available\n");
  
  // 핀 설정
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  
  // 초기 상태: OFF
  ledOff();
  digitalWrite(BUILTIN_LED, LOW);
  
  Serial.println("Initialization complete!");
  Serial.println("Starting LED demo...\n");
  
  delay(1000);
}

void loop() {
  // 1. 기본 ON/OFF 테스트
  Serial.println("=== Basic ON/OFF Test ===");
  
  Serial.println("LED ON - Watch the colors change!");
  ledOn();
  delay(5000);  // 5초 동안 색상 변화 관찰
  
  Serial.println("LED OFF");
  ledOff();
  delay(2000);
  
  // 2. 간헐적 점멸 (색상 동기화 효과)
  Serial.println("\n=== Intermittent Blink ===");
  Serial.println("Quick on/off to catch different colors");
  for (int i = 0; i < 7; i++) {
    Serial.print("Blink ");
    Serial.println(i + 1);
    ledOn();
    delay(800);  // 색상 1개 표시 시간
    ledOff();
    delay(200);
  }
  
  // 3. 장시간 ON (모든 색상 순환 관찰)
  Serial.println("\n=== Long ON - Full Color Cycle ===");
  Serial.println("Observe all 7 colors (about 7 seconds)");
  ledOn();
  for (int i = 0; i < 7; i++) {
    Serial.print("Color ");
    Serial.print(i + 1);
    Serial.println(" of 7");
    delay(1000);
  }
  ledOff();
  delay(1000);
  
  // 4. 내장 LED와 동기화
  Serial.println("\n=== Sync with Built-in LED ===");
  for (int i = 0; i < 5; i++) {
    // 둘 다 ON
    ledOn();
    digitalWrite(BUILTIN_LED, HIGH);
    delay(1000);
    
    // 둘 다 OFF
    ledOff();
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
  }
  
  // 5. 파티 모드 (빠른 점멸)
  Serial.println("\n=== Party Mode ===");
  Serial.println("Fast blinking for visual effect");
  for (int i = 0; i < 20; i++) {
    ledOn();
    delay(150);
    ledOff();
    delay(100);
  }
  
  // 6. 천천히 ON/OFF (색상 전환 관찰)
  Serial.println("\n=== Slow Pattern ===");
  for (int i = 0; i < 3; i++) {
    Serial.println("ON for 3 seconds...");
    ledOn();
    delay(3000);
    
    Serial.println("OFF for 1 second...");
    ledOff();
    delay(1000);
  }
  
  Serial.println("\nDemo complete! Restarting in 2 seconds...\n");
  delay(2000);
}

// LED ON
void ledOn() {
  if (ACTIVE_HIGH) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

// LED OFF
void ledOff() {
  if (ACTIVE_HIGH) {
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
}
```

---

## 실행 결과

### 시리얼 모니터 출력

```
================================
  7-Color Auto LED Module Test
  NUCLEO-F103RB + Arduino
================================
Note: Colors change automatically
      Only ON/OFF control available

Initialization complete!
Starting LED demo...

=== Basic ON/OFF Test ===
LED ON - Watch the colors change!
LED OFF

=== Intermittent Blink ===
Quick on/off to catch different colors
Blink 1
Blink 2
Blink 3
Blink 4
Blink 5
Blink 6
Blink 7

=== Long ON - Full Color Cycle ===
Observe all 7 colors (about 7 seconds)
Color 1 of 7
Color 2 of 7
Color 3 of 7
Color 4 of 7
Color 5 of 7
Color 6 of 7
Color 7 of 7

=== Sync with Built-in LED ===

=== Party Mode ===
Fast blinking for visual effect

=== Slow Pattern ===
ON for 3 seconds...
OFF for 1 second...

Demo complete! Restarting in 2 seconds...
```

### 예상 색상 순서

1. 🔴 빨강 (Red)
2. 🟠 주황 (Orange)
3. 🟡 노랑 (Yellow)
4. 🟢 녹색 (Green)
5. 🔵 파랑 (Blue)
6. 🟣 남색 (Indigo)
7. 💜 보라 (Violet)

> **참고**: 모듈에 따라 순서와 색상이 다를 수 있습니다.

---

## 응용 예제

### 버튼으로 ON/OFF 제어

```cpp
#define BUTTON_PIN PC13

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    ledOn();
  } else {
    ledOff();
  }
}
```

### 타이머로 자동 ON/OFF

```cpp
unsigned long previousMillis = 0;
bool ledState = false;

void loop() {
  unsigned long currentMillis = millis();
  
  // 10초마다 상태 변경
  if (currentMillis - previousMillis >= 10000) {
    previousMillis = currentMillis;
    ledState = !ledState;
    
    if (ledState) {
      ledOn();
      Serial.println("LED ON - Enjoy the colors!");
    } else {
      ledOff();
      Serial.println("LED OFF - Rest time");
    }
  }
}
```

---

## 문제 해결

### LED가 켜지지 않음

| 원인 | 해결 방법 |
|------|----------|
| 전원 부족 | VCC를 5V로 변경 시도 |
| Active Low 모듈 | `ACTIVE_HIGH`를 `false`로 변경 |
| 배선 오류 | S, V, G 순서 확인 |

### 색상이 변하지 않음

- 전원 전압 확인 (3.3V에서 5V로 변경)
- 모듈 불량 가능성 (다른 모듈로 테스트)

### 색상 변환 속도가 다름

- 모듈마다 내장 IC가 달라 변환 주기가 다를 수 있음
- 정상 동작

---

## 관련 프로젝트

- [01. RGB LED 모듈](../01_RGB_LED/) - 색상을 직접 제어하고 싶다면
- [02. SMD LED 모듈](../02_SMD_LED/)

---

## 라이선스

MIT License
