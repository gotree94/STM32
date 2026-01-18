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
