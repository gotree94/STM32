/*
 * Flame Sensor Module Test Code
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * IDE: Arduino IDE with STM32 Core
 * 
 * 불꽃감지 센서 모듈 테스트
 * - 불꽃의 적외선(IR) 파장(760nm~1100nm)을 감지
 * - 디지털 출력(DO): 임계값 초과시 LOW 출력
 * - 아날로그 출력(AO): 불꽃 강도에 비례한 값 출력
 * 
 * Wiring:
 * - VCC -> 3.3V or 5V
 * - GND -> GND
 * - DO  -> D2 (PA10)
 * - AO  -> A0 (PA0)
 */

// Pin Definitions
#define FLAME_DO_PIN  D2    // Digital Output Pin (PA10)
#define FLAME_AO_PIN  A0    // Analog Output Pin (PA0)
#define LED_PIN       LED_BUILTIN  // Onboard LED (PA5)

// Variables
int analogValue = 0;
int digitalValue = 0;
unsigned long previousMillis = 0;
const long interval = 500;  // 500ms 간격으로 출력

void setup() {
  // Serial Communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("========================================");
  Serial.println("  Flame Sensor Module Test");
  Serial.println("  Board: NUCLEO-F103RB");
  Serial.println("========================================");
  Serial.println();
  
  // Pin Mode Setup
  pinMode(FLAME_DO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // ADC Resolution (12-bit for STM32)
  analogReadResolution(12);
  
  Serial.println("Sensor initialized. Monitoring...");
  Serial.println("DO: LOW = Flame detected, HIGH = No flame");
  Serial.println();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read sensor values
  digitalValue = digitalRead(FLAME_DO_PIN);
  analogValue = analogRead(FLAME_AO_PIN);
  
  // Flame detection (DO pin is active LOW)
  if (digitalValue == LOW) {
    digitalWrite(LED_PIN, HIGH);  // LED ON when flame detected
  } else {
    digitalWrite(LED_PIN, LOW);   // LED OFF when no flame
  }
  
  // Print values at interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    printSensorData();
  }
}

void printSensorData() {
  Serial.print("Analog Value: ");
  Serial.print(analogValue);
  Serial.print(" (");
  Serial.print(map(analogValue, 0, 4095, 0, 100));
  Serial.print("%)");
  
  Serial.print(" | Digital: ");
  if (digitalValue == LOW) {
    Serial.print("FLAME DETECTED!");
  } else {
    Serial.print("No flame");
  }
  
  // Flame intensity estimation
  Serial.print(" | Intensity: ");
  if (analogValue < 500) {
    Serial.println("Strong flame nearby!");
  } else if (analogValue < 2000) {
    Serial.println("Moderate flame");
  } else if (analogValue < 3500) {
    Serial.println("Weak/distant flame");
  } else {
    Serial.println("No flame");
  }
}
