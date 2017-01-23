/*
  File:
  FonaShield.cpp

  Description:
  Class that serves as a driver for the FONA 800 cell radio. It's not a complete driver, it only
  includes methods for the functionality that Buzzer needs (HTTP GET/POST over GPRS).
*/

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "FonaShield.h"
#include "Globals.h"

FonaShield::FonaShield(SoftwareSerial *fona_serial, int rst_pin) : _fona_serial(fona_serial),
                                                                   _rst_pin(rst_pin) {}

/*
 * Method that initializes the FONA. Begins a serial connection at 4800 baud and attempts to GET
 * a response from the radio acknowledging that it's there and receiving messages. After we have
 * received an ack from the FONA, we disable command echoing.
 *
 * This method should be called before any of the other methods in this class. The rest of These
 * methods will not work unless the cell radio has been initialized by this method.
 *
 * @return true if the cell radio was successfully initialized, false otherwise.
 *
*/

bool FonaShield::initShield() {
  //init the serial interface
  _fona_serial->begin(4800);
  resetShield();
  if (!retryATCommand(F("AT"), F("AT\xD" NEW_LINE_BYTES "OK" NEW_LINE_BYTES))) return false;
  if (!retryATCommand(F("ATE0"), OK_REPLY)) return false;
  return true;
}

/*
 * This method retries at AT command for MAX_RETRIES number of times.
 *
 * @input a FlashStrPtr representing the AT command to be run.
 * @input a FlashStrPtr representing the expected response from the FONA.
 * @return true if the given command was successfully run in less than MAX_RETRIES number of times,
 * false otherwise.
*/

bool FonaShield::retryATCommand(FlashStrPtr at_command, FlashStrPtr expected_response) {
  int num_tries = 0;
  while (num_tries < MAX_RETRIES && !sendATCommandCheckReply(at_command, expected_response)) {
    delay(500);
    num_tries++;
  }
  return num_tries < MAX_RETRIES;
}

/*
 * This method configures the cell radio for GPRS usage using the Ting network.
 *
 * @return true if the cell radio was successfully configure for GPRS usage, false otherwise.
*/


bool FonaShield::enableGPRS() {
  // DEBUG_PRINTLN_FLASH("Attempting to enable GPRS");
  // DEBUG_PRINTLN_FLASH("Shutting down connections");
  if (!sendATCommandCheckReply(F("AT+CIPSHUT"), F(NEW_LINE_BYTES "SHUT OK" NEW_LINE_BYTES), 1000)) return false;
  // DEBUG_PRINTLN_FLASH("Shutting down any open PDP contexts");
  if (!sendATCommandCheckAck(F("AT+SAPBR=0,1"), 1000)) return false;
  // DEBUG_PRINTLN_FLASH("Attaching to GPRS network");
  if (!sendATCommandCheckReply(F("AT+CGATT=1"), OK_REPLY, 1000)) return false;
  // DEBUG_PRINTLN_FLASH("Set bearer profile to GPRS");
  if (!sendATCommandCheckReply(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), OK_REPLY, 1000)) return false;
  // DEBUG_PRINTLN_FLASH("Set APN in bearer profile");
  if (!sendATCommandCheckReply(F("AT+SAPBR=3,1,\"APN\",\"" APN "\""), OK_REPLY, 1000)) return false;
  // DEBUG_PRINTLN_FLASH("Set APN for PDP contexts");
  if (!sendATCommandCheckReply(F("AT+CSTT=\"" APN "\""), OK_REPLY, 1000)) return false;
  // DEBUG_PRINTLN_FLASH("Create a PDP context");
  if (!sendATCommandCheckReply(F("AT+SAPBR=1,1"), OK_REPLY, 2000)) return false;
  // DEBUG_PRINTLN_FLASH("Bring up wireless connection");
  if (!sendATCommandCheckReply(F("AT+CIICR"), OK_REPLY, 1000)) return false;

  return true;
}

/*
 * Returns the current voltage of the FONA lipo in mV.
 *
 * @return the current voltage of the FONA lipo in mV, or -1 if something went wrong.
*/

int FonaShield::GetBatteryVoltage() {
  char batt_stat_res_buf[BUF_LENGTH_SMALL];
  sendATCommand(F("AT+CBC"));
  if (!readAvailBytesFromSerial(batt_stat_res_buf, sizeof(batt_stat_res_buf), 500)) return -1;
  if (batt_stat_res_buf == NULL) return -1;
  char *voltage_ptr = strrchr(batt_stat_res_buf, ',');
  if (voltage_ptr == NULL) return -1;
  return atoi(++voltage_ptr);
}

/*
 * Fills the given char buf with one line of an HTTP response. It is assumed that an HTTP request
 * of some sort was initiated before this method was called, otherwise all this method will do is
 * eventually return TIMEOUT.
 *
 * If something goes wrong in collecting the response (response is there but something else has gone
 * wrong), this method will automatically close the HTTP request. This method will also close the
 * HTTP request if the HTTP status isn't 200.
 *
 * @input a char buf to fill with one line of an HTTP response.
 * @input the length of that char buf.
 * @return SUCCESS if http_res_buffer is now filled with one line of an HTTP response, TIMEOUT if
 * now response was received in HTTP_TIMEOUT, or ERROR if something went wrong in collecting the
 * response.
*/

int FonaShield::GetOneLineHTTPRes(char *http_res_buffer, int http_res_buffer_len) {
  char at_res_buffer[BUF_LENGTH_LARGE];
  unsigned long start_time = millis();
  while(sendATCommandCheckReply(F("AT+HTTPREAD"), at_res_buffer, sizeof(at_res_buffer), OK_REPLY, 1000)) {
    // Deals with millis() overflowing
    unsigned long elapsed_time = (millis() >= start_time) ? millis() - start_time : (ULONG_MAX - start_time) + millis();
    if (elapsed_time > HTTP_TIMEOUT) return TIMEOUT;
  }
  int status = getHTTPStatusFromRes(at_res_buffer);
  if (status != 200) {
    Serial.println("status fail");
    return HTTPFail();
  }
  sendATCommand(F("AT+HTTPREAD"));
  readAvailBytesFromSerial(at_res_buffer, sizeof(at_res_buffer), 1000);
  if (!getOneLineHTTPReplyFromRes(at_res_buffer, sizeof(at_res_buffer), http_res_buffer, http_res_buffer_len)) {
    Serial.println("failed at getOneLineHTTPReplyFromRes");
    return HTTPFail();
  }
  Serial.println("didn't fail");
  return SUCCESS;
}

/*
 * This method initiates an HTTP POST request for the given URL and collects one line of the HTTP
 * response. It is assumed that the char buf containing the POST data is only one line.
 *
 * @input a FlashStrPtr representing the URL to POST the data to.
 * @input a char buf with 1 line of POST data.
 * @input the length of the above char buf.
 * @input a char buf to put one line of the HTTP reply in.
 * @input the length of the above char buf.
 * @return SUCCESS if everything went smoothly and we POSTed one line of data to the given URL
 * and collected one line of the reply, ERROR otherwise.
*/

int FonaShield::HTTPPOSTOneLine(FlashStrPtr URL, char *post_data_buffer, int post_data_buffer_len,
                                char *http_res_buffer, int http_res_buffer_len) {
  if (!HTTPInit(URL)) return HTTPFail();
  if (!setHTTPParam(F("CONTENT"), F("application/json"))) return HTTPFail();
  if (!sendHTTPDataCheckReply(post_data_buffer, post_data_buffer_len)) return HTTPFail();
  if (!sendATCommandCheckReply(F("AT+HTTPACTION=1"), OK_REPLY)) return HTTPFail();
  if (GetOneLineHTTPRes(http_res_buffer, http_res_buffer_len)) return HTTPFail();
  return !HTTPFail();
}

/*
 * This method sends HTTP POST data for an HTTP request to the cell radio.
 *
 * @input a char buf with the POST data.
 * @input the length of that char buf.
 * @return true if everything went smoothly, false otherwise.
*/

bool FonaShield::sendHTTPDataCheckReply(char *post_data_buffer, int post_data_buffer_len) {
  // the 1000 represents how long in ms the cell radio will wait for more bytes of the POST data
  // before moving on.
  FlashStrPtr command_skeleton = F("AT+HTTPDATA=%d,1000");
  int num_digits_post_data_buf_len = NUM_DIGITS(post_data_buffer_len);
  // -2 because the format specifier won't who up in the actual buf
  char buf[strlen_P((prog_char *)command_skeleton)-2+num_digits_post_data_buf_len+1];
  snprintf_P(buf, sizeof(buf), (prog_char *)command_skeleton, post_data_buffer_len);
  DEBUG_PRINT_FLASH("Sent: ");
  DEBUG_PRINTLN(buf);
  _fona_serial->println(buf);
  readAvailBytesFromSerial(buf, sizeof(buf), 500);
  if (!isEqual(buf, F(NEW_LINE_BYTES "DOWNLOAD" NEW_LINE_BYTES))) return false;
  _fona_serial->println(post_data_buffer);
  // Wait 1000ms for bytes as we specified in the above AT command.
  readAvailBytesFromSerial(buf, sizeof(buf), 1000);
  return isEqual(buf, OK_REPLY);
}

/*
 * Sets a FONA HTTP parameter to a specified value. With the FONA, things like the destination URL
 * of the request are passed as HTTP parameters. Used as part of the process of initializing an
 * HTTP request.
 *
 * @input a FlashStrPtr representing the name of the parameter.
 * @input a FlashStrPtr representing the value to set the above parameter to.
 * @return true if everything went smoothly, false otherwise.
*/

bool FonaShield::setHTTPParam(FlashStrPtr param_name, FlashStrPtr param_val) {
  return sendATCommandParamCheckReply(F("AT+HTTPPARA="), param_name, param_val, OK_REPLY);
}

/*
 * This method performs an HTTP GET request. One line of the response will be placed in the given
 * char buf.
 *
 * @input a FlashStrPtr representing the URL that we will be GETing from.
 * @input a char buf where one line of the response will be placed.
 * @input the length of the above char buf.
 * @return SUCCESS if everything went fine, ERROR otherwise.
*/

int FonaShield::HTTPGETOneLine(FlashStrPtr URL, char *http_res_buffer, int http_res_buffer_len) {
  if (!HTTPInit(URL)) return HTTPFail();
  if (!sendATCommandCheckReply(F("AT+HTTPACTION=0"), OK_REPLY)) return HTTPFail();
  if (!GetOneLineHTTPRes(http_res_buffer, http_res_buffer_len)) return HTTPFail();
  return !HTTPFail();
}

/*
 * This method terminates an HTTP request and returns ERROR. Used as a helper method by the various
 * other methods that make HTTP requests.
 *
 * @return ERROR.
*/

int FonaShield::HTTPFail() {
  sendATCommandCheckReply(F("AT+HTTPTERM"), OK_REPLY);
  return ERROR;
}

/*
 * Initializes an HTTP request. Can be used to initialize either a POST or GET request.
 *
 * @input a FlashStrPtr representing the destination URL of the request.
 * @return true if everything went according to plan, false otherwise.
*/

bool FonaShield::HTTPInit(FlashStrPtr URL) {
  if (!sendATCommandCheckAck(F("AT+HTTPTERM"), 500)) return false;
  if (!sendATCommandCheckReply(F("AT+HTTPINIT"), OK_REPLY)) return false;
  if (!setHTTPParam(F("CID"), F("1"))) return false;
  if (!setHTTPParam(F("URL"), URL)) return false;
  return true;
}

/*
 * This method performs the actual dirty work of extracting one line of an HTTP response from the
 * raw bytes received from the radio.
 *
 * @input a char buf with the raw bytes received from the radio.
 * @input the length of that char buf.
 * @input a char buf where one line of the HTTP response will go.
 * @input the length of the above char buf.
 * @return true if one line of the HTTP response was successfully extracted, false otherwise.
*/

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

/*
 * This method gets the HTTP response status code from the raw bytes received from the radio.
 *
 * @input a char buf containing the raw bytes received from the radio.
 * @return the HTTP response status code, or -1 if something went wrong in the parsing.
*/

int FonaShield::getHTTPStatusFromRes(char *rep_buffer) {
  char *res_ptr = strstr(rep_buffer, ",");
  res_ptr++;
  if (res_ptr == NULL) return -1;
  // HTTP status codes always have exactly 3 digits.
  char http_status_buf[] = {res_ptr[0], res_ptr[1], res_ptr[2], '\0'};
  return atoi(http_status_buf);
}

/*
 * Sends an AT command to the cell radio by writing the command over serial and checks that the
 * reply matches the given expected reply.
 *
 * @input a FlashStrPtr that represents the AT command.
 * @input a char buf to place the response in.
 * @input the length of the above char buf.
 * @input a FlashStrPtr representing the expected response to the AT command.
 * @input an unsigned long that represents how long to wait after not receiving any bytes over
 * serial before returning.
 * @return true if everything went ok with sending the command and the response matches the given
 * expected response, false otherwise.
*/

bool FonaShield::sendATCommandCheckReply(FlashStrPtr command, char *buffer, int buffer_len, FlashStrPtr expected_reply, unsigned long timeout) {
  sendATCommand(command);
  return checkATCommandReply(expected_reply, buffer, buffer_len, timeout);
}

/*
 * Sends an AT command to the cell radio by writing the command over serial and checks that the
 * reply matches the given expected reply.
 *
 * This method does not take a char buf to collect the AT command response.
 *
 * @input a FlashStrPtr that represents the AT command.
 * @input a FlashStrPtr representing the expected response to the AT command.
 * @input an unsigned long that represents how long to wait after not receiving any bytes over
 * serial before returning.
 * @return true if everything went ok with sending the command and the response matches the given
 * expected response, false otherwise.
*/

bool FonaShield::sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply, unsigned long timeout) {
  sendATCommand(command);
  return checkATCommandReply(expected_reply, timeout);
}

/*
 * Sends an AT command where the command requires some parameter. Parameter names and values need to
 * be wrapped in quotations, hence the special method.
 *
 * This method does not have an argument for collecting the AT command response.
 *
 * @input a FlashStrPtr that represents the AT command.
 * @input a FlashStrPtr representing the parameter name.
 * @input a FlashStrPtr representing the parameter value.
 * @input a FlashStrPtr representing the expected response to the AT command.
 * @input an unsigned long that represents how long to wait after not receiving any bytes over
 * serial before returning.
 * @return true if everything went ok with sending the command and the response matches the given
 * expected response, false otherwise.
*/

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

/*
 * Sends an AT command to the cell radio by writing the command over serial and checks that the
 * reply matches the given expected reply.
 *
 * This method does not collect any AT command response, it just checks that some response from
 * the radio was received.
 *
 * @input a FlashStrPtr that represents the AT command.
 * @input an unsigned long that represents how long to wait after not receiving any bytes over
 * serial before returning.
 * @return true if some bytes were received from the cell radio, false otherwise.
*/

bool FonaShield::sendATCommandCheckAck(FlashStrPtr command, unsigned long timeout) {
  sendATCommand(command);
  char rep_buffer[BUF_LENGTH_SMALL];
  return readAvailBytesFromSerial(rep_buffer, sizeof(rep_buffer), timeout);
}

/*
 * This method reads some bytes from the cell radio serial (it is assumed this method is being
 * called after an AT command has been sent and these bytes represent a command response) and checks
 * that they match the given expected reply.
 *
 * This method does not store the AT command reply anywhere.
 *
 * @input a FlashStrPtr that represents the expected AT command reply.
 * @input an unsigned long that represents how long to wait after not receiving any bytes over
 * serial before returning.
 * @return true if the expected reply matches the received reply, false otherwise.
*/

bool FonaShield::checkATCommandReply(FlashStrPtr expected_reply, unsigned long timeout) {
  char rep_buffer[BUF_LENGTH_MEDIUM];
  readAvailBytesFromSerial(rep_buffer, sizeof(rep_buffer), timeout);
  return isEqual(rep_buffer, expected_reply);
}

/*
 * This method reads some bytes from the cell radio serial (it is assumed this method is being
 * called after an AT command has been sent and these bytes represent a command response) and checks
 * that they match the given expected reply.
 *
 * This method does stores the AT command reply in the given char buf.
 *
 * @input a FlashStrPtr that represents the expected AT command reply.
 * @input a char buf to store the AT command reply.
 * @input the length of the above char buf.
 * @input an unsigned long that represents how long to wait after not receiving any bytes over
 * serial before returning.
 * @return true if the expected reply matches the received reply, false otherwise.
*/

bool FonaShield::checkATCommandReply(FlashStrPtr expected_reply, char *rep_buffer, int buffer_len, unsigned long timeout) {
  readAvailBytesFromSerial(rep_buffer, buffer_len, timeout);
  return isEqual(rep_buffer, expected_reply);
}

/*
 * This method checks if the given char buf is equal to the given FlashStrPtr.
 *
 * @input a char buf.
 * @input a FlashStrPtr.
 * @return true if the char buf and the FlashStrPtr are equal (according to strcmp), false otherwise
*/

int FonaShield::isEqual(char *buf1, FlashStrPtr buf2) {
  return strcmp_P(buf1, (prog_char *)buf2) == 0;
}

/*
 * This method actually sends the AT command. It also logs the sent command to serial for debugging
 * purposes.
 *
 * @input a FlashStrPtr representing the AT command.
 * @input a bool. If this bool is true a newline will be written after the AT command, otherwise no
 * newline will be written.
*/

void FonaShield::sendATCommand(FlashStrPtr command, bool use_newline) {
  DEBUG_PRINT_FLASH("Sent: ");
  DEBUG_PRINT(command);
  DEBUG_PRINTLN_FLASH("");
  if (use_newline) _fona_serial->println(command);
  else _fona_serial->print(command);
}

/*
 * This method reads some bytes from cell radio serial and stores them in the given char buf.
 *
 * @input a char buf. This method will null terminate the char buf.
 * @input the length of the above char buf.
 * @input an unsigned long that represents how long to wait after not receiving any bytes over
 * serial before returning.
 * @return true if some bytes were received from the FONA, false otherwise.
*/

bool FonaShield::readAvailBytesFromSerial(char *buffer, int buffer_len, unsigned long timeout) {
  int i=0;

  unsigned long last_time_since_bytes = millis();
  int num_lines_read = 0;
  while (millis() - last_time_since_bytes < timeout) {
    if (_fona_serial->available()) {
      last_time_since_bytes = millis();
      char c = _fona_serial->read();
      buffer[i] = c;
      i++;
      if (i == buffer_len-1) break;
    }
  }
  buffer[i] = '\0';
  DEBUG_PRINT_FLASH("Received: ");
  DEBUG_PRINTLN(buffer);
  return strlen(buffer) != 0;
}

/*
 * This method resets the FONA back to factory configuration.
 *
*/

void FonaShield::resetShield() {
  digitalWrite(_rst_pin, HIGH);
  delay(100);
  digitalWrite(_rst_pin, LOW);
  delay(100);
  digitalWrite(_rst_pin, HIGH);
  sendATCommand(F("ATZ"));
}
