#ifndef FONASHIELD_H
#define FONASHIELD_H

#include <SoftwareSerial.h>

typedef const __FlashStringHelper * FlashStrPtr;


// 4154DDA4F4BDA
static const char _expected_init_res[] PROGMEM = {'A', 'T', '\r', '\r', 0xA, 'O', 'K', '\r', 0xA, '\0'};

class FonaShield {
  private:
    SoftwareSerial *_fona_serial;
    int _max_line_length = 256;
    int _rst_pin;
    int _baud_rate = 4800;
    // const char * PROGMEM progMem_string = "1";
    void ReadLine(char *buffer, int buffer_len);
    void ResetShield();
    void SendATCommand(FlashStrPtr command);
  public:
    FonaShield(SoftwareSerial *fona_serial, int rst_pin);
    bool Init();
};

#endif
