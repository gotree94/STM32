/*
 * 소형 2색 LED 모듈 테스트
 * 
 * 보드: NUCLEO-F103RB
 * 환경: Arduino IDE + STM32duino
 * 
 * 연결:
 *   R -> D3  (PB3,  PWM)
 *   G -> D5  (PB4,  PWM)
 *   - -> GND
 * 
 * 특징:
 *   - 3mm 소형 LED
 *   - 상태 표시용으로 적합
 */

// 핀 정의
#define PIN_RED     3    // D3 - Red LED
#define PIN_GREEN   5    // D5 - Green LED

// 공통 애노드인 경우 true로 변경
#define COMMON_ANODE false

// LED 밝기 제한 (소형 LED는 낮은 전류 권장)
#define MAX_BRIGHTNESS 200

void setup() {
  // 시리얼 초기화
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("================================");
  Serial.println("  Small Two Color LED Test");
  Serial.println("  NUCLEO-F103RB + Arduino");
  Serial.println("================================");
  Serial.println("Note: Using 3mm small LED module\n");
  
  // PWM 핀 설정
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  
  // 초기 상태: OFF
  setLED(0, 0);
  
  Serial.println("Initialization complete!");
  Serial.println("Starting LED demo...\n");
  
  delay(1000);
}

void loop() {
  // 1. 기본 색상 테스트
  Serial.println("=== Basic Colors ===");
  
  Serial.println("OFF");
  setLED(0, 0);
  delay(1000);
  
  Serial.println("RED");
  setLED(MAX_BRIGHTNESS, 0);
  delay(1000);
  
  Serial.println("GREEN");
  setLED(0, MAX_BRIGHTNESS);
  delay(1000);
  
  Serial.println("YELLOW");
  setLED(MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  delay(1000);
  
  // 2. 상태 표시 시뮬레이션
  Serial.println("\n=== Status Indicator ===");
  statusIndicatorDemo();
  
  // 3. 부드러운 전환
  Serial.println("\n=== Smooth Transition ===");
  smoothTransitionDemo();
  
  // 4. 경보 패턴
  Serial.println("\n=== Alert Patterns ===");
  alertPatternDemo();
  
  Serial.println("\nDemo complete! Restarting in 2 seconds...\n");
  delay(2000);
}

// LED 설정 (PWM)
void setLED(uint8_t red, uint8_t green) {
  red = constrain(red, 0, MAX_BRIGHTNESS);
  green = constrain(green, 0, MAX_BRIGHTNESS);
  
  if (COMMON_ANODE) {
    red = 255 - red;
    green = 255 - green;
  }
  
  analogWrite(PIN_RED, red);
  analogWrite(PIN_GREEN, green);
}

// 상태 표시 시뮬레이션
void statusIndicatorDemo() {
  // 정상 상태 (녹색)
  Serial.println("Status: OK (Green)");
  setLED(0, MAX_BRIGHTNESS);
  delay(1500);
  
  // 경고 상태 (노랑)
  Serial.println("Status: Warning (Yellow)");
  setLED(MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  delay(1500);
  
  // 에러 상태 (빨강)
  Serial.println("Status: Error (Red)");
  setLED(MAX_BRIGHTNESS, 0);
  delay(1500);
  
  // 대기 상태 (녹색 점멸)
  Serial.println("Status: Standby (Green Blink)");
  for (int i = 0; i < 4; i++) {
    setLED(0, MAX_BRIGHTNESS);
    delay(300);
    setLED(0, 0);
    delay(300);
  }
}

// 부드러운 전환
void smoothTransitionDemo() {
  // 꺼짐 -> 빨강
  Serial.println("Off -> Red");
  for (int i = 0; i <= MAX_BRIGHTNESS; i += 5) {
    setLED(i, 0);
    delay(30);
  }
  
  // 빨강 -> 노랑
  Serial.println("Red -> Yellow");
  for (int i = 0; i <= MAX_BRIGHTNESS; i += 5) {
    setLED(MAX_BRIGHTNESS, i);
    delay(30);
  }
  
  // 노랑 -> 녹색
  Serial.println("Yellow -> Green");
  for (int i = MAX_BRIGHTNESS; i >= 0; i -= 5) {
    setLED(i, MAX_BRIGHTNESS);
    delay(30);
  }
  
  // 녹색 -> 꺼짐
  Serial.println("Green -> Off");
  for (int i = MAX_BRIGHTNESS; i >= 0; i -= 5) {
    setLED(0, i);
    delay(30);
  }
}

// 경보 패턴
void alertPatternDemo() {
  // 빠른 빨강 점멸 (위험)
  Serial.println("DANGER: Fast red blink");
  for (int i = 0; i < 10; i++) {
    setLED(MAX_BRIGHTNESS, 0);
    delay(100);
    setLED(0, 0);
    delay(100);
  }
  
  // 느린 노랑 점멸 (주의)
  Serial.println("CAUTION: Slow yellow blink");
  for (int i = 0; i < 3; i++) {
    setLED(MAX_BRIGHTNESS, MAX_BRIGHTNESS);
    delay(500);
    setLED(0, 0);
    delay(500);
  }
  
  // 녹색 유지 (정상)
  Serial.println("NORMAL: Steady green");
  setLED(0, MAX_BRIGHTNESS);
  delay(2000);
  
  setLED(0, 0);
}
