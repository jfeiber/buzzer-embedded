#ifndef FONASHIELD_H
#define FONASHIELD_H

#include <SoftwareSerial.h>

typedef const __FlashStringHelper * FlashStrPtr;

#define _baud_rate 4800
#define _max_line_length 256
#define _max_init_retires 5
#define APN "wholesale"
#define OK_REPLY F("OK")

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
    void sendATCommand(FlashStrPtr command);
    bool sendATCommandCheckReply(FlashStrPtr command, FlashStrPtr expected_reply, unsigned long timeout = 50);
    bool checkATCommandReply(FlashStrPtr expected_reply, unsigned long timeout);
    bool sendATCommandCheckAck(FlashStrPtr command, unsigned long timeout);
  public:
    FonaShield(SoftwareSerial *fona_serial, int rst_pin);
    bool initShield();
    bool enableGPRS();
};

#endif
