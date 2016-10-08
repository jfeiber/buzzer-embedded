#include "BuzzerFSM.h"
#include "BuzzerFSMCallbacks.h"
#include "Pins.h"

BuzzerFSM buzzer_fsm({BUZZER_ON, BUZZER_ON, InitFunc}, INIT);

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  buzzer_fsm.AddState({BUZZER_OFF, BUZZER_ON, BuzzerOnFunc}, BUZZER_ON);
  buzzer_fsm.AddState({INIT, INIT, BuzzerOffFunc}, BUZZER_OFF);
}

void loop() {
  Serial.println("hallo");
  buzzer_fsm.ProcessState();
}
