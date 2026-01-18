/*
 * 버튼 스위치 모듈 테스트 (Push Button Module)
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   - VCC: 3.3V 또는 5V
 *   - GND: GND
 *   - Signal: PA0
 */

#define BUTTON_PIN PA0       // 버튼 입력 핀
#define LED_PIN LED_BUILTIN  // 내장 LED (PA5)

// 버튼 상태 변수
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// 버튼 이벤트 변수
bool buttonPressed = false;
bool buttonReleased = false;
bool buttonHeld = false;

// 롱프레스 감지
unsigned long buttonPressTime = 0;
const unsigned long longPressTime = 1000;  // 1초 이상 누르면 롱프레스
bool longPressDetected = false;

// 더블클릭 감지
unsigned long lastClickTime = 0;
const unsigned long doubleClickTime = 300;  // 300ms 이내 두 번 클릭
int clickCount = 0;

// LED 상태 (토글용)
bool ledState = false;

// 통계
unsigned long pressCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // 내부 풀업 저항 사용
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("================================");
  Serial.println("버튼 스위치 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println();
  Serial.println("버튼 동작:");
  Serial.println("  - 클릭: LED 토글");
  Serial.println("  - 더블클릭: LED 빠른 깜빡임 3회");
  Serial.println("  - 롱프레스(1초): LED 천천히 깜빡임");
  Serial.println();
}

void loop() {
  // 버튼 상태 읽기 및 디바운싱
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // 상태 변화 감지
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == LOW) {
        // 버튼 눌림
        buttonPressed = true;
        buttonPressTime = millis();
        longPressDetected = false;
        Serial.println("[이벤트] 버튼 눌림");
      } else {
        // 버튼 뗌
        buttonReleased = true;
        unsigned long pressDuration = millis() - buttonPressTime;
        
        Serial.print("[이벤트] 버튼 뗌 (");
        Serial.print(pressDuration);
        Serial.println("ms)");
        
        // 롱프레스가 아닌 경우에만 클릭 처리
        if (!longPressDetected) {
          processClick();
        }
      }
    }
  }
  
  // 롱프레스 감지 (버튼이 눌려있는 동안)
  if (buttonState == LOW && !longPressDetected) {
    if ((millis() - buttonPressTime) > longPressTime) {
      longPressDetected = true;
      Serial.println("[이벤트] 롱프레스 감지!");
      longPressAction();
    }
  }
  
  // 더블클릭 타임아웃 처리
  if (clickCount == 1 && (millis() - lastClickTime) > doubleClickTime) {
    // 싱글 클릭 확정
    clickCount = 0;
    singleClickAction();
  }
  
  lastButtonState = reading;
}

void processClick() {
  unsigned long currentTime = millis();
  
  if ((currentTime - lastClickTime) < doubleClickTime) {
    // 더블클릭
    clickCount = 0;
    Serial.println("[이벤트] 더블클릭!");
    doubleClickAction();
  } else {
    // 첫 번째 클릭 또는 싱글 클릭 대기
    clickCount = 1;
    lastClickTime = currentTime;
    pressCount++;
    Serial.print("[통계] 총 클릭 횟수: ");
    Serial.println(pressCount);
  }
}

void singleClickAction() {
  // 싱글 클릭: LED 토글
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);
  Serial.print("[동작] LED ");
  Serial.println(ledState ? "ON" : "OFF");
}

void doubleClickAction() {
  // 더블클릭: LED 빠른 깜빡임 3회
  Serial.println("[동작] LED 빠른 깜빡임");
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  digitalWrite(LED_PIN, ledState);  // 원래 상태로 복귀
}

void longPressAction() {
  // 롱프레스: LED 천천히 깜빡임
  Serial.println("[동작] LED 천천히 깜빡임");
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
  digitalWrite(LED_PIN, ledState);  // 원래 상태로 복귀
}
