/**
 * Variable frequency sinusoidal PWM 
 * Author : Hannah Sawiuk
 **/ 

/** Notes
 * 1 sine wave period = 256 entries in the look up table / readSpeed 
 * readSpeed is the speed at which index of the look up table increases. Cycling through the look up table once represents one waveform.
 * Since period = 1/frequency, it follows that readSpeed = frequency * 256 
 * To alter the frequency of the PWM sine waveform, the output compare register of the timerthe OCR2A register is changed since OCR2A = sysCLK /readSpeed
 * Because OCR2A is an 8-bit register with a BOTTOM of 0x00 and a maximum TOP value of 255, the readSpeed needs to be rounded to the nearest positive integer
 * So, the round() function is used to round the value to the nearest integer
 * 
 * To conclude: OCR2A = round(sysClk / frequency * 256)
 * 
 * To alter the frequency, a potentiometer is used to vary an analog input. 
 * To retrieve the analog data, analogRead() is used. It converts the input voltage (0 to 5 volt range) to a digital value between 0 and 1023
 * Say a frequency in the range of 0 to 120 Hz is desired: 
 *    frequency = (analog data * maximum frequency) / max digital value
 *              = (analog data * 120) / 1023 
 * IMPORTANT: the analog data MUST be multiplied by 120 BEFORE dividing by 1023 or else the values will be 0
 *
 * Once the analog data is converted into a frequency, it is then used in updating the OCR2A register.
 * In CTC mode, OCR2A is the TOP value for Timer2. The TOP value dictates when the timer compare flag is set.
 * An ISR is configured for the Timer2 output compare flag, so the output compare registers associated with
 * the 3 output PWM sinewave signals are updated with respect to the value of OCR2A which is affected by the analog input (potentiometer).
 * 
 * f(Hz)    readSpeed    OCR2A 
 *  1    |    256     |   244
 *  2    |    512     |   122
 *  3    |    768     |    81
 * 10    |   2560     |    24
 * 30    |   7680     |     8
 * 60    |  15360     |     4
 * 120   |  30720     |     2
 * 
 * Works best for lower frequency applications since.
 * 
 * To alter the frequency of the PWM sine waveform, the OCR2A register is changed: 
 *    OCR2A = sysCLK/prescaler / readSpeed = round(sysCLK / prescaler * frequency * 256)
 *    
 * Because the values in the look up table are for the situation when OCR2A=0xFF, they must be made proportional
 * to the desired frequency. A divisor is used to make the PWM OCRxx reg values proportional:
 *      round(pgm_read_byte(&sinewaveLUT[index])/divisor);
 * 
 * This is not the best method, because the precision of the output frequency is not great.  
 * Some future changes could be:
 *    - Using 16-bit registers, instead of 8-bit 
 *    - Phase and frequency correct mode instead of Fast PWM since using a low frequency anyway
 *    - Have a phase accumulator instead of an index for the LUT. This would ensure that the LUT entry being accessed is the correct one.
 *    - Generate LUT entries dynamically with shifts in frequency. This would be inefficient for fast changes in frequency, but would output more precise waveforms
 * 
**/

//***************************//
//         Constants         //
//***************************//
#define LUT_entries 255 // number of entries in the sine wave look up table
#define sysCLK 16000000 // 16MHz internal clock (arduino UNO) NOTE: change if using external xtal (connect at PB6 pin)
#define prescaler 256   // prescaler of the output compare interrupt timer

//atmega328p (Arduino Uno or Nano) PWM pins: 3, 5, 6, 9, 10, 11 NOTE: match with the output compare registers
#define pwmOUT1 6    // port D6 (OC0A) 
#define pwmOUT2 9    // port B1 (OC1A) 
#define pwmOUT3 10   // port B2 (OC1B) 

//***************************//
//   Header and Libraries    //
//***************************//
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>  // pgm_read_byte and PROGMEM
#include <util/delay.h>
#include "sinewave_LUT.h"  // LUT file

/******************************/
/*     Volatile Variables     */
/******************************/
volatile uint16_t index;  // index of LUT 
volatile int analogIn;    // analog input
volatile float frequency; 

/******************************/
/*          Strings           */
/******************************/
String str2;
String str1 = String("Frequency: ");

/******************************/
/*        Timer 2 ISR         */
/******************************/
ISR(TIMER2_COMPA_vect)  // output compare interrupt
{
  /* Update the 8-bit compare register OCR2A (TOP value for CTC mode of Timer2) */
  if (frequency > 0) OCR2A = round (sysCLK / (prescaler * frequency * 256)); 
  if (index >= LUT_entries) { 
    index = 0; 
  }
  else { 
    /* Update look up table index and output compare register values */
    int divisor = 256/OCR2A;  // make the pulse width proportional to the frequency
    /* Rounding used again since OCRxx regs can only take integer values */
    OCR0A = round(pgm_read_byte(&sinewaveLUT[index])/divisor);
    OCR1A = round(pgm_read_byte(&sinewaveLUT[index + 85])/divisor);
    OCR1B = round(pgm_read_byte(&sinewaveLUT[index + 170])/divisor);
    index++;
  }
}

/******************************/
/*        System Setup        */
/******************************/
void setup (void)
{ 
  pinMode(pwmOUT1, OUTPUT); // enables port B1 (OC1A) as an output pin
  pinMode(pwmOUT2, OUTPUT); // enables port B2 (OC1B) as an output pin
  pinMode(pwmOUT3, OUTPUT); // enables port B3 (OC2A) as an output pin

  /* Sets Timer0 in Fast PWM mode. Clears OC0A on Compare Match, set OC0A at BOTTOM (non-inverting mode).
   Then, waveform generation is set to mode 3: Fast PWM with TOP of 0xFF*/
  TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00) | (1 << WGM01); // page 104 and 106
  TCCR0B = (1 << CS00); /*No pre-scaling (page 108)*/

  /* Sets Timer1 in Fast PWM mode. Clears OC1A/B on Compare Match, set OC1A/B at BOTTOM (non-inverting mode).
   Then, waveform generation is set to mode 3: Fast PWM with TOP of 0xFF. Prescaler of 256 */
  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM12) | (1 << WGM10); // page 171 and 172
  TCCR1B = (1 << CS12);                                                 // sysClk/256 (page 173)

  /* Sets Timer2 in CTC mode mode (non-pwm). Generates output compare interrupt
  TOP = OCR2A, update of OCR2 at immediate, prescaler of 256 */
  TCCR2A = (1 << COM2A1) | (1 << WGM21);  // page 203 and 205 
  TCCR2B = (1 << CS21) | (1 << CS22);     // sysClk/256

  cli(); // disable interrupts
  
  TIMSK0 = (1 << TOIE0);  // Enable Timer0  
  TIMSK1 = (1 << TOIE1);  // Enable Timer1   
  TIMSK2 = (1 << OCIE2A); // Configure Timer2 interrupts to send LUT value 
  
  /* Note: OCR2A is set after TCCR1x initialization to avoid overwriting/reset */
  OCR2A = 0;  // Reset the 8-bit compare register OCR2A (TOP value for CTC mode of Timer2)
  
  /* reset variables */
  index     = 0; 
  analogIn  = 0;
  frequency = 0.0;
  
  sei();  // enable interrupts

  Serial.begin(19200);  // initialize Baud rate 
}

/******************************/
/*     Main System Loop       */
/******************************/
void loop (void) 
{ 
  analogIn = analogRead(A0);              // Read analog value from pin A0: connected to potentiometer to allow interface
  frequency = (analogIn * 120.0)/1023.0;  // Convert analog value into a frequency in range(120) 
  str2 = str1 + frequency + '\r'; 
  Serial.print(str2);                     // Print frequency to console 
  delay(200);                             // wait
}

/******************************/
/*  256 Entry Sinewave LUT    */
/******************************/
// Included in a separate header file
// const uint8_t sinewaveLUT[] PROGMEM = {
//     0x80, 0x83, 0x86, 0x89, 0x8c, 0x8f, 0x92, 0x95, 0x98, 0x9c, 0x9f, 0xa2,
//     0xa5, 0xa8, 0xab, 0xae, 0xb0, 0xb3, 0xb6, 0xb9, 0xbc, 0xbf, 0xc1, 0xc4,
//     0xc7, 0xc9, 0xcc, 0xce, 0xd1, 0xd3, 0xd5, 0xd8, 0xda, 0xdc, 0xde, 0xe0,
//     0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xed, 0xef, 0xf0, 0xf2, 0xf3, 0xf5,
//     0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfc, 0xfd, 0xfe, 0xfe, 0xff,
//     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe,
//     0xfd, 0xfc, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7, 0xf6, 0xf5, 0xf3, 0xf2,
//     0xf0, 0xef, 0xed, 0xec, 0xea, 0xe8, 0xe6, 0xe4, 0xe2, 0xe0, 0xde, 0xdc,
//     0xda, 0xd8, 0xd5, 0xd3, 0xd1, 0xce, 0xcc, 0xc9, 0xc7, 0xc4, 0xc1, 0xbf,
//     0xbc, 0xb9, 0xb6, 0xb3, 0xb0, 0xae, 0xab, 0xa8, 0xa5, 0xa2, 0x9f, 0x9c,
//     0x98, 0x95, 0x92, 0x8f, 0x8c, 0x89, 0x86, 0x83, 0x80, 0x7c, 0x79, 0x76,
//     0x73, 0x70, 0x6d, 0x6a, 0x67, 0x63, 0x60, 0x5d, 0x5a, 0x57, 0x54, 0x51,
//     0x4f, 0x4c, 0x49, 0x46, 0x43, 0x40, 0x3e, 0x3b, 0x38, 0x36, 0x33, 0x31,
//     0x2e, 0x2c, 0x2a, 0x27, 0x25, 0x23, 0x21, 0x1f, 0x1d, 0x1b, 0x19, 0x17,
//     0x15, 0x13, 0x12, 0x10, 0x0f, 0x0d, 0x0c, 0x0a, 0x09, 0x08, 0x07, 0x06,
//     0x05, 0x04, 0x03, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04,
//     0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0c, 0x0d, 0x0f, 0x10, 0x12, 0x13,
//     0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f, 0x21, 0x23, 0x25, 0x27, 0x2a, 0x2c,
//     0x2e, 0x31, 0x33, 0x36, 0x38, 0x3b, 0x3e, 0x40, 0x43, 0x46, 0x49, 0x4c,
//     0x4f, 0x51, 0x54, 0x57, 0x5a, 0x5d, 0x60, 0x63, 0x67, 0x6a, 0x6d, 0x70,
//     0x73, 0x76, 0x79, 0x7c};
