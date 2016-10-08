#include "BuzzerFSM.h"
#include "BuzzerFSMCallbacks.h"

BuzzerFSM buzzer_fsm({OTHERSTATE, SETUP, SetupFunc}, SETUP);

void setup() {
  Serial.begin(9600);
  buzzer_fsm.AddState({SETUP, SETUP, OtherFunc}, OTHERSTATE);
}

void loop() {
  Serial.println("hallo");
  buzzer_fsm.ProcessState();
}
