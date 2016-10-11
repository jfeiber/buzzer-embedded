#ifndef FONASHIELD_H
#define FONASHIELD_H

#include <SoftwareSerial.h>

typedef const __FlashStringHelper * FlashStrPtr;

static const char _expected_init_res[] PROGMEM = "AT\rOK\r";

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
    void Init();
};

#endif
