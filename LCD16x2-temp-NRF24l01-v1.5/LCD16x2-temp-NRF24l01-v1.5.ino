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
unsigned long previousScreenMillis = 0;        // will store last time LED was updated
unsigned long previousTxMillis = 0;
// loop delay in milli seconds
const long screen_refresh_interval = 1250;
const long tx_data_interval=60000;
boolean toggle_2nd_row=true;


char tempOUT[6];//12.34 - 5 chars + 1 trailing char 
char vcc[5];//4653
    // vcc[4]='\0'; //Add the trailing chars
    // tempOUT[5]='\0';
char blankvcc[5]={'-','-','-','-','\0'};
char blanktempOUT[6]={'-','-','-','-','-','\0'};
char got_time[16]; //an Array of 8 bytes - since the payload size is 8 bytes

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
 //Fill the placeholder chars with blanks - until data is received
memcpy(vcc,blankvcc,5);
memcpy(tempOUT,blanktempOUT,6);
}

void loop() {

 
  // print the number of seconds since reset:
  //lcd.print(millis() / 1000);
  

   //A delay of 1000ms in the loop worked more consistently 
    //delay(1000);
    //radio.print_status(radio.get_status()); // need to use KVRF24.h - these are private methods - made public in my version
    if( radio.available()){
      //printf("Got payload \r\n");       // Variable for the received timestamp
      //while (radio.available()) {                                   // While there is data ready
        //delay(100);
        radio.read( got_time, 16 );             // Get the payload
       previousTxMillis=millis();
   // }
      
      
    
          //If the data is being sent as ASCII Hex - below will print out the ASCII Chars      Serial.print(got_time[0]);
      //Serial.print(got_time[0]);//V
      //Serial.print(got_time[1]);//1
      //Serial.print(got_time[2]);//2
      //Serial.print(got_time[3]);//3
      //Serial.print(got_time[4]);//4
      //Serial.print(got_time[5]);//t
      //Serial.print(got_time[6]);//1
      //Serial.print(got_time[7]);//2
      //Serial.print(got_time[8]);//.
      //Serial.print(got_time[9]);//2
      //Serial.print(got_time[10]);//3
      //Serial.print(got_time[11]);
      //Serial.print("\r\n");
     
 memcpy(vcc,got_time+1,4);
 memcpy(tempOUT,got_time+6,5); 

  // if (got_time[0]=='V'){memcpy(vcc,got_time,8);
 //Serial.println(String(vcc));
// }
//   else if (got_time[0]=='t'){memcpy(temp,got_time,8);
// Serial.println(String(temp));
// }
 
  }

  
      unsigned long currentTxMillis = millis();
if(currentTxMillis - previousTxMillis >= tx_data_interval) {
   previousTxMillis=currentTxMillis;
    //No data received in last tx-data_interval
    Serial.println("No data received");
    //Fill with blanks again - until data is received    
     memcpy(vcc,blankvcc,5);
     memcpy(tempOUT,blanktempOUT,6);
}
  
  

  //String strVcc(vcc);
  //String strTemp(temp);

    unsigned long currentScreenMillis = millis();
if(currentScreenMillis - previousScreenMillis >= screen_refresh_interval) {
   
    previousScreenMillis = currentScreenMillis; 
  //1st row always displays Outside temp 
  lcd.setCursor(0, 0);
  lcd.print("OUT = "+String(tempOUT)+" degC");
    // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);

if(toggle_2nd_row){

      toggle_2nd_row=false;
      float tempIN;
  //float temp2;
 tempIN=Thermistor(analogRead(ThermistorPIN1));
 //temp2=Thermistor(analogRead(ThermistorPIN2));
  
  
  //Serial.println("temp 1 = "+String(temp1));

 
 //lcd.print("OUT = "+String(temp2));
 //Serial.println("temp 2 = "+String(temp2));
lcd.print("IN = "+String(tempIN)+" degC");
//Serial.println("IN = "+String(tempIN)+" degC");
Serial.println("OUT = "+String(tempOUT)+" degC"); //tempOUT just to test
} else {
toggle_2nd_row=true;

lcd.print("Out V = "+String(vcc)+" mv");
Serial.println("Out V = "+String(vcc)+" mv");

}


}



}



