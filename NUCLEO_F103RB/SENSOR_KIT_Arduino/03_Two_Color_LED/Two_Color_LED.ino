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
