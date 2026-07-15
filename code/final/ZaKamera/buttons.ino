#include <Arduino.h>

bool buttons[5] = {false}; //sequence up-right-down-left-any
bool last_btns[4] = {false};
uint8_t btn_pins[4] = {9, 8, 7, 10};

void setUpBTNS() {
  for(int i = 0; i < 4; i++) {
    pinMode(btn_pins[i], INPUT_PULLUP);
  }
}

void loopBTNS() {
  buttons[4] = false;
  for(int i = 0; i < 4; i++) {
    if (!digitalRead(btn_pins[i])) {
      if (!last_btns[i]) {
        buttons[i] = true;
        last_btns[i] = true;
        buttons[4] = true;
      } else {
        buttons[i] = false;
      }
    } else {
      last_btns[i] = false;
      buttons[i] = false;
    }
  }
}