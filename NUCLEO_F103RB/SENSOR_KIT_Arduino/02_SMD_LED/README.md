# 02. SMD LED 모듈

NUCLEO-F103RB에서 SMD LED 모듈을 제어하는 프로젝트입니다.

## 📋 목차

- [부품 정보](#부품-정보)
- [하드웨어 연결](#하드웨어-연결)
- [코드 설명](#코드-설명)
- [실행 결과](#실행-결과)
- [문제 해결](#문제-해결)

---

## 부품 정보

### SMD LED 모듈 사양

| 항목 | 사양 |
|------|------|
| 동작 전압 | 3.3V ~ 5V |
| LED 타입 | SMD (Surface Mount Device) |
| 핀 구성 | S (Signal), V (VCC), G (GND) 또는 +, - |
| 제어 방식 | 디지털 ON/OFF 또는 PWM |

### SMD LED 특징

- 표면 실장형 LED로 소형화
- 일반 LED보다 밝기가 균일
- 모듈에 저항 내장 (별도 저항 불필요)
- 다양한 색상 (백색, 적색, 청색 등)

---

## 하드웨어 연결

### 회로도

```
SMD LED 모듈           NUCLEO-F103RB
┌──────────┐          ┌─────────────┐
│          │          │             │
│  S ──────┼──────────┤ D7  (PA8)   │  디지털 출력
│          │          │             │
│  V ──────┼──────────┤ 3.3V        │  (또는 5V)
│          │          │             │
│  G ──────┼──────────┤ GND         │
│          │          │             │
└──────────┘          └─────────────┘
```

### 연결 표

| SMD LED 모듈 핀 | NUCLEO 핀 | Arduino 핀 | 기능 |
|----------------|-----------|------------|------|
| S (Signal) | PA8 | D7 | 디지털 출력 |
| V (VCC) | 3.3V | 3.3V | 전원 |
| G (GND) | GND | GND | 접지 |

> ⚠️ **참고**: 일부 모듈은 S핀에 HIGH를 주면 켜지고, 일부는 LOW를 주면 켜집니다. 코드에서 `ACTIVE_HIGH` 설정을 확인하세요.

---

## 코드 설명

### 메인 코드 (SMD_LED.ino)

```cpp
/*
 * SMD LED 모듈 테스트
 * 
 * 보드: NUCLEO-F103RB
 * 환경: Arduino IDE + STM32duino
 * 
 * 연결:
 *   S -> D7  (PA8)
 *   V -> 3.3V
 *   G -> GND
 */

// 핀 정의
#define LED_PIN     7    // D7 - SMD LED Signal

// Active High인 경우 true (HIGH에서 켜짐)
#define ACTIVE_HIGH true

// 내장 LED (비교용)
#define BUILTIN_LED PA5

void setup() {
  // 시리얼 초기화
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("================================");
  Serial.println("  SMD LED Module Test");
  Serial.println("  NUCLEO-F103RB + Arduino");
  Serial.println("================================");
  
  // LED 핀 설정
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
  
  Serial.println("LED ON");
  ledOn();
  delay(1000);
  
  Serial.println("LED OFF");
  ledOff();
  delay(1000);
  
  // 2. Blink 테스트 (빠른 점멸)
  Serial.println("\n=== Blink Test (5 times) ===");
  for (int i = 0; i < 5; i++) {
    Serial.print("Blink ");
    Serial.println(i + 1);
    ledOn();
    delay(200);
    ledOff();
    delay(200);
  }
  
  // 3. 내장 LED와 번갈아 점멸
  Serial.println("\n=== Alternate with Built-in LED ===");
  for (int i = 0; i < 5; i++) {
    // SMD LED ON, 내장 LED OFF
    ledOn();
    digitalWrite(BUILTIN_LED, LOW);
    delay(300);
    
    // SMD LED OFF, 내장 LED ON
    ledOff();
    digitalWrite(BUILTIN_LED, HIGH);
    delay(300);
  }
  digitalWrite(BUILTIN_LED, LOW);
  
  // 4. PWM 밝기 조절 (PWM 지원 핀 사용 시)
  Serial.println("\n=== PWM Brightness Test ===");
  pwmDemo();
  
  // 5. SOS 신호
  Serial.println("\n=== SOS Signal ===");
  sosSignal();
  
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

// PWM 밝기 조절 데모
void pwmDemo() {
  Serial.println("Fade In...");
  for (int brightness = 0; brightness <= 255; brightness += 5) {
    int pwmValue = ACTIVE_HIGH ? brightness : (255 - brightness);
    analogWrite(LED_PIN, pwmValue);
    delay(30);
  }
  
  Serial.println("Fade Out...");
  for (int brightness = 255; brightness >= 0; brightness -= 5) {
    int pwmValue = ACTIVE_HIGH ? brightness : (255 - brightness);
    analogWrite(LED_PIN, pwmValue);
    delay(30);
  }
  
  // 디지털 모드로 복귀
  pinMode(LED_PIN, OUTPUT);
  ledOff();
}

// SOS 신호 (... --- ...)
void sosSignal() {
  // S: 짧게 3번
  for (int i = 0; i < 3; i++) {
    ledOn();
    delay(150);
    ledOff();
    delay(150);
  }
  delay(300);
  
  // O: 길게 3번
  for (int i = 0; i < 3; i++) {
    ledOn();
    delay(400);
    ledOff();
    delay(150);
  }
  delay(300);
  
  // S: 짧게 3번
  for (int i = 0; i < 3; i++) {
    ledOn();
    delay(150);
    ledOff();
    delay(150);
  }
}
```

---

## 실행 결과

### 시리얼 모니터 출력

```
================================
  SMD LED Module Test
  NUCLEO-F103RB + Arduino
================================
Initialization complete!
Starting LED demo...

=== Basic ON/OFF Test ===
LED ON
LED OFF

=== Blink Test (5 times) ===
Blink 1
Blink 2
Blink 3
Blink 4
Blink 5

=== Alternate with Built-in LED ===

=== PWM Brightness Test ===
Fade In...
Fade Out...

=== SOS Signal ===

Demo complete! Restarting in 2 seconds...
```

---

## 문제 해결

### LED가 켜지지 않음

| 원인 | 해결 방법 |
|------|----------|
| Active Low 모듈 | `ACTIVE_HIGH`를 `false`로 변경 |
| 배선 오류 | VCC, GND 연결 확인 |
| 모듈 불량 | 다른 GPIO 핀으로 테스트 |

### LED가 항상 켜져 있음

- `ACTIVE_HIGH` 설정이 반대일 수 있음
- 모듈의 회로도 확인 필요

---

## 관련 프로젝트

- [01. RGB LED 모듈](../01_RGB_LED/)
- [03. 2색 LED 모듈](../03_Two_Color_LED/)

---

## 라이선스

MIT License
