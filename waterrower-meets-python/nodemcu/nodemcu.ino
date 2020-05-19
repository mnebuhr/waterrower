/*
 Project WaterRower meets Pythonein

 @author: Marion Nebuhr
 @created: 2020-05-15
 
*/
#include "commands.h"
#include "waterrower.h"


const byte PIN_MODE_WRITE = 1;
const byte PIN_MODE_READ  = 0;

const byte PIN_STATE_HIGH = 1;
const byte PIN_STATE_LOW  = 0;

#define DEBUG

void setup()
{   
  pinMode(LED_BUILTIN, OUTPUT);
  // Setup console
  Serial.begin(9600);
  delay(10);
  
  pinMode(ROWER_PIN, INPUT_PULLUP);
  digitalWrite(ROWER_PIN,HIGH);

  Serial.println("Start measuring");
  startMeasuring();
  startClock();
}


void loop()
{
  delay(1000);
  Serial.println(getDistanceInMeter());
}
