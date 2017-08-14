/*
 * Variable frequency sine wave PWM 
 * Created: 2017-07-18 6:32:14 PM
 * Author : Hannah Sawiuk
 */ 

/* Notes
 * 1 sine wave period = 256 entries in the look up table / readSpeed 
 * readSpeed is the speed at which index of the look up table increases. Cycling through the look up table once represents one waveform.
 * Since period = 1/frequency, it follows that readSpeed = frequency * 256 
 * To alter the frequency of the PWM sine waveform, the OCR2A register is changed. OCR2A = sysCLK /readSpeed
 * Because OCR2A is an 8-bit register with a BOTTOM of 0x00 and a maximum TOP value of 255, the readSpeed needs to be rounded to the nearest positive integer
 * So, the round() function is used to round the value to the nearest integer
 * 
 * To conclude: OCR2A = sysClk / round(frequency * 256)
 * 
 * To alter the frequency, a potentiometer is used to vary an analog input. 
 * To retrieve the analog data, analogRead() is used. It converts the input voltage (0 to 5 volt range) to a digital value between 0 and 1023
 * Since a frequency in the range of 0 to 120 Hz is desired: 
 *    frequency = (analog data * maximum frequency) / max digital value
 *              = (analog data * 120) / 1023 
 * IMPORTANT: the analog data MUST be multiplied by 120 BEFORE dividing by 1023 or else the values will be 0
 *
 * Once the analog data is converted into a frequency, it is then used in updating the OCR2A register (TOP value)
 * In CTC mode, OCR2A is the TOP value for Timer2. The TOP value dictates when the timer compare flag is set.
 * An ISR is configured for the Timer2 output compare flag, so the output compare registers associated with
 * the 3 output PWM sinewave signals are updated with respect to the value of OCR2A which is affected by the analog input.
 * An analog input was used because it mimics an acceleration pedal. Also, it simplifies the code.
 * 
 * f(Hz)    readSpeed     OCR2A 
 * 10    |   2560     |   6250
 * 30    |   7680     |   2083
 * 60    |  15360     |   1042
 * 100   |  25600     |    625
 * 120   |  30720     |    521
 * 
 * To alter the frequency of the PWM sine waveform, the OCR2A register is changed. OCR2A = sysCLK /readSpeed = sysclk / floor(frequency * 256)
*/

//***************************//
//         Constants         //
//***************************//
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
volatile uint16_t index; /*index of */
volatile int analogIn; /*analog input*/
volatile float frequency; 

/******************************/
/*          Strings           */
/******************************/
String str2;
String str1 = String("Frequency: ");

/******************************/
/*        Timer 2 ISR         */
/******************************/
ISR(TIMER2_COMPA_vect)
{
  /*Update the 16-bit compare register OCR2A (TOP value for CTC mode of Timer2)*/
  OCR2A = sysCLK / round(frequency * 256);
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
  OCR2A = 0;/*Set the 16-bit compare register OCR2A (TOP value for CTC mode of Timer2)*/
  
  /*reset variables*/
  index = 0; 
  analogIn = 0;
  frequency = 0.0;
  
  sei(); /*enable interrupts*/
  Serial.begin(19200); /*initialize Baud rate*/
}

/******************************/
/*      Main System Run       */
/******************************/
void loop (void) 
{ 
  analogIn = analogRead(A0); /*Read analog value from pin A0*/
  frequency = (analogIn * 120.0)/1023.0; /*Convert analog value into a frequency*/
  str2 = str1 + frequency + '\r';
  Serial.print(str2); /*Print present frequency to console. Using PuTTY*/
  delay(200); /*delay for 200 ms*/
} 



