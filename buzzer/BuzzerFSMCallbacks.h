/*
  File:
  BuzzerFSMCallbacks.h

  Description:
  Contains all the functions that represent the work a state needs to perform. Called by
  BuzzerFSM::DoState.
*/

#ifndef BUZZERFSMCALLBACKS_H
#define BUZZERFSMCALLBACKS_H

#include "Helpers.h"
#include "Pins.h"

#define PARTY_AVAIL_FIELD "p_a"
#define PARTY_NAME_FIELD "n"
#define PARTY_WAIT_TIME_FIELD "t"
#define PARTY_ID_FIELD "id"
#define BUZZER_NAME_FIELD "bn"
#define IS_ACTIVE_FIELD "i_a"
#define BUZZ_FIELD "b"
#define IS_BUZZER_REGISTERED_FIELD "i_reg"
#define ERROR_STATUS_FIELD "e"
#define ERROR_MESSAGE_FIELD "e_msg"

static int iteration_err_start = 0;

int InitFunc(unsigned long state_start_time, int num_iterations_in_state);
int BuzzFunc(unsigned long state_start_time, int num_iterations_in_state);
int InitFonaShieldFunc(unsigned long state_start_time, int num_iterations_in_state);
int InitGPRSFunc(unsigned long state_start_time, int num_iterations_in_state);
int GetBuzzerNameFunc(unsigned long state_start_time, int num_iterations_in_state);
int IdleFunc(unsigned long state_start_time, int num_iterations_in_state);
int CheckBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state);
int WaitBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state);
static int IsBuzzerRegistered(bool *is_buzzer_registered);
int GetAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state);
static int APIPOSTBuzzerName(FlashStrPtr api_endpoint, char *rep_buf, int rep_buf_len, bool is_buzzing);
int AcceptAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state);
int HeartbeatFunc(unsigned long state_start_time, int num_iterations_in_state);
int ShutdownFunc(unsigned long state_start_time, int num_iterations_in_state);
int SleepFunc(unsigned long state_start_time, int num_iterations_in_state);
int WakeupFunc(unsigned long state_start_time, int num_iterations_in_state);
int ChargeFunc(unsigned long state_start_time, int num_iterations_in_state);
int FatalErrorFunc(unsigned long state_start_time, int num_iterations_in_state);
static void UpdateBatteryPercentage(int row, int num_iterations_in_state);

#endif
