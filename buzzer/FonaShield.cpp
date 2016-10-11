#include <Arduino.h>
#include "FonaShield.h"

FonaShield::FonaShield(SoftwareSerial *fona_serial, int rst_pin) : _fona_serial(fona_serial),
                                                                   _rst_pin(rst_pin) {}

void FonaShield::Init() {
  //init the serial interface
  _fona_serial->begin(4800);

  ResetShield();

  SendATCommand(F("AT"));

  Serial.println("Got this response from FONA: ");
  char res_buffer[_max_line_length];
  ReadLine(res_buffer, sizeof(res_buffer));
  // Serial.print(res_buffer, HEX);
  // serial.print("\n")
  if (strcmp_P(res_buffer, _expected_init_res) == 0) {
    Serial.println(F("yay!"));
  }
  // strcmp_P("askjdfh", F("asdjfh"));
  // F("asjdfsa")->equals(F("ajhsdfg"))
}

void FonaShield::SendATCommand(FlashStrPtr command) {
  _fona_serial->println(command);
  delay(100);
}

void FonaShield::ReadLine(char *buffer, int buffer_len) {
  //while there are bytes to be read
  int i = 0;
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
