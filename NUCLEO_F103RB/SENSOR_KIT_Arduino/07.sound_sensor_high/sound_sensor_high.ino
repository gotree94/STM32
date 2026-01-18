/*
 * ============================================================================
 * 고감도 소리센서 모듈 테스트 코드 (High Sensitivity Sound Sensor Test)
 * ============================================================================
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   소리센서 모듈    NUCLEO-F103RB
 *   -------------    -------------
 *   VCC          ->  3.3V 또는 5V
 *   GND          ->  GND
 *   AO (아날로그) ->  A0 (PA0)
 *   DO (디지털)   ->  D2 (PA10)
 * 
 * 동작 설명:
 *   - 아날로그 출력: 소리 크기에 비례한 전압값
 *   - 디지털 출력: 설정 임계값 초과 시 HIGH/LOW 전환
 *   - 가변저항으로 감도 조절 가능
 * 
 * 센서 특징:
 *   - LM393 비교기 사용
 *   - 고감도 마이크로폰 탑재
 *   - 감도 조절용 가변저항 포함
 * ============================================================================
 */

// 핀 정의
#define SOUND_ANALOG_PIN   PA0    // A0 - 아날로그 출력
#define SOUND_DIGITAL_PIN  PA10   // D2 - 디지털 출력
#define LED_PIN            LED_BUILTIN  // 내장 LED (PA5)

// 상수 정의
#define SAMPLE_COUNT       50     // 샘플링 횟수
#define SOUND_THRESHOLD    100    // 소리 감지 임계값 (소프트웨어)
#define NOISE_FLOOR        30     // 노이즈 바닥값

// 변수
int soundAnalogValue = 0;
int soundDigitalValue = 0;
int soundPeak = 0;
int soundMin = 4095;
unsigned long lastPrintTime = 0;
unsigned long soundDetectedTime = 0;
bool soundDetected = false;

void setup() {
    // 시리얼 통신 초기화
    Serial.begin(115200);
    while (!Serial) {
        ; // 시리얼 포트 연결 대기
    }
    
    // 핀 설정
    pinMode(SOUND_ANALOG_PIN, INPUT);
    pinMode(SOUND_DIGITAL_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    // ADC 해상도 설정 (12비트: 0-4095)
    analogReadResolution(12);
    
    Serial.println("================================================");
    Serial.println("  고감도 소리센서 테스트 (High Sensitivity)");
    Serial.println("================================================");
    Serial.println("보드: NUCLEO-F103RB");
    Serial.println("아날로그 핀: PA0 (A0)");
    Serial.println("디지털 핀: PA10 (D2)");
    Serial.println("ADC 해상도: 12비트 (0-4095)");
    Serial.println("------------------------------------------------");
    Serial.println("Tip: 모듈의 가변저항으로 감도를 조절하세요.");
    Serial.println("================================================");
    Serial.println();
    
    delay(1000);
}

void loop() {
    // 아날로그 값 읽기 (다중 샘플링으로 평균값 계산)
    long sumValue = 0;
    int maxValue = 0;
    int minValue = 4095;
    
    // 빠른 샘플링으로 피크값 감지
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        int sample = analogRead(SOUND_ANALOG_PIN);
        sumValue += sample;
        if (sample > maxValue) maxValue = sample;
        if (sample < minValue) minValue = sample;
        delayMicroseconds(100);  // 100us 간격 샘플링
    }
    
    soundAnalogValue = sumValue / SAMPLE_COUNT;
    int soundAmplitude = maxValue - minValue;  // 진폭 계산
    
    // 디지털 값 읽기
    soundDigitalValue = digitalRead(SOUND_DIGITAL_PIN);
    
    // 피크값 업데이트
    if (soundAmplitude > soundPeak) {
        soundPeak = soundAmplitude;
    }
    
    // 소리 감지 판단 (진폭 기반)
    if (soundAmplitude > SOUND_THRESHOLD) {
        soundDetected = true;
        soundDetectedTime = millis();
        digitalWrite(LED_PIN, HIGH);
    }
    
    // LED 자동 OFF (500ms 후)
    if (soundDetected && (millis() - soundDetectedTime > 500)) {
        soundDetected = false;
        digitalWrite(LED_PIN, LOW);
    }
    
    // 시리얼 출력 (200ms 간격)
    if (millis() - lastPrintTime >= 200) {
        lastPrintTime = millis();
        
        // 아날로그 값 출력
        Serial.print("아날로그: ");
        Serial.print(soundAnalogValue);
        Serial.print(" | 진폭: ");
        Serial.print(soundAmplitude);
        Serial.print(" | 피크: ");
        Serial.print(soundPeak);
        
        // 디지털 값 출력
        Serial.print(" | 디지털: ");
        Serial.print(soundDigitalValue ? "HIGH" : "LOW ");
        
        // 소리 감지 상태
        Serial.print(" | ");
        if (soundAmplitude > SOUND_THRESHOLD) {
            Serial.print(">>> 소리 감지! <<<");
        }
        
        // 그래프 바 출력
        Serial.print(" [");
        int barLength = map(soundAmplitude, 0, 500, 0, 20);
        barLength = constrain(barLength, 0, 20);
        for (int i = 0; i < 20; i++) {
            if (i < barLength) {
                Serial.print("#");
            } else {
                Serial.print("-");
            }
        }
        Serial.println("]");
    }
}

// 소리 레벨을 dB 근사값으로 변환 (참고용)
float convertToDecibels(int analogValue) {
    if (analogValue <= NOISE_FLOOR) {
        return 0.0;
    }
    // 간단한 로그 스케일 변환 (실제 dB 측정이 아닌 근사값)
    float voltage = (analogValue / 4095.0) * 3.3;
    float dB = 20.0 * log10(voltage / 0.001);  // 1mV 기준
    return dB;
}

// 피크값 리셋 함수
void resetPeak() {
    soundPeak = 0;
}

// 특정 주파수 범위의 소리 감지 (간단한 구현)
bool detectSoundInRange(int minAmplitude, int maxAmplitude) {
    long sumValue = 0;
    int maxValue = 0;
    int minValue = 4095;
    
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        int sample = analogRead(SOUND_ANALOG_PIN);
        if (sample > maxValue) maxValue = sample;
        if (sample < minValue) minValue = sample;
        delayMicroseconds(100);
    }
    
    int amplitude = maxValue - minValue;
    return (amplitude >= minAmplitude && amplitude <= maxAmplitude);
}
