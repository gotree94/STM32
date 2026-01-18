/*
 * ============================================================================
 * 트래킹 모듈 테스트 코드 (Line Tracking Sensor Module Test)
 * ============================================================================
 * 보드: NUCLEO-F103RB (STM32F103RBT6)
 * 환경: Arduino IDE with STM32duino
 * 
 * 핀 연결 (3채널 트래킹 센서 기준):
 *   트래킹 모듈      NUCLEO-F103RB
 *   ------------    -------------
 *   VCC         ->  5V
 *   GND         ->  GND
 *   L (왼쪽)    ->  D4 (PB5)
 *   C (중앙)    ->  D5 (PB4)
 *   R (오른쪽)  ->  D6 (PB10)
 * 
 * 동작 설명:
 *   - 적외선 반사를 이용한 라인 감지
 *   - 검은색 라인: LOW 출력 (적외선 흡수)
 *   - 흰색 바닥: HIGH 출력 (적외선 반사)
 *   - 라인 트레이서 로봇에 활용
 * 
 * 센서 특징:
 *   - TCRT5000 적외선 센서 기반
 *   - 감지 거리: 1~25mm
 *   - 가변저항으로 감도 조절 가능
 * ============================================================================
 */

// 핀 정의 (3채널 트래킹 센서)
#define TRACK_LEFT_PIN    PB5    // D4 - 왼쪽 센서
#define TRACK_CENTER_PIN  PB4    // D5 - 중앙 센서
#define TRACK_RIGHT_PIN   PB10   // D6 - 오른쪽 센서
#define LED_PIN           LED_BUILTIN  // 내장 LED

// 라인 감지 상태 정의
#define LINE_DETECTED     0      // 검은색 라인 감지 (LOW)
#define NO_LINE           1      // 흰색 바닥 (HIGH)

// 이동 방향 열거형
enum Direction {
    STOP,
    FORWARD,
    TURN_LEFT,
    TURN_RIGHT,
    SHARP_LEFT,
    SHARP_RIGHT,
    LINE_LOST
};

// 센서 상태 구조체
struct TrackingSensorState {
    bool left;
    bool center;
    bool right;
};

// 변수
TrackingSensorState sensorState;
Direction currentDirection = STOP;
unsigned long lastPrintTime = 0;

void setup() {
    // 시리얼 통신 초기화
    Serial.begin(115200);
    while (!Serial) {
        ; // 시리얼 포트 연결 대기
    }
    
    // 핀 설정
    pinMode(TRACK_LEFT_PIN, INPUT);
    pinMode(TRACK_CENTER_PIN, INPUT);
    pinMode(TRACK_RIGHT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("================================================");
    Serial.println("  트래킹 모듈 테스트 (Line Tracking Sensor)");
    Serial.println("================================================");
    Serial.println("보드: NUCLEO-F103RB");
    Serial.println("왼쪽 센서: PB5 (D4)");
    Serial.println("중앙 센서: PB4 (D5)");
    Serial.println("오른쪽 센서: PB10 (D6)");
    Serial.println("------------------------------------------------");
    Serial.println("검은색 라인 = 감지 (LOW)");
    Serial.println("흰색 바닥   = 미감지 (HIGH)");
    Serial.println("================================================");
    Serial.println();
    
    delay(1000);
}

void loop() {
    // 센서 값 읽기
    readSensors();
    
    // 방향 결정
    currentDirection = determineDirection();
    
    // LED 표시 (라인 감지 시 점등)
    if (sensorState.left || sensorState.center || sensorState.right) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
    
    // 시리얼 출력 (100ms 간격)
    if (millis() - lastPrintTime >= 100) {
        lastPrintTime = millis();
        printStatus();
    }
}

// 센서 값 읽기
void readSensors() {
    // LOW = 라인 감지, HIGH = 라인 없음
    sensorState.left = (digitalRead(TRACK_LEFT_PIN) == LINE_DETECTED);
    sensorState.center = (digitalRead(TRACK_CENTER_PIN) == LINE_DETECTED);
    sensorState.right = (digitalRead(TRACK_RIGHT_PIN) == LINE_DETECTED);
}

// 방향 결정 (라인 트레이서 로직)
Direction determineDirection() {
    // 센서 상태에 따른 방향 결정
    // L C R -> 동작
    // 0 0 0 -> LINE_LOST (라인 이탈)
    // 0 0 1 -> SHARP_RIGHT (급우회전)
    // 0 1 0 -> FORWARD (직진)
    // 0 1 1 -> TURN_RIGHT (우회전)
    // 1 0 0 -> SHARP_LEFT (급좌회전)
    // 1 0 1 -> FORWARD (직진 - 교차로)
    // 1 1 0 -> TURN_LEFT (좌회전)
    // 1 1 1 -> FORWARD (직진 - 넓은 라인 또는 교차로)
    
    int sensorValue = (sensorState.left << 2) | (sensorState.center << 1) | sensorState.right;
    
    switch (sensorValue) {
        case 0b000: return LINE_LOST;     // 0 0 0
        case 0b001: return SHARP_RIGHT;   // 0 0 1
        case 0b010: return FORWARD;       // 0 1 0
        case 0b011: return TURN_RIGHT;    // 0 1 1
        case 0b100: return SHARP_LEFT;    // 1 0 0
        case 0b101: return FORWARD;       // 1 0 1 (교차로)
        case 0b110: return TURN_LEFT;     // 1 1 0
        case 0b111: return FORWARD;       // 1 1 1 (넓은 라인)
        default:    return STOP;
    }
}

// 상태 출력
void printStatus() {
    // 센서 상태 시각화
    Serial.print("센서: [");
    Serial.print(sensorState.left ? "■" : "□");
    Serial.print("|");
    Serial.print(sensorState.center ? "■" : "□");
    Serial.print("|");
    Serial.print(sensorState.right ? "■" : "□");
    Serial.print("]");
    
    // Raw 값 출력
    Serial.print("  L:");
    Serial.print(sensorState.left ? "1" : "0");
    Serial.print(" C:");
    Serial.print(sensorState.center ? "1" : "0");
    Serial.print(" R:");
    Serial.print(sensorState.right ? "1" : "0");
    
    // 방향 출력
    Serial.print("  -> ");
    Serial.print(getDirectionString(currentDirection));
    
    // 방향 화살표
    Serial.print("  ");
    printDirectionArrow(currentDirection);
    
    Serial.println();
}

// 방향 문자열 반환
const char* getDirectionString(Direction dir) {
    switch (dir) {
        case STOP:        return "정지      ";
        case FORWARD:     return "직진      ";
        case TURN_LEFT:   return "좌회전    ";
        case TURN_RIGHT:  return "우회전    ";
        case SHARP_LEFT:  return "급좌회전  ";
        case SHARP_RIGHT: return "급우회전  ";
        case LINE_LOST:   return "라인이탈  ";
        default:          return "알수없음  ";
    }
}

// 방향 화살표 출력
void printDirectionArrow(Direction dir) {
    switch (dir) {
        case FORWARD:     Serial.print("    ↑    "); break;
        case TURN_LEFT:   Serial.print("  ↖      "); break;
        case TURN_RIGHT:  Serial.print("      ↗  "); break;
        case SHARP_LEFT:  Serial.print("←        "); break;
        case SHARP_RIGHT: Serial.print("        →"); break;
        case LINE_LOST:   Serial.print("    ?    "); break;
        default:          Serial.print("    ●    "); break;
    }
}

// 센서 보정 함수 (사용 전 호출)
void calibrateSensors() {
    Serial.println("------------------------------------------------");
    Serial.println("센서 보정 시작...");
    Serial.println("1. 센서를 흰색 바닥 위에 놓으세요.");
    delay(3000);
    
    int whiteLeft = digitalRead(TRACK_LEFT_PIN);
    int whiteCenter = digitalRead(TRACK_CENTER_PIN);
    int whiteRight = digitalRead(TRACK_RIGHT_PIN);
    
    Serial.println("2. 센서를 검은색 라인 위에 놓으세요.");
    delay(3000);
    
    int blackLeft = digitalRead(TRACK_LEFT_PIN);
    int blackCenter = digitalRead(TRACK_CENTER_PIN);
    int blackRight = digitalRead(TRACK_RIGHT_PIN);
    
    Serial.println("보정 결과:");
    Serial.print("  왼쪽 - 흰색: ");
    Serial.print(whiteLeft);
    Serial.print(", 검은색: ");
    Serial.println(blackLeft);
    
    Serial.print("  중앙 - 흰색: ");
    Serial.print(whiteCenter);
    Serial.print(", 검은색: ");
    Serial.println(blackCenter);
    
    Serial.print("  오른쪽 - 흰색: ");
    Serial.print(whiteRight);
    Serial.print(", 검은색: ");
    Serial.println(blackRight);
    
    Serial.println("------------------------------------------------");
}

// 단일 센서 테스트 함수
void testSingleSensor(int pin, const char* name) {
    int value = digitalRead(pin);
    Serial.print(name);
    Serial.print(": ");
    Serial.println(value == LINE_DETECTED ? "라인 감지" : "라인 없음");
}
