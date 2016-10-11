#include <Arduino.h>
#include "FonaShield.h"

FonaShield::FonaShield(SoftwareSerial *fona_serial, int rst_pin) : _fona_serial(fona_serial),
                                                                   _rst_pin(rst_pin) {}

bool FonaShield::Init() {
  //init the serial interface
  _fona_serial->begin(4800);

  ResetShield();

  SendATCommand(F("AT"));

  char res_buffer[_max_line_length];
  ReadLine(res_buffer, sizeof(res_buffer));
  return strcmp_P(res_buffer, _expected_init_res) == 0;
}

void FonaShield::SendATCommand(FlashStrPtr command) {
  _fona_serial->println(command);
  delay(100);
}

void FonaShield::ReadLine(char *buffer, int buffer_len) {
  int i = 0;
  //while there are bytes to be read
  while (_fona_serial->available()) {
    char c = _fona_serial->read();
    buffer[i] = c;
    i++;
    if (i == buffer_len-1) break;
  }
  buffer[i] = '\0';
}

void FonaShield::ResetShield() {
  digitalWrite(_rst_pin, HIGH);
  delay(100);
  digitalWrite(_rst_pin, LOW);
  delay(100);
  digitalWrite(_rst_pin, HIGH);
}
