/*
 * Analog Light Sensor Module Test (Photoresistor / LDR)
 * Board: NUCLEO-F103RB (STM32F103RB)
 * Arduino IDE with STM32duino
 * 
 * 연결:
 *   VCC -> 3.3V or 5V
 *   GND -> GND
 *   DO  -> D2 (PA10) - Digital Output (threshold)
 *   AO  -> A0 (PA0) - Analog Output (light level)
 */

// Pin Definitions
#define LIGHT_DIGITAL_PIN   D2      // Digital output pin (threshold)
#define LIGHT_ANALOG_PIN    A0      // Analog output pin
#define LED_BUILTIN_PIN     LED_BUILTIN  // Onboard LED (PA5)

// Constants
#define ADC_RESOLUTION      4095    // 12-bit ADC
#define V_REF               3.3     // Reference voltage

// Light level thresholds (adjust based on your environment)
#define DARK_THRESHOLD      500     // Below this = Dark
#define DIM_THRESHOLD       1500    // Below this = Dim
#define NORMAL_THRESHOLD    2500    // Below this = Normal
#define BRIGHT_THRESHOLD    3500    // Below this = Bright, above = Very Bright

// Variables
int digitalValue = 0;
int analogValue = 0;
int lightPercentage = 0;
bool isDark = false;
String lightLevel = "";

// Moving average filter
#define SAMPLE_COUNT 10
int samples[SAMPLE_COUNT];
int sampleIndex = 0;
long sampleSum = 0;
bool samplesInitialized = false;

void setup() {
    // Initialize Serial Communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Pin Mode Configuration
    pinMode(LIGHT_DIGITAL_PIN, INPUT);
    pinMode(LIGHT_ANALOG_PIN, INPUT);
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    
    // Initialize sample array
    initializeSamples();
    
    Serial.println("================================");
    Serial.println("Analog Light Sensor Module Test");
    Serial.println("Board: NUCLEO-F103RB");
    Serial.println("================================");
    Serial.println();
    delay(1000);
}

void loop() {
    // Read Digital Value (threshold detection)
    digitalValue = digitalRead(LIGHT_DIGITAL_PIN);
    
    // Read Analog Value with averaging
    analogValue = getFilteredAnalogValue();
    
    // Calculate light percentage
    lightPercentage = map(analogValue, 0, ADC_RESOLUTION, 0, 100);
    
    // Determine light level category
    lightLevel = getLightLevelString(analogValue);
    
    // Determine dark status from digital pin
    isDark = (digitalValue == LOW);
    
    // LED indication (on when dark)
    digitalWrite(LED_BUILTIN_PIN, isDark ? HIGH : LOW);
    
    // Print sensor values
    printSensorData();
    
    // Print light level bar
    printLightBar(lightPercentage);
    
    delay(300);
}

/*
 * Initialize samples array with current readings
 */
void initializeSamples() {
    int initialValue = analogRead(LIGHT_ANALOG_PIN);
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        samples[i] = initialValue;
    }
    sampleSum = initialValue * SAMPLE_COUNT;
    samplesInitialized = true;
}

/*
 * Get filtered analog value using moving average
 */
int getFilteredAnalogValue() {
    // Remove old sample from sum
    sampleSum -= samples[sampleIndex];
    
    // Read new sample
    samples[sampleIndex] = analogRead(LIGHT_ANALOG_PIN);
    
    // Add new sample to sum
    sampleSum += samples[sampleIndex];
    
    // Move to next index
    sampleIndex = (sampleIndex + 1) % SAMPLE_COUNT;
    
    // Return average
    return sampleSum / SAMPLE_COUNT;
}

/*
 * Get light level as string
 */
String getLightLevelString(int value) {
    if (value < DARK_THRESHOLD) {
        return "Dark";
    } else if (value < DIM_THRESHOLD) {
        return "Dim";
    } else if (value < NORMAL_THRESHOLD) {
        return "Normal";
    } else if (value < BRIGHT_THRESHOLD) {
        return "Bright";
    } else {
        return "Very Bright";
    }
}

/*
 * Print sensor data to serial
 */
void printSensorData() {
    Serial.print("ADC: ");
    Serial.print(analogValue);
    Serial.print(" | Light: ");
    Serial.print(lightPercentage);
    Serial.print("% | Level: ");
    Serial.print(lightLevel);
    Serial.print(" | Digital: ");
    Serial.println(isDark ? "DARK" : "LIGHT");
}

/*
 * Print visual light bar
 */
void printLightBar(int percentage) {
    Serial.print("[");
    int bars = percentage / 5; // 20 characters max
    for (int i = 0; i < 20; i++) {
        if (i < bars) {
            Serial.print("█");
        } else {
            Serial.print("░");
        }
    }
    Serial.print("] ");
    Serial.print(percentage);
    Serial.println("%");
    Serial.println();
}

/*
 * Get voltage from analog reading
 */
float getVoltage() {
    return (analogValue * V_REF) / ADC_RESOLUTION;
}

/*
 * Get raw analog reading
 */
int getRawAnalogValue() {
    return analogRead(LIGHT_ANALOG_PIN);
}

/*
 * Check if environment is dark
 */
bool isEnvironmentDark() {
    return digitalRead(LIGHT_DIGITAL_PIN) == LOW;
}

/*
 * Get light level as integer (0-4)
 * 0 = Dark, 1 = Dim, 2 = Normal, 3 = Bright, 4 = Very Bright
 */
int getLightLevelInt() {
    int value = getFilteredAnalogValue();
    if (value < DARK_THRESHOLD) return 0;
    if (value < DIM_THRESHOLD) return 1;
    if (value < NORMAL_THRESHOLD) return 2;
    if (value < BRIGHT_THRESHOLD) return 3;
    return 4;
}
