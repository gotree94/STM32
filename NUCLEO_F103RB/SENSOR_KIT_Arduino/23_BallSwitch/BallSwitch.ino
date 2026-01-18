/*
 * 볼 스위치 모듈 테스트 (KY-017 Mercury/Ball Switch Module)
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   - VCC: 3.3V 또는 5V
 *   - GND: GND
 *   - Signal: PA0
 */

#define BALL_PIN PA0         // 볼 스위치 입력
#define LED_PIN LED_BUILTIN  // 내장 LED (PA5)

// 상태 변수
volatile bool motionDetected = false;
bool currentState = false;
bool lastState = false;

// 디바운싱
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 20;

// 움직임 감지 통계
unsigned long motionCount = 0;
unsigned long sessionStartTime = 0;

// 움직임 강도 측정 (일정 시간 내 감지 횟수)
const unsigned long INTENSITY_WINDOW = 1000;  // 1초
int intensityCount = 0;
unsigned long intensityWindowStart = 0;

// 정지 감지
unsigned long lastMotionTime = 0;
const unsigned long STILL_THRESHOLD = 2000;  // 2초간 움직임 없으면 정지
bool isStill = true;

// 모드
enum Mode {
  MODE_BASIC,      // 기본 감지
  MODE_MOTION,     // 움직임 감지 (모션센서처럼)
  MODE_SHAKE,      // 흔들기 감지
  MODE_ALARM       // 경보 모드
};

Mode currentMode = MODE_BASIC;

// 흔들기 감지 변수
int shakeCount = 0;
unsigned long shakeWindowStart = 0;
const int SHAKE_THRESHOLD = 5;  // 1초에 5회 이상이면 흔들기

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  pinMode(BALL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // 인터럽트 설정
  attachInterrupt(digitalPinToInterrupt(BALL_PIN), motionISR, CHANGE);
  
  sessionStartTime = millis();
  intensityWindowStart = millis();
  shakeWindowStart = millis();
  
  Serial.println("================================");
  Serial.println("볼 스위치 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println();
  Serial.println("명령어:");
  Serial.println("  1: 기본 모드");
  Serial.println("  2: 움직임 감지 모드");
  Serial.println("  3: 흔들기 감지 모드");
  Serial.println("  4: 경보 모드");
  Serial.println("  s: 상태 출력");
  Serial.println("  r: 리셋");
  Serial.println();
  Serial.println("모듈을 움직이거나 흔들어보세요!");
  Serial.println();
}

void loop() {
  unsigned long currentTime = millis();
  
  // 시리얼 명령 처리
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
  }
  
  // 폴링 방식 감지 (인터럽트 보완)
  bool reading = digitalRead(BALL_PIN);
  
  if (reading != lastState) {
    lastDebounceTime = currentTime;
  }
  
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    if (reading != currentState) {
      currentState = reading;
      motionDetected = true;
    }
  }
  lastState = reading;
  
  // 움직임 감지 처리
  if (motionDetected) {
    processMotion(currentTime);
    motionDetected = false;
  }
  
  // 정지 상태 확인
  if ((currentTime - lastMotionTime) > STILL_THRESHOLD) {
    if (!isStill) {
      isStill = true;
      Serial.println("[상태] 정지됨");
      digitalWrite(LED_PIN, LOW);
    }
  }
  
  // 강도 윈도우 리셋
  if ((currentTime - intensityWindowStart) > INTENSITY_WINDOW) {
    if (intensityCount > 0 && currentMode == MODE_MOTION) {
      printIntensity(intensityCount);
    }
    intensityCount = 0;
    intensityWindowStart = currentTime;
  }
  
  // 흔들기 윈도우 리셋
  if ((currentTime - shakeWindowStart) > 1000) {
    if (shakeCount >= SHAKE_THRESHOLD && currentMode == MODE_SHAKE) {
      Serial.println("🎉 [흔들기] 흔들기 감지!");
      blinkLED(5, 50);
    }
    shakeCount = 0;
    shakeWindowStart = currentTime;
  }
  
  delay(5);
}

void motionISR() {
  motionDetected = true;
}

void processMotion(unsigned long currentTime) {
  motionCount++;
  lastMotionTime = currentTime;
  intensityCount++;
  shakeCount++;
  
  if (isStill) {
    isStill = false;
    Serial.println("[상태] 움직임 시작");
  }
  
  // 모드별 처리
  switch (currentMode) {
    case MODE_BASIC:
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // 토글
      Serial.print("[감지] 볼 이동 #");
      Serial.println(motionCount);
      break;
      
    case MODE_MOTION:
      digitalWrite(LED_PIN, HIGH);
      // 강도는 윈도우 종료 시 출력
      break;
      
    case MODE_SHAKE:
      digitalWrite(LED_PIN, HIGH);
      // 흔들기는 윈도우 종료 시 판정
      break;
      
    case MODE_ALARM:
      triggerAlarm();
      break;
  }
}

void processCommand(char cmd) {
  switch (cmd) {
    case '1':
      currentMode = MODE_BASIC;
      Serial.println("[모드] 기본 모드");
      break;
    case '2':
      currentMode = MODE_MOTION;
      Serial.println("[모드] 움직임 감지 모드");
      Serial.println("       움직임 강도를 측정합니다.");
      break;
    case '3':
      currentMode = MODE_SHAKE;
      Serial.println("[모드] 흔들기 감지 모드");
      Serial.println("       1초에 5회 이상 흔들면 감지됩니다.");
      break;
    case '4':
      currentMode = MODE_ALARM;
      Serial.println("[모드] 경보 모드");
      Serial.println("       움직임 감지 시 경보가 울립니다.");
      break;
    case 's':
    case 'S':
      printStatus();
      break;
    case 'r':
    case 'R':
      resetStats();
      break;
    default:
      if (cmd != '\n' && cmd != '\r') {
        Serial.println("알 수 없는 명령입니다.");
      }
      break;
  }
}

void printIntensity(int count) {
  Serial.print("[강도] ");
  if (count <= 2) {
    Serial.println("약함 (미세한 움직임)");
  } else if (count <= 5) {
    Serial.println("보통 (일반 움직임)");
  } else if (count <= 10) {
    Serial.println("강함 (빠른 움직임)");
  } else {
    Serial.println("매우 강함 (흔들림)");
  }
}

void printStatus() {
  unsigned long elapsed = (millis() - sessionStartTime) / 1000;
  
  Serial.println();
  Serial.println("========== 볼 스위치 상태 ==========");
  Serial.print("현재 상태: ");
  Serial.println(isStill ? "정지" : "움직이는 중");
  Serial.print("현재 모드: ");
  switch (currentMode) {
    case MODE_BASIC: Serial.println("기본"); break;
    case MODE_MOTION: Serial.println("움직임 감지"); break;
    case MODE_SHAKE: Serial.println("흔들기 감지"); break;
    case MODE_ALARM: Serial.println("경보"); break;
  }
  Serial.print("총 감지 횟수: ");
  Serial.println(motionCount);
  Serial.print("동작 시간: ");
  Serial.print(elapsed);
  Serial.println("초");
  if (elapsed > 0) {
    Serial.print("평균 감지율: ");
    Serial.print((float)motionCount / elapsed, 2);
    Serial.println("회/초");
  }
  Serial.println("=====================================");
  Serial.println();
}

void resetStats() {
  motionCount = 0;
  sessionStartTime = millis();
  Serial.println("[설정] 통계 리셋 완료");
}

void triggerAlarm() {
  Serial.println("🚨 [경보] 움직임 감지! 🚨");
  blinkLED(10, 50);
}

void blinkLED(int times, int interval) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(interval);
    digitalWrite(LED_PIN, LOW);
    delay(interval);
  }
}
