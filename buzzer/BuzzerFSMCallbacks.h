#ifndef BUZZERFSMCALLBACKS_H
#define BUZZERFSMCALLBACKS_H

#include "Pins.h"
#include <Arduino.h>

int InitFunc(unsigned long state_start_time, int num_iterations_in_state) {
  Serial.println("I will wait for 2 seconds");
  delay(2000);
  return SUCCESS;
}

int BuzzerOnFunc(unsigned long state_start_time, int num_iterations_in_state) {
  analogWrite(BUZZER_PIN, 255);
  Serial.println(millis());
  Serial.println(state_start_time);
  if ( (millis() - state_start_time)/1000 >= 2 ) return SUCCESS;
  return FAILURE;
}

int BuzzerOffFunc(unsigned long state_start_time, int num_iterations_in_state) {
  analogWrite(BUZZER_PIN, 0);
  return SUCCESS;
}

#endif
