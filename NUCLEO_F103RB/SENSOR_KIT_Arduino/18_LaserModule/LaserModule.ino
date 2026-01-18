/*
 * 레이저 모듈 테스트 (KY-008 Laser Module)
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   - VCC(중앙): 5V
 *   - GND(-): GND
 *   - Signal(S): PA0
 */

#define LASER_PIN PA0        // 레이저 제어 핀
#define LED_PIN LED_BUILTIN  // 내장 LED (PA5)

// 동작 모드
enum LaserMode {
  MODE_ON,
  MODE_OFF,
  MODE_BLINK,
  MODE_SOS,
  MODE_FADE
};

LaserMode currentMode = MODE_OFF;
unsigned long previousMillis = 0;
bool laserState = false;

// SOS 모드용 변수
const int sosPattern[] = {1, 0, 1, 0, 1, 0, 0, 0,   // S: ...
                          1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0,  // O: ---
                          1, 0, 1, 0, 1, 0, 0, 0, 0, 0};  // S: ...
const int sosLength = 30;
int sosIndex = 0;
const int dotDuration = 150;

// PWM Fade 모드용 변수
int brightness = 0;
int fadeAmount = 5;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  pinMode(LASER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LASER_PIN, LOW);
  
  Serial.println("================================");
  Serial.println("레이저 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println();
  Serial.println("명령어:");
  Serial.println("  1: 레이저 ON");
  Serial.println("  2: 레이저 OFF");
  Serial.println("  3: 깜빡임 모드");
  Serial.println("  4: SOS 모드");
  Serial.println("  5: 밝기 조절(Fade) 모드");
  Serial.println();
  Serial.println("⚠️  주의: 눈에 직접 비추지 마세요!");
  Serial.println();
}

void loop() {
  // 시리얼 명령 처리
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
  }
  
  // 현재 모드에 따른 동작
  switch (currentMode) {
    case MODE_ON:
      digitalWrite(LASER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      break;
      
    case MODE_OFF:
      digitalWrite(LASER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      break;
      
    case MODE_BLINK:
      blinkMode();
      break;
      
    case MODE_SOS:
      sosMode();
      break;
      
    case MODE_FADE:
      fadeMode();
      break;
  }
}

void processCommand(char cmd) {
  switch (cmd) {
    case '1':
      currentMode = MODE_ON;
      Serial.println("[모드] 레이저 ON");
      break;
      
    case '2':
      currentMode = MODE_OFF;
      Serial.println("[모드] 레이저 OFF");
      break;
      
    case '3':
      currentMode = MODE_BLINK;
      Serial.println("[모드] 깜빡임 모드 (500ms 간격)");
      break;
      
    case '4':
      currentMode = MODE_SOS;
      sosIndex = 0;
      Serial.println("[모드] SOS 모드");
      break;
      
    case '5':
      currentMode = MODE_FADE;
      brightness = 0;
      fadeAmount = 5;
      Serial.println("[모드] Fade 모드 (PWM)");
      break;
      
    default:
      // 개행 문자 등 무시
      if (cmd != '\n' && cmd != '\r') {
        Serial.println("알 수 없는 명령입니다. 1-5 중 선택하세요.");
      }
      break;
  }
}

void blinkMode() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;
    laserState = !laserState;
    digitalWrite(LASER_PIN, laserState);
    digitalWrite(LED_PIN, laserState);
  }
}

void sosMode() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= dotDuration) {
    previousMillis = currentMillis;
    
    if (sosPattern[sosIndex]) {
      digitalWrite(LASER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LASER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }
    
    sosIndex++;
    if (sosIndex >= sosLength) {
      sosIndex = 0;
      delay(500);  // SOS 패턴 반복 전 딜레이
    }
  }
}

void fadeMode() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= 30) {
    previousMillis = currentMillis;
    
    // PWM으로 밝기 조절
    analogWrite(LASER_PIN, brightness);
    
    brightness += fadeAmount;
    
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    
    // 내장 LED도 동기화
    analogWrite(LED_PIN, brightness);
  }
}
