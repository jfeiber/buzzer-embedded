/*
  File:
  BuzzerFSMCallbacks.cpp

  Description:
  Contains all the functions that represent the work a state needs to perform. Called by
  BuzzerFSM::DoState.
 */

#include <ArduinoJson.h>
#include "BuzzerFSMCallbacks.h"
#include "Globals.h"
#include "BuzzerFSM.h"
#include "EEPROMReadWrite.h"

/*
 * The intial state of the Buzzer FSM. Displays "BUZZER" on the OLED for 5 seconds then proceeds.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
*/

int InitFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0){
    oled.clear();
    oled.set1X();
    OLED_PRINTLN_FLASH("BUZZER");
  }
  delay(5000);
  return SUCCESS;
}

/*
 * State function that tries to initialize the cell radio (FONA). It tries to intialize the
 * radio for MAX_RETRIES number of times before failing.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
*/

int InitFonaShieldFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0) {
    oled.clear();
    oled.set1X();
    OLED_PRINTLN_FLASH("Initializing\ncell modem.....");
  }
  if (num_iterations_in_state >= MAX_RETRIES) {
    oled.clear();
    OLED_PRINTLN_FLASH("Failed to initialize\ncell modem.");
    delay(10000);
    return ERROR;
  }
  if (!fona_shield.initShield()) {
    delay(1000);
    return REPEAT;
  }
  return SUCCESS;
}

/*
 * Configures the cell radio for GPRS usage. This function tries to enable GPRS for MAX_RETRIES
 * number of times before failing.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
*/

int InitGPRSFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0) {
    oled.clear();
    oled.set1X();
    OLED_PRINTLN_FLASH("Initializing GPRS.....");
  }
  if (num_iterations_in_state >= MAX_RETRIES) {
    oled.clear();
    OLED_PRINTLN_FLASH("Failed to initialize\nGPRS connection.");
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

/*
 * If the Buzzer is being powered up for the first time, it needs a name. This function pings the
 * API and gets a name for the buzzer.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
*/

int GetBuzzerNameFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Getting a name.....");
  char buf[_max_line_length];
  fona_shield.HTTPGETOneLine(F("http://restaur-anteater.herokuapp.com/buzzer_api/get_new_buzzer_name"), buf, sizeof(buf));
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(buf);
  //TODO: better error handling here
  const char *error = root["error"];
  const char *buzzer_name = root["buzzer_name"];
  Serial.println(buzzer_name);
  EEPROMWrite(buzzer_name, strlen(buzzer_name)+1);
  strncpy(buzzer_name_global, buzzer_name, sizeof(buzzer_name_global));
  return SUCCESS;
}

/*
 * This helper function writes the battery percentage on the given row every 1000 state iterations.
 * For states like IDLE or HEARTBEAT that are called repeatedly this happens fairly quickly but
 * might needs some tweaking.
 *
 * @input the OLED row the battery percentage should be displayed on.
 * @input how many iterations the FSM has been in the current state.
*/

void UpdateBatteryPercentage(int row, int num_iterations_in_state) {
  if (num_iterations_in_state % 1000 == 0) {
    oled.setCol(0);
    oled.setRow(row);
    oled.clearToEOL();
    OLED_PRINT_FLASH("Battery: ");
    oled.print(batt_percentage);
    OLED_PRINTLN_FLASH("%");
  }
}

/*
 * The state that happens when the Buzzer is just sitting there without a party assigned to it.
 *
 * Displays the name of the buzzer and the battery percentage.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
*/

int IdleFunc(unsigned long state_start_time, int num_iterations_in_state) {
  has_system_been_initialized = true;
  if (num_iterations_in_state == 0) {
    oled.clear();
    OLED_PRINTLN_FLASH("Buzzer name:");
    oled.println(buzzer_name_global);
  }
  UpdateBatteryPercentage(2, num_iterations_in_state);
  if (millis() - state_start_time >= 20000) oled.setContrast(0);
  return REPEAT;
}

/*
 * The state that is called right before SLEEP. Shows a shutdown message for 5 seconds then
 * moves on.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
*/

int ShutdownFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Shutting down.\n Bye bye!");
  delay(5000);
  oled.clear();
  return SUCCESS;
}

/*
 * State that occurs when the Buzzer is charging (USB cable plugged in). Shows a message that the
 * battery is charging and the battery percentage.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
*/

int ChargeFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0) {
    oled.clear();
    OLED_PRINTLN_FLASH("Charging.....");
  }
  UpdateBatteryPercentage(1, num_iterations_in_state);
  return REPEAT;
}

/*
 * State that occurs when the Buzzer is transition out of the SLEEP state. A message is shown for
 * 5 seconds that the Buzzer is turning on before continuning.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return SUCCESS if the Buzzer was successfully initialized before being shutdown or ERROR
 * otherwise.
*/

int WakeupFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Starting up.....");
  delay(5000);
  oled.clear();
  if (has_system_been_initialized) return SUCCESS;
  return ERROR;
}

/*
 * State that occurs when the Buzzer is "shutdown". Shutting down the Buzzer consists of turning
 * off the OLED and doing nothing. Tests show that the Buzzer can go 2-3 days like this. If the
 * Buzzer was fully initialized before shutdown then we can perform a hot start and just go
 * straight to the IDLE.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return REPEAT. An external event (the USB cable being plugged in) needs to occur to transition
 * out of this state.
*/

int SleepFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0) {
    oled.clear();
  }
  delay(500);
  return REPEAT;
}

/*
 * Helper method that pings the API to see if a Buzzer is registered and returns whether it is.
 *
 * @return true if the Buzzer is registered, false otherwise.
*/

bool IsBuzzerRegistered() {
  char rep_buf[_max_line_length];
 //TODO: better error handling
  APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/is_buzzer_registered"), rep_buf, sizeof(rep_buf), false);
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  bool is_buzzer_registered = root["is_buzzer_registered"];
  return is_buzzer_registered;
}

/*
 * Helper method for calls to the API that just need the buzzer name as a POST parameter.
 *
 * @input a FlashStrPtr that represents the API endpoint to ping.
 * @input a ptr to a char buf where the API reply will be put.
 * @input the size of the above buffer.
 * @input a bool representing whether or not the buzzer is buzzing. Used for debugging purposes
 * but may be removed soon to save space.
 * @return the result of FonaShield::HTTPPOSTOneLine().
*/

int APIPOSTBuzzerName(FlashStrPtr api_endpoint, char *rep_buf, int rep_buf_len, bool is_buzzing) {
  FlashStrPtr json_skeleton = F("{\"buzzer_name\":\"\",\"buzzing\": 0}");
  char post_data[strlen_P((prog_char *)json_skeleton)+strlen(buzzer_name_global)+1];
  Serial.println(strlen_P((prog_char *)json_skeleton));
  snprintf(post_data, sizeof(post_data), "{\"buzzer_name\":\"%s\",\"buzzing\": %d}", buzzer_name_global, is_buzzing);
  return fona_shield.HTTPPOSTOneLine(api_endpoint, post_data, sizeof(post_data), rep_buf, rep_buf_len);
}

/*
 * The state the runs repeatedly when a Buzzer is assigned a party. This calls the 'heartbeat' API
 * endpoint to get status updates (whether or not a table is ready or the party has been deleted).
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return SUCCESS if the Buzzer should buzz, REPEAT if the state should be repeated, or TIMEOUT
 * if the party was deleted (is_active is false).
*/

int HeartbeatFunc(unsigned long state_start_time, int num_iterations_in_state) {
  if (num_iterations_in_state == 0) {
    oled.clear();
    OLED_PRINTLN_FLASH("Party name:");
    oled.println(party_name);
    OLED_PRINTLN_FLASH("Expected wait time: ");
    int wait_time_hrs = wait_time/60;
    int wait_time_min = wait_time - wait_time_hrs*60;
    int num_digits_hrs = wait_time_hrs < 10 ? NUM_DIGITS(wait_time_hrs) + 1 : NUM_DIGITS(wait_time_hrs);
    int num_digits_min = wait_time_min < 10 ? NUM_DIGITS(wait_time_min) + 1 : NUM_DIGITS(wait_time_min);
    char buf[num_digits_min + num_digits_hrs + 4];
    snprintf(buf, sizeof(buf), "%02dh:%02dm", wait_time_hrs, wait_time_min);
    oled.println(buf);
  }
  UpdateBatteryPercentage(4, num_iterations_in_state);
  PrintFreeRAM();
  char rep_buf[_max_line_length];
 //TODO: better error handling
  PrintFreeRAM();
  APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/heartbeat"), rep_buf, sizeof(rep_buf), false);
  PrintFreeRAM();
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  PrintFreeRAM();
  if (strcmp(root["status"], "success") != 0) return REPEAT;
  bool is_active = root["is_active"];
  if (!is_active) return TIMEOUT;
  bool buzz = root["buzz"];
  if (buzz) return SUCCESS;
  wait_time = root["wait_time"];
  return REPEAT;
}

int AcceptAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state) {
  char rep_buf[_max_line_length];
  FlashStrPtr json_skeleton = F("{\"buzzer_name\":\"\",\"party_id\":\"\"}");
  int num_digits = floor(log10(abs(party_id))) + 1;
  char post_data[strlen_P((prog_char *)json_skeleton)+strlen(buzzer_name_global)+num_digits+1];
  snprintf(post_data, sizeof(post_data), "{\"buzzer_name\":\"%s\",\"party_id\":\"%d\"}", buzzer_name_global, party_id);
  fona_shield.HTTPPOSTOneLine(F("http://restaur-anteater.herokuapp.com/buzzer_api/accept_party"), post_data, sizeof(post_data), rep_buf, sizeof(rep_buf));
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  if (strcmp(root["status"], "success") == 0) return SUCCESS;
  return ERROR;
}

int GetAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Checking for parties");
  OLED_PRINTLN_FLASH("with no buzzer");
  delay(100);
  char rep_buf[_max_line_length];
  APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/get_available_party"), rep_buf, sizeof(rep_buf), false);
  StaticJsonBuffer<_max_line_length> jsonBuffer;
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
  oled.clear();
  OLED_PRINTLN_FLASH("Checking if this\nbuzzer is registered");

  if (IsBuzzerRegistered()) {
    oled.clear();
    OLED_PRINTLN_FLASH("Buzzer registered!");
    delay(2000);
    return SUCCESS;
  }
  return ERROR;
}

int WaitBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Please register");
  OLED_PRINTLN_FLASH("buzzer.");
  OLED_PRINTLN_FLASH("Buzzer name: ");
  oled.println(buzzer_name_global);
  if (!IsBuzzerRegistered()) return REPEAT;
  oled.clear();
  OLED_PRINTLN_FLASH("Buzzer successfully");
  OLED_PRINTLN_FLASH("registered!");
  delay(5000);
  return SUCCESS;
}

int BuzzFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Table Ready!");
  analogWrite(BUZZER_PIN, 255);
  delay(2000);
  analogWrite(BUZZER_PIN, 0);
  char rep_buf[_max_line_length];
 //TODO: better error handling
  APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/heartbeat"), rep_buf, sizeof(rep_buf), true);
  StaticJsonBuffer<_max_line_length> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  bool is_active = root["is_active"];
  if (!is_active) return SUCCESS;
  return REPEAT;
}
