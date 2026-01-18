/*
 * 각도 스위치 모듈 테스트 (KY-020 Tilt Switch Module)
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   - VCC: 3.3V 또는 5V
 *   - GND: GND
 *   - Signal: PA0
 */

#define TILT_PIN PA0         // 각도 스위치 입력
#define LED_PIN LED_BUILTIN  // 내장 LED (PA5)

// 상태 변수
bool tiltState = false;
bool lastTiltState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// 통계
unsigned long tiltCount = 0;
unsigned long lastTiltTime = 0;
unsigned long totalTiltTime = 0;
bool isTiming = false;

// 안정성 감지
int stabilityCounter = 0;
const int STABILITY_THRESHOLD = 100;  // 100회 연속 같은 상태면 안정
bool isStable = false;

// 경보 모드
bool alarmMode = false;
bool alarmTriggered = false;
unsigned long alarmStartTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  pinMode(TILT_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("================================");
  Serial.println("각도 스위치 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println();
  Serial.println("명령어:");
  Serial.println("  s: 상태 및 통계 출력");
  Serial.println("  r: 통계 리셋");
  Serial.println("  a: 경보 모드 토글");
  Serial.println();
  Serial.println("모듈을 기울여보세요!");
  Serial.println();
}

void loop() {
  // 시리얼 명령 처리
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
  }
  
  // 각도 스위치 상태 읽기
  bool reading = digitalRead(TILT_PIN);
  
  // 안정성 체크
  static bool prevReading = reading;
  if (reading == prevReading) {
    stabilityCounter++;
    if (stabilityCounter >= STABILITY_THRESHOLD && !isStable) {
      isStable = true;
      Serial.println("[상태] 안정됨");
    }
  } else {
    stabilityCounter = 0;
    if (isStable) {
      isStable = false;
      Serial.println("[상태] 움직임 감지");
    }
  }
  prevReading = reading;
  
  // 디바운싱
  if (reading != lastTiltState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != tiltState) {
      tiltState = reading;
      
      if (tiltState == LOW) {
        // 기울어짐 (스위치 ON)
        tiltCount++;
        lastTiltTime = millis();
        isTiming = true;
        digitalWrite(LED_PIN, HIGH);
        
        Serial.print("[감지] 기울어짐! (총 ");
        Serial.print(tiltCount);
        Serial.println("회)");
        
        // 경보 모드에서 경보 발동
        if (alarmMode && !alarmTriggered) {
          triggerAlarm();
        }
      } else {
        // 원위치 (스위치 OFF)
        if (isTiming) {
          unsigned long tiltDuration = millis() - lastTiltTime;
          totalTiltTime += tiltDuration;
          isTiming = false;
          
          Serial.print("[감지] 원위치 (기울어진 시간: ");
          Serial.print(tiltDuration);
          Serial.println("ms)");
        }
        digitalWrite(LED_PIN, LOW);
      }
    }
  }
  
  lastTiltState = reading;
  
  // 경보 처리
  if (alarmTriggered) {
    handleAlarm();
  }
  
  delay(10);
}

void processCommand(char cmd) {
  switch (cmd) {
    case 's':
    case 'S':
      printStatus();
      break;
      
    case 'r':
    case 'R':
      resetStats();
      break;
      
    case 'a':
    case 'A':
      alarmMode = !alarmMode;
      Serial.print("[설정] 경보 모드: ");
      Serial.println(alarmMode ? "ON" : "OFF");
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
  Serial.println("========== 상태 정보 ==========");
  Serial.print("현재 상태: ");
  Serial.println(tiltState == LOW ? "기울어짐" : "정위치");
  Serial.print("안정 상태: ");
  Serial.println(isStable ? "안정" : "불안정");
  Serial.println();
  Serial.println("========== 통계 정보 ==========");
  Serial.print("총 기울임 횟수: ");
  Serial.println(tiltCount);
  Serial.print("총 기울어진 시간: ");
  Serial.print(totalTiltTime / 1000.0, 2);
  Serial.println("초");
  if (tiltCount > 0) {
    Serial.print("평균 기울임 시간: ");
    Serial.print((totalTiltTime / tiltCount) / 1000.0, 2);
    Serial.println("초");
  }
  Serial.print("경보 모드: ");
  Serial.println(alarmMode ? "ON" : "OFF");
  Serial.println("================================");
  Serial.println();
}

void resetStats() {
  tiltCount = 0;
  totalTiltTime = 0;
  Serial.println("[설정] 통계 리셋 완료");
}

void triggerAlarm() {
  alarmTriggered = true;
  alarmStartTime = millis();
  Serial.println();
  Serial.println("🚨 [경보] 기울기 감지! 🚨");
}

void handleAlarm() {
  unsigned long elapsed = millis() - alarmStartTime;
  
  if (elapsed < 3000) {
    // 3초간 빠른 LED 깜빡임
    digitalWrite(LED_PIN, (elapsed / 100) % 2);
  } else {
    alarmTriggered = false;
    digitalWrite(LED_PIN, tiltState == LOW ? HIGH : LOW);
    Serial.println("[경보] 경보 종료");
  }
}
