/*
 * Photo Interrupt Module Test (Photo Interrupter / Optical Sensor)
 * Board: NUCLEO-F103RB (STM32F103RB)
 * Arduino IDE with STM32duino
 * 
 * 연결:
 *   VCC -> 3.3V or 5V
 *   GND -> GND
 *   DO  -> D2 (PA10) - Digital Output (blocked/unblocked)
 *   AO  -> A0 (PA0) - Analog Output (optional, if available)
 */

// Pin Definitions
#define PHOTO_DIGITAL_PIN   D2      // Digital output pin
#define PHOTO_ANALOG_PIN    A0      // Analog output pin (if available)
#define LED_BUILTIN_PIN     LED_BUILTIN  // Onboard LED (PA5)

// Constants
#define ADC_RESOLUTION      4095    // 12-bit ADC
#define DEBOUNCE_TIME       10      // Debounce time in ms

// Variables for state tracking
volatile bool stateChanged = false;
volatile bool isBlocked = false;
volatile unsigned long lastInterruptTime = 0;

// RPM calculation variables
volatile unsigned long pulseCount = 0;
unsigned long lastRPMCalculation = 0;
float rpm = 0.0;
int slotsPerRevolution = 1;  // Number of slots in encoder wheel

// Timing measurement
unsigned long lastBlockTime = 0;
unsigned long lastUnblockTime = 0;
unsigned long blockDuration = 0;
unsigned long unblockDuration = 0;

// Event counter
unsigned long eventCount = 0;

void setup() {
    // Initialize Serial Communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Pin Mode Configuration
    pinMode(PHOTO_DIGITAL_PIN, INPUT_PULLUP);
    pinMode(PHOTO_ANALOG_PIN, INPUT);
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    
    // Read initial state
    isBlocked = (digitalRead(PHOTO_DIGITAL_PIN) == LOW);
    
    // Attach interrupt for edge detection
    attachInterrupt(digitalPinToInterrupt(PHOTO_DIGITAL_PIN), photoISR, CHANGE);
    
    Serial.println("================================");
    Serial.println("Photo Interrupt Module Test");
    Serial.println("Board: NUCLEO-F103RB");
    Serial.println("================================");
    Serial.println();
    Serial.println("Block/Unblock the sensor slot...");
    Serial.println();
    delay(1000);
    
    lastRPMCalculation = millis();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check for state change
    if (stateChanged) {
        processStateChange();
        stateChanged = false;
    }
    
    // Calculate RPM every second
    if (currentTime - lastRPMCalculation >= 1000) {
        calculateRPM();
        lastRPMCalculation = currentTime;
    }
    
    // Update LED based on blocked state
    digitalWrite(LED_BUILTIN_PIN, isBlocked ? HIGH : LOW);
    
    // Read and display analog value (if sensor has AO)
    int analogValue = analogRead(PHOTO_ANALOG_PIN);
    
    // Periodic status update (every 2 seconds)
    static unsigned long lastStatusUpdate = 0;
    if (currentTime - lastStatusUpdate >= 2000) {
        printStatus(analogValue);
        lastStatusUpdate = currentTime;
    }
    
    delay(10);
}

/*
 * Interrupt Service Routine for photo interrupter
 */
void photoISR() {
    unsigned long currentTime = millis();
    
    // Debounce
    if (currentTime - lastInterruptTime > DEBOUNCE_TIME) {
        // Read current state
        bool currentState = (digitalRead(PHOTO_DIGITAL_PIN) == LOW);
        
        // Check if state actually changed
        if (currentState != isBlocked) {
            isBlocked = currentState;
            stateChanged = true;
            pulseCount++;
        }
        
        lastInterruptTime = currentTime;
    }
}

/*
 * Process state change
 */
void processStateChange() {
    unsigned long currentTime = millis();
    eventCount++;
    
    Serial.println("-----------------------------");
    Serial.print("Event #");
    Serial.print(eventCount);
    Serial.print(": ");
    
    if (isBlocked) {
        // Object blocking the sensor
        Serial.println(">>> BLOCKED <<<");
        
        // Calculate unblock duration
        if (lastUnblockTime > 0) {
            unblockDuration = currentTime - lastUnblockTime;
            Serial.print("Open duration: ");
            Serial.print(unblockDuration);
            Serial.println(" ms");
        }
        lastBlockTime = currentTime;
        
    } else {
        // Sensor unblocked
        Serial.println(">>> CLEAR <<<");
        
        // Calculate block duration
        if (lastBlockTime > 0) {
            blockDuration = currentTime - lastBlockTime;
            Serial.print("Block duration: ");
            Serial.print(blockDuration);
            Serial.println(" ms");
        }
        lastUnblockTime = currentTime;
    }
    
    // Print timing info
    Serial.print("Time since start: ");
    Serial.print(currentTime / 1000.0, 2);
    Serial.println(" sec");
}

/*
 * Calculate RPM from pulse count
 */
void calculateRPM() {
    // RPM = (pulseCount / slotsPerRevolution) * 60
    rpm = ((float)pulseCount / slotsPerRevolution / 2.0) * 60.0;
    
    // Reset pulse count for next calculation
    pulseCount = 0;
    
    // Only print if there was rotation
    if (rpm > 0) {
        Serial.println();
        Serial.print(">>> RPM: ");
        Serial.print(rpm, 1);
        Serial.print(" (");
        Serial.print(slotsPerRevolution);
        Serial.println(" slot/rev) <<<");
        Serial.println();
    }
}

/*
 * Print current status
 */
void printStatus(int analogValue) {
    Serial.println();
    Serial.println("=== Status Update ===");
    Serial.print("State: ");
    Serial.println(isBlocked ? "BLOCKED" : "CLEAR");
    Serial.print("Total events: ");
    Serial.println(eventCount);
    Serial.print("Analog value: ");
    Serial.println(analogValue);
    
    if (blockDuration > 0 || unblockDuration > 0) {
        Serial.print("Last block: ");
        Serial.print(blockDuration);
        Serial.print(" ms | Last clear: ");
        Serial.print(unblockDuration);
        Serial.println(" ms");
    }
    
    Serial.println("====================");
    Serial.println();
}

/*
 * Set number of slots per revolution for RPM calculation
 */
void setSlotsPerRevolution(int slots) {
    slotsPerRevolution = slots;
}

/*
 * Get current RPM
 */
float getRPM() {
    return rpm;
}

/*
 * Check if sensor is blocked
 */
bool isSensorBlocked() {
    return isBlocked;
}

/*
 * Get total event count
 */
unsigned long getEventCount() {
    return eventCount;
}

/*
 * Reset counters
 */
void resetCounters() {
    eventCount = 0;
    pulseCount = 0;
    rpm = 0;
    blockDuration = 0;
    unblockDuration = 0;
}

/*
 * Get duty cycle (percentage of time blocked)
 */
float getDutyCycle() {
    if (blockDuration + unblockDuration == 0) return 0;
    return (float)blockDuration / (blockDuration + unblockDuration) * 100.0;
}

/*
 * Calculate frequency from timing
 */
float getFrequency() {
    unsigned long period = blockDuration + unblockDuration;
    if (period == 0) return 0;
    return 1000.0 / period; // Hz
}
