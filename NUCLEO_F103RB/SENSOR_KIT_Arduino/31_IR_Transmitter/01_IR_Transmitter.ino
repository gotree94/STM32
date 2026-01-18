/**
 ******************************************************************************
 * @file    01_IR_Transmitter.ino
 * @brief   IR Transmitter Module for NUCLEO-F103RB (Arduino)
 * @author  Embedded Systems Lab
 * @version V1.0.0
 * @date    2025-01-18
 ******************************************************************************
 * @details
 * IR LED를 이용하여 38kHz 캐리어로 변조된 NEC 프로토콜 신호를 송신합니다.
 * 
 * Hardware Setup:
 * - IR LED: PA8 (D7) - 38kHz PWM 출력
 * - User Button: PC13 (내장 버튼)
 * - User LED: PA5 (D13, 내장 LED)
 * - Serial: USB Virtual COM Port (115200 baud)
 * 
 * 회로 연결:
 *   PA8 --- [100Ω] --- [IR LED] --- GND
 *   (또는 트랜지스터 드라이버 사용 권장)
 * 
 * 동작: 버튼을 누르면 설정된 NEC 코드를 송신합니다.
 ******************************************************************************
 */

/* Pin Definitions */
#define IR_LED_PIN      PA8     // IR LED (Timer PWM)
#define USER_BTN_PIN    PC13    // User Button (Active LOW)
#define USER_LED_PIN    PA5     // Onboard LED

/* NEC Protocol Timing (microseconds) */
#define NEC_LEADER_PULSE    9000
#define NEC_LEADER_SPACE    4500
#define NEC_BIT_PULSE       560
#define NEC_ONE_SPACE       1690
#define NEC_ZERO_SPACE      560
#define NEC_REPEAT_SPACE    2250

/* IR Code Settings */
uint8_t ir_address = 0x00;      // Device address
uint8_t ir_command = 0x0C;      // Command (예: Power)

/* Variables */
volatile bool buttonPressed = false;
uint32_t lastButtonTime = 0;

/* Function Prototypes */
void IR_CarrierOn(void);
void IR_CarrierOff(void);
void IR_SendPulse(uint16_t onTime, uint16_t offTime);
void IR_SendByte(uint8_t data);
void IR_SendNEC(uint8_t address, uint8_t command);
void IR_SendRepeat(void);

/**
 * @brief Setup function
 */
void setup() {
    // Serial 초기화
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("  IR Transmitter - NUCLEO-F103RB");
    Serial.println("  NEC Protocol, 38kHz Carrier");
    Serial.println("========================================");
    
    // GPIO 초기화
    pinMode(USER_LED_PIN, OUTPUT);
    pinMode(USER_BTN_PIN, INPUT);
    pinMode(IR_LED_PIN, OUTPUT);
    
    digitalWrite(USER_LED_PIN, LOW);
    digitalWrite(IR_LED_PIN, LOW);
    
    // 버튼 인터럽트 설정
    attachInterrupt(digitalPinToInterrupt(USER_BTN_PIN), buttonISR, FALLING);
    
    Serial.print("IR Address: 0x");
    Serial.println(ir_address, HEX);
    Serial.print("IR Command: 0x");
    Serial.println(ir_command, HEX);
    Serial.println();
    Serial.println("Press USER button to transmit IR signal...");
    Serial.println();
}

/**
 * @brief Main loop
 */
void loop() {
    if (buttonPressed) {
        // 디바운싱
        if (millis() - lastButtonTime > 200) {
            lastButtonTime = millis();
            
            digitalWrite(USER_LED_PIN, HIGH);
            
            Serial.print("[TX] Sending NEC: Addr=0x");
            Serial.print(ir_address, HEX);
            Serial.print(", Cmd=0x");
            Serial.println(ir_command, HEX);
            
            // NEC 코드 송신
            IR_SendNEC(ir_address, ir_command);
            
            Serial.println("[TX] Complete!");
            Serial.println();
            
            digitalWrite(USER_LED_PIN, LOW);
        }
        buttonPressed = false;
    }
}

/**
 * @brief Button interrupt handler
 */
void buttonISR() {
    buttonPressed = true;
}

/**
 * @brief 38kHz 캐리어 ON (소프트웨어 PWM)
 */
void IR_CarrierOn() {
    // 38kHz = 26.3us period, 50% duty cycle
    // 13us HIGH, 13us LOW
    digitalWrite(IR_LED_PIN, HIGH);
}

/**
 * @brief 캐리어 OFF
 */
void IR_CarrierOff() {
    digitalWrite(IR_LED_PIN, LOW);
}

/**
 * @brief 38kHz 변조된 펄스 송신
 * @param onTime  캐리어 ON 시간 (us)
 * @param offTime 캐리어 OFF 시간 (us)
 */
void IR_SendPulse(uint16_t onTime, uint16_t offTime) {
    uint32_t startTime;
    uint32_t elapsed;
    
    // 38kHz 캐리어로 변조된 펄스 (약 26us 주기)
    startTime = micros();
    while ((micros() - startTime) < onTime) {
        digitalWrite(IR_LED_PIN, HIGH);
        delayMicroseconds(13);
        digitalWrite(IR_LED_PIN, LOW);
        delayMicroseconds(13);
    }
    
    // Space (캐리어 OFF)
    digitalWrite(IR_LED_PIN, LOW);
    delayMicroseconds(offTime);
}

/**
 * @brief 1바이트 송신 (LSB first)
 * @param data 송신할 데이터
 */
void IR_SendByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        if (data & 0x01) {
            // Bit '1': 560us pulse + 1690us space
            IR_SendPulse(NEC_BIT_PULSE, NEC_ONE_SPACE);
        } else {
            // Bit '0': 560us pulse + 560us space
            IR_SendPulse(NEC_BIT_PULSE, NEC_ZERO_SPACE);
        }
        data >>= 1;
    }
}

/**
 * @brief NEC 프로토콜 프레임 송신
 * @param address 디바이스 주소
 * @param command 명령 코드
 */
void IR_SendNEC(uint8_t address, uint8_t command) {
    noInterrupts();
    
    // Leader code: 9ms pulse + 4.5ms space
    IR_SendPulse(NEC_LEADER_PULSE, NEC_LEADER_SPACE);
    
    // Address (8-bit)
    IR_SendByte(address);
    
    // Address Inverse (8-bit)
    IR_SendByte(~address);
    
    // Command (8-bit)
    IR_SendByte(command);
    
    // Command Inverse (8-bit)
    IR_SendByte(~command);
    
    // Stop bit: 560us pulse
    IR_SendPulse(NEC_BIT_PULSE, 0);
    
    interrupts();
}

/**
 * @brief NEC 리피트 코드 송신
 */
void IR_SendRepeat() {
    noInterrupts();
    
    // Repeat: 9ms pulse + 2.25ms space + 560us pulse
    IR_SendPulse(NEC_LEADER_PULSE, NEC_REPEAT_SPACE);
    IR_SendPulse(NEC_BIT_PULSE, 0);
    
    interrupts();
}
