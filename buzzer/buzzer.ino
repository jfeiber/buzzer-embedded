#include "Helpers.h"
#include "BuzzerFSMCallbacks.h"
#include "Globals.h"
#include "Pins.h"
#include "EEPROMReadWrite.h"
#include "LPF.h"

BuzzerFSM buzzer_fsm({INIT_FONA, INIT, INIT, InitFunc}, INIT);
SoftwareSerial fona_serial = SoftwareSerial(FONA_TX_PIN, FONA_RX_PIN);
FonaShield fona_shield(&fona_serial, FONA_RST_PIN);
SSD1306AsciiAvrI2c oled;
char buzzer_name_global[30];
int party_id = NO_PARTY;
int wait_time = NO_PARTY;
char party_name[30];
int batt_percentage = 100;
bool has_system_been_initialized = false;
unsigned long button_press_start = 0;
unsigned long last_batt_update = 0;
bool usb_cabled_plugged_in = false;

void init_fsm() {
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
  buzzer_fsm.AddState({IDLE, INIT, INIT, ChargeFunc}, CHARGING);
}

void get_buzzer_name_from_eeprom() {
  EEPROMRead(buzzer_name_global, sizeof(buzzer_name_global));
  DEBUG_PRINTLN_FLASH("Stored in eeprom: ");
  DEBUG_PRINTLN(buzzer_name_global);
  Serial.println(buzzer_name_global[0], HEX);
}

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
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FONA_RST_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  // enable internal pull up resistor in arduino
  // digitalWrite(BUTTON_PIN, HIGH);
}

void init_oled() {
  oled.reset(OLED_RST);
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  // invert display?
  // oled.ssd1306WriteCmd(SSD1306_INVERTDISPLAY);
  oled.ssd1306WriteCmd(SSD1306_SEGREMAP);
  oled.ssd1306WriteCmd(SSD1306_COMSCANINC);
}

void setup() {
  Serial.begin(115200);
  // ClearEEPROM();
  setup_pins();
  init_oled();
  buzz_twice();
  get_buzzer_name_from_eeprom();
  init_fsm();
}

void loop() {
  buzzer_fsm.ProcessState();
  if (last_batt_update == 0 || millis() - last_batt_update >= 15000) {
    int fona_batt_voltage = fona_shield.GetBatteryVoltage();
    int arduino_batt_voltage = (readVcc() >= 5000) ? ((analogRead(A0)/1023.0*5.0)*1000)-200 : readVcc();
    if (fona_batt_voltage != -1) {
      int total_batt_voltage = fona_batt_voltage + arduino_batt_voltage;
      int instantaneous_total_batt_voltage = ((total_batt_voltage-7400)/(float)(8400-7400))*100;
      if (get_curr_lpf_val() == 0) seed_lpf(instantaneous_total_batt_voltage);
      add_val_to_lpf(instantaneous_total_batt_voltage);
      batt_percentage = get_curr_lpf_val();
    }
    last_batt_update = millis();
  }
  if (readVcc() >= 5000 && !usb_cabled_plugged_in) {
    usb_cabled_plugged_in = true;
    buzzer_fsm.USBCablePluggedIn();
  }
  if (readVcc() < 5000 && usb_cabled_plugged_in) {
    usb_cabled_plugged_in = false;
    buzzer_fsm.USBCableUnplugged();
  }

  if (digitalRead(BUTTON_PIN) == HIGH && button_press_start == 0) button_press_start = millis();

  if (button_press_start != 0) {
    // The ternary operator is to deal with the fact that millis() overflows when the arduino has been running
    // for ~30 minutes.
    unsigned long button_press_duration = get_button_press_duration(button_press_start);
    DEBUG_PRINTLN_FLASH("button press duration: ");
    Serial.println(button_press_duration);
    if (button_press_duration >= 5000) {
      DEBUG_PRINTLN_FLASH("Long button press registered");
      buzzer_fsm.LongButtonPress();
      button_press_start = 0;
    }
  }

  if (digitalRead(BUTTON_PIN) == LOW && button_press_start != 0) {
    unsigned long button_press_duration = get_button_press_duration(button_press_start);
    if (button_press_duration > 0 && button_press_duration < 5000) {
      DEBUG_PRINTLN_FLASH("Short button press registered.");
      buzzer_fsm.ShortButtonPress();
    }
    button_press_start = 0;
  }
}
