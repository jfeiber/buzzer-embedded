#include "BuzzerFSM.h"
#include <Arduino.h>

BuzzerFSM::BuzzerFSM(State initial_state, int initial_state_id): _curr_state_id(initial_state_id) {
  _states[initial_state_id] = initial_state;
}

int BuzzerFSM::DoState() {
  if (_state_start_time == NEW_STATE) _state_start_time = millis();
  return _states[_curr_state_id].state_func(_state_start_time, _num_iterations_in_state);
}

void BuzzerFSM::TransitionToNextState(int do_state_ret_val) {
  int prev_state = _curr_state_id;
  if (do_state_ret_val == SUCCESS) _curr_state_id = _states[_curr_state_id].next_state_success;
  if (do_state_ret_val == FAILURE) _curr_state_id = _states[_curr_state_id].next_state_failure;
  if (prev_state == _curr_state_id) {
        _num_iterations_in_state++;
  } else {
    _num_iterations_in_state = 0;
    _state_start_time = NEW_STATE;
  }
}

void BuzzerFSM::AddState(State state_to_add, int state_id) {
  _states[state_id] = state_to_add;
}

void BuzzerFSM::ProcessState() {
  int ret_val = DoState();
  TransitionToNextState(ret_val);
}
