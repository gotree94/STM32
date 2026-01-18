/*
 * Hall Magnetic Sensor Module Test
 * Board: NUCLEO-F103RB (STM32F103RB)
 * Arduino IDE with STM32duino
 * 
 * 연결:
 *   VCC -> 3.3V or 5V
 *   GND -> GND
 *   DO  -> D2 (PA10) - Digital Output
 *   AO  -> A0 (PA0) - Analog Output (if available)
 */

// Pin Definitions
#define HALL_DIGITAL_PIN    D2      // Digital output pin (PA10)
#define HALL_ANALOG_PIN     A0      // Analog output pin (PA0)
#define LED_BUILTIN_PIN     LED_BUILTIN  // Onboard LED (PA5)

// Variables
int digitalValue = 0;
int analogValue = 0;
bool magnetDetected = false;
bool lastMagnetState = false;

void setup() {
    // Initialize Serial Communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Pin Mode Configuration
    pinMode(HALL_DIGITAL_PIN, INPUT);
    pinMode(HALL_ANALOG_PIN, INPUT);
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    
    Serial.println("================================");
    Serial.println("Hall Magnetic Sensor Module Test");
    Serial.println("Board: NUCLEO-F103RB");
    Serial.println("================================");
    Serial.println();
    delay(1000);
}

void loop() {
    // Read Digital Value (LOW when magnet detected)
    digitalValue = digitalRead(HALL_DIGITAL_PIN);
    
    // Read Analog Value
    analogValue = analogRead(HALL_ANALOG_PIN);
    
    // Determine magnet detection status
    // Note: Most Hall sensors output LOW when magnet is detected
    magnetDetected = (digitalValue == LOW);
    
    // LED indication
    digitalWrite(LED_BUILTIN_PIN, magnetDetected ? HIGH : LOW);
    
    // Print status on state change
    if (magnetDetected != lastMagnetState) {
        Serial.println("-----------------------------");
        if (magnetDetected) {
            Serial.println(">>> MAGNET DETECTED! <<<");
        } else {
            Serial.println(">>> Magnet removed <<<");
        }
        lastMagnetState = magnetDetected;
    }
    
    // Print sensor values
    Serial.print("Digital: ");
    Serial.print(digitalValue == LOW ? "LOW (Detected)" : "HIGH (None)");
    Serial.print(" | Analog: ");
    Serial.print(analogValue);
    Serial.print(" (");
    Serial.print(map(analogValue, 0, 4095, 0, 100));
    Serial.println("%)");
    
    delay(200);
}

/*
 * Additional Functions for Advanced Usage
 */

// Calculate magnetic field strength (relative)
int getMagneticStrength() {
    int value = analogRead(HALL_ANALOG_PIN);
    // Map to percentage (adjust based on your sensor characteristics)
    return map(value, 0, 4095, 0, 100);
}

// Check if magnet is present (digital)
bool isMagnetPresent() {
    return digitalRead(HALL_DIGITAL_PIN) == LOW;
}

// Get raw analog reading
int getRawAnalogValue() {
    return analogRead(HALL_ANALOG_PIN);
}
