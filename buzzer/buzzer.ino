#include "BuzzerFSMCallbacks.h"
#include "Globals.h"
#include "Pins.h"
#include "EEPROMReadWrite.h"
#include <EEPROM.h>

BuzzerFSM buzzer_fsm({INIT_FONA, INIT, INIT, InitFunc}, INIT);
SoftwareSerial fona_serial = SoftwareSerial(FONA_TX_PIN, FONA_RX_PIN);
FonaShield fona_shield(&fona_serial, FONA_RST_PIN);
SSD1306AsciiAvrI2c oled;
char buzzer_name_global[30];
int party_id;

void ClearEEPROM() {
  for (int i=0; i<EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
}

void setup() {
  Serial.begin(115200);
  // ClearEEPROM();
  pinMode(FONA_RST_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  party_id = NO_PARTY;
  DEBUG_PRINTLN_FLASH("Attempting to init display");
  oled.reset(OLED_RST);
  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  DEBUG_PRINTLN_FLASH("Display successfully initialized");
  pinMode(BUZZER_PIN, OUTPUT);
  EEPROMRead(buzzer_name_global, sizeof(buzzer_name_global));
  Serial.print("Stored in eeprom: ");
  Serial.println(buzzer_name_global);
  //TODO: there needs to be a "catastrophic" error state to go into
  buzzer_fsm.AddState({INIT_GPRS, INIT, INIT, InitFonaShieldFunc}, INIT_FONA);
  int init_gprs_next_state = WAIT_BUZZER_REGISTRATION;
  if (strlen(buzzer_name_global) == 0) init_gprs_next_state = GET_BUZZER_NAME;
  buzzer_fsm.AddState({init_gprs_next_state, INIT, INIT, InitGPRSFunc}, INIT_GPRS);
  buzzer_fsm.AddState({WAIT_BUZZER_REGISTRATION, INIT, INIT, GetBuzzerNameFunc}, GET_BUZZER_NAME);
  buzzer_fsm.AddState({GET_AVAILABLE_PARTY, INIT, INIT, IdleFunc}, IDLE);
  buzzer_fsm.AddState({IDLE, INIT, INIT, WaitBuzzerRegFunc}, WAIT_BUZZER_REGISTRATION);
  buzzer_fsm.AddState({ACCEPT_AVAILABLE_PARTY, IDLE, INIT, GetAvailPartyFunc}, GET_AVAILABLE_PARTY);
  buzzer_fsm.AddState({HEARTBEAT, INIT, INIT, AcceptAvailPartyFunc}, ACCEPT_AVAILABLE_PARTY);
  // buzzer_fsm.AddState
}

void loop() {
  buzzer_fsm.ProcessState();
  extern int __heap_start, *__brkval;
  int v;
  DEBUG_PRINTLN_FLASH("FREE RAM: ");
  Serial.println((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
}
