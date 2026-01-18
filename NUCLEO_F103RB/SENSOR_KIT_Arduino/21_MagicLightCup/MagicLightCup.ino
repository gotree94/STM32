/*
 * 매직 라이트컵 모듈 테스트 (KY-027 Magic Light Cup)
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 모듈 구성: 수은 스위치 + LED 내장
 * 
 * 핀 연결:
 *   - VCC: 3.3V
 *   - GND: GND
 *   - Signal (S): PA0 (수은 스위치 출력)
 *   - LED (L): PA1 (LED 제어, PWM)
 */

#define SWITCH_PIN PA0       // 수은 스위치 입력
#define MODULE_LED_PIN PA1   // 모듈 내장 LED (PWM)
#define BOARD_LED LED_BUILTIN // 보드 내장 LED

// 기울기 상태
bool isTilted = false;
bool lastTiltState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 30;

// LED 밝기 제어
int brightness = 0;
int targetBrightness = 0;
const int fadeSpeed = 10;  // 밝기 변화 속도

// 물 흐르는 효과 변수
bool waterEffect = false;
int waterLevel = 0;
int waterDirection = 1;

// 동작 모드
enum Mode {
  MODE_BASIC,      // 기본 기울기 감지
  MODE_FADE,       // 부드러운 페이드
  MODE_WATER,      // 물 흐르는 효과
  MODE_BRIGHTNESS  // 기울기로 밝기 조절
};

Mode currentMode = MODE_FADE;

// 기울기 각도 추정 (연속 감지 시간 기반)
unsigned long tiltStartTime = 0;
int estimatedAngle = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  pinMode(SWITCH_PIN, INPUT);
  pinMode(MODULE_LED_PIN, OUTPUT);
  pinMode(BOARD_LED, OUTPUT);
  
  analogWrite(MODULE_LED_PIN, 0);
  
  Serial.println("================================");
  Serial.println("매직 라이트컵 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println();
  Serial.println("명령어:");
  Serial.println("  1: 기본 모드 (ON/OFF)");
  Serial.println("  2: 페이드 모드 (부드러운 전환)");
  Serial.println("  3: 물결 효과 모드");
  Serial.println("  4: 밝기 조절 모드");
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
  
  // 기울기 상태 읽기 (디바운싱)
  bool reading = digitalRead(SWITCH_PIN);
  
  if (reading != lastTiltState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != isTilted) {
      isTilted = reading;
      
      if (isTilted) {
        tiltStartTime = millis();
        Serial.println("[기울기] 기울어짐 감지");
      } else {
        unsigned long tiltDuration = millis() - tiltStartTime;
        Serial.print("[기울기] 원위치 복귀 (기울어진 시간: ");
        Serial.print(tiltDuration);
        Serial.println("ms)");
      }
    }
  }
  
  lastTiltState = reading;
  
  // 현재 모드에 따른 LED 제어
  switch (currentMode) {
    case MODE_BASIC:
      basicMode();
      break;
    case MODE_FADE:
      fadeMode();
      break;
    case MODE_WATER:
      waterMode();
      break;
    case MODE_BRIGHTNESS:
      brightnessMode();
      break;
  }
  
  delay(10);
}

void processCommand(char cmd) {
  switch (cmd) {
    case '1':
      currentMode = MODE_BASIC;
      Serial.println("[모드] 기본 모드 (ON/OFF)");
      break;
    case '2':
      currentMode = MODE_FADE;
      Serial.println("[모드] 페이드 모드");
      break;
    case '3':
      currentMode = MODE_WATER;
      waterLevel = 0;
      Serial.println("[모드] 물결 효과 모드");
      break;
    case '4':
      currentMode = MODE_BRIGHTNESS;
      Serial.println("[모드] 밝기 조절 모드");
      break;
    default:
      if (cmd != '\n' && cmd != '\r') {
        Serial.println("알 수 없는 명령입니다. 1-4 중 선택하세요.");
      }
      break;
  }
}

// 기본 모드: 단순 ON/OFF
void basicMode() {
  if (isTilted) {
    analogWrite(MODULE_LED_PIN, 255);
    digitalWrite(BOARD_LED, HIGH);
  } else {
    analogWrite(MODULE_LED_PIN, 0);
    digitalWrite(BOARD_LED, LOW);
  }
}

// 페이드 모드: 부드러운 밝기 전환
void fadeMode() {
  // 목표 밝기 설정
  targetBrightness = isTilted ? 255 : 0;
  
  // 부드럽게 목표 밝기로 전환
  if (brightness < targetBrightness) {
    brightness = min(brightness + fadeSpeed, targetBrightness);
  } else if (brightness > targetBrightness) {
    brightness = max(brightness - fadeSpeed, targetBrightness);
  }
  
  analogWrite(MODULE_LED_PIN, brightness);
  digitalWrite(BOARD_LED, brightness > 127 ? HIGH : LOW);
}

// 물결 효과 모드: 기울이면 물이 차오르는 효과
void waterMode() {
  if (isTilted) {
    // 기울어지면 물이 차오름
    waterLevel += 3;
    if (waterLevel > 255) waterLevel = 255;
  } else {
    // 원위치면 물이 빠짐
    waterLevel -= 5;
    if (waterLevel < 0) waterLevel = 0;
  }
  
  // 출렁이는 효과 추가
  int wave = (int)(sin(millis() / 100.0) * 20);
  int finalLevel = constrain(waterLevel + wave, 0, 255);
  
  analogWrite(MODULE_LED_PIN, finalLevel);
  digitalWrite(BOARD_LED, finalLevel > 127 ? HIGH : LOW);
}

// 밝기 조절 모드: 기울어진 시간에 따라 밝기 조절
void brightnessMode() {
  static int savedBrightness = 128;
  
  if (isTilted) {
    // 기울어진 시간에 따라 밝기 증가
    unsigned long tiltDuration = millis() - tiltStartTime;
    int addBrightness = tiltDuration / 50;  // 50ms당 1씩 증가
    brightness = constrain(savedBrightness + addBrightness, 0, 255);
  } else {
    // 원위치 시 현재 밝기 저장
    savedBrightness = brightness;
  }
  
  analogWrite(MODULE_LED_PIN, brightness);
  digitalWrite(BOARD_LED, brightness > 127 ? HIGH : LOW);
  
  // 주기적으로 현재 밝기 출력
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500 && isTilted) {
    Serial.print("[밝기] ");
    Serial.print(map(brightness, 0, 255, 0, 100));
    Serial.println("%");
    lastPrint = millis();
  }
}
