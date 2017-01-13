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
int wait_time;
char party_name[30];

void ClearEEPROM() {
  for (int i=0; i<EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // ClearEEPROM();
  pinMode(FONA_RST_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  //enable internal pull up resistor in arduino
  digitalWrite(BUTTON_PIN, HIGH);
  party_id = NO_PARTY;
  wait_time = NO_PARTY;
  DEBUG_PRINTLN_FLASH("Attempting to init display");
  oled.reset(OLED_RST);
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  // invert display?
  // oled.ssd1306WriteCmd(SSD1306_INVERTDISPLAY);
  oled.ssd1306WriteCmd(SSD1306_SEGREMAP);
  oled.ssd1306WriteCmd(SSD1306_COMSCANINC);
  DEBUG_PRINTLN_FLASH("Display successfully initialized");
  DEBUG_PRINTLN_FLASH("Making sure buzzer works.");
  pinMode(BUZZER_PIN, OUTPUT);
  analogWrite(BUZZER_PIN, 255);
  delay(300);
  analogWrite(BUZZER_PIN, 0);
  delay(300);
  analogWrite(BUZZER_PIN, 255);
  delay(300);
  analogWrite(BUZZER_PIN, 0);
  EEPROMRead(buzzer_name_global, sizeof(buzzer_name_global));
  DEBUG_PRINTLN_FLASH("Stored in eeprom: ");
  DEBUG_PRINTLN(buzzer_name_global);
  Serial.println(buzzer_name_global[0], HEX);
  //TODO: there needs to be a "catastrophic" error state to go into
  buzzer_fsm.AddState({INIT_GPRS, INIT, INIT, InitFonaShieldFunc}, INIT_FONA);
  int init_gprs_next_state = CHECK_BUZZER_REGISTRATION;
  if (strlen(buzzer_name_global) == 0 || buzzer_name_global[0] == 0xFFFFFFFF) init_gprs_next_state = GET_BUZZER_NAME;
  buzzer_fsm.AddState({init_gprs_next_state, INIT, INIT, InitGPRSFunc}, INIT_GPRS);
  buzzer_fsm.AddState({IDLE, WAIT_BUZZER_REGISTRATION, INIT, CheckBuzzerRegFunc}, CHECK_BUZZER_REGISTRATION);
  buzzer_fsm.AddState({WAIT_BUZZER_REGISTRATION, INIT, INIT, GetBuzzerNameFunc}, GET_BUZZER_NAME);
  buzzer_fsm.AddState({GET_AVAILABLE_PARTY, INIT, INIT, IdleFunc}, IDLE);
  buzzer_fsm.AddState({IDLE, INIT, INIT, WaitBuzzerRegFunc}, WAIT_BUZZER_REGISTRATION);
  buzzer_fsm.AddState({ACCEPT_AVAILABLE_PARTY, IDLE, INIT, GetAvailPartyFunc}, GET_AVAILABLE_PARTY);
  buzzer_fsm.AddState({HEARTBEAT, IDLE, INIT, AcceptAvailPartyFunc}, ACCEPT_AVAILABLE_PARTY);
  buzzer_fsm.AddState({BUZZ, INIT, IDLE, HeartbeatFunc}, HEARTBEAT);
  buzzer_fsm.AddState({IDLE, INIT, BUZZ, BuzzFunc}, BUZZ);
  buzzer_fsm.AddState({IDLE, INIT, INIT, WakeupFunc}, WAKEUP);
  buzzer_fsm.AddState({SLEEP, INIT, INIT, ShutdownFunc}, SHUTDOWN);
  buzzer_fsm.AddState({IDLE, INIT, INIT, SleepFunc}, SLEEP);
}

long readVcc() {
  long result; // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC)); result = ADCL; result |= ADCH<<8; result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

unsigned long button_press_start = 0;

void loop() {
  if (button_press_start != 0 && millis() - button_press_start >= 5000) {
    Serial.println("Registering that a startup or shutdown has been requested.");
    buzzer_fsm.ShutdownOrStartupRequested();
    button_press_start = 0;
  }
  buzzer_fsm.ProcessState();
  if (digitalRead(BUTTON_PIN) == HIGH && button_press_start == 0) button_press_start = millis();
  if (digitalRead(BUTTON_PIN) == LOW && button_press_start != 0) button_press_start = 0;
  // Serial.println("hello");
  // Serial.print("\n\nVcc: ");
  // Serial.println( readVcc(), DEC );

  // digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  // delay(1000);                       // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  // delay(1000);
  // extern int __heap_start, *__brkval;
  // int v;
  // DEBUG_PRINTLN_FLASH("FREE RAM: ");
  // Serial.println((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
}
