/*
 * 조이스틱 모듈 (Joystick Module)
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * Framework: Arduino
 * 
 * 연결:
 *   VCC -> 3.3V 또는 5V
 *   GND -> GND
 *   VRx -> A0 (PA0)
 *   VRy -> A1 (PA1)
 *   SW  -> D2 (PA10)
 */

// 핀 정의
#define JOYSTICK_X    A0    // X축 아날로그 입력
#define JOYSTICK_Y    A1    // Y축 아날로그 입력
#define JOYSTICK_SW   2     // 스위치 (버튼)

// 조이스틱 중앙값 (캘리브레이션 필요할 수 있음)
#define CENTER_X      512
#define CENTER_Y      512
#define DEADZONE      50    // 데드존 설정

// 변수
int xValue = 0;
int yValue = 0;
bool buttonPressed = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    
    // 핀 모드 설정
    pinMode(JOYSTICK_X, INPUT);
    pinMode(JOYSTICK_Y, INPUT);
    pinMode(JOYSTICK_SW, INPUT_PULLUP);  // 내부 풀업 사용
    
    Serial.println("=================================");
    Serial.println("  Joystick Module Test");
    Serial.println("  NUCLEO-F103RB + Arduino");
    Serial.println("=================================");
    Serial.println();
}

void loop() {
    // 아날로그 값 읽기
    xValue = analogRead(JOYSTICK_X);
    yValue = analogRead(JOYSTICK_Y);
    
    // 버튼 디바운싱 처리
    bool reading = digitalRead(JOYSTICK_SW);
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading == LOW && !buttonPressed) {
            buttonPressed = true;
            Serial.println(">> Button PRESSED!");
        } else if (reading == HIGH) {
            buttonPressed = false;
        }
    }
    lastButtonState = reading;
    
    // 방향 판별
    String direction = getDirection(xValue, yValue);
    
    // 시리얼 출력
    Serial.print("X: ");
    Serial.print(xValue);
    Serial.print("\tY: ");
    Serial.print(yValue);
    Serial.print("\tDirection: ");
    Serial.print(direction);
    Serial.print("\tButton: ");
    Serial.println(buttonPressed ? "PRESSED" : "Released");
    
    delay(100);
}

String getDirection(int x, int y) {
    // 중앙 기준으로 방향 계산
    int dx = x - CENTER_X;
    int dy = y - CENTER_Y;
    
    // 데드존 체크
    if (abs(dx) < DEADZONE && abs(dy) < DEADZONE) {
        return "CENTER";
    }
    
    // 8방향 판별
    if (abs(dx) > abs(dy)) {
        // 좌우가 더 강함
        if (dx > 0) {
            if (dy > DEADZONE) return "RIGHT-DOWN";
            else if (dy < -DEADZONE) return "RIGHT-UP";
            else return "RIGHT";
        } else {
            if (dy > DEADZONE) return "LEFT-DOWN";
            else if (dy < -DEADZONE) return "LEFT-UP";
            else return "LEFT";
        }
    } else {
        // 상하가 더 강함
        if (dy > 0) {
            if (dx > DEADZONE) return "DOWN-RIGHT";
            else if (dx < -DEADZONE) return "DOWN-LEFT";
            else return "DOWN";
        } else {
            if (dx > DEADZONE) return "UP-RIGHT";
            else if (dx < -DEADZONE) return "UP-LEFT";
            else return "UP";
        }
    }
}
