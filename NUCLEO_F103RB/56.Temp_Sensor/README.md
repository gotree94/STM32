# Digital Temperature Sensor KY-028

![](001.png)

* The KY-028 is a digital temperature sensor for Arduino based on an NTC thermistor.
* It measures from -55°C to 125°C with ±0.5°C accuracy, providing both a digital output (threshold-triggered) and an analog output (relative temperature reading), with a potentiometer to set the detection threshold.

* Compatible with Arduino, Raspberry Pi, ESP32, and other microcontrollers.


## KY-028 Specifications

* This module consists of an NTC thermistor, an LM393 dual differential comparator, a 3296W trimmer potentiometer, 6 resistors, 2 LEDs, and 4 male header pins. The module features analog and digital outputs.

| Operating Voltage	| 3.3V ~ 5.5V | 
|:----------------:|:----------------:|
| Temperature Measurement Range	| -55°C to 125°C [-67°F to 257°F] | 
| Measurement Accuracy	|  ±0.5°C | 
| Board Dimensions	| 15mm x 36mm [0.6in x 1.4in] | 


## Connection Diagram

* Connect the board’s analog output (A0) to pin A0 on the Arduino and the digital output (D0) to pin 3.
* Connect the power line (+) and ground (G) to 5V and GND respectively.

| KY-028	| STM32F103 | 
|:-------------:|:-------------:|
| A0	| Pin A0 | 
| G	| GND | 
| +	| +5V | 
| D0	| Pin 2 | 


# Analog Temperature Sensor KY-013 (HW-498)

![](002.png)

The KY-013 — also sold as the HW-498 — is an analog temperature sensor for Arduino built around an NTC thermistor. It’s a common part in many Arduino starter kits.

It translates ambient temperature into a continuous voltage that the Arduino reads on an analog (ADC) pin, then converts to a precise reading with the Steinhart-Hart equation.

It measures from -55°C to 125°C with ±0.5°C accuracy — unlike simpler threshold-only sensors.

Compatible with popular electronics platforms like Arduino and ESP32.

KY-013 Specifications

This module consists of a 10 kΩ NTC thermistor, a matching 10 kΩ fixed resistor (which together form a voltage divider), and a 3-pin male header. Being an NTC type, the thermistor’s resistance falls as temperature rises. The Arduino reads the divider’s output voltage on an analog pin and converts it to a temperature using the Steinhart-Hart equation (or the simpler Beta equation, using the B-value listed below).

Specification	Value
Sensor type	NTC thermistor (MF52), analog output
Operating voltage	3.3 V – 5 V
Measurement range	−55 °C to +125 °C (−67 °F to 257 °F)
Accuracy	±0.5 °C
Thermistor nominal resistance	10 kΩ at 25 °C
B-value (B25/50)	3950 K
Series (divider) resistor	10 kΩ
Interface	3-pin — S (signal), middle (+V), − (GND)
Board dimensions	~19 × 15 mm

KY-013 Pinout

The KY-013 has three pins. The one thing to remember is that the middle pin is power (VCC), not the signal — a common wiring mistake. The signal pin (S) carries the analog voltage from the NTC thermistor divider, and the – pin is ground. See the Connection Diagram below for how to wire them to an Arduino.

Label	Function
S (Signal)	Analog output — voltage from the NTC divider
Middle	Power (VCC), 3.3–5 V
–	Ground
Note: Pin labels and order vary between manufacturers and clone boards (the KY-013 is also sold as the HW-498) — always check the silk-screen on your module before wiring. The middle pin is power on every version, but the outer S and – pins are occasionally swapped; if your readings move the wrong way (falling as it warms), swap those two connections.


Connection Diagram
Connect module power line (middle) and ground (-) to 5V and GND on the Arduino respectively. Connect the module signal pin (S) to pin A0 on the Arduino.

Some KY-013 have a different pin arrangement. Please check your board before connecting.

KY-013	Arduino
S	A0
middle	5V
–	GND

An analog temperature sensor like the KY-013 translates the surrounding temperature into a continuous voltage that an Arduino can read. At the heart of the module is an NTC (Negative Temperature Coefficient) thermistor — a resistor whose resistance drops as it gets warmer. The module wires this thermistor in series with a fixed 10 kΩ resistor to form a voltage divider, so the voltage on the signal pin changes smoothly with temperature.

The full signal chain works like this:

A temperature change shifts the NTC thermistor’s resistance (lower resistance when it’s warmer).
The thermistor and the fixed 10 kΩ resistor form a voltage divider that turns that resistance into a voltage.
The Arduino’s analog-to-digital converter (ADC) reads the voltage as a 0–1023 value (0–4095 on a 12-bit ESP32 ADC).
The Steinhart-Hart equation converts that value into a temperature in °C.
Because the resistance-to-temperature relationship is non-linear, you can’t read degrees straight from the ADC — you have to apply a formula. The sketches below use the Steinhart-Hart equation with coefficients derived from the KY-013’s B25/50 = 3950 K specification, tuned for accurate readings across the full −55 °C to 125 °C range.

KY-013 Arduino Code
int ThermistorPin = A0;
int Vo;
float R1 = 10000; // value of R1 on board
float logR2, R2, T;
float c1 = 0.0010222847, c2 = 0.0002531646, c3 = 0.0; // S-H coefficients derived from MF52 B=3950 K spec
void setup() {
  Serial.begin(9600);
}
void loop() {
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0); //calculate resistance on thermistor
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // temperature in Kelvin
  T = T - 273.15; //convert Kelvin to Celsius
 // T = (T * 9.0)/ 5.0 + 32.0; //convert Celsius to Fahrenheit
  Serial.print("Temperature: ");
  Serial.print(T);
  Serial.println(" C");
  delay(500);
}
KY-013 ESP32 Code
The ESP32 sketch works the same way as the Arduino version with two adjustments: connect the signal pin to GPIO34 (or any ADC1 pin, 32–39) and power the module from 3.3 V — the ESP32's ADC reference is 3.3 V, so using 5 V will damage the pin. The 12-bit ADC reads 0–4095 instead of 0–1023, so the resistance formula uses 4095.0 as the divisor.

int ThermistorPin = 34;
int Vo;
float R1 = 10000; // value of R1 on board
float logR2, R2, T;
float c1 = 0.0010222847, c2 = 0.0002531646, c3 = 0.0; // S-H coefficients derived from MF52 B=3950 K spec

void setup() {
  Serial.begin(115200);
}

void loop() {
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (4095.0 / (float)Vo - 1.0); // 12-bit ADC (0-4095)
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // temperature in Kelvin
  T = T - 273.15; // convert Kelvin to Celsius
 // T = (T * 9.0)/ 5.0 + 32.0; // convert Celsius to Fahrenheit
  Serial.print("Temperature: ");
  Serial.print(T);
  Serial.println(" C");
  delay(500);
}
Applications
The KY-013 (HW-498) NTC thermistor module suits any project that needs a continuous analog temperature reading. Common use cases include:

Ambient temperature monitoring — log readings to Serial, an LCD, or a cloud dashboard in real time.
Weather stations — pair with a humidity and pressure sensor (DHT22, BMP280) for a full climate station.
Plant and greenhouse control — trigger ventilation fans or heating mats when temperature crosses a threshold.
Aquarium management — alert when water temperature drifts outside a safe range.
Fan and cooling control — switch a relay or adjust PWM fan speed proportionally to temperature.
Cold-chain and freezer logging — sample and store temperature over time to verify storage conditions.
Arduino starter kit projects — included in most kits as an easy first sensor; pairs well with a buzzer or LED indicator.
Troubleshooting & FAQ
What is an analog temperature sensor?
An analog temperature sensor converts ambient temperature into a continuous voltage. Unlike digital sensors that output pre-processed values over a data protocol, an analog sensor outputs a raw voltage your microcontroller reads on an ADC pin and converts using a formula. The KY-013 uses an NTC thermistor — its resistance drops as temperature rises, producing a varying voltage through a voltage divider.

Is the KY-013 the same as the HW-498?
Yes. The KY-013 and HW-498 are the same module sold under different names. Both use a 10 kΩ NTC thermistor (MF52) with a 10 kΩ series resistor and a three-pin interface. The pinout and code are identical.

What is the difference between KY-013 and DS18B20 (KY-001)?
The DS18B20 is a digital temperature sensor — sold standalone or as the KY-001 module. It communicates over the 1-Wire protocol and delivers a pre-calculated temperature value directly, no formula needed. The KY-013 is analog: it outputs a raw voltage you convert yourself using the Steinhart-Hart equation. The DS18B20 offers guaranteed ±0.5 °C accuracy from the IC, supports multiple sensors on a single pin, and suits longer cable runs; the KY-013 is simpler to wire and the better learning tool for understanding ADC and thermistor math.

What is the difference between KY-013 and LM35?
The LM35 is a precision analog IC that outputs 10 mV per °C — a simple linear formula converts voltage to temperature. The KY-013 uses an NTC thermistor whose resistance changes non-linearly, requiring the Steinhart-Hart equation. The LM35 is more accurate and easier to convert; the KY-013 covers a wider range (−55 °C to 125 °C vs 0–100 °C for the standard LM35) and is cheaper and more common in Arduino starter kits.

Why are my KY-013 temperature readings inverted or backwards?
If temperature rises when you cool the sensor, the S (signal) and – (GND) pins are swapped. Remove power, swap those two outer wires, and re-test. Some clone boards print the pin labels in the wrong order.

Why are my KY-013 readings consistently off by 1–2 °C?
Verify that R1 in the sketch matches the series resistor on your board — standard KY-013 boards use 10 kΩ, but some clones differ. Also confirm you are using the correct ADC resolution: 1023.0 for Arduino Uno, 4095.0 for ESP32. For tighter accuracy on Arduino, the 5 V rail can vary ±5%; use analogReference(EXTERNAL) with a stable reference if precision matters.

Why are my KY-013 readings noisy or jumpy?
ADC noise is normal on analog pins. Average 5–10 readings in a loop before applying the formula to smooth the output. Keep sensor leads short and away from high-current devices.

Why does my ESP32 show readings pegged near 0 or at maximum?
The ESP32 ADC reference is 3.3 V. Powering the KY-013 from 5 V drives the signal pin above 3.3 V, saturating the ADC and risking GPIO damage. Always connect VCC to the ESP32’s 3.3 V pin.

Why do the sketch coefficients differ from other KY-013 examples?
Most online examples use generic Steinhart-Hart constants not calibrated to the MF52 thermistor. The coefficients here are derived from the KY-013’s B25/50 = 3950 K specification: resistance was calculated at three temperatures (0 °C, 25 °C, 85 °C) using the Beta equation, then the Steinhart-Hart 3×3 system was solved. This improves accuracy by up to 0.5 °C at temperature extremes compared to the generic values.

Related Temperature Sensor Modules
The KY-013 is one of several temperature sensors included in 37-in-1 Arduino sensor kits. Here’s how the main options compare:

Feature	KY-013 (HW-498)	KY-001 DS18B20	KY-015 DHT11
Output	Analog voltage	Digital (1-Wire)	Digital (single-wire)
Sensor element	NTC thermistor (MF52)	Silicon bandgap IC	Capacitive humidity + thermistor
Reading method	ADC + Steinhart-Hart	1-Wire library, no formula	DHT library, no formula
Measures humidity	No	No	Yes (±5 % RH)
Accuracy	±0.5 °C	±0.5 °C	±2 °C
Range	−55 to 125 °C	−55 to 125 °C	0 to 50 °C
Multiple on one pin	No	Yes	No
Operating voltage	3.3–5 V	3.0–5.5 V	3.3–5 V
For a simple temperature threshold switch (no exact reading needed), see the KY-028 digital temperature sensor.





