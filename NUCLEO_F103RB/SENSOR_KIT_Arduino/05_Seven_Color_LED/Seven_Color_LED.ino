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
