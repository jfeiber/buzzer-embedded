#ifndef BUZZERFSMCALLBACKS_H
#define BUZZERFSMCALLBACKS_H

#include "Pins.h"
#include "Macros.h"


int InitFunc(unsigned long state_start_time, int num_iterations_in_state);
int BuzzFunc(unsigned long state_start_time, int num_iterations_in_state);
int InitFonaShieldFunc(unsigned long state_start_time, int num_iterations_in_state);
int InitGPRSFunc(unsigned long state_start_time, int num_iterations_in_state);
int GetBuzzerNameFunc(unsigned long state_start_time, int num_iterations_in_state);
int IdleFunc(unsigned long state_start_time, int num_iterations_in_state);
int CheckBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state);
int WaitBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state);
static bool IsBuzzerRegistered();
int GetAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state);
static int APIPOSTBuzzerName(FlashStrPtr api_endpoint, char *rep_buf, int rep_buf_len, bool is_buzzing);
int AcceptAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state);
int HeartbeatFunc(unsigned long state_start_time, int num_iterations_in_state);
int ShutdownFunc(unsigned long state_start_time, int num_iterations_in_state);
int SleepFunc(unsigned long state_start_time, int num_iterations_in_state);
int WakeupFunc(unsigned long state_start_time, int num_iterations_in_state);
int ChargeFunc(unsigned long state_start_time, int num_iterations_in_state);

#endif
