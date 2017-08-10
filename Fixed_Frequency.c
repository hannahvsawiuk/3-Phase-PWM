/*
 * Variable PWM code
 * Created: 2017-07-18 6:32:14 PM
 * Author : Hannah Sawiuk
 */ 

/*sine wave period = 256 entries / readSpeed
   f(Hz)    readSpeed
  10    |   2560
  30    |   7680
  60    |  15360
  100   |  25600
  120   |  30720

  Using timer 2 for output compare interrupt and timer 1 and timer 0 for the pwm outputs
*/

//***************************//
//         Constants         //
//***************************//
#define readSpeed 128   //1/2 Hz  
#define LUT_entries 255 //number of entries in the sine wave look up table
#define sysCLK 16000000 //16MHz external clock (arduino UNO) NOTE: change if using external xtal (connect at PB6 pin)

//atmega328p (Arduino Uno or Nano) PWM pins: 3, 5, 6, 9, 10, 11 NOTE: match with the output compare registers
#define pwmOUT1 6    //port D6 (OC0A) 
#define pwmOUT2 9    //port B1 (OC1A) 
#define pwmOUT3 10   //port B2 (OC1B) 

//***************************//
//  Header and include files //
//***************************//
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>  /*pgm_read_byte and PROGMEM*/
#include <util/delay.h>
#include "sinewave_LUT.h" /*LUT file*/

/******************************/
/*     Volatile Variables     */
/******************************/
volatile uint16_t index; //index of LUT

/******************************/
/*        Timer 2 ISR         */
/******************************/
ISR(TIMER2_COMPA_vect)
{
  if (index >= LUT_entries) { 
    index = 0; 
  }
  else { 
    /*Update look up table index and output compare register values*/
    OCR0A = pgm_read_byte(&sinewaveLUT[index]);
    OCR1A = pgm_read_byte(&sinewaveLUT[index + 85]);
    OCR1B = pgm_read_byte(&sinewaveLUT[index + 170]);
    index++;
  }
}

/******************************/
/*        System Setup        */
/******************************/
void setup (void)
{ 
  pinMode(pwmOUT1, OUTPUT); //enables port B1 (OC1A) as an output pin
  pinMode(pwmOUT2, OUTPUT); //enables port B2 (OC1B) as an output pin
  pinMode(pwmOUT3, OUTPUT); //enables port B3 (OC2A) as an output pin

  /* Sets Timer0 in Fast PWM mode. Clears OC0A on Compare Match, set OC0A at BOTTOM (non-inverting mode).
   Then, waveform generation is set to mode 3: Fast PWM with TOP of 0xFF*/
  TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00) | (1 << WGM01); /*page 104 and page 106*/
  TCCR0B = (1 << CS00); /*No pre-scaling (page 108)*/

  /* Sets Timer1 in Fast PWM mode. Clears OC1A/B on Compare Match, set OC1A/B at BOTTOM (non-inverting mode).
   Then, waveform generation is set to mode 3: Fast PWM with TOP of 0xFF*/
  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM12) | (1 << WGM10) ; /*page 171 and 172*/
  TCCR1B = (1 << CS10); /*No pre-scaling (page 173) NOTE: must change if using an external clock*/

  /* Sets Timer2 in CTC mode mode.TOP = OCR2A, update at immediate, no pre-scaling */
  TCCR2A = (1 << COM2A1) | (1 << WGM21); /*page 203 and 205*/
  TCCR2B = (1 << CS20);

  cli(); /*disable interrupts*/
  
  TIMSK0 = (1 << TOIE0);  /*Enable Timer0*/   
  TIMSK1 = (1 << TOIE1); /*Enable Timer1*/   
  TIMSK2 = (1 << OCIE2A); /* Configure Timer2 interrupts to send LUT value */
    
  /*Note: OCR2A is set after TCCR1x initialization to avoid overwriting/reset*/
  OCR2A = sysCLK / readSpeed;/*Set the 16-bit compare register OCR2A (TOP value for CTC mode of Timer2)*/
  index = 0; /*reset index*/
  sei(); /*enable interrupts*/
}

/******************************/
/*      Main System Run       */
/******************************/
void loop (void) 
{ 
  while (1);
} 

