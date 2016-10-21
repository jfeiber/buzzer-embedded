#include <ArduinoJson.h>
#include "BuzzerFSMCallbacks.h"
#include "Globals.h"
#include "Macros.h"
#include "BuzzerFSM.h"
#include "EEPROMReadWrite.h"

int InitFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0){
    oled.clear();
    oled.set1X();
    OLED_PRINTLN_FLASH("BUZZER");
  }
  delay(5000);
  return SUCCESS;
}

int InitFonaShieldFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0) {
    oled.clear();
    oled.set1X();
    OLED_PRINTLN_FLASH("Initializing cell modem.....");
  }
  DEBUG_PRINTLN_FLASH("Initializing FONA shield.");
  if (num_iterations_in_state >= MAX_RETRIES) {
    oled.clear();
    OLED_PRINTLN_FLASH("Failed to initialize cell modem.");
    delay(10000);
    return ERROR;
  }
  if (!fona_shield.initShield()) {
    delay(1000);
    return REPEAT;
  }
  return SUCCESS;
}

int InitGPRSFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0) {
    oled.clear();
    oled.set1X();
    OLED_PRINTLN_FLASH("Initializing GPRS.....");
  }
  DEBUG_PRINTLN_FLASH("Attempting to attach to GPRS netowrk");
  if (num_iterations_in_state >= MAX_RETRIES) {
    oled.clear();
    OLED_PRINTLN_FLASH("Failed to initialize GPRS connection.");
    //TODO: Log network statistics to serial
    delay(10000);
    return ERROR;
  }
  if (!fona_shield.enableGPRS()) {
    delay(1000);
    return REPEAT;
  }
  return SUCCESS;
}

int GetBuzzerNameFunc(unsigned long state_start_time, int num_iterations_in_state) {
  Serial.println("Buzzer doesn't yet have name. Attempting to get one.....");
  oled.clear();
  oled.println("Getting a");
  oled.println("name...");
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  char buf[_max_line_length];
  fona_shield.HTTPGETOneLine(F("http://restaur-anteater.herokuapp.com/buzzer_api/get_new_buzzer_name"), buf, sizeof(buf));
  JsonObject& root = jsonBuffer.parseObject(buf);
  //TODO: better error handling here
  const char *error = root["error"];
  const char *buzzer_name = root["buzzer_name"];
  Serial.println(buzzer_name);
  EEPROMWrite(buzzer_name, strlen(buzzer_name)+1);
  strncpy(buzzer_name_global, buzzer_name, sizeof(buzzer_name_global));
  return SUCCESS;
}

int IdleFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0) {
    oled.clear();
    oled.println(buzzer_name_global);
  }
  if (millis() - state_start_time >= 20000) oled.setContrast(0);
  return REPEAT;
}

int BuzzerOnFunc(unsigned long state_start_time, int num_iterations_in_state) {
  analogWrite(BUZZER_PIN, 255);
  Serial.println(millis());
  Serial.println(state_start_time);
  if ( (millis() - state_start_time)/1000 >= 2 ) return SUCCESS;
  return ERROR;
}

int BuzzerOffFunc(unsigned long state_start_time, int num_iterations_in_state) {
  analogWrite(BUZZER_PIN, 0);
  return SUCCESS;
}
