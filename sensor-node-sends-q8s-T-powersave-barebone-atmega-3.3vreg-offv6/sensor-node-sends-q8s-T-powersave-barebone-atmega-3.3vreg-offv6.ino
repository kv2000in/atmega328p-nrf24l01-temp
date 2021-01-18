/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 12 - CHANGED to pin 8 
 * LCD Enable pin to digital pin 11 - CHANGED to pin 7
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */
/*
// include the library code:
#include <LiquidCrystal.h>
#include <math.h>
#define ThermistorPIN1 0
#define ThermistorPIN2 1
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
//SWITCHED to TMRH NRF24 Libraries - 11/16/17 OLD LIB SAVED as RF24-MASTER-MANIACBUG
unsigned long previousMillis = 0;        // will store last time LED was updated

// loop delay in milli seconds
const long interval = 250;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 7, 5, 4, 3, 2);
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10           

RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.              
const uint64_t pipes[2] = { 0xC2C2C2C2C2LL, 0xF0F0F0F0F0LL };

float Thermistor(int RawADC) {
 // [Ground] -- [10k-pad-resistor] -- | -- [thermistor] --[Vcc (5 or 3.3v)]
 //                                   |
 //                                Analog Pin 6
  float pad = 9980; // Actual measured value of the 10k pad register.
  long Resistance;  
  float Temp;  // Dual-Purpose variable to save space.

  Resistance=pad*((1024.0 / RawADC) - 1); 
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  //Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));  // For 10 K thermistor
  Temp = 1 / (0.001474447721 + (0.0002370486854 * Temp) + (0.0000001079376360 * Temp * Temp * Temp)); // For YSI 400 thermistor
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      
//vcc=readVcc();
   return Temp;                                      // Return the Temperature
}

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  //lcd.print("hello, world!");
  Serial.begin(57600);
  //printf is needed for radio to print details
  printf_begin();
  
   radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(16);
  radio.setChannel(0x60);
  radio.setDataRate(RF24_2MBPS);
  //radio.setDataRate(RF24_250KBPS);
  //radio.setPALevel(RF24_PA_MAX);
radio.setAutoAck(0);
//radio.openWritingPipe(pipes[0]);
radio.openReadingPipe(1, pipes[1]);

radio.startListening();
radio.stopListening();
delay (10);
radio.printDetails();
radio.startListening();
}

void loop() {

 
  // print the number of seconds since reset:
  //lcd.print(millis() / 1000);
  
    char temp[8];
    char vcc[8];
    char got_time[16]; //an Array of 8 bytes - since the payload size is 8 bytes
   //A delay of 1000ms in the loop worked more consistently 
    //delay(1000);
    //radio.print_status(radio.get_status()); // need to use KVRF24.h - these are private methods - made public in my version
    if( radio.available()){
      //printf("Got payload \r\n");       // Variable for the received timestamp
      //while (radio.available()) {                                   // While there is data ready
        //delay(100);
        radio.read( got_time, 16 );             // Get the payload
     
   // }
      
      
    
          //If the data is being sent as ASCII Hex - below will print out the ASCII Chars      Serial.print(got_time[0]);
      Serial.print(got_time[0]);
      Serial.print(got_time[1]);
      Serial.print(got_time[2]);
      Serial.print(got_time[3]);
      Serial.print(got_time[4]);
      Serial.print(got_time[5]);
      Serial.print(got_time[6]);
      Serial.print(got_time[7]);
      Serial.print(got_time[8]);
      Serial.print(got_time[9]);
      Serial.print(got_time[10]);
      Serial.print(got_time[11]);
      Serial.print("\r\n");
     
  
 //  if (got_time[0]=='V'){memcpy(vcc,got_time,8);
 //Serial.println(String(vcc));
// }
//   else if (got_time[0]=='t'){memcpy(temp,got_time,8);
// Serial.println(String(temp));
// }
 
  }
  

  //String strVcc(vcc);
  //String strTemp(temp);

    unsigned long currentMillis = millis();
if(currentMillis - previousMillis >= interval) {
   
    previousMillis = currentMillis; 

      
      float temp1;
  float temp2;
 temp1=Thermistor(analogRead(ThermistorPIN1));
 temp2=Thermistor(analogRead(ThermistorPIN2));
  
  lcd.setCursor(0, 0);
  lcd.print("IN = "+String(temp1));
  //Serial.println("temp 1 = "+String(temp1));
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
 
 //lcd.print("OUT = "+String(temp2));
 //Serial.println("temp 2 = "+String(temp2));
lcd.print("OUT = "+String(temp));

}



}


*/

//***** this works with above receiver code ******

/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios. 
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two 
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting 
 * with the serial monitor and sending a 'T'.  The ping node sends the current 
 * time to the pong node, which responds by sending the value back.  The ping 
 * node can then see how long the whole cycle took.
 */
 
/* Simple test of the functionality of the photo resistor

Connect the photoresistor one leg to pin 0, and pin to +5V
Connect a resistor (around 10k is a good value, higher
values gives higher readings) from pin 0 to GND. (see appendix of arduino notebook page 37 for schematics).

----------------------------------------------------

           PhotoR     10K
 +5    o---/\/\/--.--/\/\/---o GND
                  |
 Pin 3 o-----------

----------------------------------------------------
*/ 
 
 /*
 * Inputs ADC Value from Thermistor and outputs Temperature in Celsius
 *  requires: include <math.h>
 * Utilizes the Steinhart-Hart Thermistor Equation:
 *    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]3}
 *    where A = 0.001129148, B = 0.000234125 and C = 8.76741E-08  for 10K thermistor
 *
 *
 *
 *http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
 *
 * These coefficients seem to work fairly universally, which is a bit of a 
 * surprise. 
 *
 *Resistance for 3 temp values are needed to calculate the coefficents.
 *For YSI 400 series
 *5 deg 5719 ohms
 *25 deg 2252 ohms
 *45 deg 983 ohms
 
 *based on it
 *A = 1.474447721 x10^-3
 *B = 2.370486854 x 10^-4
 *C = 1.079376360 x 10^-7
 
 * Schematic:
 *   [Ground] -- [10k-pad-resistor] -- | -- [thermistor] --[Vcc (5 or 3.3v)]
 *                                               |
 *                                          Analog Pin 6
 *
 * In case it isn't obvious (as it wasn't to me until I thought about it), the analog ports
 * measure the voltage between 0v -> Vcc which for an Arduino is a nominal 5v, but for (say) 
 * a JeeNode, is a nominal 3.3v.
 *
 * The resistance calculation uses the ratio of the two resistors, so the voltage
 * specified above is really only required for the debugging that is commented out below
 *
 * Resistance = PadResistor * (1024/ADC -1)  
 *
 * I have used this successfully with some CH Pipe Sensors (http://www.atcsemitec.co.uk/pdfdocs/ch.pdf)
 * which be obtained from http://www.rapidonline.co.uk.
 
 ****MY MODIFICATIONS****
 *SEEMS like this thermistor has a resistance of approx 2600 at 25 deg C. So use an equivalent resistor vor making a voltage divider
 *I used 2430 red yellow orange brown brown resistor. It gives close temp value but not accurate yet -
 *Need to adjust that resistor or the pad value below
 *
 *Just Found out that it is YSI 400 series esophageal temp probe with resistance values
 *2252 ohms at 25 deg C and 1355 ohms at 37 deg c
 *
 */





#include<stdlib.h> 
#include <math.h>
#include <SPI.h>
//SWITCHED to TMRH NRF24 Libraries - 11/16/17 OLD LIB SAVED as RF24-MASTER-MANIACBUG

#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#define DigitalSwitchTemp 4
#define DigitalSwitchReg 5
#define ThermistorPIN 0                // Analog Pin 6 to which thermistor is connected
//#define PhotoresistorPIN 3  // Analog Pin 3 to which photoresistor is connected
//#define PressurePIN 5 // Analog Pin 5 to which Pressure is connected.
float pad = 9980; // Changed from 1982 for testing - see below
//float pad = 1982;                       // DEFAULT was 9850, I used 1 2000 and 2 x 120 ohms to get 2230 and then used 2000 ohm (actual 1982) balance/pad resistor value, set this to
                                        // the measured resistance of your pad resistor
 
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
  //Serial.println("Sleep called");
  //Serial.println("WDT value is");
  //Serial.println(f_wdt);
  //delay (50);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
  sleep_enable();
  
  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the WDT timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}

float Thermistor(int RawADC) {
  long Resistance;  
  float Temp;  // Dual-Purpose variable to save space.

  Resistance=pad*((1024.0 / RawADC) - 1); 
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  //Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));  // For 10 K thermistor
  Temp = 1 / (0.001474447721 + (0.0002370486854 * Temp) + (0.0000001079376360 * Temp * Temp * Temp)); // For YSI 400 thermistor
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      
//vcc=readVcc();
   return Temp;                                      // Return the Temperature
}

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
//const uint64_t pipes[2] = { 0xC2C2C2C2C2C3LL, 0xE7E7E7E7E7E7LL }; //WORKING if base is sending to just one address E7
//If planning on using more than two sending nodes - remember to activate more than 2 receving pipes on the receiver.
//const uint64_t pipes[1] = {0xF0F0F0F0F0LL }; // for sending node #1
const uint64_t pipes[2] = { 0xBCBCBCBCBC,0xEDEDEDEDED };
//const uint64_t pipes[2] = { 0xC2C2C2C2C2C3LL, 0xF0F0F0F0F0F0LL }; //for sending node #2
//const byte slaveAddress[5] = {'R','x','A','A','A'};

void setup(void)
{
  
   pinMode(DigitalSwitchTemp,OUTPUT);
  pinMode(DigitalSwitchReg,OUTPUT); // Setup the digital switch pin to output mode
Serial.begin(9600);
//Call printf_begin() for printf to work/
printf_begin();
 // printf("\n\rRF24/examples/GettingStarted/\n\r");
//  
//  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //
 

 

//radio.startListening();
//delay (500);
 //Remove for further powersaveing
  //Serial.begin(9600);
  //Serial.println("Initialising...");
  //delay(100); //Allow for serial print to complete.
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

 //Power up the digital switch so that opamp gets powered up. 
 //Otherwise each opamp draws about 7 mA of quiescent current
 //compared to 0.8 mA of quiescent current by Atmega 328 without opAmp
 digitalWrite(DigitalSwitchTemp,HIGH);
 digitalWrite(DigitalSwitchReg,HIGH);
 delay(50);
 
     radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(16);
  radio.setChannel(0x60);
  radio.setDataRate(RF24_2MBPS);
  //radio.setDataRate(RF24_250KBPS);
  //radio.setPALevel(RF24_PA_MAX);
//Disabling AutoACK gives more reliable results
radio.setAutoAck(1);
radio.openWritingPipe(pipes[0]);
radio.openReadingPipe(1, pipes[1]);
//radio.openWritingPipe(slaveAddress);
radio.startListening();
radio.stopListening();
delay (10);
  radio.powerUp();
  //Delay inc from 20 to 100 = bad
  delay(10);
 
// VCC data
  float vcc=readVcc();
  //char vccstr[1+4+1]="V";
  char datastr[16]={'V'};
 dtostrf(vcc,4, 0, datastr+1);
  //bool okV = radio.write( &vccstr, 8 );
 //printf("Now sending vcc %s...",vccstr);
    /*
    Data gets sent as Hex
   for eg. char mychar[3] = {'P','P','Q'};
    bool okV = radio.write( &mychar, sizeof(mychar) );
    sends the hex value 0x515050 . ASCII Hex for Q = 0x51 and P = 0x50
    If receiver was reading using an unsigned long buffer - (size - 4 bytes) - and displaying it using %lu - it will show the decimal value of 0x515050
    
    */
 //  
    //bool okV = radio.write( &vccstr, 8 );
    
 // if (okV)
 //     printf("vcc ok...");
 //   else
 //    printf("failed.\n\r");
//delay(500);
  //temp data
float temp;
 temp=Thermistor(analogRead(ThermistorPIN));
 //char tempstr[1+4+1]="t";
 datastr[5]='t';
 dtostrf(temp,4, 2, datastr+6);
 //char tempstr ='t';



/* 
// Light Data
 float photo;
  photo=analogRead(PhotoresistorPIN);
 char photostr[1+4+1]="L";
  
 dtostrf(photo,4, 0, photostr+1);
 bool okL = radio.write( &photostr, 6 );
 delay(200);

   // printf("Now sending light %s...",photostr);
   */ 
    
     
      
      
    
    //if (okL)
    //  printf("ok...");
    //else
    //  printf("failed.\n\r");
radio.printPrettyDetails();
delay(20);

 
 bool okT = radio.write( &datastr, 16);
//bool okT returns false if receiving node is turned off
//boolOKT returns true only if ACK is received from the receiver
//in Autoack mode - ACK is received but at times - no radio.available on receiver

if (okT)
      printf("temp OK...\r\n");
    else
     printf("failed.\n\r"); 

// printf("Now sending Temp %s...",tempstr);

 delay(10);
 
  //delay(500);



    //printf("Now sending pressure %s...",pressurestr);
    
   
     
    
    //if (okP)
    //  printf("ok...");
    //else
     // printf("failed.\n\r");
//delay(20);



    //printf("Now sending temp %s...",tempstr);
    
   // bool ok = radio.write( &vccstr, 6 );
     
    
   // if (okT)
   //   printf("ok...");
   // else
   //   printf("failed.\n\r");

radio.powerDown();

delay(10);
digitalWrite(DigitalSwitchReg,LOW);
digitalWrite(DigitalSwitchTemp,LOW);
}







void loop(void)
{
  
if(f_wdt == 1)
  {
    /* Do something */
   senddata();
  // Serial.println("sending");
  //delay (50-500) gave incosistent radio comm
   delay(10);
    
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
