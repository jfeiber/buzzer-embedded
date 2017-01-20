/*
  File:
  BuzzerFSM.cpp

  Description:
  Contains the main code for the Buzzer Finite State Machine (FSM).

*/

#include <Arduino.h>
#include "BuzzerFSM.h"
#include "Globals.h"

/*
 * Constructor for BuzzerFSM.
 *
 * @input a State struct that represents the initial state that will be run.
 * @input the ID of this initial state.
*/

BuzzerFSM::BuzzerFSM(State initial_state, int initial_state_id): _curr_state_id(initial_state_id) {
  _states[initial_state_id] = initial_state;
}

/*
 * Calls the state_func associated with the current state.
 *
 * If this state was just transitioned to, the state start time is set to the current time.
*/

int BuzzerFSM::DoState() {
  if (_state_start_time == NEW_STATE) _state_start_time = millis();
  return _states[_curr_state_id].state_func(_state_start_time, _num_iterations_in_state);
}

/*
 * Given the return value of DoState, this method makes a decision as to whether or not the
 * current state should be repeated or a transition to a new state should be made.
 *
 * @input the return value of DoState.
*/

void BuzzerFSM::TransitionToNextState(int do_state_ret_val) {
  int prev_state = _curr_state_id;
  if (do_state_ret_val == SUCCESS) _curr_state_id = _states[_curr_state_id].next_state_success;
  if (do_state_ret_val == ERROR) _curr_state_id = _states[_curr_state_id].next_state_failure;
  if (do_state_ret_val == TIMEOUT) _curr_state_id = _states[_curr_state_id].next_state_timeout;
  if (prev_state == _curr_state_id) {
    // State is being repeated so increment _num_iterations_in_state.
    _num_iterations_in_state++;
  } else {
    // FSM is transition to a new state so clear _num_iterations_in_state and _state_start_time
    _num_iterations_in_state = 0;
    _state_start_time = NEW_STATE;
  }
}

/*
 * Called by loop() in buzzer.ino when the USB cable is plugged in.
 *
 * The FSM forces a transition to the CHARGING state when this external event happens.
*/

void BuzzerFSM::USBCablePluggedIn() {
  ForceState(CHARGING);
}

/*
 * Called by loop() in buzzer.ino when the USB cable is unplugged.
 *
 * The FSM forces a transition to IDLE if the system was fully initialized when the charging
 * cable was plugged in or INIT_GPRS if the system was not fully initialized.
*/

void BuzzerFSM::USBCableUnplugged() {
  if (has_system_been_initialized) ForceState(IDLE);
  else ForceState(INIT_GPRS);
}

/*
 * Called by loop() in buzzer.ino when the button is pressed for (0, 5000) seconds.
 *
 * If the FSM is in the IDLE state then the FSM forces a transition to the GET_AVAILABLE_PARTY state.
*/

void BuzzerFSM::ShortButtonPress() {
  if (_curr_state_id == IDLE) ForceState(GET_AVAILABLE_PARTY);
}

/*
 * Called by loop() in buzzer.ino when the button is pressed for [5000, inf) seconds.
 *
 * If the FSM is in the SLEEP state the the FSM forces a transition to the WAKEUP state otherwise
 * it forces a transition to the SHUTDOWN state.
*/

void BuzzerFSM::LongButtonPress() {
  if (_curr_state_id == SLEEP) ForceState(WAKEUP);
  else ForceState(SHUTDOWN);
}

/*
 * Forces a transition to the state with the given ID. Meant to be used when some external event
 * occurs (a button being pressed, USB cable being plugged in) and a transition outside of the
 * predefined FSM transitions needs to occur.
 *
 * @input the ID of the state to transition to.
*/

void BuzzerFSM::ForceState(int new_state_id) {
  _state_start_time = NEW_STATE;
  _num_iterations_in_state = 0;
  _curr_state_id = new_state_id;
}

/*
 * Adds a state to the FSM.
 *
 * @input a State struct that represents the state to add.
 * @input the ID of the state to add. Bad things will happen if this ID isn't in the state_ids
 * enum in BuzzerFSM.h.
*/

void BuzzerFSM::AddState(State state_to_add, int state_id) {
  _states[state_id] = state_to_add;
}

/*
 * Actual performs the work for the current state and then calls TransitionToNextState based
 * on the return value of the state work function.
*/

void BuzzerFSM::ProcessState() {
  int ret_val = DoState();
  TransitionToNextState(ret_val);
}
