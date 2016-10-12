#include <Arduino.h>
#include <avr/pgmspace.h>
#include "FonaShield.h"

FonaShield::FonaShield(SoftwareSerial *fona_serial, int rst_pin) : _fona_serial(fona_serial),
                                                                   _rst_pin(rst_pin) {}

bool FonaShield::initShield() {
  //init the serial interface
  _fona_serial->begin(4800);

  resetShield();

  int retries = _max_init_retires;
  do {
    bool res = sendATCommandCheckReply(F("ATE0"), F("OK"));
    if (res) return true;
    retries--;
  } while (retries > 0);
  return false;
}

bool FonaShield::sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply) {
  sendATCommand(command);
  return checkATCommandReply(expected_reply);
}

bool FonaShield::checkATCommandReply(FlashStrPtr expected_reply) {
  char rep_buffer[_max_line_length];
  readAvailBytesFromSerial(rep_buffer, sizeof(rep_buffer));
  return strcmp(rep_buffer, (prog_char *)expected_reply);
}

void FonaShield::sendATCommand(FlashStrPtr command) {
  _fona_serial->println(command);
  delay(100);
}

bool FonaShield::readAvailBytesFromSerial(char *buffer, int buffer_len) {
  int i = 0;
  //while there are bytes to be read
  while (_fona_serial->available()) {
    char c = _fona_serial->read();
    if (buffer[i] == '\r' || buffer[i] == 0xA) continue;
    buffer[i] = c;
    i++;
    if (i == buffer_len-1) break;
  }
  buffer[i] = '\0';
}

void FonaShield::resetShield() {
  digitalWrite(_rst_pin, HIGH);
  delay(100);
  digitalWrite(_rst_pin, LOW);
  delay(100);
  digitalWrite(_rst_pin, HIGH);
}
