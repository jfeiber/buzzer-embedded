#ifndef BUZZERFSMCALLBACKS_H
#define BUZZERFSMCALLBACKS_H

#include "Pins.h"


int InitFunc(unsigned long state_start_time, int num_iterations_in_state);
int BuzzerOnFunc(unsigned long state_start_time, int num_iterations_in_state);
int BuzzerOffFunc(unsigned long state_start_time, int num_iterations_in_state);
int InitFonaShieldFunc(unsigned long state_start_time, int num_iterations_in_state);
int InitGPRSFunc(unsigned long state_start_time, int num_iterations_in_state);
int GetBuzzerNameFunc(unsigned long state_start_time, int num_iterations_in_state);
int IdleFunc(unsigned long state_start_time, int num_iterations_in_state);
int CheckBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state);
int WaitBuzzerRegFunc(unsigned long state_start_time, int num_iterations_in_state);
static bool IsBuzzerRegistered();
int GetAvailPartyFunc(unsigned long state_start_time, int num_iterations_in_state);

#endif
