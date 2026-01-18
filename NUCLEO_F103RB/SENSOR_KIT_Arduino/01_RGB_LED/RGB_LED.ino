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
