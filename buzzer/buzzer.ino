#include <SoftwareSerial.h>
#include "BuzzerFSM.h"
#include "BuzzerFSMCallbacks.h"
#include "FonaShield.h"
#include "Pins.h"

BuzzerFSM buzzer_fsm({BUZZER_ON, BUZZER_ON, InitFunc}, INIT);
SoftwareSerial fona_serial = SoftwareSerial(FONA_TX_PIN, FONA_RX_PIN);
FonaShield fona_shield(&fona_serial, FONA_RST_PIN);

void setup() {
  Serial.begin(115200);
  pinMode(FONA_RST_PIN, OUTPUT);
  // pinMode(BUZZER_PIN, OUTPUT);
  // buzzer_fsm.AddState({BUZZER_OFF, BUZZER_ON, BuzzerOnFunc}, BUZZER_ON);
  // buzzer_fsm.AddState({INIT, INIT, BuzzerOffFunc}, BUZZER_OFF);
}

void loop() {
  // Serial.println("hallo");
  // buzzer_fsm.ProcessState();
  bool init_res = fona_shield.Init();
  if (init_res) {
    Serial.println("Successfully configured FONA");
  } else {
    Serial.println("Could not configure FONA");
  }
}
