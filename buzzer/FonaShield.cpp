#include <Arduino.h>
#include <avr/pgmspace.h>
#include "FonaShield.h"
#include "Globals.h"

FonaShield::FonaShield(SoftwareSerial *fona_serial, int rst_pin) : _fona_serial(fona_serial),
                                                                   _rst_pin(rst_pin) {}

bool FonaShield::initShield() {
  //init the serial interface
  _fona_serial->begin(4800);

  resetShield();

  if (!retryATCommand(F("AT"), F("AT\xD" NEW_LINE_BYTES "OK" NEW_LINE_BYTES))) {
    DEBUG_PRINTLN_FLASH("No ack from FONA.");
    return false;
  }

  if (!retryATCommand(F("ATE0"), OK_REPLY)) {
    DEBUG_PRINTLN_FLASH("Could not turn echo off.");
    return false;
  }
  return true;
}

bool FonaShield::retryATCommand(FlashStrPtr at_command, FlashStrPtr expected_response) {
  int num_tries = 0;
  while (num_tries < MAX_RETRIES && !sendATCommandCheckReply(at_command, expected_response)) {
    delay(500);
    num_tries++;
  }
  return num_tries < MAX_RETRIES;
}

bool FonaShield::enableGPRS() {
  DEBUG_PRINTLN_FLASH("Attempting to enable GPRS");
  DEBUG_PRINTLN_FLASH("Shutting down connections");
  if (!sendATCommandCheckReply(F("AT+CIPSHUT"), F(NEW_LINE_BYTES "SHUT OK" NEW_LINE_BYTES), 1000)) return false;
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
  if (!sendATCommandCheckReply(F("AT+SAPBR=1,1"), OK_REPLY, 2000)) return false;
  DEBUG_PRINTLN_FLASH("Bring up wireless connection");
  if (!sendATCommandCheckReply(F("AT+CIICR"), OK_REPLY, 1000)) return false;

  return true;
}

int FonaShield::GetOneLineHTTPRes(char *http_res_buffer, int http_res_buffer_len) {
  char at_res_buffer[_max_line_length];
  unsigned long start_time = millis();
  while(sendATCommandCheckReply(F("AT+HTTPREAD"), at_res_buffer, sizeof(at_res_buffer), OK_REPLY, 2000)) {
    if (millis() - start_time > HTTP_TIMEOUT) return TIMEOUT;
    delay(1000);
  }
  int status = getHTTPStatusFromRes(at_res_buffer);
  if (status != 200) {
    DEBUG_PRINTLN_FLASH("Received a response that wasn't 200");
    return HTTPFail();
  }
  sendATCommand(F("AT+HTTPREAD"));
  readAvailBytesFromSerial(at_res_buffer, sizeof(at_res_buffer), 1000);
  if (!getOneLineHTTPReplyFromRes(at_res_buffer, sizeof(at_res_buffer), http_res_buffer, http_res_buffer_len)) return HTTPFail();
  return SUCCESS;
}

int FonaShield::HTTPPOSTOneLine(FlashStrPtr URL, char *post_data_buffer, int post_data_buffer_len,
                                char *http_res_buffer, int http_res_buffer_len) {
  if (!HTTPInit(URL)) return HTTPFail();
  if (!setHTTPParam(F("CONTENT"), F("application/json"))) return HTTPFail();
  if (!sendHTTPDataCheckReply(post_data_buffer, post_data_buffer_len)) return HTTPFail();
  if (!sendATCommandCheckReply(F("AT+HTTPACTION=1"), OK_REPLY)) return HTTPFail();
  if (!GetOneLineHTTPRes(http_res_buffer, http_res_buffer_len)) return HTTPFail();
  return !HTTPFail();
}

bool FonaShield::sendHTTPDataCheckReply(char *post_data_buffer, int post_data_buffer_len) {
  char buf[_max_line_length];
  sprintf_P(buf, (prog_char *)F("AT+HTTPDATA=%d,1000"), post_data_buffer_len);
  DEBUG_PRINT_FLASH("Sent: ");
  DEBUG_PRINTLN(buf);
  _fona_serial->println(buf);
  readAvailBytesFromSerial(buf, sizeof(buf), 500);
  if (!isEqual(buf, F(NEW_LINE_BYTES "DOWNLOAD" NEW_LINE_BYTES))) return false;
  _fona_serial->println(post_data_buffer);
  // delay(2001);
  readAvailBytesFromSerial(buf, sizeof(buf), 1000);
  return isEqual(buf, OK_REPLY);
}

bool FonaShield::setHTTPParam(FlashStrPtr param_name, FlashStrPtr param_val) {
  return sendATCommandParamCheckReply(F("AT+HTTPPARA="), param_name, param_val, OK_REPLY);
}

int FonaShield::HTTPGETOneLine(FlashStrPtr URL, char *http_res_buffer, int http_res_buffer_len) {
  if (!HTTPInit(URL)) return HTTPFail();
  if (!sendATCommandCheckReply(F("AT+HTTPACTION=0"), OK_REPLY)) return HTTPFail();
  if (!GetOneLineHTTPRes(http_res_buffer, http_res_buffer_len)) return HTTPFail();
  return !HTTPFail();
}

int FonaShield::HTTPFail() {
  sendATCommandCheckReply(F("AT+HTTPTERM"), OK_REPLY);
  return ERROR;
}

bool FonaShield::HTTPInit(FlashStrPtr URL) {
  if (!sendATCommandCheckAck(F("AT+HTTPTERM"), 500)) return false;
  if (!sendATCommandCheckReply(F("AT+HTTPINIT"), OK_REPLY)) return false;
  if (!setHTTPParam(F("CID"), F("1"))) return false;
  if (!setHTTPParam(F("URL"), URL)) return false;
  return true;
}

bool FonaShield::getOneLineHTTPReplyFromRes(char *at_res_buffer, int at_res_buffer_len, char *http_res_buffer, int http_res_buffer_len) {
  if (!at_res_buffer || !http_res_buffer) return false;
  char *line = strtok(at_res_buffer, "\r\xA");
  if (!line) return false;
  line = strtok(NULL, "\r\xA");
  int num_bytes_to_copy = (strlen(line) +1 <= http_res_buffer_len) ? strlen(line) : http_res_buffer_len-1;
  memcpy(http_res_buffer, line, num_bytes_to_copy);
  http_res_buffer[num_bytes_to_copy] = '\0';
  return true;
}

int FonaShield::getHTTPStatusFromRes(char *rep_buffer) {
  char *res_ptr = strstr(rep_buffer, ",");
  res_ptr++;
  if (res_ptr == NULL) return -1;
  char http_status_buf[] = {res_ptr[0], res_ptr[1], res_ptr[2], '\0'};
  return atoi(http_status_buf);
}

bool FonaShield::sendATCommandCheckReply(FlashStrPtr command, char *buffer, int buffer_len, FlashStrPtr expected_reply, unsigned long timeout) {
  sendATCommand(command);
  return checkATCommandReply(expected_reply, buffer, buffer_len, timeout);
}

bool FonaShield::sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply, unsigned long timeout) {
  sendATCommand(command);
  return checkATCommandReply(expected_reply, timeout);
}

bool FonaShield::sendATCommandParamCheckReply(FlashStrPtr at_command, FlashStrPtr param_name, FlashStrPtr param_val, FlashStrPtr expected_reply, unsigned long timeout) {
  sendATCommand(at_command, false);
  sendATCommand(F("\""), false);
  sendATCommand(param_name, false);
  sendATCommand(F("\","), false);
  sendATCommand(F("\""), false);
  sendATCommand(param_val, false);
  sendATCommand(F("\""));
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
  return isEqual(rep_buffer, expected_reply);
}

bool FonaShield::checkATCommandReply(FlashStrPtr expected_reply, char *rep_buffer, int buffer_len, unsigned long timeout) {
  readAvailBytesFromSerial(rep_buffer, buffer_len, timeout);
  return isEqual(rep_buffer, expected_reply);
}

int FonaShield::isEqual(char *buf1, FlashStrPtr buf2) {
  return strcmp_P(buf1, (prog_char *)buf2) == 0;
}

void FonaShield::sendATCommand(FlashStrPtr command, bool use_newline) {
  DEBUG_PRINT_FLASH("Sent: ");
  DEBUG_PRINT(command);
  DEBUG_PRINTLN_FLASH("");
  if (use_newline) _fona_serial->println(command);
  else _fona_serial->print(command);
}

bool FonaShield::readAvailBytesFromSerial(char *buffer, int buffer_len, unsigned long timeout) {
  int i=0;
  unsigned long last_time_since_bytes = millis();
  int num_lines_read = 0;
  memset(buffer, 0, buffer_len);
  while (millis() - last_time_since_bytes < timeout) {
    if (_fona_serial->available()) {
      last_time_since_bytes = millis();
      char c = _fona_serial->read();
      buffer[i] = c;
      i++;
      buffer[i] = '\0';
      if (i == buffer_len-1) {
        DEBUG_PRINTLN_FLASH("Maxed out buffer passed to readAvailBytesFromSerial");
        break;
      }
    }
    if (strstr_P(buffer, (prog_char *)F("OK" NEW_LINE_BYTES)) != NULL) {
      DEBUG_PRINTLN_FLASH("Broke early because I found an OK");
       break;
    }
  }
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
