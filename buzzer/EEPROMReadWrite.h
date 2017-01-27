/*
  File:
  EEPROMReadWrite.h

  Description:
  Contains a few helper methods for reading/writing from the EEPROM.
*/

#ifndef EEPROMREADWRITE_H
#define EEPROMREADWRITE_H

#include <EEPROM.h>
#include <Arduino.h>

#define LONGEST_BUZZER_NAME 26
#define BASE_ADDRESS 0
#define LONGEST_PARTY_NAME 20

struct EEPROMData {
  char buzzer_name[LONGEST_BUZZER_NAME+1];
  int curr_party_id;
  short wait_time;
  // party names received from the API calls are truncated at 20 characters.
  char party_name[LONGEST_PARTY_NAME+1];
};

/*
 * Writes the given char buf to the EEPROM starting at address 0.
 *
 * This will overwrite any data already at addr 0.
 *
 * @input a char buf, assumed to be null terminated. Bad things will happen if it isn't and you
 * call EEPROMRead.
 * @input the length of the char buf.
 * @return false if there was a problem (buffer length longer than EEPROM size), true otherwise.
*/

inline bool EEPROMWriteBuzzerName(const char *buf, int buf_len) {
  if (buf_len > EEPROM.length()) return false;
  int addr = 0;
  while(addr < buf_len) {
    EEPROM.write(addr, buf[addr]);
    addr++;
  }
  return true;
}

/*
 * Reads bytes from the EEPROM starting at addr 0 into the provided char buf.
 *
 * @input a char buf. This function does not NULL terminate the char buf.
 * @input the length of the char buf.
 * @return false if there was a problem (buffer length longer than EEPROM size), true otherwise.
*/

inline bool EEPROMRead(char *buf, int buf_len) {
  if (buf_len > EEPROM.length()) return false;
  int addr = 0;
  while (addr < buf_len) {
    buf[addr] = EEPROM.read(addr);
    if (buf[addr] == '\0') break;
    addr++;
  }
  return true;
}

/*
 * Writes the EEPROMData struct to the EEPROM.
*/

inline void EEPROMWrite(EEPROMData *data) {
  EEPROM.put(BASE_ADDRESS, *data);
}

#endif
