# <img src="https://github.com/mkulesh/avrDigitalClock/blob/master/images/atmega644.jpg" align="center" height="48" width="48"> "A digital clock based on ATmega644 MCU"

This repository provides hardware layout and firmware for a digital clock based on ATmega644 MCU
![In operation](https://github.com/mkulesh/avrDigitalClock/blob/master/images/in_operation.jpg)

## Hardware design
This clock consist of two boards:
- the first display board contains four 0.8 inch. single digit numerical displays and one 2x16 chars LCD-display.
![Display board](https://github.com/mkulesh/avrDigitalClock/blob/master/images/display_board2.jpg)

- the second board contains ATmega644 MCU with necessary periphery: voltage regulator, light sensor, temperature sensor, buttons, programming connector, 32.7680kHz crystal, DCF77 receiver with ferrite core.
![Mcu board](https://github.com/mkulesh/avrDigitalClock/blob/master/images/mcu_board1.jpg)

PCB are developed in Eagle CAD (see directory pcb):
![Mcu board layout](https://github.com/mkulesh/avrDigitalClock/blob/master/images/mcu_board0.png)
![Display board layout](https://github.com/mkulesh/avrDigitalClock/blob/master/images/display_board0.png)

## Firmware
The firmware is written in C++ in Atmel Studio 6.0 (see directory src). The main idea here is to collect low-level routines in an object-oriented library and develop the main logic of the application using objects that implements different hardware elements.

## List of components
- DC connector: 1x DC-8N
- voltage regulators: 2x LD1117S33CTR
- temperature sensor: 1x LMT86LP
- voltage reference: 1x TL431CL3T
- 32.7680kHz crystal: 1x EuroQuartz PH32768X
- DCF77 receiver: 1x ELV Elektronik DCF-2 with ferrite core
- LED 3mm: 4x
- light sensor: 1v VT935G-B
- MCU ATMEGA644PA-AU TQFP-44
- transistors: 1xBC337-40, 1x J105
- piezo buzzer: 1x RMP-14SP
- buttons: 4x D6X
- shift register: 4x SN74HC595DR
- digital/analog converter: 1x MCP4901
- 7-segment display: 4x SC08-11SRWA
- LCD-display: 1x DOG-M162 with back-light (i.e EA LED55X31-R)

## There are some known problems in this project
- DCF-2 works really bad (instable signal): ether ferrite core or software problem
- Resting in Eagle design rules is invalid
- Wrong position of the DC connector: it shall be placed on bottom instaead of top of board
- voltage of 7-segment display and LCD back-light is regulated using MCP4901 and J105: regulation interal is too short

## License

This software is published under the *GNU General Public License, Version 3*

Copyright (C) 2014-2017 Mikhail Kulesh

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.

If not, see [www.gnu.org/licenses](http://www.gnu.org/licenses).
