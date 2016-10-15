#ifndef FONASHIELD_H
#define FONASHIELD_H

#include <SoftwareSerial.h>

typedef const __FlashStringHelper * FlashStrPtr;

#define _baud_rate 4800
#define _max_line_length 256
#define _max_init_retires 5
#define APN "wholesale"
#define NEW_LINE_BYTES "\r\xA"
#define OK_REPLY F(NEW_LINE_BYTES "OK" NEW_LINE_BYTES)

#define DEBUG_PRINT_FLASH(str) Serial.print(F(str))
#define DEBUG_PRINTLN_FLASH(str) Serial.println(F(str))
#define DEBUG_PRINT(str) Serial.print(str)
#define DEBUG_PRINTLN(str) Serial.println(str)

typedef char PROGMEM 	prog_char;

class FonaShield {
  private:
    SoftwareSerial *_fona_serial;
    int _rst_pin;
    bool readAvailBytesFromSerial(char *buffer, int buffer_len, unsigned long timeout);
    void resetShield();
    void sendATCommand(FlashStrPtr command, bool use_newline = true);
    bool sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply, unsigned long timeout = 50);
    bool sendATCommandParamCheckReply(FlashStrPtr command, FlashStrPtr param, FlashStrPtr expected_reply, unsigned long timeout = 50);
    bool sendATCommandCheckReply(FlashStrPtr command, char *buffer, int buffer_len, FlashStrPtr expected_reply, unsigned long timeout = 50);
    bool sendATCommandCheckAck(FlashStrPtr command, unsigned long timeout = 50);
    bool checkATCommandReply(FlashStrPtr expected_reply, unsigned long timeout);
    bool checkATCommandReply(FlashStrPtr expected_reply, char *rep_buffer, int buffer_len, unsigned long timeout);
    int getHTTPStatusFromRes(char *rep_buffer);
    int isEqual(char *buf1, FlashStrPtr buf2);
    bool getOneLineHTTPReplyFromRes(char *at_res_buffer, int at_res_buffer_len, char *http_res_buffer, int http_res_buffer_len);
    bool HTTPInit(FlashStrPtr URL);
    bool HTTPFail();
  public:
    FonaShield(SoftwareSerial *fona_serial, int rst_pin);
    bool initShield();
    bool enableGPRS();
    bool HTTPGETOneLine(FlashStrPtr URL, char *http_res_buffer, int http_res_buffer_len);
};

#endif
