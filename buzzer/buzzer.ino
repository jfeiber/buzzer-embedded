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
  if (fona_shield.initShield()) {
    Serial.println("Successfully configured FONA");
  } else {
    Serial.println("FONA not configured.");
  }
  if (fona_shield.enableGPRS()) {
    Serial.println("Successfully configed GPRS");
  } else {
    Serial.println("Nope");
  }
  // char buf[256];
  // fona_shield.HTTPGETOneLine(F("http://restaur-anteater.herokuapp.com/sample_json"), buf, sizeof(buf));
  // Serial.print("Got reply: ");
  // Serial.println(buf);
  char buf[] = "{test: \"testing 1 2 3\"}";
  char buf1[256];
  fona_shield.HTTPPOSTOneLine(F("http://restaur-anteater.herokuapp.com/sample_json"), buf, sizeof(buf), buf1, sizeof(buf1));
  Serial.print("Got reply: ");
  Serial.println(buf1);
}

void loop() {
  // Serial.println("hallo");
  // buzzer_fsm.ProcessState();
}
