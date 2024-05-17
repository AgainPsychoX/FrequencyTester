
# Frequency tester based on ATMega8 and counters

Created for my dad.

It supports frequencies up to around 10 MHz. Something wrong with hardware/software to beat that right now.

![Image of the finished tester case](https://i.imgur.com/Nz751yE.jpeg)



## Hardware

![Schematic](https://i.imgur.com/pT2oFcy.png)

Microcontroller: [ATMega8](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2486-8-bit-AVR-microcontroller-ATmega8_L_datasheet.pdf)<br/>
Display: [SPLC780D](https://newhavendisplay.com/content/app_notes/SPLC780D.pdf) based 1x8 display. Simple custom small library used.<br/>
External Prescaler counter: [74HC93E, 4-bit fast binary ripple counter](https://www.ti.com/lit/ds/symlink/cd74hc93.pdf)

Extra transistor ([2N3903](https://www.onsemi.com/pdf/datasheet/2n3903-d.pdf)) and Schmitt's gates ([74HC132N](https://www.mouser.com/datasheet/2/308/74HC132-D-310410.pdf)) used to try to fix the signal into clearer square wave.



## Notes

Inspired by [40MHz-Frequency counter with ATmega8 on www.avr-asm-tutorial.net](https://web.archive.org/web/20231211191901/http://www.avr-asm-tutorial.net/avr_en/fcount/fcount_m8.html).

### To-do

+ Figure out why it breaks if testing frequencies above 10 MHz.
+ Calibrate battery voltage measurement better.
+ Custom `itoa` instead `sprintf`.
+ Consider rewriting to assembly, basing more on the code from the `fcount_m8` (see "inspired by" link above).
