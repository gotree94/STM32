/**
 ******************************************************************************
 * @file    03_IR_Remote_Decoder.ino
 * @brief   IR Remote Control Decoder for NUCLEO-F103RB (Arduino)
 * @author  Embedded Systems Lab
 * @version V1.0.0
 * @date    2025-01-18
 ******************************************************************************
 * @details
 * IR 리모컨에서 입력되는 신호를 받아서 NEC 프로토콜로 디코딩하고,
 * 디코딩된 값(주소, 명령)을 시리얼로 PC에 출력합니다.
 * 
 * Hardware Setup:
 * - IR Receiver: PA0 (A0) - Signal input
 * - User LED: PA5 (D13, 내장 LED)
 * - Serial: USB Virtual COM Port (115200 baud)
 * 
 * NEC Protocol:
 * - Leader: 9ms pulse + 4.5ms space
 * - Bit 0: 560us pulse + 560us space
 * - Bit 1: 560us pulse + 1690us space
 * - 32-bit data: Address(8) + ~Address(8) + Command(8) + ~Command(8)
 * - Repeat: 9ms pulse + 2.25ms space + 560us pulse
 ******************************************************************************
 */

/* Pin Definitions */
#define IR_RX_PIN       PA0     // IR Receiver signal
#define USER_LED_PIN    PA5     // Onboard LED

/* NEC Protocol Timing (microseconds) with tolerance */
#define NEC_LEADER_PULSE_MIN    8000
#define NEC_LEADER_PULSE_MAX    10000
#define NEC_LEADER_SPACE_MIN    4000
#define NEC_LEADER_SPACE_MAX    5000
#define NEC_REPEAT_SPACE_MIN    2000
#define NEC_REPEAT_SPACE_MAX    2500
#define NEC_BIT_PULSE_MIN       400
#define NEC_BIT_PULSE_MAX       700
#define NEC_ZERO_SPACE_MIN      400
#define NEC_ZERO_SPACE_MAX      700
#define NEC_ONE_SPACE_MIN       1500
#define NEC_ONE_SPACE_MAX       1800

/* Buffer Settings */
#define MAX_PULSES      70      // NEC = 67 edges
#define TIMEOUT_US      100000  // 100ms timeout

/* Decode Result */
typedef struct {
    uint8_t address;
    uint8_t addressInv;
    uint8_t command;
    uint8_t commandInv;
    bool isRepeat;
    bool isValid;
    bool isExtended;    // NEC Extended (16-bit address)
} IRResult_t;

/* Variables */
volatile uint32_t pulseBuffer[MAX_PULSES];
volatile uint16_t pulseIndex = 0;
volatile uint32_t lastEdgeTime = 0;
volatile bool capturing = false;
volatile bool dataReady = false;

IRResult_t lastResult;
uint32_t lastDecodeTime = 0;
uint32_t totalDecoded = 0;
uint32_t totalRepeats = 0;
uint32_t totalErrors = 0;

/* Common Remote Button Names */
typedef struct {
    uint8_t address;
    uint8_t command;
    const char* name;
} ButtonName_t;

// 일반적인 리모컨 버튼 이름 (주소 0x00 기준)
const ButtonName_t commonButtons[] = {
    {0x00, 0x45, "CH-"},
    {0x00, 0x46, "CH"},
    {0x00, 0x47, "CH+"},
    {0x00, 0x44, "PREV"},
    {0x00, 0x40, "NEXT"},
    {0x00, 0x43, "PLAY/PAUSE"},
    {0x00, 0x07, "VOL-"},
    {0x00, 0x15, "VOL+"},
    {0x00, 0x09, "EQ"},
    {0x00, 0x16, "0"},
    {0x00, 0x19, "100+"},
    {0x00, 0x0D, "200+"},
    {0x00, 0x0C, "1"},
    {0x00, 0x18, "2"},
    {0x00, 0x5E, "3"},
    {0x00, 0x08, "4"},
    {0x00, 0x1C, "5"},
    {0x00, 0x5A, "6"},
    {0x00, 0x42, "7"},
    {0x00, 0x52, "8"},
    {0x00, 0x4A, "9"},
};
const int numButtons = sizeof(commonButtons) / sizeof(commonButtons[0]);

/* Function Prototypes */
void irISR(void);
bool decodeNEC(IRResult_t* result);
const char* getButtonName(uint8_t addr, uint8_t cmd);
void printResult(IRResult_t* result);
void printRawData(void);
void printStatistics(void);

/**
 * @brief Setup function
 */
void setup() {
    // Serial 초기화
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    Serial.println();
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║   IR Remote Decoder - NUCLEO-F103RB    ║");
    Serial.println("║   NEC Protocol Decoder                 ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    // GPIO 초기화
    pinMode(USER_LED_PIN, OUTPUT);
    pinMode(IR_RX_PIN, INPUT);
    
    digitalWrite(USER_LED_PIN, LOW);
    
    // 인터럽트 설정
    attachInterrupt(digitalPinToInterrupt(IR_RX_PIN), irISR, CHANGE);
    
    Serial.println("Hardware: TSOP1838, VS1838B, or compatible");
    Serial.println("Input Pin: PA0 (A0)");
    Serial.println("Protocol: NEC / NEC Extended");
    Serial.println();
    Serial.println("Commands:");
    Serial.println("  's' - Show statistics");
    Serial.println("  'r' - Reset statistics");
    Serial.println("  'h' - Show this help");
    Serial.println();
    Serial.println("Point your remote at the IR receiver and press any button...");
    Serial.println();
}

/**
 * @brief Main loop
 */
void loop() {
    // 시리얼 명령 처리
    if (Serial.available()) {
        char cmd = Serial.read();
        switch (cmd) {
            case 's':
            case 'S':
                printStatistics();
                break;
            case 'r':
            case 'R':
                totalDecoded = 0;
                totalRepeats = 0;
                totalErrors = 0;
                Serial.println("[INFO] Statistics reset");
                break;
            case 'h':
            case 'H':
                Serial.println("\nCommands: 's'-stats, 'r'-reset, 'h'-help\n");
                break;
        }
    }
    
    // 캡처 완료 확인
    if (pulseIndex > 0 && !capturing) {
        if (micros() - lastEdgeTime > 50000) {  // 50ms 무신호 = 프레임 종료
            dataReady = true;
        }
    }
    
    // 디코딩 처리
    if (dataReady) {
        IRResult_t result;
        
        if (decodeNEC(&result)) {
            if (result.isRepeat) {
                // 리피트 코드
                totalRepeats++;
                Serial.print("[REPEAT] ");
                Serial.print("Addr=0x");
                if (lastResult.address < 0x10) Serial.print("0");
                Serial.print(lastResult.address, HEX);
                Serial.print(", Cmd=0x");
                if (lastResult.command < 0x10) Serial.print("0");
                Serial.print(lastResult.command, HEX);
                
                const char* name = getButtonName(lastResult.address, lastResult.command);
                if (name != NULL) {
                    Serial.print(" (");
                    Serial.print(name);
                    Serial.print(")");
                }
                Serial.println();
            } else {
                // 새로운 코드
                totalDecoded++;
                lastResult = result;
                printResult(&result);
            }
            
            // LED 깜빡임
            digitalWrite(USER_LED_PIN, HIGH);
            delay(50);
            digitalWrite(USER_LED_PIN, LOW);
        } else {
            // 디코딩 실패
            totalErrors++;
            Serial.println("[ERROR] Decode failed - Unknown protocol or noise");
            printRawData();
        }
        
        // 버퍼 리셋
        pulseIndex = 0;
        dataReady = false;
        lastEdgeTime = 0;
    }
}

/**
 * @brief IR 인터럽트 핸들러
 */
void irISR() {
    uint32_t now = micros();
    
    if (lastEdgeTime > 0) {
        uint32_t duration = now - lastEdgeTime;
        
        if (duration > 200 && duration < TIMEOUT_US) {  // 노이즈 필터
            if (pulseIndex < MAX_PULSES) {
                pulseBuffer[pulseIndex++] = duration;
                capturing = true;
            }
        }
    }
    
    lastEdgeTime = now;
}

/**
 * @brief NEC 프로토콜 디코딩
 * @param result 디코딩 결과 저장 구조체
 * @return 디코딩 성공 여부
 */
bool decodeNEC(IRResult_t* result) {
    result->isValid = false;
    result->isRepeat = false;
    result->isExtended = false;
    
    if (pulseIndex < 3) {
        return false;
    }
    
    // Leader pulse 확인
    if (pulseBuffer[0] < NEC_LEADER_PULSE_MIN || pulseBuffer[0] > NEC_LEADER_PULSE_MAX) {
        return false;
    }
    
    // Repeat 코드 확인
    if (pulseBuffer[1] >= NEC_REPEAT_SPACE_MIN && pulseBuffer[1] <= NEC_REPEAT_SPACE_MAX) {
        result->isRepeat = true;
        result->isValid = true;
        return true;
    }
    
    // Leader space 확인
    if (pulseBuffer[1] < NEC_LEADER_SPACE_MIN || pulseBuffer[1] > NEC_LEADER_SPACE_MAX) {
        return false;
    }
    
    // 데이터 비트 디코딩 (32비트 = 64 edges + leader 2 = 66)
    if (pulseIndex < 66) {
        return false;
    }
    
    uint32_t data = 0;
    int bitIndex = 0;
    
    for (int i = 2; i < 66 && bitIndex < 32; i += 2) {
        // Pulse 확인
        if (pulseBuffer[i] < NEC_BIT_PULSE_MIN || pulseBuffer[i] > NEC_BIT_PULSE_MAX) {
            return false;
        }
        
        // Space로 비트 결정
        uint32_t space = pulseBuffer[i + 1];
        
        if (space >= NEC_ONE_SPACE_MIN && space <= NEC_ONE_SPACE_MAX) {
            data |= (1UL << bitIndex);  // Bit 1
        } else if (space >= NEC_ZERO_SPACE_MIN && space <= NEC_ZERO_SPACE_MAX) {
            // Bit 0 (이미 0)
        } else {
            return false;  // 유효하지 않은 타이밍
        }
        
        bitIndex++;
    }
    
    // 데이터 추출
    result->address = data & 0xFF;
    result->addressInv = (data >> 8) & 0xFF;
    result->command = (data >> 16) & 0xFF;
    result->commandInv = (data >> 24) & 0xFF;
    
    // 검증
    if (result->address == (uint8_t)(~result->addressInv)) {
        // Standard NEC
        result->isExtended = false;
        result->isValid = (result->command == (uint8_t)(~result->commandInv));
    } else {
        // NEC Extended (16-bit address)
        result->isExtended = true;
        result->isValid = (result->command == (uint8_t)(~result->commandInv));
    }
    
    return result->isValid;
}

/**
 * @brief 버튼 이름 찾기
 */
const char* getButtonName(uint8_t addr, uint8_t cmd) {
    for (int i = 0; i < numButtons; i++) {
        if (commonButtons[i].address == addr && commonButtons[i].command == cmd) {
            return commonButtons[i].name;
        }
    }
    return NULL;
}

/**
 * @brief 디코딩 결과 출력
 */
void printResult(IRResult_t* result) {
    Serial.println("┌──────────────────────────────────────┐");
    Serial.print("│ [DECODED] ");
    
    if (result->isExtended) {
        // NEC Extended: 16-bit address
        uint16_t addr16 = result->address | (result->addressInv << 8);
        Serial.print("NEC Extended          │\n");
        Serial.print("│ Address: 0x");
        if (addr16 < 0x1000) Serial.print("0");
        if (addr16 < 0x100) Serial.print("0");
        if (addr16 < 0x10) Serial.print("0");
        Serial.print(addr16, HEX);
        Serial.println("                      │");
    } else {
        // Standard NEC
        Serial.print("NEC Standard          │\n");
        Serial.print("│ Address: 0x");
        if (result->address < 0x10) Serial.print("0");
        Serial.print(result->address, HEX);
        Serial.println("                          │");
    }
    
    Serial.print("│ Command: 0x");
    if (result->command < 0x10) Serial.print("0");
    Serial.print(result->command, HEX);
    Serial.print(" (");
    Serial.print(result->command);
    Serial.println(")                    │");
    
    // 버튼 이름
    const char* name = getButtonName(result->address, result->command);
    if (name != NULL) {
        Serial.print("│ Button:  ");
        Serial.print(name);
        for (int i = strlen(name); i < 26; i++) Serial.print(" ");
        Serial.println("│");
    }
    
    Serial.println("└──────────────────────────────────────┘");
    
    // Raw 32-bit 값
    uint32_t raw = result->address | 
                   (result->addressInv << 8) | 
                   (result->command << 16) | 
                   (result->commandInv << 24);
    Serial.print("Raw 32-bit: 0x");
    Serial.println(raw, HEX);
    Serial.println();
}

/**
 * @brief Raw 펄스 데이터 출력
 */
void printRawData() {
    Serial.print("Raw pulses (");
    Serial.print(pulseIndex);
    Serial.println("):");
    
    for (int i = 0; i < pulseIndex && i < 20; i++) {
        Serial.print(pulseBuffer[i]);
        Serial.print(" ");
        if ((i + 1) % 10 == 0) Serial.println();
    }
    
    if (pulseIndex > 20) {
        Serial.print("... (");
        Serial.print(pulseIndex - 20);
        Serial.println(" more)");
    }
    Serial.println();
}

/**
 * @brief 통계 출력
 */
void printStatistics() {
    Serial.println();
    Serial.println("╔════════════════════════════════╗");
    Serial.println("║         STATISTICS             ║");
    Serial.println("╠════════════════════════════════╣");
    Serial.print("║ Decoded:  ");
    Serial.print(totalDecoded);
    for (int i = String(totalDecoded).length(); i < 19; i++) Serial.print(" ");
    Serial.println("║");
    Serial.print("║ Repeats:  ");
    Serial.print(totalRepeats);
    for (int i = String(totalRepeats).length(); i < 19; i++) Serial.print(" ");
    Serial.println("║");
    Serial.print("║ Errors:   ");
    Serial.print(totalErrors);
    for (int i = String(totalErrors).length(); i < 19; i++) Serial.print(" ");
    Serial.println("║");
    Serial.println("╚════════════════════════════════╝");
    Serial.println();
}
