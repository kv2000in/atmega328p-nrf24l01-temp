#include<stdlib.h> 
#include <math.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#define DigitalSwitchTemp 4
#define DigitalSwitchReg 5
#define ThermistorPIN 0  
int HowManySleepCycles = 4;
int currentsleepcycle = 0;              
float pad = 9980; 
byte myDDRB;
byte myPORTB;


volatile int f_wdt=1;

/***************************************************
 *  Name:        ISR(WDT_vect)
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Watchdog Interrupt Service. This
 *               is executed when watchdog timed out.
 *
 ***************************************************/
ISR(WDT_vect)
{
  if(f_wdt == 0)
  {
    f_wdt=1;
  }
  else
  {
    Serial.println("WDT Overrun!!!");
  }
}

/***************************************************
 *  Name:        enterSleep
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Enters the arduino into sleep mode.
 *
 ***************************************************/
void enterSleep(void)
{

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
   ADCSRA &= ~ (1 << ADEN);            // turn off ADC
   //power_all_disable ();  
  sleep_enable();
  
  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the WDT timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
 
  ADCSRA |= (1 << ADEN); //enable ADC
  DDRB=myDDRB; //Reset PORTB Direction and status
  PORTB=myPORTB;

}

float Thermistor(int RawADC) {
  long Resistance;  
  float Temp;  // Dual-Purpose variable to save space.

  Resistance=pad*((1024.0 / RawADC) - 1); 
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.001474447721 + (0.0002370486854 * Temp) + (0.0000001079376360 * Temp * Temp * Temp)); // For YSI 400 thermistor
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      
   return Temp;                                     
}


RF24 radio(9,10);
const uint64_t pipes[2] = { 0xBCBCBCBCBC,0xEDEDEDEDED };
void setup(void)
{
Serial.begin(9600);
printf_begin();

  /*** Setup the WDT ***/
  
  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);
 
/**** END WDT setup*****/

}
long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void senddata(){
 digitalWrite(DigitalSwitchTemp,HIGH);
 digitalWrite(DigitalSwitchReg,HIGH);
delay(10);
 pinMode(DigitalSwitchTemp,OUTPUT);
 pinMode(DigitalSwitchReg,OUTPUT);
  delay(50);
 radio.powerUp();
 delay(10);
 radio.begin();
 delay(30);
 radio.setRetries(15,15);
 radio.setPayloadSize(16);
 radio.setChannel(0x80);
 radio.setDataRate(RF24_250KBPS);
 radio.setAutoAck(0);
 radio.disableCRC();
 radio.openWritingPipe(pipes[0]);
 radio.openReadingPipe(1, pipes[1]);
 radio.startListening();
 radio.stopListening();
 delay (10);

 // VCC data
  float vcc=readVcc();
  char datastr[16]={'V'};
  dtostrf(vcc,4, 0, datastr+1);
  //temp data
  float temp;
  temp=Thermistor(analogRead(ThermistorPIN));
  datastr[5]='t';
  dtostrf(temp,4, 2, datastr+6);
delay(20);

 
 bool okT = radio.write( &datastr, 16);
if (okT)
      printf("temp OK...\r\n");
    else
     printf("failed.\n\r"); 


 delay(20);
 
//radio.stopListening(); //some bad nrf24l01 modules don't power down. this was suggested in forums as an alternative. still didn't work.trial and error of different radios.

radio.powerDown(); 

delay(20);

pinMode(DigitalSwitchTemp,INPUT);
pinMode(DigitalSwitchReg,INPUT);
delay(10);
  
myDDRB=DDRB; //Save the direction and status of PORTB
myPORTB=PORTB;

SPI.end();
delay(10);

DDRB=0;
PORTB=0;
delay(10);


}







void loop(void)
{
  
if(f_wdt == 1)
  {
   currentsleepcycle++;
    
    if (currentsleepcycle>HowManySleepCycles){
    /* Do something */
   senddata();

   delay(10);
   currentsleepcycle=0; 
    }
    /* Don't forget to clear the flag. */
    f_wdt = 0;
    
    /* Re-enter sleep mode. */
     enterSleep();

  
  }
  else
  {
    /* Do nothing. */
  }
  }

// vim:cin:ai:sts=2 sw=2 ft=cpp
