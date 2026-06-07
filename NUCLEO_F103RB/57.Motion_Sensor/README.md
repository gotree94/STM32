# MH-SR602 PIR Motion Sensor Guide
Electronics
By
T.K. Hareendran
Updated On
May 27, 2024
A passive infrared (PIR) motion sensor allows you to sense motion, almost always used to detect whether a human has moved in or out of its range. For many do-it-yourself projects or devices that need to detect when a person has entered or left a specific area or has approached, PIR sensor is a great choice.


In these days you can buy pretty compact, inexpensive, low-power, and easy-to-use digital PIR sensors. A great example is the MH-SR602 PIR motion sensor module. It is ideal for detecting movement at a distance of 0 to 3.5 meter. The module has high sensitivity, fast response, low static energy consumption, and a small size, so it is easy to install.

MH-SR602
Quick Specifications
Model Number: SR602
Sensing Distance: Up to 5 meters (0-3.5 meters recommended)
Output: High Level (H=3.3V/L=0V)
Power Supply: 3.3V-15V DC
Quiescent Current: 20uA
Pin Data

– : 0V/GND
+: 3.3V to 15V DC INPUT
OUT: Logic H/L OUTPUT (3.3V/0V)
Key Points

The high-level (H) output time of this sensor is adjustable from 2.5 seconds to 1 hour but it is  2.5 seconds by default. You can of course change one chip resistor if you need to change the output signal duration (see below figure). Note at this point that if the default delay time is modified, the time when the sensor module outputs the high-level signal after the initial power-on will increase accordingly.

Likewise, the sensitivity of this sensor is adjustable, and you need to change a chip resistor for that. Moreover, it has an option to add an external photosensor so that after installing the photosensor, the motion sensor mechanism will not work during the day, but will work at night (detection sensitivity of the photosensor can also be altered by changing a chip resistor).

MH-SR602 MODs
Day/Night Detection
From the datasheet, the recommended photosensor for the aforesaid day/night detection is the F3 Photodiode which is a high-speed and highly sensitive PIN photodiode that comes in a standard 3mm plastic package (see below for its quick specs copied from a seller’s page).


Core Part
The actual passive infrared motion sensor on this module has 6 pins but no label to identify its part number. On a physical examination, it looks a bit like the popular BM612 smart digital motion detector.

The BM612 offers a complete motion detector solution, with all electronic circuitry built into the detector housing. Only a power supply and power-switching components need to be added to make the entire motion switch, a timer is included. It also offers optional ambient light level and sensitivity adjustments.

Test Setup

Do not forget that you do not need a fancy microcontroller to quickly test a digital PIR motion sensor module like the one demoed here.

The MH-SR602 module has a 3.3V LDO linear voltage regulator chip on board, so just follow the schematic below to start your quick tests.

MH-SR602 Basic Test Circuit
This is a very simple pulse-stretching circuit combined with a logic-level mosfet to drive an active buzzer for a predetermined minimum duration when the PIR sensor is triggered.

SR602 Quick Test
Anybody interested can tweak the circuit as desired. That is, if you are not happy with the default pulse length of my basic circuitry, you can play with the resistor and/or the capacitor to change the behaviour of the test setup.

SR602 Quick Test
