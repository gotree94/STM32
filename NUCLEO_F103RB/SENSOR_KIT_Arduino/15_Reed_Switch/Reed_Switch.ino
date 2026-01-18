/*
 * Reed Switch Module Test Code
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * IDE: Arduino IDE with STM32 Core
 * 
 * 리드 스위치 모듈 테스트
 * - 자기장에 의해 동작하는 스위치
 * - 문/창문 열림 감지, 위치 감지 등에 활용
 * - 디지털 출력: 자석 접근 시 상태 변화
 * 
 * Wiring:
 * - VCC -> 3.3V or 5V
 * - GND -> GND
 * - DO  -> D2 (PA10)
 * - AO  -> A0 (PA0) (일부 모듈만 지원)
 */

// Pin Definitions
#define REED_DO_PIN   D2    // Digital Output Pin (PA10)
#define REED_AO_PIN   A0    // Analog Output Pin (PA0) - optional
#define LED_PIN       LED_BUILTIN  // Onboard LED (PA5)

// Configuration
#define USE_ANALOG    false  // Set true if your module has AO pin
#define ACTIVE_LOW    true   // Most reed switch modules are active LOW

// Variables
int reedState = 0;
int lastReedState = 0;
int analogValue = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Event tracking
unsigned long openCount = 0;
unsigned long closeCount = 0;
unsigned long lastOpenTime = 0;
unsigned long lastCloseTime = 0;
unsigned long totalOpenTime = 0;
bool isDoorOpen = false;
unsigned long doorOpenStartTime = 0;

void setup() {
  // Serial Communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("========================================");
  Serial.println("  Reed Switch Module Test");
  Serial.println("  Board: NUCLEO-F103RB");
  Serial.println("========================================");
  Serial.println();
  
  // Pin Mode Setup
  pinMode(REED_DO_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  if (USE_ANALOG) {
    analogReadResolution(12);
  }
  
  // Initial state
  reedState = digitalRead(REED_DO_PIN);
  lastReedState = reedState;
  isDoorOpen = isOpen(reedState);
  
  if (isDoorOpen) {
    doorOpenStartTime = millis();
  }
  
  Serial.println("Reed switch initialized.");
  Serial.print("Switch type: ");
  Serial.println(ACTIVE_LOW ? "Active LOW (normally open)" : "Active HIGH (normally closed)");
  Serial.println();
  Serial.print("Initial state: ");
  Serial.println(isDoorOpen ? "OPEN (no magnet)" : "CLOSED (magnet detected)");
  Serial.println();
  Serial.println("Commands via Serial:");
  Serial.println("  's' - Show statistics");
  Serial.println("  'r' - Reset statistics");
  Serial.println();
  Serial.println("Monitoring for state changes...");
  Serial.println();
}

void loop() {
  // Read current state
  int reading = digitalRead(REED_DO_PIN);
  
  // Debounce
  if (reading != lastReedState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != reedState) {
      reedState = reading;
      
      // State changed
      handleStateChange();
    }
  }
  
  // Update LED based on state
  if (isDoorOpen) {
    // Blink LED when open
    digitalWrite(LED_PIN, (millis() / 500) % 2);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  
  // Handle serial commands
  handleSerialCommands();
  
  // Save last state
  lastReedState = reading;
  
  // Read analog if available
  if (USE_ANALOG) {
    analogValue = analogRead(REED_AO_PIN);
  }
}

bool isOpen(int state) {
  // Determine if door/window is open based on active level
  if (ACTIVE_LOW) {
    return (state == HIGH);  // No magnet = HIGH = Open
  } else {
    return (state == LOW);   // No magnet = LOW = Open
  }
}

void handleStateChange() {
  bool newOpenState = isOpen(reedState);
  unsigned long currentTime = millis();
  
  Serial.println("========================================");
  Serial.print("STATE CHANGE at ");
  printTimestamp();
  Serial.println();
  
  if (newOpenState && !isDoorOpen) {
    // Door/Window opened
    openCount++;
    lastOpenTime = currentTime;
    doorOpenStartTime = currentTime;
    
    Serial.println(">>> OPENED (Magnet removed)");
    Serial.print("Open count: ");
    Serial.println(openCount);
    
  } else if (!newOpenState && isDoorOpen) {
    // Door/Window closed
    closeCount++;
    lastCloseTime = currentTime;
    
    // Calculate how long it was open
    unsigned long openDuration = currentTime - doorOpenStartTime;
    totalOpenTime += openDuration;
    
    Serial.println("<<< CLOSED (Magnet detected)");
    Serial.print("Close count: ");
    Serial.println(closeCount);
    Serial.print("Was open for: ");
    printDuration(openDuration);
    Serial.println();
  }
  
  isDoorOpen = newOpenState;
  
  // Print analog value if available
  if (USE_ANALOG) {
    Serial.print("Analog value: ");
    Serial.println(analogValue);
  }
  
  Serial.println("========================================");
  Serial.println();
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 's':
      case 'S':
        printStatistics();
        break;
      case 'r':
      case 'R':
        resetStatistics();
        break;
    }
  }
}

void printStatistics() {
  Serial.println();
  Serial.println("=========== REED SWITCH STATISTICS ===========");
  Serial.print("Current state: ");
  Serial.println(isDoorOpen ? "OPEN" : "CLOSED");
  Serial.println();
  Serial.print("Open events: ");
  Serial.println(openCount);
  Serial.print("Close events: ");
  Serial.println(closeCount);
  Serial.println();
  
  if (lastOpenTime > 0) {
    Serial.print("Last opened: ");
    printDuration(millis() - lastOpenTime);
    Serial.println(" ago");
  }
  
  if (lastCloseTime > 0) {
    Serial.print("Last closed: ");
    printDuration(millis() - lastCloseTime);
    Serial.println(" ago");
  }
  
  Serial.println();
  Serial.print("Total open time: ");
  unsigned long currentOpenTime = totalOpenTime;
  if (isDoorOpen) {
    currentOpenTime += (millis() - doorOpenStartTime);
  }
  printDuration(currentOpenTime);
  Serial.println();
  
  if (USE_ANALOG) {
    Serial.print("Current analog value: ");
    Serial.println(analogValue);
  }
  
  Serial.println("==============================================");
  Serial.println();
}

void resetStatistics() {
  openCount = 0;
  closeCount = 0;
  lastOpenTime = 0;
  lastCloseTime = 0;
  totalOpenTime = 0;
  
  if (isDoorOpen) {
    doorOpenStartTime = millis();
  }
  
  Serial.println();
  Serial.println("*** Statistics Reset ***");
  Serial.println();
}

void printTimestamp() {
  unsigned long totalSeconds = millis() / 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;
  
  if (hours < 10) Serial.print("0");
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.print(seconds);
}

void printDuration(unsigned long ms) {
  unsigned long totalSeconds = ms / 1000;
  
  if (totalSeconds >= 3600) {
    Serial.print(totalSeconds / 3600);
    Serial.print("h ");
  }
  if (totalSeconds >= 60) {
    Serial.print((totalSeconds % 3600) / 60);
    Serial.print("m ");
  }
  Serial.print(totalSeconds % 60);
  Serial.print("s");
}
