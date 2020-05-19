#include "waterrower.h"
/**
*/

#define DEBUG

volatile unsigned long tick             = 0;         // Counts the signals coming from the waterrower
volatile unsigned long lasttick         = 0;         // Saves the last tick
volatile unsigned long meter_per_second = 0.0;       // As the name says. Value for meter per second
volatile unsigned long seconds          = 0;         // Seconds the workout is running
volatile float distance                 = 0.0;       // Distance in meters for this workout

unsigned long last_seconds = 0;              // Which 'seconds' value was send the last time

volatile unsigned long avg_speed_sum    = 0;
volatile unsigned long max_speed        = 0;

/**
   4.805 ticks (interrupts) is equal to one meter distance.
*/
const float ratio = 4.805;

volatile long lastDebounceTime     =  0;      // the last time the output pin was toggled in millis
const unsigned long debounceDelay  = 20;      // Duration in millis to ignore interrupts
volatile boolean measuring_running = false;
/**
   The ISR that is called every time the waterrower gives a signal.
   Ticke is updated. A debounce time is used to avoid ticks that come
   frome bounces.
*/
ICACHE_RAM_ATTR void tick_ISR(void) {
  unsigned long m = millis();
  if (m - lastDebounceTime > debounceDelay) {
    tick++;
  }
  lastDebounceTime = m;
}

void startISR() {
  attachInterrupt(digitalPinToInterrupt(ROWER_PIN), tick_ISR, FALLING);
}

void stopISR() {
#ifdef DEBUG
  Serial.println("Detaching Rower ISR");
#endif
  detachInterrupt(digitalPinToInterrupt(ROWER_PIN));
}

/**
   Activates measuring. If not already measuring, this
   also resets internal variables for measurement.
*/
void startMeasuring() {
  if (measuring_running) return;
  reset();
  measuring_running = true;
  lastDebounceTime = millis();
  startISR();
}

boolean is_measuring() {
  return measuring_running;
}

/**
   Stops the measurement.
   The ISR will be detached, so no more ticks are counted.
   All variables will be resetted.
*/
void stopMeasuring() {
  if (!measuring_running) return;
  stopISR();
  reset();
  measuring_running = false;
  meter_per_second = 0.0;
}

/**
   Sets all variable back to their default value.
   No tickes and no seconds.
*/
void reset() {
  tick = 0;
  lasttick = 0;
  seconds = 0;
  last_seconds = 0;
  distance = 0.0;
  avg_speed_sum = 0;
  max_speed = 0;
}

/**
   The interrupt service routine (ISR) that is called every second.
   If measuring, then the following variables will be updated.

   seconds, meter_per_second, lasttick
*/
ICACHE_RAM_ATTR void timer_0_ISR(void) {
  if (is_measuring()) {
    seconds++;
    unsigned long current_tick = tick;
    distance = (float) (current_tick - lasttick);
    meter_per_second = (distance) / ratio;
    if (max_speed < meter_per_second) {
      max_speed = meter_per_second;
    }
    avg_speed_sum += meter_per_second;
    lasttick = current_tick;
  }
  timer0_write(ESP.getCycleCount() + 80000000); //80Mhz -> 80*10^6 = 1 second
}

/**
   Initialiert den Timer 0.
   Jede Sekunde wird die Funktion timer_0_isr() aufgerufen.
*/
void startClock() {
#ifdef DEBUG
  Serial.println("Starting timer0");
#endif
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(timer_0_ISR);
  timer0_write(ESP.getCycleCount() + 80000000); //80Mhz -> 80*10^6 = 1 second
  interrupts();
}

unsigned long getTicks() {
  return tick;
}

unsigned long getSeconds() {
  return seconds;
}

unsigned long getLastSeconds() {
  return last_seconds;
}

float getMeterPerSecond() {
  return meter_per_second;
}

unsigned long getDistanceInMeter() {
  return (unsigned long) (getTicks() / ratio);
}

void markTime(unsigned long seconds) {
  last_seconds = seconds;
}
