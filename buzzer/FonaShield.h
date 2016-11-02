#ifndef FONASHIELD_H
#define FONASHIELD_H

#include <SoftwareSerial.h>
#include "Macros.h"

#define _baud_rate 4800
#define APN "wholesale"
#define NEW_LINE_BYTES "\r\xA"
#define OK_REPLY F(NEW_LINE_BYTES "OK" NEW_LINE_BYTES)

#define HTTP_TIMEOUT 20000 //ms

class FonaShield {
  private:
    SoftwareSerial *_fona_serial;
    int _rst_pin;
    bool readAvailBytesFromSerial(char *buffer, int buffer_len, unsigned long timeout);
    void resetShield();
    void sendATCommand(FlashStrPtr command, bool use_newline = true);
    bool sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply, unsigned long timeout = 50);
    bool sendATCommandParamCheckReply(FlashStrPtr at_command, FlashStrPtr param_name, FlashStrPtr param_val, FlashStrPtr expected_reply, unsigned long timeout = 50);
    bool sendATCommandCheckReply(FlashStrPtr command, char *buffer, int buffer_len, FlashStrPtr expected_reply, unsigned long timeout = 50);
    bool sendATCommandCheckAck(FlashStrPtr command, unsigned long timeout = 50);
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
};

#endif
