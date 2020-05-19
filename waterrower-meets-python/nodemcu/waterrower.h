#ifndef WATERROWER_H
#define WATERROWER_H

#include <ESP8266WiFi.h>
//#include <PubSubClient.h>

#include <stddef.h>
#include <stdlib.h>

const int ROWER_PIN = 4;
const int LED_PIN   = 5;

boolean is_measuring();

void startMeasuring();

void stopMeasuring();

void reset();

void startClock();

unsigned long getSeconds();

unsigned long getLastSeconds();

unsigned long getTicks();

float getMeterPerSecond();

void markTime(unsigned long seconds);

unsigned long getDistanceInMeter();

#endif
