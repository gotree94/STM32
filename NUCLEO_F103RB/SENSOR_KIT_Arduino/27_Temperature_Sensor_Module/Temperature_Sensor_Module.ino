/*
 * Temperature Sensor Module Test (NTC Thermistor / LM35)
 * Board: NUCLEO-F103RB (STM32F103RB)
 * Arduino IDE with STM32duino
 * 
 * 연결:
 *   VCC -> 3.3V or 5V
 *   GND -> GND
 *   DO  -> D2 (PA10) - Digital Output (threshold)
 *   AO  -> A0 (PA0) - Analog Output (temperature)
 */

// Pin Definitions
#define TEMP_DIGITAL_PIN    D2      // Digital output pin (threshold)
#define TEMP_ANALOG_PIN     A0      // Analog output pin
#define LED_BUILTIN_PIN     LED_BUILTIN  // Onboard LED (PA5)

// Constants for NTC Thermistor calculation
#define ADC_RESOLUTION      4095    // 12-bit ADC
#define V_REF               3.3     // Reference voltage
#define R_FIXED             10000   // Fixed resistor value (10K ohm)
#define THERMISTOR_NOMINAL  10000   // Thermistor resistance at 25°C
#define TEMPERATURE_NOMINAL 25      // Nominal temperature (25°C)
#define B_COEFFICIENT       3950    // Beta coefficient of thermistor

// Sensor Type Selection (uncomment one)
#define USE_NTC_THERMISTOR
// #define USE_LM35

// Variables
int digitalValue = 0;
int analogValue = 0;
float temperature = 0.0;
float temperatureF = 0.0;
bool overThreshold = false;

// Moving average filter
#define SAMPLE_COUNT 10
int samples[SAMPLE_COUNT];
int sampleIndex = 0;
long sampleSum = 0;

void setup() {
    // Initialize Serial Communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Pin Mode Configuration
    pinMode(TEMP_DIGITAL_PIN, INPUT);
    pinMode(TEMP_ANALOG_PIN, INPUT);
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    
    // Initialize sample array
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        samples[i] = 0;
    }
    
    Serial.println("================================");
    Serial.println("Temperature Sensor Module Test");
    Serial.println("Board: NUCLEO-F103RB");
    #ifdef USE_NTC_THERMISTOR
    Serial.println("Sensor: NTC Thermistor");
    #else
    Serial.println("Sensor: LM35");
    #endif
    Serial.println("================================");
    Serial.println();
    delay(1000);
}

void loop() {
    // Read Digital Value (threshold detection)
    digitalValue = digitalRead(TEMP_DIGITAL_PIN);
    
    // Read Analog Value with averaging
    analogValue = getFilteredAnalogValue();
    
    // Calculate temperature based on sensor type
    #ifdef USE_NTC_THERMISTOR
    temperature = calculateTemperatureNTC(analogValue);
    #else
    temperature = calculateTemperatureLM35(analogValue);
    #endif
    
    // Convert to Fahrenheit
    temperatureF = celsiusToFahrenheit(temperature);
    
    // Determine threshold status
    overThreshold = (digitalValue == LOW);
    
    // LED indication (on when over threshold)
    digitalWrite(LED_BUILTIN_PIN, overThreshold ? HIGH : LOW);
    
    // Print sensor values
    Serial.print("ADC: ");
    Serial.print(analogValue);
    Serial.print(" | Temp: ");
    Serial.print(temperature, 1);
    Serial.print("°C (");
    Serial.print(temperatureF, 1);
    Serial.print("°F) | Threshold: ");
    Serial.println(overThreshold ? "OVER" : "Normal");
    
    delay(500);
}

/*
 * Calculate temperature from NTC Thermistor
 * Using Steinhart-Hart equation (simplified)
 */
float calculateTemperatureNTC(int adcValue) {
    if (adcValue == 0) return -273.15; // Prevent division by zero
    
    // Calculate thermistor resistance
    float resistance = R_FIXED * ((float)ADC_RESOLUTION / adcValue - 1.0);
    
    // Steinhart-Hart equation
    float steinhart;
    steinhart = resistance / THERMISTOR_NOMINAL;          // (R/Ro)
    steinhart = log(steinhart);                           // ln(R/Ro)
    steinhart /= B_COEFFICIENT;                           // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURE_NOMINAL + 273.15);    // + (1/To)
    steinhart = 1.0 / steinhart;                          // Invert
    steinhart -= 273.15;                                  // Convert to Celsius
    
    return steinhart;
}

/*
 * Calculate temperature from LM35 sensor
 * LM35 outputs 10mV per degree Celsius
 */
float calculateTemperatureLM35(int adcValue) {
    float voltage = (adcValue * V_REF) / ADC_RESOLUTION;
    float tempC = voltage * 100.0; // 10mV/°C = 100°C/V
    return tempC;
}

/*
 * Convert Celsius to Fahrenheit
 */
float celsiusToFahrenheit(float celsius) {
    return (celsius * 9.0 / 5.0) + 32.0;
}

/*
 * Get filtered analog value using moving average
 */
int getFilteredAnalogValue() {
    // Remove old sample from sum
    sampleSum -= samples[sampleIndex];
    
    // Read new sample
    samples[sampleIndex] = analogRead(TEMP_ANALOG_PIN);
    
    // Add new sample to sum
    sampleSum += samples[sampleIndex];
    
    // Move to next index
    sampleIndex = (sampleIndex + 1) % SAMPLE_COUNT;
    
    // Return average
    return sampleSum / SAMPLE_COUNT;
}

/*
 * Get raw analog reading
 */
int getRawAnalogValue() {
    return analogRead(TEMP_ANALOG_PIN);
}

/*
 * Check if temperature is over threshold
 */
bool isOverThreshold() {
    return digitalRead(TEMP_DIGITAL_PIN) == LOW;
}
