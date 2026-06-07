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

KY-028 Arduino Code
When the temperature threshold is reached, the digital interface will send a HIGH signal turning on the LED on the Arduino (pin 13). Turn the potentiometer clockwise to increase the detection threshold and counter-clockwise to decrease it.

The analog interface returns a numeric value that depends on the temperature and the potentiometer’s position. 

Since the analog output pin is directly connected to the potentiometer it isn’t possible to use the Steinhart-Hart equation to calculate the temperature as we did with the KY-013, we can only use this value to measure relative changes in temperature.

```
int led = 13; // define the LED pin
int digitalPin = 2; // KY-028 digital interface
int analogPin = A0; // KY-028 analog interface
int digitalVal; // digital readings
int analogVal; //analog readings

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(digitalPin, INPUT);
  //pinMode(analogPin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  // Read the digital interface
  digitalVal = digitalRead(digitalPin); 
  if(digitalVal == HIGH) // if temperature threshold reached
  {
    digitalWrite(led, HIGH); // turn ON Arduino's LED
  }
  else
  {
    digitalWrite(led, LOW); // turn OFF Arduino's LED
  }

  // Read the analog interface
  analogVal = analogRead(analogPin); 
  Serial.println(analogVal); // print analog value to serial

  delay(100);
}
```
---

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
```
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
```

KY-013 ESP32 Code
The ESP32 sketch works the same way as the Arduino version with two adjustments: connect the signal pin to GPIO34 (or any ADC1 pin, 32–39) and power the module from 3.3 V — the ESP32's ADC reference is 3.3 V, so using 5 V will damage the pin. The 12-bit ADC reads 0–4095 instead of 0–1023, so the resistance formula uses 4095.0 as the divisor.

```
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
```

## Applications
* The KY-013 (HW-498) NTC thermistor module suits any project that needs a continuous analog temperature reading. Common use cases include:

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

---

# DS18B20 Temperature Sensor KY-001

![](003.png)


The KY-001 is a DS18B20 digital temperature sensor module found in 37-in-1 and similar Arduino sensor kits (Keyes, ELEGOO, SunFounder, Joy-IT, and others). It mounts a Maxim/Dallas DS18B20 1-Wire sensor on a small breakout board together with a pull-up resistor and an indicator LED, giving you calibrated digital temperature readings over a single data line — no analog wiring or calibration required.

The DS18B20 measures from −55 °C to +125 °C with ±0.5 °C accuracy across most of that range and reports temperature straight to your microcontroller as a digital value at 9- to 12-bit resolution. The module runs on 3.0–5.5 V, so it works with 5 V boards such as the Arduino Uno and Mega as well as 3.3 V boards like the ESP32, ESP8266, and Raspberry Pi. You may also see it listed as “KY-01”.

Because the DS18B20 uses the 1-Wire protocol, every sensor carries a unique 64-bit address — so multiple KY-001 modules can share the same data pin and be read individually, which is ideal for multi-zone temperature monitoring.

The KY-001 includes an onboard pull-up resistor (typically 4.7 kΩ) on the 1-Wire data line, so a single module on a short lead needs no external resistor — unlike a bare DS18B20. You only need to add or adjust a pull-up for long cable runs or when chaining several modules on one bus, where their onboard resistors stack in parallel.

KY-001 DS18B20 Pinout

KY-001 DS18B20 Specifications

Sensor element	Dallas / Maxim DS18B20
Operating voltage	3.0 V to 5.5 V
Temperature range	−55 °C to +125 °C (−57 °F to +257 °F)
Accuracy	±0.5 °C (−10 °C to +85 °C)
Resolution	9 to 12 bit, user-configurable (default: 12 bit)
Conversion time	Up to 750 ms (at 12-bit resolution)
Active current	1 mA typ / 1.5 mA max (during temperature conversion)
Onboard components	Pull-up resistor (typically 4.7 kΩ) + indicator LED
Board dimensions	18.5 × 15 mm (0.73 × 0.59 in)
Also known as	DS18B20 module, KY-01
KY-001 Pinout
Pin	Label	Description
1	S	1-Wire signal / data
2	+	VCC — 3.0 to 5.5 V
3	−	GND
KY-001 Wiring Diagram
Connect the power line (middle) and ground (-) on the module to +5V and GND on the Arduino respectively. Connect the signal pin (S) to pin 2 on the Arduino.

KY-001	Arduino
S	Pin 2
middle	+5V
–	GND
Arduino KY-001 connection diagram
KY-001 Arduino Code

The KY-001 talks to your Arduino over the 1-Wire bus, so the sketches below use the OneWire and DallasTemperature libraries. Wire the module’s S pin to Arduino digital pin 2, + to 5 V, and − to GND.

Links to the required libraries can be found on the Downloads section below.

Basic Temperature Reading
This sketch reads a single DS18B20 once per second and prints the temperature in both Celsius and Fahrenheit. The delay(1000) keeps the 1-Wire bus from being polled continuously, and the disconnect check catches the −127 °C value returned when no sensor is found.

#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 data line (KY-001 "S" pin) on Arduino digital pin 2
#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
  // Ask every sensor on the bus for a fresh reading
  sensors.requestTemperatures();

  float tempC = sensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: no DS18B20 found (check wiring and pull-up).");
  } else {
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print(" C  |  ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.println(" F");
  }

  delay(1000); // one read per second
}
Reading Multiple DS18B20 Sensors

Because every DS18B20 carries a unique 64-bit address, you can put several on the same data pin and read them individually — ideal for multi-zone monitoring. One caveat specific to KY-001 modules: each board has its own onboard pull-up, so wiring several in parallel stacks those resistors (two 4.7 kΩ in parallel ≈ 2.35 kΩ). For more than two or three sensors, use bare DS18B20s with a single shared 4.7 kΩ pull-up instead.

The simplest approach reads each sensor by its index on the bus:

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int deviceCount = 0;

void setup() {
  Serial.begin(9600);
  sensors.begin();

  deviceCount = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(deviceCount);
  Serial.println(" sensor(s) on the bus.");
}

void loop() {
  sensors.requestTemperatures();

  for (int i = 0; i < deviceCount; i++) {
    float tempC = sensors.getTempCByIndex(i);
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(tempC);
    Serial.print(" C  |  ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.println(" F");
  }

  delay(1000);
}
Index order isn’t guaranteed between power-ups, so to reliably track a specific sensor, address it by its ROM code. Run this discovery sketch once to print each sensor’s address:

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  sensors.begin();

  int count = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(count);
  Serial.println(" device(s):");

  DeviceAddress addr;
  for (int i = 0; i < count; i++) {
    if (sensors.getAddress(addr, i)) {
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(" address = { ");
      for (uint8_t b = 0; b < 8; b++) {
        Serial.print("0x");
        if (addr[b] < 16) Serial.print("0");
        Serial.print(addr[b], HEX);
        if (b < 7) Serial.print(", ");
      }
      Serial.println(" }");
    }
  }
}

void loop() {}
Then paste the addresses into your sketch and read each sensor directly:

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Replace these with the addresses printed by the discovery sketch
DeviceAddress sensor1 = { 0x28, 0xFF, 0x64, 0x1E, 0x8C, 0x16, 0x03, 0xAA };
DeviceAddress sensor2 = { 0x28, 0xFF, 0x52, 0x1C, 0x7A, 0x11, 0x02, 0xBB };

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();

  Serial.print("Sensor 1: ");
  Serial.print(sensors.getTempC(sensor1));
  Serial.println(" C");

  Serial.print("Sensor 2: ");
  Serial.print(sensors.getTempC(sensor2));
  Serial.println(" C");

  delay(1000);
}
Setting the Resolution

The DS18B20 supports 9- to 12-bit resolution. Lower resolution converts faster but is coarser; 12-bit (the default) resolves 0.0625 °C but takes up to 750 ms per reading. Set it in setup() after sensors.begin():

sensors.setResolution(12); // 9, 10, 11, or 12 bits
Resolution	Step	Max conversion time
9-bit	0.5 °C	93.75 ms
10-bit	0.25 °C	187.5 ms
11-bit	0.125 °C	375 ms
12-bit (default)	0.0625 °C	750 ms
Troubleshooting & FAQ
The DS18B20 reads −127 °C
−127 °C is the DallasTemperature library’s DEVICE_DISCONNECTED_C sentinel — the Arduino received no valid reply from the sensor. Check that the data line is on the pin your sketch declares (#define ONE_WIRE_BUS 2 in the examples above); confirm VCC and GND are not swapped; verify the pull-up resistor is present (the KY-001’s onboard ~4.7 kΩ covers a single module); look for a cold solder joint or loose jumper; and for long cables, check that the pull-up is low enough for the line to reach a valid logic HIGH.

The DS18B20 reads a constant 85 °C
85 °C is the DS18B20’s power-on reset default — its temperature register holds +85 °C until the first conversion completes. A persistent 85 °C almost always means a power problem: an undersized supply, a brownout, or parasite power that can’t sustain the conversion current. Make sure power is stable, call requestTemperatures() and let it finish before reading (in non-blocking mode add up to 750 ms delay at 12-bit), and power the module from its + pin rather than using parasite mode.

Garbage or fluctuating readings / CRC errors
Wildly jumping values or CRC errors are signal-integrity problems, not a faulty sensor. Long or unshielded cables pick up noise and add capacitance the pull-up must overcome. For runs beyond ~1 m, use shielded or twisted-pair cable, lower the pull-up to 3.3 kΩ or 2.2 kΩ, and keep the data line away from motors, relays, and mains wiring.

How many DS18B20 sensors can share one data line?
Many — the DS18B20 supports a 1-Wire bus with multiple sensors on a single pin, each addressed by its unique 64-bit ROM code (see the Reading Multiple DS18B20 Sensors section above). For KY-001 modules, keep it to two or three per bus: each module has its own onboard pull-up, and paralleling them lowers total resistance until the bus stops working reliably. For larger arrays, use bare DS18B20s with a single shared 4.7 kΩ pull-up. If one sensor reads but others don’t, address by ROM code rather than by index — index order is not guaranteed across power-ups.

How do I convert the DS18B20 reading to Fahrenheit?
The DallasTemperature library includes a static helper: DallasTemperature::toFahrenheit(tempC). Pass your Celsius float to it and it returns the equivalent Fahrenheit value. The basic sketch in the code section above already uses this.

What is the DS18B20’s default resolution and how do I change it?
The default is 12-bit (0.0625 °C steps, up to 750 ms conversion time). See the Setting the Resolution section above for the full step-by-step and a conversion-time table comparing 9-bit through 12-bit.

Which Arduino sensor kit includes the KY-001?
The KY-001 is included in the popular 37-in-1 Arduino sensor kit, available from several manufacturers including Keyes, ELEGOO, SunFounder, and Joy-IT. The exact modules in each kit can vary by seller and edition, so check the contents list before buying.

Does the KY-001 work with ESP32, ESP8266, and Raspberry Pi?
Yes — no level shifting needed. The module runs on 3.0–5.5 V, so it works with 3.3 V boards like the ESP32 and ESP8266 as well as 5 V Arduino boards. Connect S to any GPIO, + to 3.3 V or 5 V, and − to GND. The same OneWire and DallasTemperature libraries run on ESP32 and ESP8266. On Raspberry Pi, enable the w1-gpio overlay in /boot/config.txt or use a Python library such as w1thermsensor.

What is the difference between the DS18B20 (KY-001) and the DHT22 or LM35?
The DS18B20 measures temperature only (−55 to +125 °C, ±0.5 °C accuracy), uses a 1-Wire digital output, and supports multiple sensors on one pin — making it the best choice for temperature-only readings over long distances or from several points at once. The DHT22 also reads humidity but cannot be daisy-chained on one pin. The LM35 outputs an analog voltage proportional to temperature: simpler to wire, but loses precision over long cable runs and ties up an ADC pin.

KY-001 Applications

The DS18B20’s digital output and multi-sensor capability make the KY-001 a good fit for projects that need accurate, reliable temperature readings over a single wire.

Thermostat and HVAC control — monitor ambient temperature and trigger a relay or fan to maintain a set point
3D printer temperature monitoring — log enclosure or bed temperature to keep prints consistent across long jobs
Aquarium and terrarium management — track water or air temperature and trigger heaters or coolers automatically
Home brewing and sous-vide — maintain precise fermentation or cooking temperatures where ±0.5 °C accuracy matters
Multi-zone temperature logging — chain several DS18B20 sensors on one data line to monitor multiple rooms, pipes, or enclosures from a single Arduino pin
Weather station and data logging — log outdoor temperature to an SD card or IoT platform alongside other environmental sensors
Related Temperature Sensor Modules
If the KY-001 doesn’t suit your project, here are the other temperature sensor modules in the KY series.

Module	Sensor	Output	Range	Accuracy
KY-001 (this page)	DS18B20	Digital temperature value (1-Wire)	−55 to +125 °C	±0.5 °C
KY-013	NTC thermistor	Analog voltage	−55 to +125 °C	±0.5 °C
KY-028	NTC + LM393	Digital threshold trigger — not a temperature value	—	—
KY-015	DHT11	Digital temperature + humidity	0 to +50 °C	±2 °C
See also: KY-013 Analog Temperature Sensor, KY-028 Digital Temperature Sensor, and KY-015 DHT11 Temperature & Humidity Sensor.


* https://arduinomodules.info/

