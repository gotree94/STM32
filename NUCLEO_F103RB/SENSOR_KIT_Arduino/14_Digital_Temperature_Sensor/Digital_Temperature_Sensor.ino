/*
 * Digital Temperature Sensor Module Test Code
 * Board: NUCLEO-F103RB (STM32F103RBT6)
 * IDE: Arduino IDE with STM32 Core
 * 
 * 디지털 온도 센서 모듈 테스트 (DS18B20)
 * - 1-Wire 프로토콜 사용
 * - 측정 범위: -55°C ~ +125°C
 * - 분해능: 9~12bit (기본 12bit: 0.0625°C)
 * - 정확도: ±0.5°C (-10°C ~ +85°C)
 * 
 * Wiring:
 * - VCC -> 3.3V or 5V
 * - GND -> GND
 * - DATA -> D2 (PA10) with 4.7k pull-up resistor
 * 
 * Required Libraries:
 * - OneWire by Paul Stoffregen
 * - DallasTemperature by Miles Burton
 */

#include <OneWire.h>
#include <DallasTemperature.h>

// Pin Definitions
#define ONE_WIRE_BUS  D2    // Data Pin (PA10)
#define LED_PIN       LED_BUILTIN  // Onboard LED (PA5)

// OneWire & DallasTemperature setup
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Device address
DeviceAddress sensorAddress;

// Variables
float temperatureC = 0.0;
float temperatureF = 0.0;
float minTemp = 1000.0;
float maxTemp = -1000.0;
unsigned long readCount = 0;
unsigned long previousMillis = 0;
const long interval = 1000;  // 1초 간격으로 측정

// Temperature alarm
const float TEMP_HIGH_ALARM = 30.0;  // 고온 경보 임계값
const float TEMP_LOW_ALARM = 10.0;   // 저온 경보 임계값
bool alarmActive = false;

void setup() {
  // Serial Communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("========================================");
  Serial.println("  Digital Temperature Sensor Test");
  Serial.println("  Sensor: DS18B20");
  Serial.println("  Board: NUCLEO-F103RB");
  Serial.println("========================================");
  Serial.println();
  
  // Pin Mode Setup
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize DS18B20
  sensors.begin();
  
  // Get sensor count
  int deviceCount = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(deviceCount);
  Serial.println(" DS18B20 sensor(s)");
  
  if (deviceCount == 0) {
    Serial.println("ERROR: No DS18B20 sensor found!");
    Serial.println("Check wiring and 4.7k pull-up resistor.");
    while (1) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(200);
    }
  }
  
  // Get sensor address
  if (sensors.getAddress(sensorAddress, 0)) {
    Serial.print("Sensor Address: ");
    printAddress(sensorAddress);
    Serial.println();
  }
  
  // Set resolution (9, 10, 11, or 12 bits)
  sensors.setResolution(sensorAddress, 12);
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(sensorAddress));
  Serial.println(" bits");
  
  Serial.println();
  Serial.println("Temperature alarm thresholds:");
  Serial.print("  High: ");
  Serial.print(TEMP_HIGH_ALARM);
  Serial.println("°C");
  Serial.print("  Low: ");
  Serial.print(TEMP_LOW_ALARM);
  Serial.println("°C");
  Serial.println();
  Serial.println("Starting measurements...");
  Serial.println();
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Request temperature conversion
    sensors.requestTemperatures();
    
    // Read temperature
    temperatureC = sensors.getTempC(sensorAddress);
    
    // Check for read error
    if (temperatureC == DEVICE_DISCONNECTED_C) {
      Serial.println("ERROR: Sensor disconnected!");
      digitalWrite(LED_PIN, HIGH);
      return;
    }
    
    temperatureF = sensors.toFahrenheit(temperatureC);
    readCount++;
    
    // Update min/max
    if (temperatureC < minTemp) minTemp = temperatureC;
    if (temperatureC > maxTemp) maxTemp = temperatureC;
    
    // Check alarm conditions
    checkAlarm();
    
    // Print temperature data
    printTemperatureData();
  }
}

void checkAlarm() {
  if (temperatureC > TEMP_HIGH_ALARM || temperatureC < TEMP_LOW_ALARM) {
    alarmActive = true;
    digitalWrite(LED_PIN, HIGH);
  } else {
    alarmActive = false;
    digitalWrite(LED_PIN, LOW);
  }
}

void printTemperatureData() {
  Serial.print("[");
  Serial.print(readCount);
  Serial.print("] ");
  
  // Celsius
  Serial.print("Temp: ");
  Serial.print(temperatureC, 2);
  Serial.print("°C");
  
  // Fahrenheit
  Serial.print(" (");
  Serial.print(temperatureF, 2);
  Serial.print("°F)");
  
  // Min/Max
  Serial.print(" | Min: ");
  Serial.print(minTemp, 1);
  Serial.print("°C");
  Serial.print(" Max: ");
  Serial.print(maxTemp, 1);
  Serial.print("°C");
  
  // Alarm status
  if (alarmActive) {
    Serial.print(" | ⚠ ALARM!");
  }
  
  // Visual bar graph
  Serial.print(" |");
  printTempBar(temperatureC);
  Serial.println("|");
}

void printTempBar(float temp) {
  // Map temperature 0-50°C to bar width 0-20
  int barWidth = constrain(map((int)(temp * 10), 0, 500, 0, 20), 0, 20);
  
  for (int i = 0; i < 20; i++) {
    if (i < barWidth) {
      if (temp > TEMP_HIGH_ALARM) {
        Serial.print("!");  // High temp indicator
      } else if (temp < TEMP_LOW_ALARM) {
        Serial.print("*");  // Low temp indicator
      } else {
        Serial.print("=");  // Normal
      }
    } else {
      Serial.print(" ");
    }
  }
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(":");
  }
}
