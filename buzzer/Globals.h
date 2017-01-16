#ifndef GLOBALS_H
#define GLOBALS_H

#include <SoftwareSerial.h>
#include "BuzzerFSM.h"
#include "FonaShield.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

extern BuzzerFSM buzzer_fsm;
extern SoftwareSerial fona_serial;
extern FonaShield fona_shield;
extern SSD1306AsciiAvrI2c oled;
enum ret_vals {SUCCESS, ERROR, REPEAT, TIMEOUT};
//longest length of buzzer name should be around this
extern char buzzer_name_global[30];
extern int party_id;
extern int wait_time;
extern char party_name[30];
extern int batt_percentage;
extern bool has_system_been_initialized;
extern unsigned long button_press_start;
extern bool usb_cabled_plugged_in;

#define MAX_RETRIES 10
#define _max_line_length 120
#define NO_PARTY -1

#endif
