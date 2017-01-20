/*
  File:
  FonaShield.h

  Description:
  Class that serves as a driver for the FONA 800 cell radio. It's not a complete driver, it only
  includes methods for the functionality that Buzzer needs (HTTP GET/POST over GPRS).
*/

#ifndef FONASHIELD_H
#define FONASHIELD_H

#include <SoftwareSerial.h>
#include "Helpers.h"

// Baud rate of cell radio.
#define _baud_rate 4800
// Ting APN.
#define APN "wholesale"
// All messages received from the cell radio begin and end with these 2 bytes.
#define NEW_LINE_BYTES "\r\xA"
// Standard OK ack after AT command echoing has been turned off.
#define OK_REPLY F(NEW_LINE_BYTES "OK" NEW_LINE_BYTES)

// How long to wait after an HTTP GET or POST request before failing.
#define HTTP_TIMEOUT 20000 //ms

// The default amount of time to wait for new bytes to be received from the cell radio before we
// assume no new bytes are coming.
#define AT_TIMEOUT 50

// Main class that serves as the FONA 800 driver.
class FonaShield {
  private:
    SoftwareSerial *_fona_serial;
    int _rst_pin;
    bool readAvailBytesFromSerial(char *buffer, int buffer_len, unsigned long timeout);
    void resetShield();
    void sendATCommand(FlashStrPtr command, bool use_newline = true);
    bool sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply, unsigned long timeout = AT_TIMEOUT);
    bool sendATCommandParamCheckReply(FlashStrPtr at_command, FlashStrPtr param_name, FlashStrPtr param_val, FlashStrPtr expected_reply, unsigned long timeout = AT_TIMEOUT);
    bool sendATCommandCheckReply(FlashStrPtr command, char *buffer, int buffer_len, FlashStrPtr expected_reply, unsigned long timeout = AT_TIMEOUT);
    bool sendATCommandCheckAck(FlashStrPtr command, unsigned long timeout = AT_TIMEOUT);
    bool checkATCommandReply(FlashStrPtr expected_reply, unsigned long timeout);
    bool checkATCommandReply(FlashStrPtr expected_reply, char *rep_buffer, int buffer_len, unsigned long timeout);
    int getHTTPStatusFromRes(char *rep_buffer);
    int isEqual(char *buf1, FlashStrPtr buf2);
    bool getOneLineHTTPReplyFromRes(char *at_res_buffer, int at_res_buffer_len, char *http_res_buffer, int http_res_buffer_len);
    bool HTTPInit(FlashStrPtr URL);
    int HTTPFail();
    bool setHTTPParam(FlashStrPtr param_name, FlashStrPtr param_val);
    bool sendHTTPDataCheckReply(char *post_data_buffer, int post_data_buffer_len);
    int GetOneLineHTTPRes(char *http_res_buffer, int http_res_buffer_len);
    bool retryATCommand(FlashStrPtr at_command, FlashStrPtr expected_response);
  public:
    FonaShield(SoftwareSerial *fona_serial, int rst_pin);
    bool initShield();
    bool enableGPRS();
    int HTTPGETOneLine(FlashStrPtr URL, char *http_res_buffer, int http_res_buffer_len);
    int HTTPPOSTOneLine(FlashStrPtr URL, char *post_data_buffer, int post_data_buffer_len,
                         char *http_res_buffer, int http_res_buffer_len);
    int GetBatteryVoltage();
};

#endif
