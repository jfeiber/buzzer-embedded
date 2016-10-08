#ifndef BUZZERFSM_H
#define BUZZERFSM_H

struct State {
  int next_state_success;
  int next_state_failure;
  int (*state_func)(int);
};

enum state_ids {SETUP, OTHERSTATE};
enum ret_vals {SUCCESS, FAILURE, REPEAT};

class BuzzerFSM {
  private:
    int _elapsed_time_in_curr_state = 0;
    int _curr_state_id;
    State states[SETUP];
    int DoState();
    void TransitionToNextState(int do_state_ret_val);
  public:
    void AddState(State state_to_add, int state_id);
    void ProcessState();
    BuzzerFSM(State initial_state, int initial_state_id);
    BuzzerFSM();
};

#endif
