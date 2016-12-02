/*
Created 1 December 2016
by Erica Jewell

For Sea Sensing, a design project bringing citizen science to sea level rise
www.ericajewell.com


Using: 
1. 16x2 LCD display with Hitachi HD44780 driver
   with the LiquidCrystal library
   https://www.arduino.cc/en/Reference/LiquidCrystal
   Pins:
     * LCD RS pin to digital pin 7
     * LCD Enable pin to digital pin 6
     * LCD D4 pin to digital pin 5
     * LCD D5 pin to digital pin 4
     * LCD D6 pin to digital pin 3
     * LCD D7 pin to digital pin 2
     * LCD R/W pin to ground
2. MS5540C Miniature Barometer Module (used here to measure depth and temperature
   using code developed by MiGeRa
   http://files.migera.ru/arduino/ms5540/MS5540.pde
   and the SPI library
   https://www.arduino.cc/en/Reference/SPI
   Pins:
     * MOSI to digital pin 11
     * MISO to digital pin 12
     * SCK to digital pin 14
     * MCLK to digital pin 9 (or use external clock generator on 32kHz)
     * CS is not in use, but might be pin 10
3. DIY salinity sensor made from two nails or other metal probes
   adapted from:
   https://www.teachengineering.org/activities/view/nyu_probe_activity1
   and:
   http://www.octiva.net/projects/ppm/
   Pins:
     * Connect one probe to ground
     * Other probe to digital pin 8
   It is important to calibrate your conductivity meter 
   for accurate salinity readings. You can use deionized
   water and table salt to do this. 1mg of solute per 1L
   of water equals a 1ppm salinity reading, so dissolve 
   2 grams of salt into 1 litre of deionized water for a
   2000ppm reading, pour half out and add half deionized
   water for a 1000ppm reading, and so on. Adjust the code
   according to your calibration.
*/

 // include the library for the LCD display:
#include <LiquidCrystal.h>
 // include the SPI library for the pressure sensor:
#include <SPI.h>

  // initialize the LiquidCrystal library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

 // variables for salinity sensor
const int switchPin = 8; // conductivity pin
int switchState = 0;
int condVal;
 // variable for pressure sensor
const int clock = 9; // generate a MCKL signal pin

void resetsensor() //this function keeps the sketch a little shorter
{
  SPI.setDataMode(SPI_MODE0); 
  SPI.transfer(0x15);
  SPI.transfer(0x55);
  SPI.transfer(0x40);
}

void setup(void) {
  Serial.begin(9600);
    // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
   // define inputs and outputs
  pinMode(switchPin, INPUT);
  SPI.begin(); //see SPI library details on arduino.cc for details
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV32); //divide 16 MHz to communicate on 500 kHz
  pinMode(clock, OUTPUT);
  delay(100);
}

void loop(void) {
 // begin pressure sensor
TCCR1B = (TCCR1B & 0xF8) | 1 ; //generates the MCKL signal
  analogWrite (clock, 128) ; 

  resetsensor(); //resets the sensor - caution: afterwards mode = SPI_MODE0!

   //Calibration word 1
  unsigned int result1 = 0;
  unsigned int inbyte1 = 0;
  SPI.transfer(0x1D); //send first byte of command to get calibration word 1
  SPI.transfer(0x50); //send second byte of command to get calibration word 1
  SPI.setDataMode(SPI_MODE1); //change mode in order to listen
  result1 = SPI.transfer(0x00); //send dummy byte to read first byte of word
  result1 = result1 << 8; //shift returned byte 
  inbyte1 = SPI.transfer(0x00); //send dummy byte to read second byte of word
  result1 = result1 | inbyte1; //combine first and second byte of word
  Serial.print("Calibration word 1 = ");
  Serial.print(result1,HEX);
  Serial.print(" ");  
  Serial.println(result1);  

  resetsensor(); //resets the sensor

   //Calibration word 2; see comments on calibration word 1
  unsigned int result2 = 0;
  byte inbyte2 = 0; 
  SPI.transfer(0x1D);
  SPI.transfer(0x60);
  SPI.setDataMode(SPI_MODE1); 
  result2 = SPI.transfer(0x00);
  result2 = result2 <<8;
  inbyte2 = SPI.transfer(0x00);
  result2 = result2 | inbyte2;
  Serial.print("Calibration word 2 = ");
  Serial.print(result2,HEX);  
  Serial.print(" ");  
  Serial.println(result2);  

  resetsensor(); //resets the sensor

   //Calibration word 3; see comments on calibration word 1
  unsigned int result3 = 0;
  byte inbyte3 = 0;
  SPI.transfer(0x1D);
  SPI.transfer(0x90); 
  SPI.setDataMode(SPI_MODE1); 
  result3 = SPI.transfer(0x00);
  result3 = result3 <<8;
  inbyte3 = SPI.transfer(0x00);
  result3 = result3 | inbyte3;
  Serial.print("Calibration word 3 = ");
  Serial.print(result3,HEX);  
  Serial.print(" ");  
  Serial.println(result3);  

  resetsensor(); //resets the sensor

   //Calibration word 4; see comments on calibration word 1
  unsigned int result4 = 0;
  byte inbyte4 = 0;
  SPI.transfer(0x1D);
  SPI.transfer(0xA0);
  SPI.setDataMode(SPI_MODE1); 
  result4 = SPI.transfer(0x00);
  result4 = result4 <<8;
  inbyte4 = SPI.transfer(0x00);
  result4 = result4 | inbyte4;
  Serial.print("Calibration word 4 = ");
  Serial.print(result4,HEX);
  Serial.print(" ");  
  Serial.println(result4);  
  
   //now we do some bitshifting to extract the calibration factors 
   //out of the calibration words;
  long c1 = (result1 >> 1) & 0x7FFF;
  long c2 = ((result3 & 0x003F) << 6) | (result4 & 0x003F);
  long c3 = (result4 >> 6) & 0x03FF;
  long c4 = (result3 >> 6) & 0x03FF;
  long c5 = ((result1 & 0x0001) << 10) | ((result2 >> 6) & 0x03FF);
  long c6 = result2 & 0x003F;

  Serial.print("c1 = ");
  Serial.println(c1);
  Serial.print("c2 = ");
  Serial.println(c2);
  Serial.print("c3 = ");
  Serial.println(c3);
  Serial.print("c4 = ");
  Serial.println(c4);
  Serial.print("c5 = ");
  Serial.println(c5);
  Serial.print("c6 = ");
  Serial.println(c6);

  resetsensor(); //resets the sensor

   //Pressure:
  unsigned int presMSB = 0; //first byte of value
  unsigned int presLSB = 0; //last byte of value
  unsigned int D1 = 0;
  SPI.transfer(0x0F); //send first byte of command to get pressure value
  SPI.transfer(0x40); //send second byte of command to get pressure value
  delay(35); //wait for conversion end
  SPI.setDataMode(SPI_MODE1); //change mode in order to listen
  presMSB = SPI.transfer(0x00); //send dummy byte to read first byte of value
  presMSB = presMSB << 8; //shift first byte
  presLSB = SPI.transfer(0x00); //send dummy byte to read second byte of value
  D1 = presMSB | presLSB; //combine first and second byte of value
  Serial.print("D1 - Pressure raw = ");
  Serial.println(D1);

  resetsensor(); //resets the sensor  

   //Temperature:
  unsigned int tempMSB = 0; //first byte of value
  unsigned int tempLSB = 0; //last byte of value
  unsigned int D2 = 0;
  SPI.transfer(0x0F); //send first byte of command to get temperature value
  SPI.transfer(0x20); //send second byte of command to get temperature value
  delay(35); //wait for conversion end
  SPI.setDataMode(SPI_MODE1); //change mode in order to listen
  tempMSB = SPI.transfer(0x00); //send dummy byte to read first byte of value
  tempMSB = tempMSB << 8; //shift first byte
  tempLSB = SPI.transfer(0x00); //send dummy byte to read second byte of value
  D2 = tempMSB | tempLSB; //combine first and second byte of value
  Serial.print("D2 - Temperature raw = ");
  Serial.println(D2); //voila!

   //calculation of the real values by means of the calibration factors and the maths
   //in the datasheet. const MUST be long
  const long UT1 = (c5 << 3) + 20224;
  const long dT = D2 - UT1;
  const long TEMP = 200 + ((dT * (c6 + 50)) >> 10);
  const long OFF  = (c2 * 4) + (((c4 - 512) * dT) >> 12);
  const long SENS = c1 + ((c3 * dT) >> 10) + 24576;
  const long X = (SENS * (D1 - 7168) >> 14) - OFF;
  long PCOMP = ((X * 10) >> 5) + 2500;
  float TEMPREAL = TEMP/10;
  float PCOMPHG = PCOMP * 750.06 / 10000; // mbar*10 -> mmHg === ((mbar/10)/1000)*750/06
  
  Serial.print("Real Temperature in C = ");
  Serial.println(TEMPREAL);

  Serial.print("Compensated pressure in mbar = ");
  Serial.println(PCOMP);
  Serial.print("Compensated pressure in mmHg = ");
  Serial.println(PCOMPHG);

  //2-nd order compensation only for T < 20°C or T > 45°C
  
  long T2 = 0;
  float P2 = 0;

  if (TEMP < 200)
    {
      T2 = (11 * (c6 + 24) * (200 - TEMP) * (200 - TEMP) ) >> 20;
      P2 = (3 * T2 * (PCOMP - 3500) ) >> 14;
    }
  else if (TEMP > 450)
    {
      T2 = (3 * (c6 + 24) * (450 - TEMP) * (450 - TEMP) ) >> 20;
      P2 = (T2 * (PCOMP - 10000) ) >> 13;    
    }

  if ((TEMP < 200) || (TEMP > 450))
  {
    const float TEMP2 = TEMP - T2;
    const float PCOMP2 = PCOMP - P2;

    float TEMPREAL2 = TEMP2/10;
    float PCOMPHG2 = PCOMP2 * 750.06 / 10000; // mbar*10 -> mmHg === ((mbar/10)/1000)*750/06

    Serial.print("2-nd Real Temperature in C = ");
    Serial.println(TEMPREAL2);

    Serial.print("2-nd Compensated pressure in mbar = ");
    Serial.println(PCOMP2);
    Serial.print("2-nd Compensated pressure in mmHg = ");
    Serial.println(PCOMPHG2);
  }

  delay(5000);
  
   //convert pressure in mbar to depth in cm
    // at 4 degrees C, 1 mBar in H20 is equal to 1.01971621298 cm.
    // the accuracy of this conversion is questionable, 
    // so it may be better to rely on the mbar reading.
  float depth = PCOMP*1.01971621298;
  
    // begin conductivity reading and salinity conversion
   // adjust according to calibration
  condVal = analogRead(A0);
  float voltage = condVal*(5.0/1023.0);
  float salinity = (voltage*1000)/2;
  
   // print the depth, temperature, and salinity values on the LCD.
  lcd.setCursor(0,0);
  lcd.print("D: ");
  lcd.print(depth, 1); //adding ", 1" rounds the value. remove for whole float
  lcd.print("cm");
  lcd.setCursor(0,1);
  lcd.print("T: ");
  lcd.print(TEMPREAL);
  lcd.print(" *C");
  lcd.print("S: ");
  lcd.print(salinity, 1); //adding ", 1" rounds the value. remove for whole float
  lcd.print(" ppm");

  delay(50); 
}
