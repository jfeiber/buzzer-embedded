/*
  File:
  Pins.h

  Description:
  Defines for the various peripheral pins.
 */

#ifndef PINS_H
#define PINS_H

#include "Version.h"


#if BOARD_TYPE == V2 || BOARD_TYPE == V1
  #define BUTTON_LOGIC_HIGH LOW
  #define BUTTON_LOGIC_LOW HIGH
#elif BOARD_TYPE == JANKBOARD
  #define BUTTON_LOGIC_HIGH HIGH
  #define BUTTON_LOGIC_LOW LOW
#endif

#define BUZZER_PIN 6
#define FONA_RX_PIN 12
#define FONA_TX_PIN 3

#if BOARD_TYPE == V2 || BOARD_TYPE == V1
  #define FONA_RST_PIN 13
#endif

#define BUTTON_PIN 8
#define ARDUINO_RST_PIN 10
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3D
#define OLED_RST 9

#endif
