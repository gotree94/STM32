/*
 * 미니 리드 스위치 모듈 테스트
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   - VCC: 3.3V 또는 5V
 *   - GND: GND
 *   - DO: PA0 (A0)
 */

#define REED_PIN PA0      // 리드 스위치 디지털 출력 핀
#define LED_PIN LED_BUILTIN  // 내장 LED (PA5)

bool lastState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // 시리얼 포트 연결 대기
  }
  
  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("================================");
  Serial.println("미니 리드 스위치 모듈 테스트");
  Serial.println("NUCLEO-F103RB");
  Serial.println("================================");
  Serial.println("자석을 가까이 가져가면 상태가 변합니다.");
  Serial.println();
}

void loop() {
  bool currentState = digitalRead(REED_PIN);
  
  // 디바운싱 처리
  if (currentState != lastState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentState == LOW) {
      // 자석 감지됨 (리드 스위치 ON)
      digitalWrite(LED_PIN, HIGH);
      Serial.println("[감지] 자석이 감지되었습니다! (스위치 ON)");
    } else {
      // 자석 없음 (리드 스위치 OFF)
      digitalWrite(LED_PIN, LOW);
      Serial.println("[대기] 자석이 감지되지 않습니다. (스위치 OFF)");
    }
  }
  
  lastState = currentState;
  delay(100);
}
