#include <avr/sleep.h>

void setup () 
{

for (byte i = 0; i <= A5; i++)
{
pinMode (i, OUTPUT);    // changed as per below
digitalWrite (i, LOW);  //     ditto
}

// disable ADC
ADCSRA = 0;  

set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
noInterrupts ();           // timed sequence follows
sleep_enable();

// turn off brown-out enable in software
MCUCR = bit (BODS) | bit (BODSE);
MCUCR = bit (BODS); 
interrupts ();             // guarantees next instruction executed
sleep_cpu ();              // sleep within 3 clock cycles of above

}  // end of setup

void loop () { }
