# Digital Temperature Sensor KY-028

![](001.png)

he KY-028 is a digital temperature sensor for Arduino based on an NTC thermistor. It measures from -55°C to 125°C with ±0.5°C accuracy, providing both a digital output (threshold-triggered) and an analog output (relative temperature reading), with a potentiometer to set the detection threshold.

Compatible with Arduino, Raspberry Pi, ESP32, and other microcontrollers.


KY-028 Specifications
This module consists of an NTC thermistor, an LM393 dual differential comparator, a 3296W trimmer potentiometer, 6 resistors, 2 LEDs, and 4 male header pins. The module features analog and digital outputs.

Operating Voltage	3.3V ~ 5.5V
Temperature Measurement Range	-55°C to 125°C [-67°F to 257°F]
Measurement Accuracy	±0.5°C
Board Dimensions	15mm x 36mm [0.6in x 1.4in]


Connection Diagram
Connect the board’s analog output (A0) to pin A0 on the Arduino and the digital output (D0) to pin 3.


Connect the power line (+) and ground (G) to 5V and GND respectively.

KY-028	Arduino
A0	Pin A0
G	GND
+	+5V
D0	Pin 2


# Analog Temperature Sensor KY-028
