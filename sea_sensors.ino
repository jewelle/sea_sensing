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
2. HC-SR04 ultrasonic sensor
   adapted from:
   http://randomnerdtutorials.com/
   Pins:
     * Trigger (input) pin to digital pin 11
     * Echo (output) pin to digital pin 12
3. DS18B20 temperature sensor
   with the OneWire Dallas Temperature library:
   http://milesburton.com/Dallas_Temperature_Control_Library
   Pins:
     * DQ pin (yellow or white wire) to digital pin 10
*/

 // include the library for the LCD display:
#include <LiquidCrystal.h>
 // include the library for the temperature probe:
#include <OneWire.h>

 // initialize the LiquidCrystal library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
 // initialize the OneWire library with the number of the interface pin
OneWire  ds(10);

 // ultrasonic sensor variables
int trigPin = 11; // input pin
int echoPin = 12; // output pin
float duration, cm, inches;

void setup(void) {
  Serial.begin(9600);
    // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
   // define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}


void loop(void) {
  // beging ultrasonic sensor
   // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
   // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
   // Read the signal from the sensor: a HIGH pulse whose
   // duration is the time (in microseconds) from the sending
   // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
   // convert the time into a distance
  cm = (duration/2) / 29.1;
  inches = (duration/2) / 74; 
  Serial.println(cm);

  
   // begin temperature sensor
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
 
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
   // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 
  

  ds.reset();
  ds.select(addr);
  ds.write(0x44);        // start conversion, with(out) parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
   // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

 ( Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

   // Convert the data to actual temperature
   // because the result is a 16 bit signed integer, it should
   // be stored to an "int16_t" type, which is always 16 bits
   // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
     // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
     //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
   // print temperature values to the serial monitor
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");

   // print the distance between the ultrasonic sensor and the water,
   // and temperature values on the LCD.
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("D: ");
  lcd.print(cm);
  lcd.print(" cm");
  lcd.setCursor(0,1);
  lcd.print("T: ");
  lcd.print(fahrenheit);
  lcd.print(" f");
}
