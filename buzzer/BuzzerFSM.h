#ifndef BUZZERFSM_H
#define BUZZERFSM_H

struct State {
  int next_state_success;
  int next_state_failure;
  int (*state_func)(unsigned long, int);
};

enum state_ids {INIT, BUZZER_ON, BUZZER_OFF};
enum ret_vals {SUCCESS, FAILURE};

class BuzzerFSM {
  private:
    unsigned long _state_start_time = 0;
    int _num_iterations_in_state = 0;
    int _curr_state_id;
    int NEW_STATE = 0;
    State _states[INIT];
    int DoState();
    void TransitionToNextState(int do_state_ret_val);
  public:
    void AddState(State state_to_add, int state_id);
    void ProcessState();
    BuzzerFSM(State initial_state, int initial_state_id);
    BuzzerFSM();
};

#endif
