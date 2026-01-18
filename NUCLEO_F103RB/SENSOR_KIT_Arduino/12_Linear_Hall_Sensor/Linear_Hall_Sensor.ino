/*
 * Linear Hall Sensor Module Test Code
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * IDE: Arduino IDE with STM32 Core
 * 
 * 리니어 홀 센서 모듈 테스트
 * - 자기장의 세기를 선형적으로 측정
 * - 아날로그 출력: 자기장 세기에 비례
 * - 디지털 출력: 임계값 초과시 신호 출력
 * 
 * Wiring:
 * - VCC -> 3.3V or 5V
 * - GND -> GND
 * - DO  -> D2 (PA10)
 * - AO  -> A0 (PA0)
 */

// Pin Definitions
#define HALL_DO_PIN   D2    // Digital Output Pin (PA10)
#define HALL_AO_PIN   A0    // Analog Output Pin (PA0)
#define LED_PIN       LED_BUILTIN  // Onboard LED (PA5)

// Variables
int analogValue = 0;
int digitalValue = 0;
int baselineValue = 0;      // 자기장 없을 때 기준값
int calibratedValue = 0;
unsigned long previousMillis = 0;
const long interval = 200;  // 200ms 간격으로 출력

// Calibration
bool isCalibrated = false;
const int CALIBRATION_SAMPLES = 50;

void setup() {
  // Serial Communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("========================================");
  Serial.println("  Linear Hall Sensor Module Test");
  Serial.println("  Board: NUCLEO-F103RB");
  Serial.println("========================================");
  Serial.println();
  
  // Pin Mode Setup
  pinMode(HALL_DO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // ADC Resolution (12-bit for STM32)
  analogReadResolution(12);
  
  // Calibration
  Serial.println("Calibrating... Keep magnets away!");
  calibrateSensor();
  
  Serial.println();
  Serial.println("Calibration complete!");
  Serial.print("Baseline value: ");
  Serial.println(baselineValue);
  Serial.println();
  Serial.println("Bring a magnet close to the sensor...");
  Serial.println();
}

void calibrateSensor() {
  long sum = 0;
  
  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    sum += analogRead(HALL_AO_PIN);
    delay(20);
    
    // Progress indicator
    if (i % 10 == 0) {
      Serial.print(".");
    }
  }
  Serial.println();
  
  baselineValue = sum / CALIBRATION_SAMPLES;
  isCalibrated = true;
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read sensor values
  digitalValue = digitalRead(HALL_DO_PIN);
  analogValue = analogRead(HALL_AO_PIN);
  
  // Calculate calibrated value (deviation from baseline)
  calibratedValue = analogValue - baselineValue;
  
  // Magnetic field detection
  if (abs(calibratedValue) > 100) {
    digitalWrite(LED_PIN, HIGH);  // LED ON when magnetic field detected
  } else {
    digitalWrite(LED_PIN, LOW);   // LED OFF when no field
  }
  
  // Print values at interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    printSensorData();
  }
}

void printSensorData() {
  Serial.print("Raw: ");
  Serial.print(analogValue);
  Serial.print(" | Calibrated: ");
  Serial.print(calibratedValue);
  
  Serial.print(" | Digital: ");
  Serial.print(digitalValue == LOW ? "TRIGGERED" : "Normal");
  
  // Magnetic polarity and strength
  Serial.print(" | Field: ");
  if (calibratedValue > 200) {
    Serial.print("N-pole (Strong)");
  } else if (calibratedValue > 100) {
    Serial.print("N-pole (Weak)");
  } else if (calibratedValue < -200) {
    Serial.print("S-pole (Strong)");
  } else if (calibratedValue < -100) {
    Serial.print("S-pole (Weak)");
  } else {
    Serial.print("None/Minimal");
  }
  
  // Visual bar graph
  Serial.print(" |");
  printBarGraph(calibratedValue);
  Serial.println("|");
}

void printBarGraph(int value) {
  // Map value to bar width (-10 to +10)
  int barValue = constrain(map(value, -500, 500, -10, 10), -10, 10);
  
  // Print left side (S-pole)
  for (int i = -10; i < 0; i++) {
    if (i >= barValue && barValue < 0) {
      Serial.print("=");
    } else {
      Serial.print(" ");
    }
  }
  
  Serial.print("|");  // Center marker
  
  // Print right side (N-pole)
  for (int i = 0; i < 10; i++) {
    if (i < barValue && barValue > 0) {
      Serial.print("=");
    } else {
      Serial.print(" ");
    }
  }
}
