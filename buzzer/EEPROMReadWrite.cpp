#include <EEPROM.h>
#include <Arduino.h>
#include "EEPROMReadWrite.h"



//assumes that the char buf is null terminated
bool EEPROMWrite(const char *buf, int buf_len) {
  if (buf_len > EEPROM.length()) return false;
  int addr = 0;
  while(addr < buf_len) {
    EEPROM.write(addr, buf[addr]);
    addr++;
  }
  return true;
}

bool EEPROMRead(char *buf, int buf_len) {
  if (buf_len > EEPROM.length()) return false;
  int addr = 0;
  while (addr < buf_len) {
    buf[addr] = EEPROM.read(addr);
    if (buf[addr] == '\0') break;
    addr++;
  }
  return true;
}
