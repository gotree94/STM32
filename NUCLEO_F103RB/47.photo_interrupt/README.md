# Photo Interrupt Projects for STM32F103

<img src="004.png"> <img src="006.png" width="23%">

<img src="007.png"> 

## Description
  * The KY-010 Photo Interrupter module is a switch that will trigger a signal when the light between the sensor’s gap is blocked.
  * This module is suitable for various electronic platforms like Arduino, Raspberry Pi, ESP32, and others.

## Specifications:
  * This module consists of an optical emitter/detector and 3 male header pins on the front. On the back, there are two resistors of 10kΩ and 33Ω.
  * The sensor uses a beam of light between the emitter and detector to check if the path between both is being blocked by an opaque object.

| Operating Voltage	| Board Dimensions | 
|:--------:|:--------:|
| 3.3V ~ 5V	|  18.5mm x 15mm | 

## Connection Diagram
   * Connect the power line (middle) and ground (left) to +5V and GND respectively. Connect signal (S) to pin 3 on the Arduino.

| KY-010	| Arduino | 
|:--------:|:--------:|
| – (left)	| GND | 
| middle	| +5V | 
| S (right)	| Pin 3 | 
