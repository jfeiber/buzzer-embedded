#ifndef BUZZERFSMCALLBACKS_H
#define BUZZERFSMCALLBACKS_H

int SetupFunc(int elapsed_time_in_curr_state) {
  Serial.println("this is a test1");
  delay(2000);
  return SUCCESS;
}

int OtherFunc(int elapsed_time_in_curr_state) {
  Serial.println("this is another test!");
  delay(2000);
  return SUCCESS;
}

#endif
