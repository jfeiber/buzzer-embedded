#ifndef HELPERS_H
#define HELPERS_H

/*
  File:
  Helpers.h

  Description:
  Contains various helper methods/macros that are useful in the project.

 */

#include <EEPROM.h>
#include <Arduino.h>
#include <limits.h>

#define DEBUG_PRINT_FLASH(str) Serial.print(F(str))
#define DEBUG_PRINTLN_FLASH(str) Serial.println(F(str))
#define DEBUG_PRINT(str) Serial.print(str)
#define DEBUG_PRINTLN(str) Serial.println(str)

#define OLED_PRINTLN_FLASH(str) oled.println(F(str))
#define OLED_PRINT_FLASH(str) oled.print(F(str))

#define NUM_DIGITS(x) ((x == 0) ? 1 : floor(log10(abs(x))) + 1)


typedef char PROGMEM prog_char;
typedef const __FlashStringHelper * FlashStrPtr;

inline void ClearEEPROM() {
  for (int i=0; i<EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
}

inline unsigned long get_button_press_duration(unsigned long button_press_start) {
  // The if statement is to deal with the fact that millis() overflows when the arduino has
  // been running for ~30 minutes.
  if (millis() >= button_press_start) return millis()-button_press_start;
  else return (ULONG_MAX - button_press_start) + millis();
}

inline void PrintFreeRAM() {
  extern int __heap_start, *__brkval;
  int v;
  DEBUG_PRINTLN_FLASH("FREE RAM: ");
  Serial.println((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
}

/*
  * From: https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
  * Uses the internal Arduino 1.1V reference to to calculate what Vcc is.
  * When the Arduino is being powered by the lipo then Vcc is the lipo voltage. When the micro
  * USB cable is plugged in the Vcc will be >= 5V.
  *
  * @return Vcc in mV.
*/
inline long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

#endif
