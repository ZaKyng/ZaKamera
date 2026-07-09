bool buttons[5] = {false}; //sequence left-up-right-down-any
bool last_btns[4] = {false};
uint8_t btn_pins[4] = {10, 9, 8, 7};

void setup() {
  Serial.begin(9600);
  delay(1000);

  for(int i = 0; i < 4; i++) {
    pinMode(btn_pins[i], INPUT_PULLUP);
  }

}

void loop() {
  //set button states
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

  for(int i = 0; i < 4; i++) {
    Serial.print(last_btns[i]);
  }

  Serial.print("\n");

  delay(50);
}