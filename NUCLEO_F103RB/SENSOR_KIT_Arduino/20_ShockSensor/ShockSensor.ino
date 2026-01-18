/*
 * 충격 센서 모듈 테스트 (KY-031 Knock/Shock Sensor)
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   - VCC: 3.3V 또는 5V
 *   - GND: GND
 *   - Signal: PA0
 */

#define SHOCK_PIN PA0        // 충격 센서 핀
#define LED_PIN LED_BUILTIN  // 내장 LED (PA5)

// 충격 감지 변수
volatile bool shockDetected = false;
unsigned long lastShockTime = 0;
const unsigned long debounceTime = 100;  // 100ms 디바운싱

// 충격 카운터
unsigned long shockCount = 0;

// 충격 강도 측정 (연속 충격 횟수로 추정)
int rapidShockCount = 0;
unsigned long rapidShockStartTime = 0;
const unsigned long rapidShockWindow = 500;  // 500ms 윈도우

// 경보 모드
bool alarmMode = false;
bool alarmTriggered = false;
unsigned long alarmStartTime = 0;
const unsigned long alarmDuration = 3000;  // 3초 경보

// LED 상태
bool ledBlinking = false;
unsigned long lastBlinkTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  pinMode(SHOCK_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // 인터럽트 설정 (선택적 - 폴링 방식도 사용)
  attachInterrupt(digitalPinToInterrupt(SHOCK_PIN), shockISR, FALLING);
  
  Serial.println("================================");
  Serial.println("충격 센서 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println();
  Serial.println("명령어:");
  Serial.println("  a: 경보 모드 ON/OFF 토글");
  Serial.println("  r: 카운터 리셋");
  Serial.println("  s: 상태 출력");
  Serial.println();
  Serial.println("센서를 두드리거나 흔들어보세요!");
  Serial.println();
}

void loop() {
  // 시리얼 명령 처리
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
  }
  
  // 충격 감지 처리 (인터럽트에서 플래그 설정)
  if (shockDetected) {
    processShock();
  }
  
  // 폴링 방식으로도 충격 감지 (인터럽트 보완)
  static bool lastState = HIGH;
  bool currentState = digitalRead(SHOCK_PIN);
  
  if (currentState == LOW && lastState == HIGH) {
    if ((millis() - lastShockTime) > debounceTime) {
      shockDetected = true;
    }
  }
  lastState = currentState;
  
  // 경보 처리
  if (alarmTriggered) {
    handleAlarm();
  }
  
  // LED 깜빡임 처리
  if (ledBlinking && !alarmTriggered) {
    if ((millis() - lastBlinkTime) > 50) {
      digitalWrite(LED_PIN, LOW);
      ledBlinking = false;
    }
  }
}

// 인터럽트 서비스 루틴
void shockISR() {
  if ((millis() - lastShockTime) > debounceTime) {
    shockDetected = true;
  }
}

void processShock() {
  unsigned long currentTime = millis();
  
  // 디바운싱 확인
  if ((currentTime - lastShockTime) < debounceTime) {
    shockDetected = false;
    return;
  }
  
  lastShockTime = currentTime;
  shockCount++;
  shockDetected = false;
  
  // LED 깜빡임
  digitalWrite(LED_PIN, HIGH);
  ledBlinking = true;
  lastBlinkTime = currentTime;
  
  // 빠른 연속 충격 감지
  if ((currentTime - rapidShockStartTime) < rapidShockWindow) {
    rapidShockCount++;
  } else {
    rapidShockCount = 1;
    rapidShockStartTime = currentTime;
  }
  
  // 충격 강도 판정
  String intensity = "약함";
  if (rapidShockCount >= 5) {
    intensity = "매우 강함";
  } else if (rapidShockCount >= 3) {
    intensity = "강함";
  } else if (rapidShockCount >= 2) {
    intensity = "중간";
  }
  
  Serial.print("[충격 감지] #");
  Serial.print(shockCount);
  Serial.print(" | 강도: ");
  Serial.print(intensity);
  Serial.print(" (연속 ");
  Serial.print(rapidShockCount);
  Serial.println("회)");
  
  // 경보 모드에서 강한 충격 시 경보 발동
  if (alarmMode && rapidShockCount >= 3) {
    triggerAlarm();
  }
}

void processCommand(char cmd) {
  switch (cmd) {
    case 'a':
    case 'A':
      alarmMode = !alarmMode;
      Serial.print("[설정] 경보 모드: ");
      Serial.println(alarmMode ? "ON" : "OFF");
      if (alarmMode) {
        Serial.println("        강한 충격(3회 연속) 감지 시 경보가 발동됩니다.");
      }
      break;
      
    case 'r':
    case 'R':
      shockCount = 0;
      rapidShockCount = 0;
      Serial.println("[설정] 카운터 리셋");
      break;
      
    case 's':
    case 'S':
      printStatus();
      break;
      
    default:
      if (cmd != '\n' && cmd != '\r') {
        Serial.println("알 수 없는 명령입니다.");
      }
      break;
  }
}

void printStatus() {
  Serial.println();
  Serial.println("=== 현재 상태 ===");
  Serial.print("총 충격 횟수: ");
  Serial.println(shockCount);
  Serial.print("경보 모드: ");
  Serial.println(alarmMode ? "ON" : "OFF");
  Serial.print("센서 상태: ");
  Serial.println(digitalRead(SHOCK_PIN) == HIGH ? "정상" : "감지중");
  Serial.println("==================");
  Serial.println();
}

void triggerAlarm() {
  if (!alarmTriggered) {
    alarmTriggered = true;
    alarmStartTime = millis();
    Serial.println();
    Serial.println("🚨 [경보] 강한 충격이 감지되었습니다! 🚨");
    Serial.println();
  }
}

void handleAlarm() {
  unsigned long elapsed = millis() - alarmStartTime;
  
  if (elapsed < alarmDuration) {
    // 빠른 LED 깜빡임
    if ((elapsed / 100) % 2 == 0) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  } else {
    // 경보 종료
    alarmTriggered = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("[경보] 경보 종료");
  }
}
