# Linear Hall Sensor Projects for STM32F103

<img src="linear_hall_sensor.png"> <img src="010.png" width="47%">

<img src="011.png">

<img src="005.gif">

| Digital Output | Linear/Analog Output |
|:--------------------:|:--------------------:|
| <img src="scope_1.png"> |  <img src="scope_2.png"> | 

* 측정할때 위치는 센서를 옆으로 편상태에서 윗면에 자성을 인가해야 함.

## 1. 핵심 원리: 홀 효과 (Hall Effect)
   * 도체나 반도체에 전류가 흐르고 있는 상태에서, 수직 방향으로 자기장($B$)이 가해지면 전하(전자)가
   * 한쪽으로 쏠리는 현상이 발생합니다.
   * 이때 양단에 발생하는 전압차를 **'홀 전압($V_H$)'**이라고 하며,
   * 선형 홀 센서는 이 전압을 측정합니다.

## 2. 주요 특징 및 출력 방식
   * 선형 홀 센서(예: AH3503, SS49E 등)는 일반적인 '홀 스위치'와 출력이 다릅니다.
   * 선형 출력 (Linear Output): 자석이 가까워질수록 출력 전압이 매끄럽게 변합니다. 자석의 N극이 오는지 S극이 오는지에 따라 전압이 기준치보다 높아지거나 낮아집니다.
   * 정지 전압 (Null Voltage): 자기장이 없을 때 대략 전원의 절반(예: 5V 입력 시 2.5V) 정도의 전압을 유지합니다.
   * 비접촉 측정: 물리적 마찰 없이 위치나 속도를 측정할 수 있어 수명이 매우 깁니다.

## 3. 회로 구성 (주요 포인트)
* 보통 KY-024(선형 홀 모듈) 같은 형태의 회로로 많이 사용되며, 구성은 터치 센서와 유사합니다.
  1. Hall Element: 자기장을 감지하는 소자입니다.
  2. Op-Amp (LM393): * 아날로그 출력 (A0): 홀 소자에서 나온 증폭된 전압을 그대로 보냅니다. 자석과의 거리를 측정할 때 사용합니다.
     * 디지털 출력 (D0): 특정 자계 강도를 넘었을 때 'On/Off' 신호만 보냅니다. (가변저항으로 기준치 설정 가능)
     *
## 4. 활용 사례거리 측정: 
   * 자석과의 거리에 따른 전압 변화를 이용해 정밀한 위치 파악.
   * 전류 측정: 전선 주위에 발생하는 자기장을 측정하여 비접촉식으로 전류량을 계산 (ACS712 등).
   * 속도 제어: 전동 킥보드나 전기 자전거의 스로틀(가속 손잡이) 내부에 자석과 함께 위치하여 가속도를 조절.
   * 진동 및 기울기: 미세한 자기 변화를 포착하여 기기의 떨림을 감지.

## 5. 홀 효과(Hall Effect) 센서의 응용 분야

* 홀 효과 센서는 다양한 산업 분야에서 폭넓게 활용되고 있습니다.

### 1. 자동차 산업 (Automotive Industry)
* 휠 속도 센서 (ABS 제동 시스템)
* 스로틀 위치 센서 (Throttle Position Sensor)
* 엔진 타이밍 센서 (Engine Timing Sensor)

<img src="012.png">

### 2. 산업용 응용 분야 (Industrial Applications)
* 근접 감지 (Proximity Sensing)
* 컨베이어 벨트 속도 측정
* 비접촉식 전류 측정 (Contactless Current Measurement)

### 3. 소비자 전자기기 (Consumer Electronics)
* 스마트폰 (플립 커버 감지)
* 키보드 (기계식 키보드의 키 입력 감지)
* 게임 컨트롤러

### 4. 의료 기기 (Medical Devices)
* 자기공명영상장치(MRI, Magnetic Resonance Imaging)
* 생체의료 센서 (근접 기반 의료 도구)

### 5. 항공우주 및 방위 산업 (Aerospace and Defense)
* 자기장 측정 (Magnetic Field Measurement)
* 우주선 자세 제어 (Spacecraft Attitude Control)
* 정밀 항법 시스템 (Precision Navigation)

----

# Analog Hall Sensor KY-035


The KY-035 is an analog hall sensor for Arduino that detects magnetic fields and measures their polarity and relative strength using a 49E linear Hall-effect sensor. Unlike KY-024, it has a single analog output and no potentiometer — simpler wiring for applications that only need analog magnetic readings.

When no magnetic field is detected, the output signal is roughly half of the applied input voltage. When a magnetic field is detected, the output signal goes up or down depending on the polarity and proximity of the magnet.

This module offers a more detailed proximity reading than the KY-003, a digital magnetic sensor that looks very similar to this module. It is also functionally similar to the KY-024, a digital/analog magnetic sensor.

Compatible with popular microcontroller boards like Arduino, ESP32 and ESP8266. Rapsberry Pi users will need an external analog-to-digital board to use this module.

KY-035 Analog Hall Magnetic Sensor Module Fritzing part
KY-035 Analog Hall Magnetic Sensor Module
KY-035 Specifications

This module is very simple, it consists of a 49E Linear Hall-Effect sensor and 3 male header pins.

Operating Voltage	2.7V to 6V
Power Consumption	~ 6mA
Sensitivity	1.4 to 2.0mV/GS
Operating Temperature	-40 °C to 85 °C [-40 °F to 185 °F]
Board Dimensions	18.5mm x 15mm [0.728in x 0.591in]
Connection Diagram
Connect the KY-035’s power line (middle) and ground (-) to +5 and GND on the Arduino respectively.


Connect signal (S) to pin A0 on the Arduino.

KY-035	Arduino
S	Pin A0
middle	5V
–	GND
Arduino KY-035 Analog Hall Magnetic Sensor connection diagram
KY-035 Arduino Code

The following Arduino sketch will read values from the module. The output signal starts at an initial value determined by the input voltage.

The value will decrease when a magnetic field with the same polarity is detected, if the polarity is inverted, the value will increase.

int sensorPin = A0;   // interface pin with magnetic sensor
int val;              // variable to store read values

void setup() {
  pinMode(A0, INPUT);   // set analog pin as input
  Serial.begin(9600);   // initialize serial interface
}

void loop() {
  val = analogRead(sensorPin);  // read sensor value
  Serial.println(val);          // print value to serial

  delay(100);  
}
Use Tools > Serial Plotter on the Arduino IDE to visualize changes on intensity and polarity of the magnetic field.



---

# Hall Effect Sensor KY-003 

The KY-003 is a hall effect magnetic sensor module for Arduino built around the A3144 unipolar hall switch. It produces a clean digital output — the signal pin goes LOW when a magnet’s south pole faces the marked side of the chip — making it ideal for proximity detection, RPM/speed measurement and position sensing. The KY-003 is also included in popular 37-in-1 Arduino sensor kits from brands such as Elegoo and Keyestudio.

It runs at 4.5–24 V (5 V is typical on Arduino) and pairs with Arduino Uno, Mega and Nano as well as ESP32/ESP8266 — though note 3.3 V is below the A3144’s rated minimum (see how it works). The KY-003 (also written KY003) gives a simple on/off reading; for an analog field-strength reading use the KY-035, and for a digital/analog combo the KY-024.

This guide covers the KY-003 pinout, wiring diagram, working code examples, an RPM/tachometer project, magnet-polarity troubleshooting, and a Fritzing part download.

KY-003 Fritzing custom part image
KEYES KY-003 Hall magnetic sensor module for arduino
KY-003 Specifications
The KY-003 module is built around the A3144 (also marked 3144EUA-S, A3144E, AH3144) unipolar hall-effect switch, a 680 Ω resistor, a power LED, and 3 male header pins. Board dimensions: 18.5 × 15 mm.

Parameter	Value
Sensor IC	A3144 (A3144E / AH3144 / OH3144 / 3144EUA-S)
Sensor type	Unipolar hall-effect switch
Output	Open-collector NPN, active LOW, non-latching
Operating voltage	4.5–24 V (5 V typical; 3.3 V is below rated spec)
Trigger	South pole of magnet to branded/marked face of IC
On-board components	A3144 + 680 Ω resistor + LED
Operating temperature	−40 to 85 °C
Board dimensions	18.5 × 15 mm
KY-003 Pinout
The KY-003 has three 2.54 mm header pins. The signal pin is marked S; the middle pin is VCC and the – pin is ground.

Label	Function	Connect to
– (left)	Ground	GND
middle	VCC	+5 V
S (right)	Signal — open-collector, LOW when a magnet is detected	Any digital pin (pin 3 on Uno)
Connection Diagram
Connect the board power line (middle) and ground (-) to +5 and GND on the Arduino respectively.

Connect signal (S) to pin 3 on the Arduino.

KY-003	Arduino
S	Pin 3
middle	+5V
–	GND
Arduino KY-003 connection diagram
How the KY-003 Works
Inside the KY-003 is the A3144, a unipolar hall-effect switch. When the magnetic field at the IC exceeds its threshold, an internal transistor switches on and pulls the signal pin LOW. Unipolar means it responds to one pole only — specifically the south pole presented to the marked face of the chip. Approach with the north pole, or from the back face, and the output stays HIGH. This is the single most common reason a KY-003 appears not to work.

The output is open-collector: the A3144 can only pull the pin LOW; it cannot drive it HIGH on its own. That is why the code uses INPUT_PULLUP — the Arduino’s internal pull-up holds the pin HIGH when no magnet is present, giving a clean reading in both states. The A3144 also includes a Schmitt trigger with built-in hysteresis, so the output does not chatter as a magnet slowly crosses the edge of the detection field.

The KY-003 is non-latching: the output returns HIGH as soon as the magnetic field drops below the release threshold. Detection range depends on magnet strength; a typical small magnet trips the sensor within a few millimetres to ~10 mm.

KY-003 Arduino Code
The examples below go from a simple on/off detector to interrupt-driven magnet counting and RPM measurement. All use INPUT_PULLUP for a reliable signal from the A3144’s open-collector output.

Basic — Detect a Magnet
Lights the built-in LED on pin 13 when the south pole of a magnet is near the sensor. The signal pin goes LOW when a magnet is detected — INPUT_PULLUP ensures a clean HIGH when there is none.

const int sensor = 3;
const int led = 13;

void setup() {
  // A3144 output is open-collector — INPUT_PULLUP gives a clean HIGH when no magnet is present
  pinMode(sensor, INPUT_PULLUP);
  pinMode(led, OUTPUT);
}

void loop() {
  int val = digitalRead(sensor);
  if (val == LOW) { // south pole of magnet near A3144 marked face → output LOW
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }
}
Count Magnet Passes
Uses an interrupt on pin 3 so no magnet pass is missed, even in a busy loop. Open the Serial Monitor at 9600 baud to see the running count.

const int sensor = 3;
volatile int count = 0; // volatile: modified inside the ISR, read in the main loop

void onMagnet() {
  count++; // called instantly each time the signal falls LOW (magnet detected)
}

void setup() {
  Serial.begin(9600);
  pinMode(sensor, INPUT_PULLUP);
  // Pin 3 = interrupt 1 on Uno/Nano
  // FALLING triggers when the pin goes HIGH→LOW, i.e. when the magnet arrives
  attachInterrupt(digitalPinToInterrupt(sensor), onMagnet, FALLING);
}

void loop() {
  // Print the running total every 500 ms
  // The interrupt keeps counting in the background regardless of this delay
  Serial.print("Magnet passes: ");
  Serial.println(count);
  delay(500);
}
Measure RPM
Counts pulses over a one-second window and converts to RPM. Mount one magnet on the rotating wheel; increase magnetsPerRev if you add more. The pulse count is read atomically with noInterrupts() to avoid a race condition with the ISR.

const int sensor = 3;
const int magnetsPerRev = 1; // increase if you mount multiple magnets on the wheel
volatile int pulses = 0;

void onPulse() {
  pulses++;
}

void setup() {
  Serial.begin(9600);
  pinMode(sensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensor), onPulse, FALLING);
}

void loop() {
  // Snapshot and reset the pulse count atomically
  noInterrupts();
  int count = pulses;
  pulses = 0;
  interrupts();

  // RPM = pulses-per-second × 60 ÷ magnets-per-revolution
  float rpm = (count / (float)magnetsPerRev) * 60.0;
  Serial.print("RPM: ");
  Serial.println(rpm);
  delay(1000); // 1-second measurement window
}
KY-003 Applications
Proximity and presence detection — sense when a magnet comes within range without contact, for example to detect a passing object on a conveyor or a closed lid.
RPM and speed measurement — mount one or more magnets on a rotating shaft or wheel and count pulses per second to calculate speed (see the RPM example above).
Position and end-stop sensing — detect when a moving part reaches a set point, such as axis limit switches on a 3D printer or CNC machine.
Door and lid security — pair a magnet on the door with the sensor on the frame to detect open/closed state and trigger an alarm or log an event.
Flow-meter pulse counting — a magnet on a turbine blade triggers the sensor on each rotation, giving a pulse count proportional to flow rate.
Brushless motor commutation — hall-effect sensors are used in BLDC motor controllers to detect rotor position and time the switching sequence.
Troubleshooting & FAQ
How does the KY-003 detect a magnet?
The KY-003 uses the A3144, a unipolar hall-effect switch. When a magnetic field exceeding the IC threshold is present, an internal transistor pulls the signal pin LOW. Unipolar means it responds to one pole only — the south pole presented to the marked face of the chip. If the sensor never triggers, flip the magnet: presenting the wrong pole has no effect.

What type of output does the A3144 have?
The A3144 has an open-collector output — it can pull the signal pin LOW but cannot drive it HIGH on its own. This is why INPUT_PULLUP is needed in the sketch: the Arduino internal pull-up holds the pin HIGH when no magnet is present, giving a clean reading in both states. If the output sits LOW or reads randomly with no magnet near it, the pull-up is missing — set pinMode(sensor, INPUT_PULLUP).

What voltage does the KY-003 require?
The A3144 is rated for 4.5–24 V, with 5 V being typical on Arduino. Running the module at 3.3 V (ESP32/ESP8266) is below the rated minimum and operation is not guaranteed — if nothing happens on a 3.3 V board, power the module from a 5 V rail and level-shift the signal line if your MCU GPIO is not 5 V tolerant.

What is the difference between the KY-003, KY-035 and KY-024?
The KY-003 gives a simple digital on/off output — LOW when a magnet is detected, HIGH otherwise. The KY-035 gives an analog output proportional to field strength, for when you need to measure how strong the field is. The KY-024 provides both a digital and an analog output on the same board.

Can the KY-003 measure RPM or speed?
Yes. Mount one or more magnets on a rotating shaft or wheel and use attachInterrupt(FALLING) to count pulses per second. RPM = (pulses per second × 60) ÷ magnets per revolution. Use an interrupt rather than a polled digitalRead loop — a polled loop can miss pulses when the magnet passes quickly. See the Measure RPM example above for working code.

Is the KY-003 output digital or analog?
Digital only. The A3144 is a Schmitt trigger switch — the output is either HIGH or LOW, never a value in between. For a continuous analog reading proportional to field strength, use the KY-035 instead.

The detection range is too short
The trigger distance depends entirely on magnet strength. A weak or small magnet may only trip the sensor at 1–2 mm. Use a stronger magnet, or bring it closer to the marked face of the IC.





