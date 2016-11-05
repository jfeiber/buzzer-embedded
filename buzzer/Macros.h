#ifndef MACROS_H
#define MACROS_H

#define DEBUG_PRINT_FLASH(str) Serial.print(F(str))
#define DEBUG_PRINTLN_FLASH(str) Serial.println(F(str))
#define DEBUG_PRINT(str) Serial.print(str)
#define DEBUG_PRINTLN(str) Serial.println(str)

#define OLED_PRINTLN_FLASH(str) oled.println(F(str))

#define NUM_DIGITS(x) ((x == 0) ? 1 : floor(log10(abs(x))) + 1)

typedef char PROGMEM prog_char;
typedef const __FlashStringHelper * FlashStrPtr;

#endif
