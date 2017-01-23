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
 * @return SUCCESS always.
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
 * @return SUCCESS if the cell radio has been fully initialized, REPEAT if the cell radio wasn't
 * successfully initialized but we haven't yet retried MAX_RETRIES number of times, and ERROR if
 * we have tried to init the cell radio for MAX_RETRIES number of times and it still won't work.
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
 * @return SUCCESS if the cell radio was configured for GPRS, REPEAT if the cell radio wasn't
 * successfully configured but we haven't yet retried MAX_RETRIES number of times, and ERROR if
 * we have tried to configure the cell radio for MAX_RETRIES number of times and it still won't work
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
 * @return SUCCESS if everything went ok, or ERROR if an unrecoverable error happened with the API.
*/

int GetBuzzerNameFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Getting a name.....");
  char buf[BUF_LENGTH_MEDIUM];
  int err = fona_shield.HTTPGETOneLine(F("http://restaur-anteater.herokuapp.com/buzzer_api/get_new_buzzer_name"), buf, sizeof(buf));
  if (err) return (num_iterations_in_state < MAX_RETRIES) ? REPEAT : ERROR;
  StaticJsonBuffer<BUF_LENGTH_MEDIUM> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(buf);
  if (root[ERROR_STATUS_FIELD]) return (num_iterations_in_state < MAX_RETRIES) ? REPEAT : ERROR;
  const char *buzzer_name = root[BUZZER_NAME_FIELD];
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
 * @return REPEAT always. An external event (button press, charging cable plugged in) needs to occur
 * to transition away from this state.
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
 * @return SUCCESS always.
*/

int ShutdownFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Shutting down.\nBye bye!");
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
 * @return REPEAT always. The USB cable needs to be unplugged (external event) before a transition
 * away from this state occurs.
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
 * @input a pointer to a bool that after this function call will be true if the Buzzer is registered
 * and false otherwise.
 * @return 1 if an API error occurred, 0 otherwise.
*/

int IsBuzzerRegistered(bool *is_buzzer_registered) {
  char rep_buf[BUF_LENGTH_SMALL];
  int err = APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/is_buzzer_registered"), rep_buf, sizeof(rep_buf), false);
  Serial.println("API POST:");
  Serial.println(err);
  if (err == ERROR) return ERROR;
  StaticJsonBuffer<BUF_LENGTH_SMALL> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  *is_buzzer_registered = root[IS_BUZZER_REGISTERED_FIELD];
  return root[ERROR_STATUS_FIELD];
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
  FlashStrPtr json_skeleton = F("{\""BUZZER_NAME_FIELD"\":\"%s\"}");
  // -2 because the string format specificer won't be in the char buf after the snprintf.
  char post_data[strlen_P((prog_char *)json_skeleton)-2+strlen(buzzer_name_global)+1];
  Serial.println(strlen_P((prog_char *)json_skeleton));
  snprintf_P(post_data, sizeof(post_data), (prog_char *)json_skeleton, buzzer_name_global);
  return fona_shield.HTTPPOSTOneLine(api_endpoint, post_data, sizeof(post_data), rep_buf, rep_buf_len);
}

/*
 * The state the runs repeatedly when a Buzzer is assigned a party. This calls the 'heartbeat' API
 * endpoint to get status updates (whether or not a table is ready or the party has been deleted).
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return SUCCESS if the Buzzer should buzz, REPEAT if the state should be repeated, TIMEOUT
 * if the party was deleted (is_active is false), or ERROR if the API call has failed more than
 * MAX_RETRIES number of times.
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
    snprintf_P(buf, sizeof(buf), (prog_char *)F("%02dh:%02dm"), wait_time_hrs, wait_time_min);
    oled.println(buf);
  }
  UpdateBatteryPercentage(4, num_iterations_in_state);
  PrintFreeRAM();
  char rep_buf[BUF_LENGTH_MEDIUM];
  PrintFreeRAM();
  short err;
  err = APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/heartbeat"), rep_buf, sizeof(rep_buf), false);
  if (err) return ERROR;
  PrintFreeRAM();
  StaticJsonBuffer<BUF_LENGTH_MEDIUM> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  PrintFreeRAM();
  err = root[ERROR_STATUS_FIELD];
  if (err) return ERROR;
  short is_active = root[IS_ACTIVE_FIELD];
  if (!is_active) return TIMEOUT;
  short buzz = root[BUZZ_FIELD];
  if (buzz) return SUCCESS;
  wait_time = root[PARTY_WAIT_TIME_FIELD];
  return REPEAT;
}

/*
 * The state the runs when there is a party available. This pings an API endpoint that confirms
 * that the Buzzer is accepting the available party.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return SUCCESS if the party was successfully accepted or ERROR otherwise.
*/

int AcceptAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state) {
  char rep_buf[BUF_LENGTH_SMALL];
  FlashStrPtr json_skeleton = F("{\""BUZZER_NAME_FIELD"\":\"%s\",\""PARTY_ID_FIELD"\":%d}");
  // calculate the number of digits in the current party_id
  int num_digits = NUM_DIGITS(party_id);
  // -4 is because the string format specifiers won't actually be in the final char buf.
  char post_data[strlen_P((prog_char *)json_skeleton)+strlen(buzzer_name_global)+num_digits+1];
  snprintf_P(post_data, sizeof(post_data), (prog_char *)json_skeleton, buzzer_name_global, party_id);
  short err;
  err = fona_shield.HTTPPOSTOneLine(F("http://restaur-anteater.herokuapp.com/buzzer_api/accept_party"), post_data, sizeof(post_data), rep_buf, sizeof(rep_buf));
  if (err) return (num_iterations_in_state < MAX_RETRIES) ? REPEAT : ERROR;
  StaticJsonBuffer<BUF_LENGTH_SMALL> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  err = root[ERROR_STATUS_FIELD];
  if (err) return (num_iterations_in_state < MAX_RETRIES) ? REPEAT : ERROR;
  return SUCCESS;
}

/*
 * The state the runs when a short button press occurs. It pings an API endpoint to see if there are
 * any parties available. If there are, then ACCEPT_AVAILABLE_PARTY will be the next state.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return SUCCESS if there is a party available, TIMEOUT if there isn't, and ERROR if the API call
 * fails more than MAX_RETRIES number of times.
*/

int GetAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Checking for parties");
  OLED_PRINTLN_FLASH("with no buzzer");
  delay(100);
  char rep_buf[BUF_LENGTH_LARGE];
  int err = APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/get_available_party"), rep_buf, sizeof(rep_buf), false);
  if (err) return (num_iterations_in_state < MAX_RETRIES) ? REPEAT : ERROR;
  StaticJsonBuffer<BUF_LENGTH_LARGE> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  if (root[ERROR_STATUS_FIELD]) return (num_iterations_in_state < MAX_RETRIES) ? REPEAT : ERROR;
  bool party_avail = root[PARTY_AVAIL_FIELD];
  if (party_avail){
    wait_time = root[PARTY_WAIT_TIME_FIELD];
    party_id = root[PARTY_ID_FIELD];
    strncpy(party_name, root[PARTY_NAME_FIELD], sizeof(party_name)-1);
    party_name[sizeof(party_name)-1] = '\0';
    return SUCCESS;
  }
  oled.clear();
  OLED_PRINTLN_FLASH("No avail parties.");
  delay(5000);
  return TIMEOUT;
}

/*
 * This state is called in the initial Buzzer setup to see whether or not the Buzzer is registered
 * with the backend.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return SUCCESS if the Buzzer is registered, TIMEOUT if it isn't, and ERROR if the API call has
 * failed more than MAX_RETRIES number of times.
*/

int CheckBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Checking if this\nbuzzer is registered");

  bool is_buzzer_registered;
  if (IsBuzzerRegistered(&is_buzzer_registered) == ERROR)
    return (num_iterations_in_state < MAX_RETRIES) ? REPEAT : ERROR;
  if (is_buzzer_registered) {
    oled.clear();
    OLED_PRINTLN_FLASH("Buzzer registered!");
    delay(2000);
    return SUCCESS;
  }
  return TIMEOUT;
}

/*
 * If the Buzzer isn't registered, this state is called to wait for the Buzzer to be registered
 * before preceeding to normal Buzzer activities.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return SUCCESS once the Buzzer has been registered, REPEAT if the Buzzer has yet to be
 * registered (or there is a problem with the API call), ERROR if the API call has failed more than
 * MAX_RETRIES number of times.
*/

int WaitBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Please register");
  OLED_PRINTLN_FLASH("buzzer.");
  OLED_PRINTLN_FLASH("Buzzer name: ");
  oled.println(buzzer_name_global);
  bool is_buzzer_registered;
  if (IsBuzzerRegistered(&is_buzzer_registered) == ERROR)
    return (num_iterations_in_state < MAX_RETRIES) ? REPEAT : ERROR;
  if (!is_buzzer_registered) return REPEAT;
  oled.clear();
  OLED_PRINTLN_FLASH("Buzzer successfully");
  OLED_PRINTLN_FLASH("registered!");
  delay(5000);
  return SUCCESS;
}

/*
 * This function gets called when an unrecoverable/fatal error has occured.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return SUCCESS always.
*/


int FatalErrorFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  analogWrite(BUZZER_PIN, 255);
  delay(300);
  analogWrite(BUZZER_PIN, 0);
  delay(300);
  analogWrite(BUZZER_PIN, 255);
  delay(300);
  analogWrite(BUZZER_PIN, 0);
  OLED_PRINTLN_FLASH("Fatal error occured.");
  OLED_PRINTLN_FLASH("Restarting buzzer in\n10 seconds.");
  delay(10000);
  return SUCCESS;
}

/*
 * This state runs when then Buzzer should buzz. It vibrates the motor for 2 seconds then pings the
 * API to see whether or not it should keep buzzing or return to IDLE. This API interaction is
 * likely to change in the near future. API pinging takes ~5 seconds so no delay call is needed
 * before buzzing again.
 *
 * @input how long the FSM has been in the current state.
 * @input how many iterations the FSM has been in the current state.
 * @return SUCCESS if the party is no longer active (has been deleted or party has been seated),
 * REPEAT if we should keep buzzing (or if an API error has occurred), or ERROR if the API call has
 * failed.
*/

int BuzzFunc(unsigned long state_start_time, int num_iterations_in_state) {
  oled.clear();
  OLED_PRINTLN_FLASH("Table Ready!");
  analogWrite(BUZZER_PIN, 255);
  delay(2000);
  analogWrite(BUZZER_PIN, 0);
  char rep_buf[BUF_LENGTH_MEDIUM];
  short err;
  err = APIPOSTBuzzerName(F("http://restaur-anteater.herokuapp.com/buzzer_api/heartbeat"), rep_buf, sizeof(rep_buf), true);
  if (err) return ERROR;
  StaticJsonBuffer<BUF_LENGTH_MEDIUM> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rep_buf);
  err = root[ERROR_STATUS_FIELD];
  if (err) return ERROR;
  short is_active = root[IS_ACTIVE_FIELD];
  if (!is_active) return SUCCESS;
  return REPEAT;
}
