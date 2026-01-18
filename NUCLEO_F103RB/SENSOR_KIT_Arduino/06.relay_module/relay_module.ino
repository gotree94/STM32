/*
 * ============================================================================
 * 릴레이 모듈 테스트 코드 (Relay Module Test)
 * ============================================================================
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   릴레이 모듈    NUCLEO-F103RB
 *   -----------    -------------
 *   VCC        ->  5V
 *   GND        ->  GND
 *   IN (S)     ->  D7 (PA8)
 * 
 * 동작 설명:
 *   - 릴레이를 1초 간격으로 ON/OFF 반복
 *   - 시리얼 모니터로 상태 확인 가능
 * 
 * 주의사항:
 *   - 대부분의 릴레이 모듈은 LOW 신호에서 활성화됨 (Active LOW)
 *   - 릴레이로 AC 전원 제어 시 안전에 주의
 * ============================================================================
 */

// 핀 정의
#define RELAY_PIN   PA8   // D7 on NUCLEO-F103RB

// 릴레이 상태
bool relayState = false;

void setup() {
    // 시리얼 통신 초기화
    Serial.begin(115200);
    while (!Serial) {
        ; // 시리얼 포트 연결 대기
    }
    
    // 릴레이 핀 설정
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);  // 초기 상태: OFF (Active LOW 기준)
    
    Serial.println("============================================");
    Serial.println("   릴레이 모듈 테스트 (Relay Module Test)");
    Serial.println("============================================");
    Serial.println("보드: NUCLEO-F103RB");
    Serial.println("릴레이 핀: PA8 (D7)");
    Serial.println("--------------------------------------------");
    Serial.println();
    
    delay(1000);
}

void loop() {
    // 릴레이 ON
    relayOn();
    Serial.println("[릴레이 ON]  - 접점 연결됨 (COM-NO 연결)");
    delay(1000);
    
    // 릴레이 OFF
    relayOff();
    Serial.println("[릴레이 OFF] - 접점 분리됨 (COM-NC 연결)");
    delay(1000);
    
    Serial.println();
}

// 릴레이 ON 함수 (Active LOW)
void relayOn() {
    digitalWrite(RELAY_PIN, LOW);
    relayState = true;
}

// 릴레이 OFF 함수 (Active LOW)
void relayOff() {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = false;
}

// 릴레이 토글 함수
void relayToggle() {
    if (relayState) {
        relayOff();
    } else {
        relayOn();
    }
}

// 릴레이 상태 반환
bool getRelayState() {
    return relayState;
}
