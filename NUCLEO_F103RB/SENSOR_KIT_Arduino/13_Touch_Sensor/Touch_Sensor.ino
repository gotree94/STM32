/*
 * Touch Sensor Module Test Code
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * IDE: Arduino IDE with STM32 Core
 * 
 * 터치 센서 모듈 테스트
 * - 정전식 터치 감지 (TTP223 칩 기반)
 * - 디지털 출력: 터치 시 HIGH 또는 LOW (모듈에 따라 다름)
 * - 토글 모드 또는 순간 모드 지원
 * 
 * Wiring:
 * - VCC -> 3.3V or 5V
 * - GND -> GND
 * - SIG -> D2 (PA10)
 */

// Pin Definitions
#define TOUCH_PIN     D2    // Touch Sensor Signal Pin (PA10)
#define LED_PIN       LED_BUILTIN  // Onboard LED (PA5)

// Variables
int touchState = 0;
int lastTouchState = 0;
int touchCount = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;    // 디바운스 시간

// Touch duration tracking
unsigned long touchStartTime = 0;
unsigned long touchDuration = 0;
bool isTouching = false;

// Statistics
unsigned long totalTouchTime = 0;
int shortTouches = 0;   // < 500ms
int longTouches = 0;    // >= 500ms

void setup() {
  // Serial Communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("========================================");
  Serial.println("  Touch Sensor Module Test");
  Serial.println("  Board: NUCLEO-F103RB");
  Serial.println("========================================");
  Serial.println();
  
  // Pin Mode Setup
  pinMode(TOUCH_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("Touch sensor initialized.");
  Serial.println("Touch the sensor pad...");
  Serial.println();
  Serial.println("Commands via Serial:");
  Serial.println("  'r' - Reset statistics");
  Serial.println("  's' - Show statistics");
  Serial.println();
}

void loop() {
  // Read current touch state
  int reading = digitalRead(TOUCH_PIN);
  
  // Debounce
  if (reading != lastTouchState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // State has been stable for debounce period
    if (reading != touchState) {
      touchState = reading;
      
      // Touch detected (HIGH = touched for most modules)
      if (touchState == HIGH) {
        handleTouchStart();
      } else {
        handleTouchEnd();
      }
    }
  }
  
  // Update LED
  digitalWrite(LED_PIN, touchState);
  
  // Handle Serial commands
  handleSerialCommands();
  
  // Save last state
  lastTouchState = reading;
  
  // Update touch duration while touching
  if (isTouching) {
    touchDuration = millis() - touchStartTime;
  }
}

void handleTouchStart() {
  touchCount++;
  touchStartTime = millis();
  isTouching = true;
  
  Serial.println("----------------------------------------");
  Serial.print("TOUCH #");
  Serial.print(touchCount);
  Serial.println(" - Started");
  Serial.print("Time: ");
  printTimestamp();
  Serial.println();
}

void handleTouchEnd() {
  isTouching = false;
  touchDuration = millis() - touchStartTime;
  totalTouchTime += touchDuration;
  
  // Classify touch type
  String touchType;
  if (touchDuration < 500) {
    touchType = "Short tap";
    shortTouches++;
  } else {
    touchType = "Long press";
    longTouches++;
  }
  
  Serial.print("TOUCH #");
  Serial.print(touchCount);
  Serial.print(" - Released (");
  Serial.print(touchType);
  Serial.println(")");
  Serial.print("Duration: ");
  Serial.print(touchDuration);
  Serial.println(" ms");
  Serial.println("----------------------------------------");
  Serial.println();
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 'r':
      case 'R':
        resetStatistics();
        break;
      case 's':
      case 'S':
        printStatistics();
        break;
    }
  }
}

void resetStatistics() {
  touchCount = 0;
  totalTouchTime = 0;
  shortTouches = 0;
  longTouches = 0;
  
  Serial.println();
  Serial.println("*** Statistics Reset ***");
  Serial.println();
}

void printStatistics() {
  Serial.println();
  Serial.println("========== TOUCH STATISTICS ==========");
  Serial.print("Total touches: ");
  Serial.println(touchCount);
  Serial.print("Short taps (<500ms): ");
  Serial.println(shortTouches);
  Serial.print("Long presses (>=500ms): ");
  Serial.println(longTouches);
  Serial.print("Total touch time: ");
  Serial.print(totalTouchTime);
  Serial.println(" ms");
  
  if (touchCount > 0) {
    Serial.print("Average touch duration: ");
    Serial.print(totalTouchTime / touchCount);
    Serial.println(" ms");
  }
  
  Serial.print("Current state: ");
  Serial.println(touchState == HIGH ? "TOUCHING" : "Released");
  Serial.println("=======================================");
  Serial.println();
}

void printTimestamp() {
  unsigned long totalSeconds = millis() / 1000;
  unsigned long minutes = totalSeconds / 60;
  unsigned long seconds = totalSeconds % 60;
  
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.print(seconds);
}
