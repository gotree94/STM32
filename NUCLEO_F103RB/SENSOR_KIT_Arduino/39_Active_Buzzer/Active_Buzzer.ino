/*
 * 능동 부저 모듈 (Active Buzzer Module)
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * Framework: Arduino
 * 
 * 능동 부저: 내부 발진회로 내장, HIGH/LOW로 ON/OFF 제어
 * 
 * 연결:
 *   VCC -> 3.3V 또는 5V
 *   GND -> GND
 *   I/O -> D4 (PB5)
 */

// 핀 정의
#define BUZZER_PIN    4     // 부저 제어 핀
#define BUTTON_PIN    USER_BTN  // NUCLEO 보드 사용자 버튼

// 부저 설정
#define BUZZER_ON     HIGH
#define BUZZER_OFF    LOW

// 변수
bool buzzerState = false;

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    
    // 핀 모드 설정
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    
    // 부저 초기 상태: OFF
    digitalWrite(BUZZER_PIN, BUZZER_OFF);
    
    Serial.println("=================================");
    Serial.println("  Active Buzzer Module Test");
    Serial.println("  NUCLEO-F103RB + Arduino");
    Serial.println("=================================");
    Serial.println();
    Serial.println("Commands:");
    Serial.println("  '1' - Buzzer ON");
    Serial.println("  '0' - Buzzer OFF");
    Serial.println("  'b' - Short Beep");
    Serial.println("  'B' - Long Beep");
    Serial.println("  'a' - Alarm Pattern");
    Serial.println("  's' - SOS Pattern");
    Serial.println("  'm' - Melody Pattern");
    Serial.println();
    
    // 시작 알림음
    beep(100);
    delay(100);
    beep(100);
}

void loop() {
    // 시리얼 명령 처리
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        processCommand(cmd);
    }
    
    // 버튼으로 비프음
    if (digitalRead(BUTTON_PIN) == LOW) {
        beep(200);
        while (digitalRead(BUTTON_PIN) == LOW) { ; }  // 버튼 릴리즈 대기
    }
}

void processCommand(char cmd) {
    switch (cmd) {
        case '1':
            setBuzzer(true);
            Serial.println("Buzzer ON");
            break;
            
        case '0':
            setBuzzer(false);
            Serial.println("Buzzer OFF");
            break;
            
        case 'b':
            beep(100);
            Serial.println("Short Beep");
            break;
            
        case 'B':
            beep(500);
            Serial.println("Long Beep");
            break;
            
        case 'a':
            alarmPattern();
            break;
            
        case 's':
            sosPattern();
            break;
            
        case 'm':
            melodyPattern();
            break;
    }
}

void setBuzzer(bool state) {
    buzzerState = state;
    digitalWrite(BUZZER_PIN, state ? BUZZER_ON : BUZZER_OFF);
    digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
}

void beep(int duration) {
    setBuzzer(true);
    delay(duration);
    setBuzzer(false);
}

void alarmPattern() {
    Serial.println("Playing Alarm Pattern...");
    for (int i = 0; i < 5; i++) {
        beep(100);
        delay(100);
        beep(100);
        delay(300);
    }
    Serial.println("Alarm Complete");
}

void sosPattern() {
    Serial.println("Playing SOS Pattern...");
    
    // S: 짧은 비프 3번
    for (int i = 0; i < 3; i++) {
        beep(100);
        delay(100);
    }
    delay(200);
    
    // O: 긴 비프 3번
    for (int i = 0; i < 3; i++) {
        beep(300);
        delay(100);
    }
    delay(200);
    
    // S: 짧은 비프 3번
    for (int i = 0; i < 3; i++) {
        beep(100);
        delay(100);
    }
    
    Serial.println("SOS Complete");
}

void melodyPattern() {
    Serial.println("Playing Melody Pattern...");
    
    // 리듬감 있는 패턴
    int pattern[] = {100, 50, 100, 50, 200, 100, 100, 50, 300};
    int delays[]  = {100, 50, 100, 50, 100, 100, 100, 50, 200};
    int len = sizeof(pattern) / sizeof(pattern[0]);
    
    for (int i = 0; i < len; i++) {
        beep(pattern[i]);
        delay(delays[i]);
    }
    
    Serial.println("Melody Complete");
}
