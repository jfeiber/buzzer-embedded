#ifndef EEPROMREADWRITE_H
#define EEPROMREADWRITE_H

bool EEPROMWrite(const char *buf, int buf_len);
bool EEPROMRead(char *buf, int buf_len);

#endif
