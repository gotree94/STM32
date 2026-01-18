/*
 * 릴레이 모듈 (Relay Module)
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * Framework: Arduino
 * 
 * 연결:
 *   VCC -> 5V (릴레이 코일 구동용)
 *   GND -> GND
 *   IN  -> D3 (PB3)
 * 
 * 주의: 릴레이 모듈은 대부분 5V 전원 필요
 *       HIGH/LOW 트리거 타입 확인 필요
 */

// 핀 정의
#define RELAY_PIN     3     // 릴레이 제어 핀
#define BUTTON_PIN    USER_BTN  // NUCLEO 보드의 사용자 버튼 (PC13)

// 릴레이 설정 (모듈에 따라 조정)
#define RELAY_ON      LOW   // LOW 트리거 (대부분의 모듈)
#define RELAY_OFF     HIGH

// 변수
bool relayState = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// 타이머 모드용 변수
bool timerMode = false;
unsigned long timerStartTime = 0;
unsigned long timerDuration = 5000;  // 5초

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    
    // 핀 모드 설정
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    
    // 릴레이 초기 상태: OFF
    digitalWrite(RELAY_PIN, RELAY_OFF);
    
    Serial.println("=================================");
    Serial.println("  Relay Module Test");
    Serial.println("  NUCLEO-F103RB + Arduino");
    Serial.println("=================================");
    Serial.println();
    Serial.println("Commands:");
    Serial.println("  '1' - Relay ON");
    Serial.println("  '0' - Relay OFF");
    Serial.println("  't' - Toggle Relay");
    Serial.println("  'T' - Timer Mode (5sec ON)");
    Serial.println("  'b' - Blink Mode (3 times)");
    Serial.println("  Blue Button - Toggle Relay");
    Serial.println();
}

void loop() {
    // 시리얼 명령 처리
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        processCommand(cmd);
    }
    
    // 버튼 입력 처리 (디바운싱)
    bool reading = digitalRead(BUTTON_PIN);
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading == LOW && lastButtonState == HIGH) {
            // 버튼 눌림 감지
            toggleRelay();
        }
    }
    lastButtonState = reading;
    
    // 타이머 모드 처리
    if (timerMode && relayState) {
        if (millis() - timerStartTime >= timerDuration) {
            setRelay(false);
            timerMode = false;
            Serial.println("Timer: Relay turned OFF automatically");
        }
    }
}

void processCommand(char cmd) {
    switch (cmd) {
        case '1':
            setRelay(true);
            Serial.println("Command: Relay ON");
            break;
            
        case '0':
            setRelay(false);
            Serial.println("Command: Relay OFF");
            break;
            
        case 't':
            toggleRelay();
            break;
            
        case 'T':
            // 타이머 모드: 5초 후 자동 OFF
            setRelay(true);
            timerMode = true;
            timerStartTime = millis();
            Serial.println("Timer Mode: Relay ON for 5 seconds");
            break;
            
        case 'b':
            // 블링크 모드: 3번 깜빡임
            blinkRelay(3, 500);
            break;
    }
}

void setRelay(bool state) {
    relayState = state;
    digitalWrite(RELAY_PIN, state ? RELAY_ON : RELAY_OFF);
    digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
    
    Serial.print("Relay State: ");
    Serial.println(state ? "ON" : "OFF");
}

void toggleRelay() {
    setRelay(!relayState);
    Serial.println("Relay Toggled");
}

void blinkRelay(int count, int delayMs) {
    Serial.print("Blink Mode: ");
    Serial.print(count);
    Serial.println(" times");
    
    for (int i = 0; i < count; i++) {
        setRelay(true);
        delay(delayMs);
        setRelay(false);
        delay(delayMs);
    }
    Serial.println("Blink Complete");
}
