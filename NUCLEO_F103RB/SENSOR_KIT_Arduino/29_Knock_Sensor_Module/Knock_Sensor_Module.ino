/*
 * Knock Sensor Module Test (Piezoelectric / Vibration)
 * Board: NUCLEO-F103RB (STM32F103RB)
 * Arduino IDE with STM32duino
 * 
 * 연결:
 *   VCC -> 3.3V or 5V
 *   GND -> GND
 *   DO  -> D2 (PA10) - Digital Output (knock detected)
 *   AO  -> A0 (PA0) - Analog Output (vibration level)
 */

// Pin Definitions
#define KNOCK_DIGITAL_PIN   D2      // Digital output pin
#define KNOCK_ANALOG_PIN    A0      // Analog output pin
#define LED_BUILTIN_PIN     LED_BUILTIN  // Onboard LED (PA5)

// Constants
#define ADC_RESOLUTION      4095    // 12-bit ADC
#define DEBOUNCE_TIME       50      // Debounce time in ms
#define KNOCK_TIMEOUT       500     // Timeout for knock pattern in ms

// Knock detection thresholds
#define LIGHT_KNOCK         200     // Light knock threshold
#define MEDIUM_KNOCK        800     // Medium knock threshold
#define HARD_KNOCK          1500    // Hard knock threshold

// Variables
volatile bool knockDetected = false;
int analogValue = 0;
int peakValue = 0;
unsigned long lastKnockTime = 0;
int knockCount = 0;
unsigned long patternStartTime = 0;
String knockIntensity = "";

// Knock pattern detection
#define MAX_PATTERN_LENGTH  10
unsigned long knockTimes[MAX_PATTERN_LENGTH];
int patternIndex = 0;

void setup() {
    // Initialize Serial Communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Pin Mode Configuration
    pinMode(KNOCK_DIGITAL_PIN, INPUT);
    pinMode(KNOCK_ANALOG_PIN, INPUT);
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    
    // Attach interrupt for digital knock detection
    attachInterrupt(digitalPinToInterrupt(KNOCK_DIGITAL_PIN), knockISR, FALLING);
    
    Serial.println("================================");
    Serial.println("Knock Sensor Module Test");
    Serial.println("Board: NUCLEO-F103RB");
    Serial.println("================================");
    Serial.println("Tap or knock on the sensor...");
    Serial.println();
    delay(1000);
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check for interrupt-detected knock
    if (knockDetected) {
        processKnock();
        knockDetected = false;
    }
    
    // Poll analog value for continuous monitoring
    analogValue = analogRead(KNOCK_ANALOG_PIN);
    
    // Update peak value if higher
    if (analogValue > peakValue) {
        peakValue = analogValue;
    }
    
    // Check for knock pattern timeout
    if (knockCount > 0 && (currentTime - lastKnockTime > KNOCK_TIMEOUT)) {
        // Pattern complete
        printKnockPattern();
        resetPattern();
    }
    
    // Reset peak value periodically
    static unsigned long lastPeakReset = 0;
    if (currentTime - lastPeakReset > 1000) {
        peakValue = 0;
        lastPeakReset = currentTime;
    }
    
    // Small delay to prevent flooding
    delay(10);
}

/*
 * Interrupt Service Routine for knock detection
 */
void knockISR() {
    unsigned long currentTime = millis();
    
    // Debounce
    if (currentTime - lastKnockTime > DEBOUNCE_TIME) {
        knockDetected = true;
        lastKnockTime = currentTime;
    }
}

/*
 * Process detected knock
 */
void processKnock() {
    // Read analog value for intensity
    int intensity = 0;
    
    // Take multiple readings to catch peak
    for (int i = 0; i < 10; i++) {
        int reading = analogRead(KNOCK_ANALOG_PIN);
        if (reading > intensity) {
            intensity = reading;
        }
        delayMicroseconds(100);
    }
    
    // Determine knock intensity
    knockIntensity = getKnockIntensity(intensity);
    
    // Blink LED
    digitalWrite(LED_BUILTIN_PIN, HIGH);
    
    // Record knock time for pattern detection
    if (patternIndex < MAX_PATTERN_LENGTH) {
        if (knockCount == 0) {
            patternStartTime = millis();
        }
        knockTimes[patternIndex] = millis() - patternStartTime;
        patternIndex++;
    }
    knockCount++;
    
    // Print knock info
    Serial.println("-----------------------------");
    Serial.print(">>> KNOCK #");
    Serial.print(knockCount);
    Serial.println(" DETECTED! <<<");
    Serial.print("Intensity: ");
    Serial.print(intensity);
    Serial.print(" (");
    Serial.print(knockIntensity);
    Serial.println(")");
    
    // Print intensity bar
    printIntensityBar(intensity);
    
    delay(50);
    digitalWrite(LED_BUILTIN_PIN, LOW);
}

/*
 * Get knock intensity as string
 */
String getKnockIntensity(int value) {
    if (value < LIGHT_KNOCK) {
        return "Very Light";
    } else if (value < MEDIUM_KNOCK) {
        return "Light";
    } else if (value < HARD_KNOCK) {
        return "Medium";
    } else {
        return "Hard";
    }
}

/*
 * Print visual intensity bar
 */
void printIntensityBar(int value) {
    int bars = map(value, 0, ADC_RESOLUTION, 0, 20);
    Serial.print("Power: [");
    for (int i = 0; i < 20; i++) {
        if (i < bars) {
            Serial.print("█");
        } else {
            Serial.print("░");
        }
    }
    Serial.print("] ");
    Serial.print(map(value, 0, ADC_RESOLUTION, 0, 100));
    Serial.println("%");
}

/*
 * Print knock pattern
 */
void printKnockPattern() {
    Serial.println();
    Serial.println("=============================");
    Serial.print("Knock Pattern Complete: ");
    Serial.print(knockCount);
    Serial.println(" knocks");
    Serial.print("Pattern timing (ms): ");
    
    for (int i = 0; i < patternIndex; i++) {
        Serial.print(knockTimes[i]);
        if (i < patternIndex - 1) {
            Serial.print(", ");
        }
    }
    Serial.println();
    
    // Calculate intervals
    if (patternIndex > 1) {
        Serial.print("Intervals (ms): ");
        for (int i = 1; i < patternIndex; i++) {
            Serial.print(knockTimes[i] - knockTimes[i-1]);
            if (i < patternIndex - 1) {
                Serial.print(", ");
            }
        }
        Serial.println();
    }
    Serial.println("=============================");
    Serial.println();
}

/*
 * Reset knock pattern
 */
void resetPattern() {
    knockCount = 0;
    patternIndex = 0;
    patternStartTime = 0;
    for (int i = 0; i < MAX_PATTERN_LENGTH; i++) {
        knockTimes[i] = 0;
    }
}

/*
 * Check if specific number of knocks detected
 */
bool checkKnockCount(int targetCount) {
    return knockCount == targetCount;
}

/*
 * Get raw analog reading
 */
int getRawAnalogValue() {
    return analogRead(KNOCK_ANALOG_PIN);
}

/*
 * Get current knock count
 */
int getKnockCount() {
    return knockCount;
}
