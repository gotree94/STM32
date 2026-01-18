/**
 ******************************************************************************
 * @file    02_IR_Receiver.ino
 * @brief   IR Receiver Module for NUCLEO-F103RB (Arduino)
 * @author  Embedded Systems Lab
 * @version V1.0.0
 * @date    2025-01-18
 ******************************************************************************
 * @details
 * IR 수신 모듈(TSOP1838, VS1838B 등)을 이용하여 IR 신호를 감지합니다.
 * 수신된 신호의 펄스 폭과 개수를 측정하여 시리얼로 출력합니다.
 * 
 * Hardware Setup:
 * - IR Receiver: PA0 (A0) - Signal input
 * - User LED: PA5 (D13, 내장 LED)
 * - Serial: USB Virtual COM Port (115200 baud)
 * 
 * 회로 연결:
 *   IR Receiver Module
 *   ┌─────────┐
 *   │  OUT ───┼─── PA0 (A0)
 *   │  GND ───┼─── GND
 *   │  VCC ───┼─── 3.3V
 *   └─────────┘
 ******************************************************************************
 */

/* Pin Definitions */
#define IR_RX_PIN       PA0     // IR Receiver signal
#define USER_LED_PIN    PA5     // Onboard LED

/* Timing Constants */
#define TIMEOUT_US      100000  // 100ms timeout
#define MIN_PULSE_US    200     // 최소 유효 펄스 폭
#define MAX_PULSES      100     // 최대 펄스 저장 개수

/* Variables */
volatile uint32_t pulseBuffer[MAX_PULSES];  // 펄스 폭 저장
volatile uint16_t pulseCount = 0;           // 펄스 개수
volatile bool signalReceived = false;       // 신호 수신 플래그
volatile uint32_t lastEdgeTime = 0;         // 마지막 엣지 시간

/* Statistics */
uint32_t totalSignals = 0;
uint32_t minPulse = UINT32_MAX;
uint32_t maxPulse = 0;

/**
 * @brief Setup function
 */
void setup() {
    // Serial 초기화
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("  IR Receiver - NUCLEO-F103RB");
    Serial.println("  Signal Detection & Pulse Measurement");
    Serial.println("========================================");
    Serial.println();
    
    // GPIO 초기화
    pinMode(USER_LED_PIN, OUTPUT);
    pinMode(IR_RX_PIN, INPUT);
    
    digitalWrite(USER_LED_PIN, LOW);
    
    // 인터럽트 설정 (양쪽 엣지)
    attachInterrupt(digitalPinToInterrupt(IR_RX_PIN), irISR, CHANGE);
    
    Serial.println("Hardware: TSOP1838, VS1838B, or compatible");
    Serial.println("Input Pin: PA0 (A0)");
    Serial.println();
    Serial.println("Waiting for IR signal...");
    Serial.println();
}

/**
 * @brief Main loop
 */
void loop() {
    static uint32_t lastActivityTime = 0;
    
    // 신호 수신 완료 확인 (마지막 엣지 후 50ms 경과)
    if (pulseCount > 0 && !signalReceived) {
        if (micros() - lastEdgeTime > 50000) {
            signalReceived = true;
        }
    }
    
    // 수신된 신호 처리
    if (signalReceived) {
        processSignal();
        signalReceived = false;
        pulseCount = 0;
    }
    
    // 상태 표시 (5초마다)
    if (millis() - lastActivityTime > 5000) {
        lastActivityTime = millis();
        if (totalSignals > 0) {
            Serial.print("[STATUS] Total signals: ");
            Serial.print(totalSignals);
            Serial.print(", Min pulse: ");
            Serial.print(minPulse);
            Serial.print("us, Max pulse: ");
            Serial.print(maxPulse);
            Serial.println("us");
        } else {
            Serial.println("[STATUS] Waiting for IR signal...");
        }
    }
}

/**
 * @brief IR 수신 인터럽트 핸들러
 */
void irISR() {
    uint32_t currentTime = micros();
    uint32_t pulseDuration;
    
    if (lastEdgeTime > 0) {
        pulseDuration = currentTime - lastEdgeTime;
        
        // 유효한 펄스만 저장
        if (pulseDuration >= MIN_PULSE_US && pulseDuration < TIMEOUT_US) {
            if (pulseCount < MAX_PULSES) {
                pulseBuffer[pulseCount++] = pulseDuration;
            }
        }
    }
    
    lastEdgeTime = currentTime;
    
    // LED 토글 (신호 감지 표시)
    digitalWrite(USER_LED_PIN, !digitalRead(IR_RX_PIN));
}

/**
 * @brief 수신된 신호 분석 및 출력
 */
void processSignal() {
    if (pulseCount < 4) {
        return;  // 너무 짧은 신호는 무시
    }
    
    totalSignals++;
    
    Serial.println("========================================");
    Serial.print("[RX] Signal #");
    Serial.print(totalSignals);
    Serial.print(" - Pulses: ");
    Serial.println(pulseCount);
    Serial.println("========================================");
    
    // 펄스 데이터 출력
    Serial.println("Pulse timings (us):");
    
    uint32_t sum = 0;
    uint32_t localMin = UINT32_MAX;
    uint32_t localMax = 0;
    
    for (int i = 0; i < pulseCount; i++) {
        // 10개씩 한 줄에 출력
        Serial.print(pulseBuffer[i]);
        Serial.print("\t");
        
        if ((i + 1) % 10 == 0) {
            Serial.println();
        }
        
        // 통계 계산
        sum += pulseBuffer[i];
        if (pulseBuffer[i] < localMin) localMin = pulseBuffer[i];
        if (pulseBuffer[i] > localMax) localMax = pulseBuffer[i];
        
        // 전역 통계 업데이트
        if (pulseBuffer[i] < minPulse) minPulse = pulseBuffer[i];
        if (pulseBuffer[i] > maxPulse) maxPulse = pulseBuffer[i];
    }
    
    if (pulseCount % 10 != 0) {
        Serial.println();
    }
    
    // 신호 분석
    Serial.println();
    Serial.println("--- Signal Analysis ---");
    Serial.print("Total duration: ");
    Serial.print(sum);
    Serial.println(" us");
    Serial.print("Pulse count: ");
    Serial.println(pulseCount);
    Serial.print("Min pulse: ");
    Serial.print(localMin);
    Serial.println(" us");
    Serial.print("Max pulse: ");
    Serial.print(localMax);
    Serial.println(" us");
    Serial.print("Average: ");
    Serial.print(sum / pulseCount);
    Serial.println(" us");
    
    // 프로토콜 추정
    Serial.println();
    Serial.print("Protocol guess: ");
    if (localMax > 8000 && localMax < 10000) {
        Serial.println("NEC (Leader ~9ms detected)");
    } else if (localMax > 2000 && localMax < 3000) {
        Serial.println("RC5/RC6 or Sony SIRC");
    } else {
        Serial.println("Unknown");
    }
    
    Serial.println();
}
