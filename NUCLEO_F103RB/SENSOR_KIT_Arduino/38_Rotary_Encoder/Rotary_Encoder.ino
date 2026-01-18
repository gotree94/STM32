/*
 * 로터리 엔코더 모듈 (Rotary Encoder Module - KY-040)
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * Framework: Arduino
 * 
 * 연결:
 *   VCC -> 3.3V 또는 5V
 *   GND -> GND
 *   CLK -> D6 (PB10)
 *   DT  -> D7 (PA8)
 *   SW  -> D8 (PA9)
 */

// 핀 정의
#define ENCODER_CLK   6     // 클럭 핀 (A상)
#define ENCODER_DT    7     // 데이터 핀 (B상)
#define ENCODER_SW    8     // 스위치 핀

// 변수
volatile int encoderPosition = 0;
volatile int lastEncoderPosition = 0;
int lastCLKState;
int currentCLKState;

// 버튼 디바운싱
bool buttonPressed = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// 설정값 (예시: 볼륨, 밝기 등)
int settingValue = 50;
const int MIN_VALUE = 0;
const int MAX_VALUE = 100;
const int STEP_VALUE = 5;

// 모드 관리
enum Mode { MODE_COUNTER, MODE_VOLUME, MODE_BRIGHTNESS };
Mode currentMode = MODE_COUNTER;
const char* modeNames[] = {"Counter", "Volume", "Brightness"};

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    
    // 핀 모드 설정
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    
    // 초기 상태 읽기
    lastCLKState = digitalRead(ENCODER_CLK);
    
    Serial.println("=================================");
    Serial.println("  Rotary Encoder Module Test");
    Serial.println("  NUCLEO-F103RB + Arduino");
    Serial.println("=================================");
    Serial.println();
    Serial.println("Operations:");
    Serial.println("  Rotate CW  - Increment");
    Serial.println("  Rotate CCW - Decrement");
    Serial.println("  Press SW   - Change Mode / Reset");
    Serial.println();
    Serial.println("Modes: Counter, Volume, Brightness");
    Serial.println();
    printCurrentState();
}

void loop() {
    // 엔코더 회전 감지
    readEncoder();
    
    // 버튼 처리
    handleButton();
    
    // 위치 변경 감지 및 출력
    if (encoderPosition != lastEncoderPosition) {
        handleEncoderChange();
        lastEncoderPosition = encoderPosition;
    }
    
    // 시리얼 명령 처리
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        processSerialCommand(cmd);
    }
}

void readEncoder() {
    // CLK 핀 상태 읽기
    currentCLKState = digitalRead(ENCODER_CLK);
    
    // CLK 상태가 변경되었고, LOW가 아닐 때 (노이즈 필터링)
    if (currentCLKState != lastCLKState && currentCLKState == LOW) {
        // DT 핀 상태로 방향 판별
        if (digitalRead(ENCODER_DT) != currentCLKState) {
            // 시계방향 (CW)
            encoderPosition++;
        } else {
            // 반시계방향 (CCW)
            encoderPosition--;
        }
    }
    
    lastCLKState = currentCLKState;
}

void handleButton() {
    bool reading = digitalRead(ENCODER_SW);
    
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading == LOW && !buttonPressed) {
            buttonPressed = true;
            onButtonPress();
        } else if (reading == HIGH) {
            buttonPressed = false;
        }
    }
    
    lastButtonState = reading;
}

void onButtonPress() {
    Serial.println("\n>> Button Pressed!");
    
    // 모드 전환
    currentMode = (Mode)((currentMode + 1) % 3);
    
    // 모드 변경 시 초기화
    encoderPosition = 0;
    lastEncoderPosition = 0;
    
    if (currentMode == MODE_VOLUME || currentMode == MODE_BRIGHTNESS) {
        settingValue = 50;  // 중간값으로 초기화
    }
    
    Serial.print("Mode Changed: ");
    Serial.println(modeNames[currentMode]);
    printCurrentState();
    
    // LED 피드백
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
}

void handleEncoderChange() {
    int direction = (encoderPosition > lastEncoderPosition) ? 1 : -1;
    
    switch (currentMode) {
        case MODE_COUNTER:
            // 카운터 모드: 단순 위치 표시
            Serial.print("Counter: ");
            Serial.print(encoderPosition);
            Serial.println(direction > 0 ? " (+)" : " (-)");
            break;
            
        case MODE_VOLUME:
        case MODE_BRIGHTNESS:
            // 설정값 모드
            settingValue += direction * STEP_VALUE;
            settingValue = constrain(settingValue, MIN_VALUE, MAX_VALUE);
            
            Serial.print(modeNames[currentMode]);
            Serial.print(": ");
            Serial.print(settingValue);
            Serial.print("% ");
            printBar(settingValue);
            break;
    }
}

void printBar(int value) {
    // 그래픽 바 표시
    int barLength = map(value, MIN_VALUE, MAX_VALUE, 0, 20);
    Serial.print("[");
    for (int i = 0; i < 20; i++) {
        if (i < barLength) Serial.print("=");
        else Serial.print(" ");
    }
    Serial.println("]");
}

void printCurrentState() {
    Serial.println("----------------------------");
    Serial.print("Current Mode: ");
    Serial.println(modeNames[currentMode]);
    
    switch (currentMode) {
        case MODE_COUNTER:
            Serial.print("Position: ");
            Serial.println(encoderPosition);
            break;
            
        case MODE_VOLUME:
        case MODE_BRIGHTNESS:
            Serial.print("Value: ");
            Serial.print(settingValue);
            Serial.print("% ");
            printBar(settingValue);
            break;
    }
    Serial.println("----------------------------");
}

void processSerialCommand(char cmd) {
    switch (cmd) {
        case 'r':
        case 'R':
            // 리셋
            encoderPosition = 0;
            lastEncoderPosition = 0;
            settingValue = 50;
            Serial.println("\n>> Reset!");
            printCurrentState();
            break;
            
        case 'm':
        case 'M':
            // 모드 전환
            onButtonPress();
            break;
            
        case '?':
            // 현재 상태 출력
            printCurrentState();
            break;
    }
}
