/*
 * ============================================================================
 * 장애물 감지센서 모듈 테스트 코드 (Obstacle Avoidance Sensor Module Test)
 * ============================================================================
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결:
 *   장애물 센서 모듈    NUCLEO-F103RB
 *   ---------------    -------------
 *   VCC            ->  3.3V ~ 5V
 *   GND            ->  GND
 *   OUT            ->  D8 (PA9)
 * 
 * 동작 설명:
 *   - 적외선 송수신을 이용한 장애물 감지
 *   - 장애물 감지 시: LOW 출력
 *   - 장애물 없음: HIGH 출력
 *   - 감지 거리: 2cm ~ 30cm (가변저항으로 조절)
 * 
 * 센서 특징:
 *   - IR LED + 포토트랜지스터 조합
 *   - LM393 비교기 사용
 *   - 가변저항으로 감지 거리 조절
 * ============================================================================
 */

// 핀 정의
#define OBSTACLE_PIN    PA9         // D8 - 장애물 센서 출력
#define LED_PIN         LED_BUILTIN // 내장 LED (PA5)
#define BUZZER_PIN      PA11        // D10 - 부저 (선택사항)

// 상태 정의
#define OBSTACLE_DETECTED   LOW     // 장애물 감지됨
#define NO_OBSTACLE         HIGH    // 장애물 없음

// 상수 정의
#define DEBOUNCE_TIME       50      // 디바운스 시간 (ms)
#define WARNING_INTERVAL    500     // 경고 간격 (ms)

// 변수
bool obstaclePresent = false;
bool previousObstacleState = false;
unsigned long lastDebounceTime = 0;
unsigned long lastWarningTime = 0;
unsigned long obstacleDetectedCount = 0;
unsigned long obstacleStartTime = 0;
unsigned long totalObstacleTime = 0;

void setup() {
    // 시리얼 통신 초기화
    Serial.begin(115200);
    while (!Serial) {
        ; // 시리얼 포트 연결 대기
    }
    
    // 핀 설정
    pinMode(OBSTACLE_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    
    Serial.println("================================================");
    Serial.println("  장애물 감지센서 테스트 (Obstacle Sensor)");
    Serial.println("================================================");
    Serial.println("보드: NUCLEO-F103RB");
    Serial.println("센서 핀: PA9 (D8)");
    Serial.println("감지 거리: 2cm ~ 30cm (가변저항 조절)");
    Serial.println("------------------------------------------------");
    Serial.println("장애물 감지 = LOW (LED ON)");
    Serial.println("장애물 없음 = HIGH (LED OFF)");
    Serial.println("------------------------------------------------");
    Serial.println("Tip: 모듈의 가변저항으로 감지 거리를 조절하세요.");
    Serial.println("================================================");
    Serial.println();
    
    delay(1000);
}

void loop() {
    // 현재 센서 상태 읽기
    bool currentReading = (digitalRead(OBSTACLE_PIN) == OBSTACLE_DETECTED);
    unsigned long currentTime = millis();
    
    // 디바운스 처리
    if (currentReading != previousObstacleState) {
        lastDebounceTime = currentTime;
    }
    
    if ((currentTime - lastDebounceTime) > DEBOUNCE_TIME) {
        // 상태 변화 감지
        if (currentReading != obstaclePresent) {
            obstaclePresent = currentReading;
            
            if (obstaclePresent) {
                // 장애물 감지 시작
                obstacleDetectedCount++;
                obstacleStartTime = currentTime;
                onObstacleDetected();
            } else {
                // 장애물 제거됨
                totalObstacleTime += (currentTime - obstacleStartTime);
                onObstacleCleared();
            }
        }
    }
    
    previousObstacleState = currentReading;
    
    // LED 및 부저 제어
    if (obstaclePresent) {
        digitalWrite(LED_PIN, HIGH);
        
        // 경고음 출력 (선택사항)
        if (currentTime - lastWarningTime >= WARNING_INTERVAL) {
            lastWarningTime = currentTime;
            beepWarning();
        }
    } else {
        digitalWrite(LED_PIN, LOW);
    }
    
    // 상태 출력 (200ms 간격)
    static unsigned long lastPrintTime = 0;
    if (currentTime - lastPrintTime >= 200) {
        lastPrintTime = currentTime;
        printStatus();
    }
}

// 장애물 감지 시 호출
void onObstacleDetected() {
    Serial.println();
    Serial.println("╔════════════════════════════════════════════╗");
    Serial.println("║     ⚠️  장애물 감지! (OBSTACLE DETECTED)     ║");
    Serial.println("╚════════════════════════════════════════════╝");
    Serial.print("감지 횟수: ");
    Serial.print(obstacleDetectedCount);
    Serial.println("회");
}

// 장애물 제거 시 호출
void onObstacleCleared() {
    unsigned long duration = millis() - obstacleStartTime;
    Serial.println();
    Serial.println("────────────────────────────────────────────");
    Serial.println("    ✓ 장애물 제거됨 (OBSTACLE CLEARED)");
    Serial.print("    감지 지속 시간: ");
    Serial.print(duration);
    Serial.println("ms");
    Serial.println("────────────────────────────────────────────");
}

// 상태 출력
void printStatus() {
    Serial.print("상태: ");
    
    if (obstaclePresent) {
        Serial.print("[■ 장애물 있음]");
        unsigned long duration = millis() - obstacleStartTime;
        Serial.print("  지속: ");
        Serial.print(duration / 1000.0, 1);
        Serial.print("초");
    } else {
        Serial.print("[□ 장애물 없음]");
    }
    
    Serial.print("  | 총 감지: ");
    Serial.print(obstacleDetectedCount);
    Serial.print("회");
    
    Serial.print("  | 누적 시간: ");
    unsigned long totalTime = totalObstacleTime;
    if (obstaclePresent) {
        totalTime += (millis() - obstacleStartTime);
    }
    Serial.print(totalTime / 1000.0, 1);
    Serial.print("초");
    
    // 상태 바 출력
    Serial.print("  ");
    if (obstaclePresent) {
        Serial.print("████████");
    } else {
        Serial.print("--------");
    }
    
    Serial.println();
}

// 경고음 출력
void beepWarning() {
    // 짧은 비프음 (선택사항 - 부저 연결 시)
    // digitalWrite(BUZZER_PIN, HIGH);
    // delay(50);
    // digitalWrite(BUZZER_PIN, LOW);
}

// 통계 초기화
void resetStatistics() {
    obstacleDetectedCount = 0;
    totalObstacleTime = 0;
    Serial.println("통계가 초기화되었습니다.");
}

// 장애물 감지 여부 반환
bool isObstacleDetected() {
    return obstaclePresent;
}

// 장애물 감지 대기 (타임아웃 지원)
bool waitForObstacle(unsigned long timeoutMs) {
    unsigned long startTime = millis();
    
    while (millis() - startTime < timeoutMs) {
        if (digitalRead(OBSTACLE_PIN) == OBSTACLE_DETECTED) {
            return true;
        }
        delay(10);
    }
    return false;
}

// 장애물 제거 대기 (타임아웃 지원)
bool waitForClear(unsigned long timeoutMs) {
    unsigned long startTime = millis();
    
    while (millis() - startTime < timeoutMs) {
        if (digitalRead(OBSTACLE_PIN) == NO_OBSTACLE) {
            return true;
        }
        delay(10);
    }
    return false;
}

// 센서 테스트 (연속 읽기)
void continuousSensorTest(int durationSec) {
    Serial.print("연속 센서 테스트 시작 (");
    Serial.print(durationSec);
    Serial.println("초)");
    
    unsigned long startTime = millis();
    int totalReadings = 0;
    int obstacleReadings = 0;
    
    while (millis() - startTime < (durationSec * 1000)) {
        if (digitalRead(OBSTACLE_PIN) == OBSTACLE_DETECTED) {
            obstacleReadings++;
        }
        totalReadings++;
        delay(10);
    }
    
    Serial.println("테스트 완료!");
    Serial.print("  총 읽기 횟수: ");
    Serial.println(totalReadings);
    Serial.print("  장애물 감지 횟수: ");
    Serial.println(obstacleReadings);
    Serial.print("  장애물 감지 비율: ");
    Serial.print((float)obstacleReadings / totalReadings * 100, 1);
    Serial.println("%");
}

// 거리 추정 (참고용 - 정확하지 않음)
// 실제 거리 측정은 초음파 센서 권장
String estimateDistance() {
    static int stableCount = 0;
    
    if (obstaclePresent) {
        stableCount++;
        if (stableCount > 10) {
            return "매우 가까움 (<5cm)";
        } else if (stableCount > 5) {
            return "가까움 (5-15cm)";
        } else {
            return "감지됨 (15-30cm)";
        }
    } else {
        stableCount = 0;
        return "감지 범위 밖";
    }
}
