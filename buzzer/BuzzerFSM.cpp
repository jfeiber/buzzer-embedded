#include "BuzzerFSM.h"

BuzzerFSM::BuzzerFSM(State initial_state, int initial_state_id): _curr_state_id(initial_state_id) {
  states[initial_state_id] = initial_state;
}

int BuzzerFSM::DoState() {
  return states[_curr_state_id].state_func(_elapsed_time_in_curr_state);
}

void BuzzerFSM::TransitionToNextState(int do_state_ret_val) {
  if (do_state_ret_val == SUCCESS) _curr_state_id = states[_curr_state_id].next_state_success;
  if (do_state_ret_val == FAILURE) _curr_state_id = states[_curr_state_id].next_state_failure;
  else _curr_state_id = _curr_state_id;
}

void BuzzerFSM::AddState(State state_to_add, int state_id) {
  states[state_id] = state_to_add;
}

void BuzzerFSM::ProcessState() {
  int ret_val = DoState();
  TransitionToNextState(ret_val);
}
