RC signal (PWM) driven load switch
==================================

## Description

The device is using small mcu (cheapest possible MSP430FR2000) to decode RC signal and switch on the load NMOS tranzistor.
PCB has no connectors. The servo (3 wires), battery (2 wires) and load (2 wires) cables are soldered directly to solder pads to save space and wheight.
The signal treshold and hysteresis is set in the code (see [main.c](/fw/main.c)). The polarity switching (high or low) can be set by soldering bridge marked "NEG".

## Hardware

Schematic:

![Schematic](/doc/schematic.png)

Assembled printed circuit (visualisation):

![PCBA top](/doc/top.png)
![PCBA bottom](/doc/bottom.png)
