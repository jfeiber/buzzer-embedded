#ifndef BUZZERFSM_H
#define BUZZERFSM_H

struct State {
  int next_state_success;
  int next_state_failure;
  int next_state_timeout;
  int (*state_func)(unsigned long, int);
};

enum state_ids {INIT, INIT_FONA, INIT_GPRS, GET_BUZZER_NAME, IDLE, WAIT_BUZZER_REGISTRATION,
                GET_AVAILABLE_PARTY};

#define NEW_STATE 0

class BuzzerFSM {
  private:
    unsigned long _state_start_time = 0;
    int _num_iterations_in_state = 0;
    int _curr_state_id;
    State _states[GET_AVAILABLE_PARTY];
    int DoState();
    void TransitionToNextState(int do_state_ret_val);
  public:
    void AddState(State state_to_add, int state_id);
    void ProcessState();
    BuzzerFSM(State initial_state, int initial_state_id);
    BuzzerFSM(){};
};

#endif
