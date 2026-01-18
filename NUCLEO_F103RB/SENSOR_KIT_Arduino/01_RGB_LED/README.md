# 01. RGB LED 모듈

NUCLEO-F103RB에서 RGB LED 모듈을 PWM으로 제어하여 다양한 색상을 출력하는 프로젝트입니다.

## 📋 목차

- [부품 정보](#부품-정보)
- [하드웨어 연결](#하드웨어-연결)
- [소프트웨어 설정](#소프트웨어-설정)
- [코드 설명](#코드-설명)
- [실행 결과](#실행-결과)
- [응용 예제](#응용-예제)
- [문제 해결](#문제-해결)

---

## 부품 정보

### RGB LED 모듈 사양

| 항목 | 사양 |
|------|------|
| 동작 전압 | 3.3V ~ 5V |
| LED 타입 | 공통 캐소드 (Common Cathode) 또는 공통 애노드 |
| 핀 구성 | R, G, B, GND (또는 VCC) |
| 제어 방식 | PWM (0-255) |

### 공통 캐소드 vs 공통 애노드

| 타입 | GND/VCC | HIGH 출력 | LOW 출력 |
|------|---------|----------|----------|
| 공통 캐소드 | GND 공통 | LED ON | LED OFF |
| 공통 애노드 | VCC 공통 | LED OFF | LED ON |

> ⚠️ **참고**: 대부분의 RGB LED 모듈은 **공통 캐소드** 타입입니다. 공통 애노드인 경우 코드에서 PWM 값을 반전(255 - value)해야 합니다.

---

## 하드웨어 연결

### 회로도

```
RGB LED 모듈          NUCLEO-F103RB
┌──────────┐         ┌─────────────┐
│          │         │             │
│  R ──────┼─────────┤ D9  (PC7)   │  PWM
│          │         │             │
│  G ──────┼─────────┤ D10 (PA8)   │  PWM
│          │         │             │
│  B ──────┼─────────┤ D11 (PA7)   │  PWM
│          │         │             │
│  GND ────┼─────────┤ GND         │
│          │         │             │
└──────────┘         └─────────────┘
```

### 연결 표

| RGB LED 모듈 핀 | NUCLEO 핀 | Arduino 핀 | 기능 |
|----------------|-----------|------------|------|
| R (Red) | PC7 | D9 | PWM 출력 (TIM3_CH2) |
| G (Green) | PA8 | D10 | PWM 출력 (TIM1_CH1) |
| B (Blue) | PA7 | D11 | PWM 출력 (TIM3_CH2) |
| GND (-) | GND | GND | 접지 |

### 실제 배선 사진 참고

```
     NUCLEO-F103RB (CN10 커넥터)
    ┌────────────────────────────┐
    │  D10 ●────── Green        │
    │  D9  ●────── Red          │
    │  D8  ○                    │
    │  D7  ○                    │
    │  D6  ○                    │
    │  D5  ○                    │
    │  D4  ○                    │
    │  D3  ○                    │
    │  D2  ○                    │
    │  D1  ○                    │
    │  D0  ○                    │
    │  GND ●────── GND (검정)   │
    │  ...                      │
    │  D11 ●────── Blue         │
    └────────────────────────────┘
```

---

## 소프트웨어 설정

### 1. Arduino IDE 보드 설정

**Tools 메뉴에서 설정**:

| 항목 | 설정값 |
|------|--------|
| Board | Nucleo-64 |
| Board part number | Nucleo F103RB |
| Upload method | STM32CubeProgrammer (SWD) |
| Port | COMxx (STMicroelectronics STLink Virtual COM Port) |

### 2. 코드 업로드

1. 아래 코드를 Arduino IDE에 복사
2. **Verify** (체크 아이콘) 클릭하여 컴파일
3. **Upload** (화살표 아이콘) 클릭하여 업로드
4. 업로드 완료 메시지 확인

### 3. 시리얼 모니터 설정

- **Baud Rate**: 115200
- **Line Ending**: Both NL & CR

---

## 코드 설명

### 메인 코드 (RGB_LED.ino)

```cpp
/*
 * RGB LED 모듈 테스트
 * 
 * 보드: NUCLEO-F103RB
 * 환경: Arduino IDE + STM32duino
 * 
 * 연결:
 *   R -> D9  (PC7,  PWM)
 *   G -> D10 (PA8,  PWM)
 *   B -> D11 (PA7,  PWM)
 *   - -> GND
 */

// 핀 정의
#define PIN_RED     9    // D9  - Red LED
#define PIN_GREEN   10   // D10 - Green LED
#define PIN_BLUE    11   // D11 - Blue LED

// 공통 애노드인 경우 true로 변경
#define COMMON_ANODE false

// 색상 정의 (R, G, B)
const uint8_t COLOR_RED[]     = {255, 0, 0};
const uint8_t COLOR_GREEN[]   = {0, 255, 0};
const uint8_t COLOR_BLUE[]    = {0, 0, 255};
const uint8_t COLOR_YELLOW[]  = {255, 255, 0};
const uint8_t COLOR_CYAN[]    = {0, 255, 255};
const uint8_t COLOR_MAGENTA[] = {255, 0, 255};
const uint8_t COLOR_WHITE[]   = {255, 255, 255};
const uint8_t COLOR_ORANGE[]  = {255, 165, 0};
const uint8_t COLOR_PURPLE[]  = {128, 0, 128};
const uint8_t COLOR_OFF[]     = {0, 0, 0};

void setup() {
  // 시리얼 초기화
  Serial.begin(115200);
  while (!Serial) {
    ; // 시리얼 연결 대기
  }
  
  Serial.println("================================");
  Serial.println("  RGB LED Module Test");
  Serial.println("  NUCLEO-F103RB + Arduino");
  Serial.println("================================");
  
  // PWM 핀 설정
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  
  // 초기 상태: LED OFF
  setColor(0, 0, 0);
  
  Serial.println("Initialization complete!");
  Serial.println("Starting color demo...\n");
  
  delay(1000);
}

void loop() {
  // 1. 기본 색상 순환
  Serial.println("=== Basic Colors ===");
  
  Serial.println("Red");
  setColorArray(COLOR_RED);
  delay(1000);
  
  Serial.println("Green");
  setColorArray(COLOR_GREEN);
  delay(1000);
  
  Serial.println("Blue");
  setColorArray(COLOR_BLUE);
  delay(1000);
  
  // 2. 혼합 색상
  Serial.println("\n=== Mixed Colors ===");
  
  Serial.println("Yellow (R+G)");
  setColorArray(COLOR_YELLOW);
  delay(1000);
  
  Serial.println("Cyan (G+B)");
  setColorArray(COLOR_CYAN);
  delay(1000);
  
  Serial.println("Magenta (R+B)");
  setColorArray(COLOR_MAGENTA);
  delay(1000);
  
  Serial.println("White (R+G+B)");
  setColorArray(COLOR_WHITE);
  delay(1000);
  
  Serial.println("Orange");
  setColorArray(COLOR_ORANGE);
  delay(1000);
  
  Serial.println("Purple");
  setColorArray(COLOR_PURPLE);
  delay(1000);
  
  // 3. Fade 효과
  Serial.println("\n=== Fade Effect ===");
  fadeDemo();
  
  // 4. Rainbow 효과
  Serial.println("\n=== Rainbow Effect ===");
  rainbowDemo();
  
  // 5. OFF
  Serial.println("\nLED OFF - Restarting in 2 seconds...\n");
  setColorArray(COLOR_OFF);
  delay(2000);
}

// RGB 값으로 색상 설정
void setColor(uint8_t red, uint8_t green, uint8_t blue) {
  // 공통 애노드인 경우 값 반전
  if (COMMON_ANODE) {
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  }
  
  analogWrite(PIN_RED, red);
  analogWrite(PIN_GREEN, green);
  analogWrite(PIN_BLUE, blue);
}

// 배열로 색상 설정
void setColorArray(const uint8_t color[]) {
  setColor(color[0], color[1], color[2]);
}

// Fade 데모 (빨강 -> 녹색 -> 파랑)
void fadeDemo() {
  // Red to Green
  Serial.println("Fading: Red -> Green");
  for (int i = 0; i <= 255; i += 5) {
    setColor(255 - i, i, 0);
    delay(20);
  }
  
  // Green to Blue
  Serial.println("Fading: Green -> Blue");
  for (int i = 0; i <= 255; i += 5) {
    setColor(0, 255 - i, i);
    delay(20);
  }
  
  // Blue to Red
  Serial.println("Fading: Blue -> Red");
  for (int i = 0; i <= 255; i += 5) {
    setColor(i, 0, 255 - i);
    delay(20);
  }
}

// Rainbow 데모 (HSV to RGB 변환)
void rainbowDemo() {
  Serial.println("Rainbow cycling...");
  
  for (int hue = 0; hue < 360; hue += 2) {
    uint8_t r, g, b;
    hsvToRgb(hue, 255, 255, &r, &g, &b);
    setColor(r, g, b);
    delay(20);
  }
}

// HSV to RGB 변환
void hsvToRgb(int h, int s, int v, uint8_t *r, uint8_t *g, uint8_t *b) {
  float hf = h / 60.0;
  int i = (int)hf;
  float f = hf - i;
  
  float pf = v * (1.0 - s / 255.0);
  float qf = v * (1.0 - f * s / 255.0);
  float tf = v * (1.0 - (1.0 - f) * s / 255.0);
  
  uint8_t p = (uint8_t)pf;
  uint8_t q = (uint8_t)qf;
  uint8_t t = (uint8_t)tf;
  
  switch (i % 6) {
    case 0: *r = v; *g = t; *b = p; break;
    case 1: *r = q; *g = v; *b = p; break;
    case 2: *r = p; *g = v; *b = t; break;
    case 3: *r = p; *g = q; *b = v; break;
    case 4: *r = t; *g = p; *b = v; break;
    case 5: *r = v; *g = p; *b = q; break;
  }
}
```

### 코드 구조

```
RGB_LED.ino
├── 핀 정의 (PIN_RED, PIN_GREEN, PIN_BLUE)
├── 색상 상수 정의
├── setup()
│   ├── 시리얼 초기화 (115200)
│   ├── PWM 핀 설정
│   └── 초기화 완료 메시지
├── loop()
│   ├── 기본 색상 테스트 (R, G, B)
│   ├── 혼합 색상 테스트
│   ├── Fade 효과
│   └── Rainbow 효과
├── setColor() - RGB 값 출력
├── fadeDemo() - 색상 전환 효과
├── rainbowDemo() - 무지개 효과
└── hsvToRgb() - HSV→RGB 변환
```

---

## 실행 결과

### 시리얼 모니터 출력

```
================================
  RGB LED Module Test
  NUCLEO-F103RB + Arduino
================================
Initialization complete!
Starting color demo...

=== Basic Colors ===
Red
Green
Blue

=== Mixed Colors ===
Yellow (R+G)
Cyan (G+B)
Magenta (R+B)
White (R+G+B)
Orange
Purple

=== Fade Effect ===
Fading: Red -> Green
Fading: Green -> Blue
Fading: Blue -> Red

=== Rainbow Effect ===
Rainbow cycling...

LED OFF - Restarting in 2 seconds...
```

### 예상 동작

1. **기본 색상**: 빨강 → 녹색 → 파랑 순서로 각 1초씩
2. **혼합 색상**: 노랑, 시안, 마젠타, 흰색, 주황, 보라
3. **Fade 효과**: 부드럽게 색상 전환
4. **Rainbow 효과**: 무지개 색상 순환

---

## 응용 예제

### 1. 시리얼 명령으로 색상 변경

```cpp
void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "red")    setColorArray(COLOR_RED);
    else if (cmd == "green")  setColorArray(COLOR_GREEN);
    else if (cmd == "blue")   setColorArray(COLOR_BLUE);
    else if (cmd == "off")    setColorArray(COLOR_OFF);
    else if (cmd.startsWith("rgb")) {
      // 형식: rgb 255 128 64
      int r, g, b;
      sscanf(cmd.c_str(), "rgb %d %d %d", &r, &g, &b);
      setColor(r, g, b);
    }
  }
}
```

### 2. 버튼으로 색상 변경

```cpp
#define BUTTON_PIN PC13  // User Button

int colorIndex = 0;
const uint8_t* colors[] = {
  COLOR_RED, COLOR_GREEN, COLOR_BLUE,
  COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA
};
const int numColors = 6;

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    colorIndex = (colorIndex + 1) % numColors;
    setColorArray(colors[colorIndex]);
    delay(300);  // 디바운싱
  }
}
```

### 3. 숨쉬기 효과 (Breathing)

```cpp
void breathingEffect(uint8_t r, uint8_t g, uint8_t b) {
  // Fade In
  for (int i = 0; i <= 255; i++) {
    setColor(r * i / 255, g * i / 255, b * i / 255);
    delay(10);
  }
  // Fade Out
  for (int i = 255; i >= 0; i--) {
    setColor(r * i / 255, g * i / 255, b * i / 255);
    delay(10);
  }
}
```

---

## 문제 해결

### 1. LED가 켜지지 않음

| 원인 | 해결 방법 |
|------|----------|
| 배선 오류 | GND 연결 확인, R/G/B 핀 순서 확인 |
| 공통 애노드 모듈 | `COMMON_ANODE`를 `true`로 변경 |
| 핀 번호 오류 | D9, D10, D11이 PWM 지원 핀인지 확인 |

### 2. 색상이 이상함

| 증상 | 원인 | 해결 |
|------|------|------|
| 빨강인데 파랑이 켜짐 | R/G/B 핀 혼선 | 배선 순서 재확인 |
| 밝기가 너무 밝음 | 저항 없음 | 220Ω~330Ω 저항 추가 |
| 색상이 탁함 | PWM 주파수 문제 | 코드 확인 |

### 3. 업로드 실패

| 에러 메시지 | 해결 방법 |
|------------|----------|
| `No STM32 target found` | USB 케이블 재연결, 드라이버 확인 |
| `Cannot open port` | 포트 선택 재확인 |
| `STM32CubeProgrammer not found` | STM32CubeProgrammer 설치 확인 |

### 4. PWM이 동작하지 않음

```cpp
// 핀 확인용 테스트 코드
void setup() {
  pinMode(9, OUTPUT);
}

void loop() {
  for (int i = 0; i <= 255; i++) {
    analogWrite(9, i);
    delay(10);
  }
  for (int i = 255; i >= 0; i--) {
    analogWrite(9, i);
    delay(10);
  }
}
```

---

## 추가 정보

### PWM 주파수

STM32duino의 기본 PWM 주파수는 약 1kHz입니다. 필요시 변경 가능:

```cpp
analogWriteFrequency(1000);  // 1kHz
```

### 메모리 사용량

```
Sketch uses 15,XXX bytes (11%) of program storage space.
Global variables use 1,XXX bytes (5%) of dynamic memory.
```

---

## 관련 프로젝트

- [02. SMD LED 모듈](../02_SMD_LED/)
- [03. 2색 LED 모듈](../03_Two_Color_LED/)
- [05. 7색 LED 모듈](../05_Seven_Color_LED/)

---

## 라이선스

MIT License
