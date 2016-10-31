#include <ArduinoJson.h>
#include "BuzzerFSMCallbacks.h"
#include "Globals.h"
#include "Macros.h"
#include "BuzzerFSM.h"
#include "Pins.h"
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
  OLED_PRINTLN_FLASH("Getting a name.....");
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
  if (digitalRead(BUTTON_PIN) == HIGH) return SUCCESS;
  return REPEAT;
}

bool IsBuzzerRegistered() {
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  char rep_buf[_max_line_length];
 //TODO: better error handling
  APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/is_buzzer_registered"), rep_buf, sizeof(rep_buf));
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  bool is_buzzer_registered = root["is_buzzer_registered"];
  return is_buzzer_registered;
}

int APIPOSTBuzzerName(FlashStrPtr api_endpoint, char *rep_buf, int rep_buf_len) {
  DEBUG_PRINTLN_FLASH("In API POST BUZZER NAME");
  FlashStrPtr json_skeleton = F("{\"buzzer_name\":\"\"}");
  char post_data[strlen_P((prog_char *)json_skeleton)+strlen(buzzer_name_global)+1];
  Serial.println(strlen_P((prog_char *)json_skeleton));
  snprintf(post_data, sizeof(post_data), "{\"buzzer_name\":\"%s\"}", buzzer_name_global);
  return fona_shield.HTTPPOSTOneLine(api_endpoint, post_data, sizeof(post_data), rep_buf, rep_buf_len);
}

int HeartbeatFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  oled.println(party_name);
  oled.println(wait_time);
  // delay(5000);
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  char rep_buf[_max_line_length];
 //TODO: better error handling
  APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/heartbeat"), rep_buf, sizeof(rep_buf));
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  if (strcmp(root["status"], "success") != 0) return REPEAT;
  bool is_active = root["is_active"];
  if (!is_active) return TIMEOUT;
  bool buzz = root["buzz"];
  if (buzz) return SUCCESS;
  wait_time = root["wait_time"];
  return REPEAT;
}

int AcceptAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state) {
  // oled.clear();
  // oled.println(party_name);
  // oled.println(wait_time);
  // oled.println("Found a ")
  // delay(10000);
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  char rep_buf[_max_line_length];
  FlashStrPtr json_skeleton = F("{\"buzzer_name\":\"\",\"party_id\":\"\"}");
  int num_digits = floor(log10(abs(party_id))) + 1;
  char post_data[strlen_P((prog_char *)json_skeleton)+strlen(buzzer_name_global)+num_digits+1];
  Serial.println(strlen_P((prog_char *)json_skeleton));
  snprintf(post_data, sizeof(post_data), "{\"buzzer_name\":\"%s\",\"party_id\":\"%d\"}", buzzer_name_global, party_id);
  Serial.println(post_data);
  fona_shield.HTTPPOSTOneLine(F("http://restaur-anteater.herokuapp.com/buzzer_api/accept_party"), post_data, sizeof(post_data), rep_buf, sizeof(rep_buf));
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  if (strcmp(root["status"], "success") == 0) return SUCCESS;
  return ERROR;
}

int GetAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state) {
  DEBUG_PRINTLN_FLASH("Attempting to get an available party");
  oled.clear();
  OLED_PRINTLN_FLASH("Checking for parties");
  OLED_PRINTLN_FLASH("with no buzzer");
  delay(100);
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  char rep_buf[_max_line_length];
  APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/get_available_party"), rep_buf, sizeof(rep_buf));
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  bool party_avail = root["party_avail"];
  if (party_avail){
    wait_time = root["wait_time"];
    party_id = root["party_id"];
    strncpy(party_name, root["party_name"], sizeof(party_name)-1);
    party_name[sizeof(party_name)-1] = '\0';
    return SUCCESS;
  }
  oled.clear();
  OLED_PRINTLN_FLASH("No avail parties.");
  delay(5000);
  return ERROR;
}

int CheckBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state) {
  Serial.println("Checking if buzzer is registered");
  oled.clear();
  OLED_PRINTLN_FLASH("Checking to see if this");
  OLED_PRINTLN_FLASH("buzzer is registered.....");

  if (IsBuzzerRegistered()) return SUCCESS;
  return ERROR;
}

int WaitBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state) {
  Serial.println("Waiting for the buzzer to be registered.");
  oled.clear();
  OLED_PRINTLN_FLASH("Please register");
  OLED_PRINTLN_FLASH("buzzer.");
  OLED_PRINTLN_FLASH("Buzzer name: ");
  oled.println(buzzer_name_global);
  while (!IsBuzzerRegistered()) delay(100);
  oled.clear();
  OLED_PRINTLN_FLASH("Buzzer successfully");
  OLED_PRINTLN_FLASH("registered!");
  delay(5000);
  return SUCCESS;
}

int BuzzFunc(unsigned long state_start_time, int num_iterations_in_state) {
  DEBUG_PRINTLN("IN BUZZ STATE");
  oled.clear();
  OLED_PRINTLN_FLASH("Table Ready!");
  analogWrite(BUZZER_PIN, 255);
  delay(2000);
  analogWrite(BUZZER_PIN, 0);
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  char rep_buf[_max_line_length];
 //TODO: better error handling
  APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/heartbeat"), rep_buf, sizeof(rep_buf));
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  bool is_active = root["is_active"];
  if (!is_active) return SUCCESS;
  return REPEAT;
}
