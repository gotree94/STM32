/*
 * 수동 부저 모듈 (Passive Buzzer Module)
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * Framework: Arduino
 * 
 * 수동 부저: 외부에서 주파수 신호를 제공해야 함
 *           tone() 함수로 다양한 음계 재생 가능
 * 
 * 연결:
 *   VCC -> 3.3V 또는 5V (필요시)
 *   GND -> GND
 *   I/O -> D5 (PB4) - PWM 가능 핀 사용 권장
 */

// 핀 정의
#define BUZZER_PIN    5     // 부저 제어 핀 (PWM)
#define BUTTON_PIN    USER_BTN  // NUCLEO 보드 사용자 버튼

// 음계 주파수 정의 (Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047

#define NOTE_REST 0  // 쉼표

// 음표 길이 (ms)
#define WHOLE     1000
#define HALF      500
#define QUARTER   250
#define EIGHTH    125
#define SIXTEENTH 62

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    
    // 핀 모드 설정
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.println("=================================");
    Serial.println("  Passive Buzzer Module Test");
    Serial.println("  NUCLEO-F103RB + Arduino");
    Serial.println("=================================");
    Serial.println();
    Serial.println("Commands:");
    Serial.println("  '1'-'8' - Play note (C4-C5)");
    Serial.println("  's' - Scale (도레미파솔라시도)");
    Serial.println("  'm' - Play Melody");
    Serial.println("  'h' - Happy Birthday");
    Serial.println("  'a' - Alarm Sound");
    Serial.println("  'p' - Police Siren");
    Serial.println("  'w' - Ambulance Siren");
    Serial.println("  '0' - Stop");
    Serial.println();
    
    // 시작 알림음
    playStartupSound();
}

void loop() {
    // 시리얼 명령 처리
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        processCommand(cmd);
    }
    
    // 버튼으로 스케일 재생
    if (digitalRead(BUTTON_PIN) == LOW) {
        playScale();
        while (digitalRead(BUTTON_PIN) == LOW) { ; }
    }
}

void processCommand(char cmd) {
    switch (cmd) {
        case '1': playNote(NOTE_C4, QUARTER); break;
        case '2': playNote(NOTE_D4, QUARTER); break;
        case '3': playNote(NOTE_E4, QUARTER); break;
        case '4': playNote(NOTE_F4, QUARTER); break;
        case '5': playNote(NOTE_G4, QUARTER); break;
        case '6': playNote(NOTE_A4, QUARTER); break;
        case '7': playNote(NOTE_B4, QUARTER); break;
        case '8': playNote(NOTE_C5, QUARTER); break;
        
        case 's':
            Serial.println("Playing Scale...");
            playScale();
            break;
            
        case 'm':
            Serial.println("Playing Melody...");
            playMelody();
            break;
            
        case 'h':
            Serial.println("Playing Happy Birthday...");
            playHappyBirthday();
            break;
            
        case 'a':
            Serial.println("Playing Alarm...");
            playAlarm();
            break;
            
        case 'p':
            Serial.println("Playing Police Siren...");
            playPoliceSiren();
            break;
            
        case 'w':
            Serial.println("Playing Ambulance Siren...");
            playAmbulanceSiren();
            break;
            
        case '0':
            noTone(BUZZER_PIN);
            Serial.println("Sound Stopped");
            break;
    }
}

void playNote(int frequency, int duration) {
    if (frequency == NOTE_REST) {
        noTone(BUZZER_PIN);
        delay(duration);
    } else {
        tone(BUZZER_PIN, frequency, duration);
        delay(duration * 1.1);  // 음 사이 간격
    }
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void playStartupSound() {
    playNote(NOTE_C5, EIGHTH);
    playNote(NOTE_E5, EIGHTH);
    playNote(NOTE_G5, QUARTER);
}

void playScale() {
    int notes[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, 
                   NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
    
    // 올라가기
    for (int i = 0; i < 8; i++) {
        playNote(notes[i], QUARTER);
    }
    delay(200);
    
    // 내려가기
    for (int i = 7; i >= 0; i--) {
        playNote(notes[i], QUARTER);
    }
}

void playMelody() {
    // 간단한 멜로디 (작은 별)
    int melody[] = {
        NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4,
        NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4
    };
    int durations[] = {
        QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF,
        QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF
    };
    
    int len = sizeof(melody) / sizeof(melody[0]);
    for (int i = 0; i < len; i++) {
        playNote(melody[i], durations[i]);
    }
}

void playHappyBirthday() {
    int melody[] = {
        NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4,
        NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_G4, NOTE_F4,
        NOTE_C4, NOTE_C4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4,
        NOTE_B4, NOTE_B4, NOTE_A4, NOTE_F4, NOTE_G4, NOTE_F4
    };
    int durations[] = {
        EIGHTH, EIGHTH, QUARTER, QUARTER, QUARTER, HALF,
        EIGHTH, EIGHTH, QUARTER, QUARTER, QUARTER, HALF,
        EIGHTH, EIGHTH, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER,
        EIGHTH, EIGHTH, QUARTER, QUARTER, QUARTER, HALF
    };
    
    int len = sizeof(melody) / sizeof(melody[0]);
    for (int i = 0; i < len; i++) {
        playNote(melody[i], durations[i]);
    }
}

void playAlarm() {
    for (int i = 0; i < 3; i++) {
        for (int freq = 800; freq < 1200; freq += 50) {
            tone(BUZZER_PIN, freq, 30);
            delay(30);
        }
        for (int freq = 1200; freq > 800; freq -= 50) {
            tone(BUZZER_PIN, freq, 30);
            delay(30);
        }
    }
    noTone(BUZZER_PIN);
}

void playPoliceSiren() {
    for (int i = 0; i < 3; i++) {
        // 높은 음 -> 낮은 음
        for (int freq = 1000; freq <= 1500; freq += 25) {
            tone(BUZZER_PIN, freq, 20);
            delay(20);
        }
        for (int freq = 1500; freq >= 1000; freq -= 25) {
            tone(BUZZER_PIN, freq, 20);
            delay(20);
        }
    }
    noTone(BUZZER_PIN);
}

void playAmbulanceSiren() {
    for (int i = 0; i < 4; i++) {
        tone(BUZZER_PIN, 800, 400);
        delay(400);
        tone(BUZZER_PIN, 600, 400);
        delay(400);
    }
    noTone(BUZZER_PIN);
}
