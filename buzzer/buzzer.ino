/*
  File:
  buzzer.ino

  Description:
  The main file in this project. Includes the setup() and loop() methods. setup() is called by the
  Arduino on reset and loop is basically the main method of the program that's called repeatedly
  by the Arduino after setup() has finished.

*/

#include "Helpers.h"
#include "BuzzerFSMCallbacks.h"
#include "Globals.h"
#include "Pins.h"
#include "EEPROMReadWrite.h"
#include "LPF.h"

// Initializations of global variables definied in "Globals.h".
BuzzerFSM buzzer_fsm({INIT_FONA, INIT, INIT, InitFunc}, INIT);
SoftwareSerial fona_serial = SoftwareSerial(FONA_TX_PIN, FONA_RX_PIN);
FonaShield fona_shield(&fona_serial, FONA_RST_PIN);
SSD1306AsciiAvrI2c oled;
// char eeprom_data.buzzer_name[30];
// int party_id = NO_PARTY;
EEPROMData eeprom_data;
short wait_time = NO_PARTY;
char party_name[20];
short batt_percentage = 100;
bool has_system_been_initialized = false;
unsigned long button_press_start = 0;
unsigned long last_batt_update = 0;
bool usb_cabled_plugged_in = false;

/*
 * Adds all states to the Buzzer FSM.
*/

void init_fsm() {
  buzzer_fsm.AddState({INIT_GPRS, INIT, INIT, InitFonaShieldFunc}, INIT_FONA);
  int init_gprs_next_state = CHECK_BUZZER_REGISTRATION;
  if (strlen(eeprom_data.buzzer_name) == 0 || eeprom_data.buzzer_name[0] == 0xFFFFFFFF) init_gprs_next_state = GET_BUZZER_NAME;
  int check_buzzer_reg_next_state = (eeprom_data.curr_party_id != NO_PARTY && eeprom_data.curr_party_id != 0) ? HEARTBEAT : IDLE;
  buzzer_fsm.AddState({init_gprs_next_state, INIT, INIT, InitGPRSFunc}, INIT_GPRS);
  buzzer_fsm.AddState({check_buzzer_reg_next_state, FATAL_ERROR, WAIT_BUZZER_REGISTRATION, CheckBuzzerRegFunc}, CHECK_BUZZER_REGISTRATION);
  buzzer_fsm.AddState({WAIT_BUZZER_REGISTRATION, FATAL_ERROR, FATAL_ERROR, GetBuzzerNameFunc}, GET_BUZZER_NAME);
  buzzer_fsm.AddState({GET_AVAILABLE_PARTY, FATAL_ERROR, FATAL_ERROR, IdleFunc}, IDLE);
  buzzer_fsm.AddState({IDLE, FATAL_ERROR, FATAL_ERROR, WaitBuzzerRegFunc}, WAIT_BUZZER_REGISTRATION);
  buzzer_fsm.AddState({ACCEPT_AVAILABLE_PARTY, FATAL_ERROR, IDLE, GetAvailPartyFunc}, GET_AVAILABLE_PARTY);
  buzzer_fsm.AddState({HEARTBEAT, FATAL_ERROR, FATAL_ERROR, AcceptAvailPartyFunc}, ACCEPT_AVAILABLE_PARTY);
  buzzer_fsm.AddState({BUZZ, FATAL_ERROR, IDLE, HeartbeatFunc}, HEARTBEAT);
  buzzer_fsm.AddState({IDLE, FATAL_ERROR, BUZZ, BuzzFunc}, BUZZ);
  buzzer_fsm.AddState({IDLE, INIT, HEARTBEAT, WakeupFunc}, WAKEUP);
  buzzer_fsm.AddState({SLEEP, FATAL_ERROR, FATAL_ERROR, ShutdownFunc}, SHUTDOWN);
  buzzer_fsm.AddState({IDLE, FATAL_ERROR, FATAL_ERROR, SleepFunc}, SLEEP);
  buzzer_fsm.AddState({IDLE, FATAL_ERROR, FATAL_ERROR, ChargeFunc}, CHARGING);
  buzzer_fsm.AddState({INIT, INIT, INIT, FatalErrorFunc}, FATAL_ERROR);
}

/*
 * The name of the Buzzer is stored in EEPROM. This function reads the bytes at the location
 * where the name should be stored (0x0) and stores it in a global variable.
*/

void get_buzzer_name_from_eeprom() {
  // EEPROMRead(eeprom_data.buzzer_name, sizeof(eeprom_data.buzzer_name));
  EEPROM.get(0, eeprom_data);
  DEBUG_PRINTLN_FLASH("Stored in eeprom: ");
  DEBUG_PRINTLN(eeprom_data.buzzer_name);
  DEBUG_PRINTLN(eeprom_data.curr_party_id);
}

/*
 * Buzzes the vibration motor twice. Used in the setup sequence to verify that the buzzer is
 * working.
*/

void buzz_twice() {
  analogWrite(BUZZER_PIN, 255);
  delay(300);
  analogWrite(BUZZER_PIN, 0);
  delay(300);
  analogWrite(BUZZER_PIN, 255);
  delay(300);
  analogWrite(BUZZER_PIN, 0);
}

void setup_pins() {
  digitalWrite(ARDUINO_RST_PIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FONA_RST_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ARDUINO_RST_PIN, OUTPUT);
  // enable internal pull up resistor in arduino
  digitalWrite(BUTTON_PIN, HIGH);
}

/*
  * Initializes the OLED screen.
*/

void init_oled() {
  oled.reset(OLED_RST);
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  // invert display?
  // oled.ssd1306WriteCmd(SSD1306_INVERTDISPLAY);
  oled.ssd1306WriteCmd(SSD1306_SEGREMAP);
  oled.ssd1306WriteCmd(SSD1306_COMSCANINC);
}

/*
 * Called on reset. Sets up the GPIO pins in the right modes, initializes the OLED, tests the
 * vibration motor, gets the buzzer name from the EEPROM (if there is one), and Initializes
 * the FSM.
*/

void setup() {
  Serial.begin(115200);
  // ClearEEPROM();
  setup_pins();
  init_oled();
  buzz_twice();
  get_buzzer_name_from_eeprom();
  if (eeprom_data.buzzer_name[0] == 0xFFFFFFFF) eeprom_data.curr_party_id = -1;
  init_fsm();
}

/*
 * The core method of Buzzer. After setup has completed, this method is called repeatedly in an
 * endless loop.
 *
 * This method processes the current FSM state and feeds the current battery voltage into a low
 * pass filter (to reduce noise) every 15 seconds. If something has happened with one of the
 * peripherals (USB cable plugged in, button pressed), this method will tell the FSM about that
 * event.
*/

void loop() {

  // Do the work of the current FSM state.
  buzzer_fsm.ProcessState();

  // Feed the current battery voltage (the battery voltage of the FONA lipo and the battery voltage
  // of the arduino lipo combined) into a LPF every 7.5 seconds.
  if (last_batt_update == 0 || millis() - last_batt_update >= 7500) {
    int fona_batt_voltage = fona_shield.GetBatteryVoltage();

    // If readVcc > 4.3V (4300mV), that means the USB cable is plugged in and we should read the
    // Arduino lipo voltage from A0. The -400 at the end is a fudge factor because the ADC on the
    // Arduino has an inherent bias. When the Arduino is running of the lipo the battery voltage
    // is just Vcc. We can't measure the lipo voltage accurately from A0 because the reference
    // voltage isn't constant when running of the battery (the battery is draining).
    int arduino_batt_voltage = (readVcc() >= 4300) ? ((analogRead(A0)/1023.0*5.0)*1000)-400 : readVcc();
    if (fona_batt_voltage != -1) {
      int total_batt_voltage = fona_batt_voltage + arduino_batt_voltage;
      int instantaneous_total_batt_voltage = ((total_batt_voltage-7400)/(float)(8400-7400))*100;
      if (get_curr_lpf_val() == 0) seed_lpf(instantaneous_total_batt_voltage);
      add_val_to_lpf(instantaneous_total_batt_voltage);
      batt_percentage = get_curr_lpf_val();
    }
    last_batt_update = millis();
  }

  // Poke the FSM if the the USB cable has been plugged in or unplugged.
  if (readVcc() >= 4300 && !usb_cabled_plugged_in) {
    usb_cabled_plugged_in = true;
    buzzer_fsm.USBCablePluggedIn();
  }
  if (readVcc() < 4300 && usb_cabled_plugged_in) {
    usb_cabled_plugged_in = false;
    buzzer_fsm.USBCableUnplugged();
  }

  // Record the start time of a button press.
  if (digitalRead(BUTTON_PIN) == LOW && button_press_start == 0) button_press_start = millis();

  // If a button press duration is longer than 5 seconds (5000 ms), poke the FSM.
  if (button_press_start != 0) {
    unsigned long button_press_duration = get_button_press_duration(button_press_start);
    if (button_press_duration >= 5000) {
      DEBUG_PRINTLN_FLASH("Long button press registered");
      buzzer_fsm.LongButtonPress();
      button_press_start = 0;
    }
  }

  // If it was a short button press and the button has now been released, poke the FSM.
  if (digitalRead(BUTTON_PIN) == HIGH && button_press_start != 0) {
    unsigned long button_press_duration = get_button_press_duration(button_press_start);
    if (button_press_duration > 0 && button_press_duration < 5000) {
      DEBUG_PRINTLN_FLASH("Short button press registered.");
      buzzer_fsm.ShortButtonPress();
    }
    button_press_start = 0;
  }
}
