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

bool FonaShield::enableGPRS() {
  // Serial.println("in enable GPRS");
  Serial.println("Shutting down connections");
  if (!sendATCommandCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 10000));
  //Close any open PDP contexts
  // if (!sendATCommandCheckReply(F("AT+SAPBR=0,1"), F("OK"), 20000)) return false;
  // if (!sendATCommandCheckReply(F("AT+CGATT?"), F("OK"), 10000)) return false;
  Serial.println("Enabling GPRS");
  if (!sendATCommandCheckReply(F("AT+CGATT=1"), F("OK"), 10000));
  Serial.println("Set bearer profile to GPRS");
  if (!sendATCommandCheckReply(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), F("OK"), 10000));
  Serial.println("Set APN in bearer profile");
  if (!sendATCommandCheckReply(F("AT+SAPBR=3,1,\"APN\",\"wholesale\""), F("OK"), 10000));
  Serial.println("Set APN in other place? Not sure what this is for");
  if (!sendATCommandCheckReply(F("AT+CSTT=\"wholesale\""), F("OK"), 10000));
  Serial.println("Create a PDP context");
  if (!sendATCommandCheckReply(F("AT+SAPBR=1,1"), F("OK"), 30000));
  sendATCommandCheckReply(F("AT+CMEE?"), F(""), 10000);
  sendATCommandCheckReply(F("AT+CEER"), F(""), 10000);
  Serial.println("Bring up wireless connection");
  if (!sendATCommandCheckReply(F("AT+CIICR"), F("OK"), 10000));
  sendATCommandCheckReply(F("AT+HTTPINIT"), F(""), 10000);
  sendATCommandCheckReply(F("AT+HTTPPARA=\"URL\",\"http://ip.jsontest.com/\""), F(""), 10000);
  sendATCommandCheckReply(F("AT+HTTPACTION=0"), F(""), 10000);
  delay(5000);
  sendATCommandCheckReply(F("AT+HTTPREAD"), F(""), 10000);
  sendATCommandCheckReply(F("AT+HTTPTERM"), F(""), 10000);
  return true;
}

bool FonaShield::sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply, unsigned long timeout) {
  sendATCommand(command);
  return checkATCommandReply(expected_reply, timeout);
}

bool FonaShield::checkATCommandReply(FlashStrPtr expected_reply, unsigned long timeout) {
  char rep_buffer[_max_line_length];
  readAvailBytesFromSerial(rep_buffer, sizeof(rep_buffer), timeout);
  Serial.print(F("Received: "));
  Serial.println(rep_buffer);
  return strcmp_P(rep_buffer, (prog_char *)expected_reply) == 0;
}

void FonaShield::sendATCommand(FlashStrPtr command) {
  Serial.print(F("Sent: "));
  Serial.print(command);
  Serial.println("");
  _fona_serial->println(command);
  delay(100);
}

bool FonaShield::readAvailBytesFromSerial(char *buffer, int buffer_len, unsigned long timeout) {
  int i = 0;

  //wait a specified amount of time for bytes to come in
  unsigned long start_time = millis();
  while(millis() - start_time < timeout) if (_fona_serial->available()) break;

  if (!_fona_serial->available()) Serial.println(F("No bytes received in readAvailBytesFromSerial and timeout has expired."));
  //while there are bytes to be read
  while (_fona_serial->available()) {
    char c = _fona_serial->read();
    Serial.print(c);
    // if (c == '\r' || c == 0xA) continue;
    buffer[i] = c;
    i++;
    if (i == buffer_len-1) break;
  }
  Serial.println("");
  buffer[i] = '\0';
  return strlen(buffer) != 0;
}

void FonaShield::resetShield() {
  digitalWrite(_rst_pin, HIGH);
  delay(100);
  digitalWrite(_rst_pin, LOW);
  delay(100);
  digitalWrite(_rst_pin, HIGH);
}
