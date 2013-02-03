/*
  Fair Weather Tent aka Tentduino by Craig Cannon
  
  Use an arduino, barometer, and LED strip to 
  create a cool light display that indicates 
  incoming pressure systems, i.e. weather.
  
  Jan 2013
  
  http://craigrcannon.com
  
*/

#include <Wire.h>
// barometer library from adafruit https://github.com/adafruit/Adafruit-BMP085-Library
#include <Adafruit_BMP085.h>
// LED library from adafruit https://github.com/adafruit/LPD8806
#include "LPD8806.h"
#include "SPI.h"

int dataPin = 2;
int clockPin = 3;
int switchPin = 8;
boolean lastButton = LOW;
boolean currentButton = LOW;
boolean lightSwitch = false;
int i = 0;
boolean firstLoop = true;
double startupReading = 0;
int stripRed = 5;
int stripGreen = 5;
int stripBlue = 5;

// I have a 2m strip with 64 LEDs in total
LPD8806 strip = LPD8806(64, dataPin, clockPin);

Adafruit_BMP085 bmp;
  
void setup() {
  Serial.begin(9600);
  pinMode(switchPin, INPUT);
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }
    // Start up the LED strip
  strip.begin();

  // Update the strip, to start they are all 'off'
  strip.show();

}

boolean debounce(boolean last)
{
  boolean current = digitalRead(switchPin);
  if (last != current)
  {
    delay(5);
    current = digitalRead(switchPin);
  }
  return current;
}
  
void loop() {
    currentButton = debounce(lastButton);
    
    // checking to see if switch if flipped 
    if (lastButton == LOW && currentButton == HIGH)
    {
      Serial.println();
      Serial.println("SWITCH FLIPPED");
      lightSwitch = !lightSwitch;
    }
    
    // reading and setting initial air pressure
    initialP();
    // reading current air pressure
    double currentP = bmp.readPressure();
    // finding change between the air pressure on start and the current
    // multiplying by 100000 to make the numbers easier to work with
    double deltaP = (currentP - startupReading)/startupReading * 100000;

    // this is for viewing the live data in the serial monitor
    Serial.println();
    Serial.print("startupReading: ");
    Serial.print(startupReading);
    Serial.print(" Pa");
    Serial.println();
    Serial.print("currentP: ");
    Serial.print(currentP);
    Serial.print(" Pa");
    Serial.println();
    Serial.print("Change: ");
    Serial.print(deltaP);
    Serial.println();

    if (lightSwitch == true)
    {
      // white LEDs on hitting the switch
      while (i < 1) {
      dither(strip.Color(5,5,5), 25);
      i++;
      }
      // the three IF statements below control the LED colors
      // using the deltaP variable, they color the LEDs BLUE, RED, 
      // or WHITE. the brightness of the color depends on how much of a change
      // the current pressure is from the pressure on startup.
      // the stripRed, stripGreen, and stripBlue variables are used to adjust the R,G,B
      // values of the LED strip as the brightness increases so the strip glows more 
      // white-red or white-blue instead of just bright red or blue.
     
      // if the change in pressure is positive, LEDs are BLUE
      if (deltaP > 0) {
        deltaP = deltaP + 5;
        stripRed = 5 + deltaP/3;
        stripGreen = 5 + deltaP/3;
        dither(strip.Color(stripRed, stripGreen, deltaP), 25);
        delay (10);
      }
      // if the change in pressure is negative, LEDs are RED
      if (deltaP < 0) {
        deltaP = (deltaP * -1) + 5;
        stripGreen = 5 + deltaP/3;
        stripBlue = 5 + deltaP/3;
        dither(strip.Color(deltaP, stripGreen, stripBlue), 25);
        delay (10);
      }
      // if the change in pressure is 0, LEDs are WHITE
      if (deltaP == 0)
      {
        Serial.print("Zero");
        Serial.println();
        dither(strip.Color(10, 10, 10), 10);
        delay (10);
      }

    }  
    
    // if the button is pressed to turn off the lights
    // this is the power-down light sequence
    if (lightSwitch == false)
    {
      i = 63;
      while (i < 64 && i > -1) {
      strip.setPixelColor(i, 0, 0, 0);
      strip.show();
      i--;
      }
    }
    
    lastButton = currentButton;   
    delay(100);
}

// this collects and stores the air pressure on startup
void initialP()
  {
    if (firstLoop == true)
    {
    startupReading = bmp.readPressure();
    firstLoop = false;
    }
  }

// this is from adafruit
// I think the only adjustment I made was removing a delay at the end
//
// An "ordered dither" fills every pixel in a sequence that looks
// sparkly and almost random, but actually follows a specific order.
void dither(uint32_t c, uint8_t wait) {

  // Determine highest bit needed to represent pixel index
  int hiBit = 0;
  int n = strip.numPixels() - 1;
  for(int bit=1; bit < 0x8000; bit <<= 1) {
    if(n & bit) hiBit = bit;
  }

  int bit, reverse;
  for(int i=0; i<(hiBit << 1); i++) {
    // Reverse the bits in i to create ordered dither:
    reverse = 0;
    for(bit=1; bit <= hiBit; bit <<= 1) {
      reverse <<= 1;
      if(i & bit) reverse |= 1;
    }
    strip.setPixelColor(reverse, c);
    strip.show();
    delay(wait);
  }
}

