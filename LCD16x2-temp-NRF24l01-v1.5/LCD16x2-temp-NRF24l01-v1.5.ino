#include <LiquidCrystal.h>
#include <math.h>
#define ThermistorPIN1 0
#define ThermistorPIN2 1
#define MoisturePIN1 2
#define MoisturePIN2 3
#define MoisturePIN3 4
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
unsigned long previousScreenMillis = 0;        // will store last time LED was updated
unsigned long previousTxMillis = 0;
// loop delay in milli seconds
const long screen_refresh_interval = 1250;
const long tx_data_interval=60000;
boolean toggle_2nd_row=true;

int i = 0;
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
const uint64_t pipes[2] = { 0xBCBCBCBCBC,0xEDEDEDEDED };
float Thermistor(int RawADC) {
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
  Serial.begin(9600);
  printf_begin();
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(16);
  radio.setChannel(0x80);
  radio.setDataRate(RF24_250KBPS);
radio.setAutoAck(0);
radio.disableCRC();
radio.openWritingPipe(pipes[1]);
radio.openReadingPipe(1, pipes[0]);
radio.startListening();
radio.stopListening();
delay (10);
radio.printPrettyDetails();
radio.startListening();
 //Fill the placeholder chars with blanks - until data is received
memcpy(vcc,blankvcc,5);
memcpy(tempOUT,blanktempOUT,6);
}
void drawscreen(int whichscreenamiat){
  if (whichscreenamiat == 0)
        { 
        lcd.setCursor(0, 0);
        lcd.print("OUT = "+String(tempOUT)+" degC");
        // set the cursor to column 0, line 1
       // (note: line 1 is the second row, since counting begins with 0):
         lcd.setCursor(0, 1);
        float tempIN;
        tempIN=Thermistor(analogRead(ThermistorPIN1));
        lcd.print("IN1 = "+String(tempIN)+" degC");
        
        }
        
        if (whichscreenamiat == 1)
        {
        lcd.setCursor(0, 0);
        lcd.print("Out V = "+String(vcc)+" mv");
        lcd.setCursor(0, 1);
        float tempIN=Thermistor(analogRead(ThermistorPIN2));
        lcd.print("IN2 = "+String(tempIN)+" degC");
        }
        if (whichscreenamiat == 2)
        {
        float moist1 = analogRead(MoisturePIN1);
        lcd.setCursor(0, 0);
        lcd.print("M1 = "+String(moist1)+" %");
        lcd.setCursor(0, 1);
        moist1 = analogRead(MoisturePIN2);
        lcd.print("M2 = "+String(moist1)+" %");
        
        }
        if (whichscreenamiat == 3)
        {
        lcd.setCursor(0, 0);
        lcd.print("OUT = "+String(tempOUT)+" degC");
        lcd.setCursor(0, 1);
        float moist1 = analogRead(MoisturePIN3);
        lcd.print("M3 = "+String(moist1)+" %");
        
        }   
  
  }
void loop() {

 

    if( radio.available()){
        radio.read( got_time, 16 );             // Get the payload
       previousTxMillis=millis();
     
      memcpy(vcc,got_time+1,4);
      memcpy(tempOUT,got_time+6,5);  
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
  
  


    unsigned long currentScreenMillis = millis();
     
    if ((currentScreenMillis - previousScreenMillis)>=screen_refresh_interval)
    {
        previousScreenMillis = currentScreenMillis;
      lcd.clear();
      drawscreen(i);
      if (i>2){i=0;}
      else    {  i=i+1;}    

      
      
    }


}
