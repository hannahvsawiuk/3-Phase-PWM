# 3 Phase PWM

## Project Specification

### Brief description

 A **fixed frequency** three phase PWM generation code for an AVR ATMEGA328P/-PU microcontroller and schematic for a
variable frequency drive system for an AC induction motor. The schematic for the system consists of an AVR ATMEGA328P-PU controller, SI8234 isolated gate driver, and transistors protected with diodes and bootstrap circuits.

#### Important note regaring variable frequency logic

Variable frequency was attempted but the implementation logic was not sound (I had just finished 2nd-year electrical engineering and most of the project involved subjects that were new to me). Some additions and changes were made, but ultimately a new lookup table has to be generated to achieve consistent outputs for varying frequencies. See the notes in [variable frequency source file](src/variable_frequency.c).

### Subjects

  [pulse-width modulation](http://www.8051projects.net/wiki/Pulse_Width_Modulation), C, [look-up tables](https://en.wikipedia.org/wiki/Lookup_table), counters/timers, [MOSFETS](http://www.electronics-tutorials.ws/transistor/tran_6.html), [gate drivers](https://en.wikipedia.org/wiki/Gate_driver), [IGBT](http://www.electronics-tutorials.ws/power/insulated-gate-bipolar-transistor.html), [induction motors](https://www.youtube.com/watch?v=HWrNzUCjbkk), [AC](https://www.allaboutcircuits.com/textbook/alternating-current/chpt-1/what-is-alternating-current-ac/), [AVR PWM modes](http://www.avrfreaks.net/forum/tut-c-newbies-guide-avr-pwm-incomplete?page=all), [CTC mode](http://maxembedded.com/2011/07/avr-timers-ctc-mode/), [output compare registers](http://www.ermicro.com/blog/?p=1971), [low pass filters](http://sim.okawa-denshi.jp/en/CRtool.php), [potentiometers](http://www.electronics-tutorials.ws/resistor/potentiometer.html), [analog to digital conversion (ADC)](https://learn.sparkfun.com/tutorials/analog-to-digital-conversion), [direct digital synthesis](https://en.wikipedia.org/wiki/Direct_digital_synthesizer), [ATmega328P](http://www.atmel.com/Images/Atmel-42735-8-bit-AVR-Microcontroller-ATmega328-328P_Datasheet.pdf)

### Languages and Tools

**Languages:** C/C++

**IDEs and IDPs:** Arduino IDE, Atmel Studio 7.0

**SPICE software**: TINA-TI

**Schematic capture software:** Altium Designer, TINA-TI

**Microcontrollers:** Arduino UNO (ATmega328P-PU) as controller and Arduino Nano (ATmega328P) for testing

**Low pass filter designer:** [link](http://sim.okawa-denshi.jp/en/CRtool.php)

**Isolation bootstrap calculator:** [link](https://www.silabs.com/tools/Pages/bootstrap-calculator.aspx)

### Reports, photos, diagrams

- [Full Report](doc/final_report.pdf)
- [Variable frequency drive schematic](doc/final_schematic.pdf)
- [Testing setup](doc/arduino_setup.png)
- [Test results](doc/pwm_serial_capture.png)