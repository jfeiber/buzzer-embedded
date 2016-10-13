#ifndef FONASHIELD_H
#define FONASHIELD_H

#include <SoftwareSerial.h>

typedef const __FlashStringHelper * FlashStrPtr;

static const char _expected_ok_res[] PROGMEM = {'\r', 0xA, 'O', 'K', '\r', 0xA, '\0'};

#define _baud_rate 4800
#define _max_line_length 256
#define _max_init_retires 5
#define APN "wholesale"

typedef char PROGMEM 	prog_char;

class FonaShield {
  private:
    SoftwareSerial *_fona_serial;
    int _rst_pin;
    bool readAvailBytesFromSerial(char *buffer, int buffer_len, unsigned long timeout = 100);
    void resetShield();
    void sendATCommand(FlashStrPtr command);
    bool sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply, unsigned long timeout = 100);
    bool checkATCommandReply(FlashStrPtr expected_reply, unsigned long timeout = 100);
  public:
    FonaShield(SoftwareSerial *fona_serial, int rst_pin);
    bool initShield();
    bool enableGPRS();
};

#endif
