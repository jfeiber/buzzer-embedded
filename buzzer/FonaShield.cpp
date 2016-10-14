#include <Arduino.h>
#include <avr/pgmspace.h>
#include "FonaShield.h"

FonaShield::FonaShield(SoftwareSerial *fona_serial, int rst_pin) : _fona_serial(fona_serial),
                                                                   _rst_pin(rst_pin) {}

bool FonaShield::initShield() {
  //init the serial interface
  _fona_serial->begin(4800);

  resetShield();

  return sendATCommandCheckReply(F("ATE0"), F("OK"));
}

bool FonaShield::enableGPRS() {
  DEBUG_PRINTLN_FLASH("Attempting to enable GPRS");
  DEBUG_PRINTLN_FLASH("Shutting down connections");
  if (!sendATCommandCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 1000)) return false;
  DEBUG_PRINTLN_FLASH("Shutting down any open PDP contexts");
  if (!sendATCommandCheckAck(F("AT+SAPBR=0,1"), 1000)) return false;
  DEBUG_PRINTLN_FLASH("Attaching to GPRS network");
  if (!sendATCommandCheckReply(F("AT+CGATT=1"), OK_REPLY, 1000)) return false;
  DEBUG_PRINTLN_FLASH("Set bearer profile to GPRS");
  if (!sendATCommandCheckReply(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), OK_REPLY, 1000)) return false;
  DEBUG_PRINTLN_FLASH("Set APN in bearer profile");
  if (!sendATCommandCheckReply(F("AT+SAPBR=3,1,\"APN\",\"" APN "\""), OK_REPLY, 1000)) return false;
  DEBUG_PRINTLN_FLASH("Set APN for PDP contexts");
  if (!sendATCommandCheckReply(F("AT+CSTT=\"" APN "\""), OK_REPLY, 1000)) return false;
  DEBUG_PRINTLN_FLASH("Create a PDP context");
  if (!sendATCommandCheckReply(F("AT+SAPBR=1,1"), OK_REPLY, 1000)) return false;
  return true;
}

bool FonaShield::sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply, unsigned long timeout) {
  sendATCommand(command);
  return checkATCommandReply(expected_reply, timeout);
}

bool FonaShield::sendATCommandCheckAck(FlashStrPtr command, unsigned long timeout) {
  sendATCommand(command);
  char rep_buffer[_max_line_length];
  return readAvailBytesFromSerial(rep_buffer, sizeof(rep_buffer), timeout);
}

bool FonaShield::checkATCommandReply(FlashStrPtr expected_reply, unsigned long timeout) {
  char rep_buffer[_max_line_length];
  readAvailBytesFromSerial(rep_buffer, sizeof(rep_buffer), timeout);
  return strcmp_P(rep_buffer, (prog_char *)expected_reply) == 0;
}

void FonaShield::sendATCommand(FlashStrPtr command) {
  DEBUG_PRINT_FLASH("Sent: ");
  DEBUG_PRINT(command);
  DEBUG_PRINTLN_FLASH("");
  _fona_serial->println(command);
  delay(100);
}

bool FonaShield::readAvailBytesFromSerial(char *buffer, int buffer_len, unsigned long timeout) {
  // int i = 0;
  //
  // //wait a specified amount of time for bytes to come in
  // unsigned long start_time = millis();
  // while(millis() - start_time < timeout) if (_fona_serial->available()) break;
  //
  // if (!_fona_serial->available()) Serial.println(F("No bytes received in readAvailBytesFromSerial and timeout has expired."));
  // //while there are bytes to be read
  // Serial.print("Bytes received: ");
  // while (_fona_serial->available()) {
  //   char c = _fona_serial->read();
  //   // if (c == '\r' || c == 0xA) continue;
  //   Serial.print(c, HEX);
  //   buffer[i] = c;
  //   i++;
  //   if (i == buffer_len-1) break;
  // }
  // Serial.println("");
  // buffer[i] = '\0';
  // return strlen(buffer) != 0;
  int i=0;

  unsigned long last_time_since_bytes = millis();
  int num_lines_read = 0;
  // Serial.print("Got the following bytes: ");
  while (millis() - last_time_since_bytes < timeout) {
    if (_fona_serial->available()) {
      last_time_since_bytes = millis();
      char c = _fona_serial->read();
      if (c == '\r' || c == 0xA) continue;
      buffer[i] = c;
      // Serial.print(c, HEX);
      // if (i > 0 && c == 0xA && buffer[i-1] == '\r') {
      //   num_lines_read++;
      //   if (num_lines_read == num_lines_expected) break;
      // }
      i++;
      if (i == buffer_len-1) break;
    }
  }
  // Serial.println("");
  buffer[i] = '\0';
  DEBUG_PRINT_FLASH("Received: ");
  DEBUG_PRINTLN(buffer);
  return strlen(buffer) != 0;
}

void FonaShield::resetShield() {
  digitalWrite(_rst_pin, HIGH);
  delay(100);
  digitalWrite(_rst_pin, LOW);
  delay(100);
  digitalWrite(_rst_pin, HIGH);
}
