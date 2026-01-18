/*
 * ============================================================================
 * 소형 소리센서 모듈 테스트 코드 (Small Sound Sensor Module Test)
 * ============================================================================
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   소리센서 모듈    NUCLEO-F103RB
 *   -------------    -------------
 *   VCC          ->  3.3V
 *   GND          ->  GND
 *   OUT          ->  D3 (PB3)
 * 
 * 동작 설명:
 *   - 소형 소리센서는 디지털 출력만 제공
 *   - 소리 감지 시 OUT 핀 상태 변경
 *   - 대부분 소리 감지 시 LOW 출력 (Active LOW)
 * 
 * 센서 특징:
 *   - 소형 사이즈 (약 1.5cm x 1cm)
 *   - 디지털 출력만 지원
 *   - 간단한 ON/OFF 감지용
 * ============================================================================
 */

// 핀 정의
#define SOUND_PIN   PB3         // D3 - 소리센서 출력
#define LED_PIN     LED_BUILTIN // 내장 LED (PA5)

// 상수 정의
#define DEBOUNCE_TIME   50      // 디바운스 시간 (ms)
#define LED_ON_TIME     200     // LED 점등 시간 (ms)

// 변수
volatile bool soundTriggered = false;
unsigned long lastTriggerTime = 0;
unsigned long ledOffTime = 0;
bool ledState = false;

// 소리 감지 카운터
unsigned long soundCount = 0;
unsigned long lastCountResetTime = 0;

void setup() {
    // 시리얼 통신 초기화
    Serial.begin(115200);
    while (!Serial) {
        ; // 시리얼 포트 연결 대기
    }
    
    // 핀 설정
    pinMode(SOUND_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // 외부 인터럽트 설정 (FALLING edge - Active LOW 센서용)
    attachInterrupt(digitalPinToInterrupt(SOUND_PIN), soundISR, FALLING);
    
    Serial.println("================================================");
    Serial.println("  소형 소리센서 테스트 (Small Sound Sensor)");
    Serial.println("================================================");
    Serial.println("보드: NUCLEO-F103RB");
    Serial.println("센서 핀: PB3 (D3)");
    Serial.println("동작 방식: 인터럽트 기반");
    Serial.println("------------------------------------------------");
    Serial.println("소리를 내면 감지됩니다!");
    Serial.println("================================================");
    Serial.println();
    
    lastCountResetTime = millis();
    delay(1000);
}

void loop() {
    unsigned long currentTime = millis();
    
    // 소리 감지 처리
    if (soundTriggered) {
        soundTriggered = false;
        
        // 디바운스 처리
        if (currentTime - lastTriggerTime >= DEBOUNCE_TIME) {
            lastTriggerTime = currentTime;
            soundCount++;
            
            // LED 켜기
            digitalWrite(LED_PIN, HIGH);
            ledState = true;
            ledOffTime = currentTime + LED_ON_TIME;
            
            // 시리얼 출력
            Serial.print("[");
            Serial.print(currentTime / 1000.0, 2);
            Serial.print("s] 소리 감지! (");
            Serial.print(soundCount);
            Serial.println("회)");
        }
    }
    
    // LED 자동 OFF
    if (ledState && currentTime >= ledOffTime) {
        digitalWrite(LED_PIN, LOW);
        ledState = false;
    }
    
    // 폴링 방식으로도 상태 확인 (인터럽트 백업)
    static unsigned long lastPollTime = 0;
    if (currentTime - lastPollTime >= 100) {
        lastPollTime = currentTime;
        
        int sensorState = digitalRead(SOUND_PIN);
        
        // 상태 표시 (1초마다)
        static unsigned long lastStatusTime = 0;
        if (currentTime - lastStatusTime >= 1000) {
            lastStatusTime = currentTime;
            Serial.print("센서 상태: ");
            Serial.print(sensorState ? "HIGH (대기)" : "LOW (감지)");
            Serial.print(" | 총 감지: ");
            Serial.print(soundCount);
            Serial.println("회");
        }
    }
    
    // 1분마다 감지 횟수 초기화 및 통계 출력
    if (currentTime - lastCountResetTime >= 60000) {
        Serial.println("------------------------------------------------");
        Serial.print("최근 1분간 소리 감지 횟수: ");
        Serial.print(soundCount);
        Serial.println("회");
        Serial.println("------------------------------------------------");
        
        // 카운터 리셋 (필요시 주석 해제)
        // soundCount = 0;
        lastCountResetTime = currentTime;
    }
}

// 인터럽트 서비스 루틴
void soundISR() {
    soundTriggered = true;
}

// 감지 횟수 초기화 함수
void resetSoundCount() {
    soundCount = 0;
    Serial.println("감지 횟수가 초기화되었습니다.");
}

// 현재 감지 횟수 반환
unsigned long getSoundCount() {
    return soundCount;
}

// 특정 시간 동안 소리 감지 여부 확인
bool detectSoundWithTimeout(unsigned long timeoutMs) {
    unsigned long startTime = millis();
    unsigned long startCount = soundCount;
    
    while (millis() - startTime < timeoutMs) {
        if (soundCount > startCount) {
            return true;
        }
        delay(10);
    }
    return false;
}

// 연속 소리 패턴 감지 (예: 박수 n회)
bool detectClapPattern(int clapCount, unsigned long intervalMs) {
    unsigned long startTime = millis();
    unsigned long startCount = soundCount;
    int detectedClaps = 0;
    
    while (millis() - startTime < (intervalMs * clapCount * 2)) {
        if (soundCount > startCount + detectedClaps) {
            detectedClaps++;
            if (detectedClaps >= clapCount) {
                return true;
            }
        }
        delay(10);
    }
    return false;
}
