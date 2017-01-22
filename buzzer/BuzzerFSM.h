/*
  File:
  BuzzerFSM.h

  Description:
  Contains the main code for the Buzzer Finite State Machine (FSM).
  The IDs for all the states and the State struct are also declared in this header file.

*/

#ifndef BUZZERFSM_H
#define BUZZERFSM_H

// Struct that represents a state in the FSM. The ints are IDs of other States and the function
// pointer points to a function that will be called when that state is the current state. The two
// parameters are how long the FSM has been in that state and the int is the number of iterations
// that the state has been repeated. This function will return SUCCESS, TIMEOUT, ERROR, or
// REPEAT (defined as an enum in Globals.h). State transitions are made based on this return value.
struct State {
  int next_state_success;
  int next_state_failure;
  int next_state_timeout;
  int (*state_func)(unsigned long, int);
};

// enum that contains all the possible state IDs.
enum state_ids {INIT, INIT_FONA, INIT_GPRS, GET_BUZZER_NAME, IDLE, CHECK_BUZZER_REGISTRATION,
                WAIT_BUZZER_REGISTRATION, GET_AVAILABLE_PARTY, ACCEPT_AVAILABLE_PARTY, HEARTBEAT,
                BUZZ, CHARGING, SHUTDOWN, SLEEP, FATAL_ERROR, WAKEUP};

// _state_start_time is set to this after a state has been
// transitioned to. This is not a private class variable to save space.
#define NEW_STATE 0


// Class that actually implements the FSM.
class BuzzerFSM {
  private:
    unsigned long _state_start_time = 0;
    int _num_iterations_in_state = 0;
    int _curr_state_id;
    State _states[WAKEUP];
    int DoState();
    void TransitionToNextState(int do_state_ret_val);
    void ForceState(int new_state_id);
  public:
    void AddState(State state_to_add, int state_id);
    void ProcessState();
    void ShortButtonPress();
    void LongButtonPress();
    void USBCablePluggedIn();
    void USBCableUnplugged();
    BuzzerFSM(State initial_state, int initial_state_id);
    BuzzerFSM(){};
};

#endif
