//LCD Atmega328p with NRF24L01 runs MCUDude/Minicore Atemga328 Bootloader yes UART0 BOD 2.7V Internal 2 MHz LTO Enabled eeprom retained
//Remove all peripherals, use short USB extension - power via 3.3V from USB-TTL and set Logic jumper to 3.3V to reliably upload sketch via bootloader
//Non-LCD Atmega328p with NRF24L01 runs MCUDude/Minicore Atemga328 Bootloader yes UART0 BOD 2.7V External 18.432 MHz LTO Enabled eeprom retained
//Power it via 5V USB-TTL with Logic Jumper on 5V, can use the long USB extension.
#include <LiquidCrystal.h>
#include <math.h>
#define ThermistorPIN1 0
#define ThermistorPIN2 1
#define MoisturePIN1 2
#define MoisturePIN2 3
#define MoisturePIN3 4
#define MoisturePIN4 5
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
unsigned long previousScreenMillis = 0;        // will store last time LCD was updated
unsigned long previousRxMillis = 0;
unsigned long previousTxMillis = 0;
// loop delay in milli seconds
const long screen_refresh_interval = 1250;
const long Tx_data_interval=30000;
const long Rx_data_interval=300000;
boolean toggle_2nd_row=true;
#define DigitalSwitchforMoistureSensor 6
int i = 0;
char tempOUT[6];//12.34 - 5 chars + 1 trailing char 
char vcc[5];//4653
    // vcc[4]='\0'; //Add the trailing chars
    // tempOUT[5]='\0';
char blankvcc[5]={'-','-','-','-','\0'};
char blanktempOUT[6]={'-','-','-','-','-','\0'};
char got_time[16]; //an Array of 16 bytes - since the payload size is 16 bytes
char mysettings[16];
float moist1,moist2,moist3,moist4;
float rawmoist1adc,rawmoist2adc,rawmoist3adc, rawmoist4adc;
//Max min usable ADC values for moisture sensors
//Plan to update/calibrate dynamically via another pipe on the radio
float max1=1023.0;
float min1=0.0;
float max2=1023.0;
float min2=0.0;
float max3=1023.0;
float min3=0.0;
float max4=1023.0;
float min4=0.0;
uint8_t onwhichpipedatawasreceived;
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 7, 5, 4, 3, 2);
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10           

RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.              
const uint64_t pipes[2] = { 0xBCBCBCBCBC,0xEDEDEDEDED};
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
 pinMode(DigitalSwitchforMoistureSensor,INPUT);
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
radio.openWritingPipe(pipes[0]);
radio.openReadingPipe(0, pipes[0]); //Need to do this for receive data on Data pipe 0 other wise it stays in closed state.
radio.openReadingPipe(1, pipes[1]); 
radio.startListening();
radio.stopListening();
delay (10);

radio.startListening();
delay(50);
radio.printPrettyDetails();
//radio.printDetails();
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
        
        lcd.setCursor(0, 0);
        lcd.print("M1 = "+String(moist1)+" %");
        lcd.setCursor(0, 1);
      
        lcd.print("M2 = "+String(moist2)+" %");
        
        }
        if (whichscreenamiat == 3)
        {
        lcd.setCursor(0, 0);
        lcd.print("OUT = "+String(tempOUT)+" degC");
        lcd.setCursor(0, 1);
        lcd.print("M3 = "+String(moist3)+" %");
        
        }   

          if (whichscreenamiat == 4)
        {
        lcd.setCursor(0, 0);
        lcd.print("OUT = "+String(tempOUT)+" degC");
        lcd.setCursor(0, 1);
        lcd.print("M4 = "+String(moist4)+" %");
        
        }  
  }

void senddata(){
 digitalWrite(DigitalSwitchforMoistureSensor,HIGH);
delay(10);
 pinMode(DigitalSwitchforMoistureSensor,OUTPUT);
  delay(100);

//Have to send data from 2 thermistors and 3 moisture sensors.
//Temp will be float data. Moistures will be 0-100. raw ADC 1023 is dry, 0 is wet
//Temp in1 prefix A, temp in2 prefix a
//Moist 1 prefix B, moist 2 prefix C, Moist 3 prefix D
  rawmoist1adc =analogRead(MoisturePIN1);
  moist1 = ((max1 - rawmoist1adc)*100)/(max1-min1);
  delay(20);

  rawmoist2adc=analogRead(MoisturePIN2);
  moist2 = ((max2 - rawmoist2adc)*100)/(max2-min2);
  delay(20);

  rawmoist3adc=analogRead(MoisturePIN3);
  moist3 = ((max3 - rawmoist3adc)*100)/(max3-min3);
  delay(20);

  rawmoist4adc=analogRead(MoisturePIN4);
  moist4 = ((max4 - rawmoist4adc)*100)/(max4-min4);
  delay(20);
  char moistdatastr1[16]={'B'};
  dtostrf(rawmoist1adc,6, 1, moistdatastr1+1);//B-100.0 - max space needed = 7 chars per read
  moistdatastr1[7]={'C'};
  dtostrf(rawmoist2adc,6, 1, moistdatastr1+8); //B-999.9C-999.9 = 14 chars. 
  char moistdatastr2[16]={'D'};
  dtostrf(rawmoist3adc,6, 1, moistdatastr2+1);
  moistdatastr2[7]={'E'};
  dtostrf(rawmoist4adc,6, 1, moistdatastr2+8); //D-999.9E-999.9 = 14 chars. 
 //char array for temp data
  char mytempstr[16]={'A'};
  float temp1,temp2;
  temp1=Thermistor(analogRead(ThermistorPIN1));
  delay(1);
  dtostrf(temp1,6, 2, mytempstr+1); //22.22 = 5 chars. -22.22 = 6 chars. need to leave total A-22.22 = 7 Chars 
  temp2=Thermistor(analogRead(ThermistorPIN2));
  delay(1);
  //mytempstr[7]=':'; causing unnecessarry saving issues
  mytempstr[8]='a';
  dtostrf(temp2,6, 2, mytempstr+9); //A-22.22:a-22.22 
delay(10);

 //Switch to Tx mode on the radio
  radio.stopListening();
 delay(20);
//send the moisture data
bool okT = radio.write( &moistdatastr1, 16);
if (okT)
      printf("moist1 OK...\r\n");
    else
     printf("failed.\n\r"); 
delay(20);
okT = radio.write( &moistdatastr2, 16);
if (okT)
      printf("moist2 OK...\r\n");
    else
     printf("failed.\n\r"); 
delay(20);
 
 
//send the temp data
okT = radio.write( &mytempstr, 16);
if (okT)
      printf("temp OK...\r\n");
    else
     printf("failed.\n\r"); 


 delay(20);

//Set the radio back in listen mode
radio.startListening();
delay(20);
//Turn off the moisture switch
pinMode(DigitalSwitchforMoistureSensor,INPUT);
delay(10);

}
void handleconfig(char *myconfigdata)
{
  //order of configdata will be 0xFF 0xFF Max for B 0x11 0x11 min for B, 0xFF 0xFF max for C and so on
  //take the 4 bytes - convert into an integer and use the value.
  //ints to hold the values temporarily before being cast to floats defined above
  int mymax1,mymax2,mymax3,mymax4, mymin1,mymin2, mymin3,mymin4;
 memcpy(&mymax1,myconfigdata,2);
  memcpy(&mymin1,myconfigdata+2,2);
   memcpy(&mymax2,myconfigdata+4,2);
  memcpy(&mymin2,myconfigdata+6,2);
   memcpy(&mymax3,myconfigdata+8,2);
  memcpy(&mymin3,myconfigdata+10,2);
   memcpy(&mymax4,myconfigdata+12,2);
  memcpy(&mymin4,myconfigdata+14,2);
max1=mymax1;
max2=mymax2;
max3=mymax3;
max4=mymax4;
min1=mymin1;
min2=mymin2;
min3=mymin3;
min4=mymin4;
Serial.print("Max1= "); Serial.print(max1); Serial.print("Min1= "); Serial.println(min1);
Serial.print("Max2= "); Serial.print(max2); Serial.print("Min2= "); Serial.println(min2);
Serial.print("Max3= "); Serial.print(max3); Serial.print("Min3= "); Serial.println(min3);
Serial.print("Max4= "); Serial.print(max4); Serial.print("Min4= "); Serial.println(min4);
  }
void loop() {

 

    if( radio.available(&onwhichpipedatawasreceived)){
        if (onwhichpipedatawasreceived ==0){
        radio.read( got_time, 16 );             // Get the payload
       previousRxMillis=millis();
      memcpy(vcc,got_time+1,4);
      memcpy(tempOUT,got_time+6,5);  
        }
        if (onwhichpipedatawasreceived ==1){
        //data received on config pipe. Config to be sent as 16 bytes - 2 bytes max/2 bytes min - 4bytes per sensor x 4 sensors = 16 bytes
        radio.read( mysettings, 16 );             // Get the payload
        handleconfig(mysettings);
      }
        
  
  }

  
      unsigned long currentRxMillis = millis();
if(currentRxMillis - previousRxMillis >= Rx_data_interval) {
   previousRxMillis=currentRxMillis;
    //No data received in last Rx-data_interval
    Serial.println("No data received");
    //Fill with blanks again - until data is received    
     memcpy(vcc,blankvcc,5);
     memcpy(tempOUT,blanktempOUT,6);
}
  
  
      unsigned long currentTxMillis = millis();
if(currentTxMillis - previousTxMillis >= Tx_data_interval) {
   previousTxMillis=currentTxMillis;
    //Time to send out some sensor data on the ether
  senddata();
}

    unsigned long currentScreenMillis = millis();
     
    if ((currentScreenMillis - previousScreenMillis)>=screen_refresh_interval)
    {
        previousScreenMillis = currentScreenMillis;
      lcd.clear();
      drawscreen(i);
      if (i>3){i=0;}
      else    {  i=i+1;}    

      
      
    }


}
