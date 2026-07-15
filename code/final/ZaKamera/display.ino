#include <Arduino.h>
#include <U8g2lib.h> 
#include <Wire.h> 
U8G2_SH1107_PIMORONI_128X128_2_HW_I2C u8g2(U8G2_R0);


void setup() {
  u8g2.begin();
  u8g2.setContrast(255);

  setUpBTNS();
  setUpServos();
  setUpSD();
}

void loop() {
  loopBTNS();
  loopServos();
  

  if (running) {
    if (down) {
      value += 3;
    } else {
      value -= 3;
    }
    
    if (value > 180) {
      value = 180;
      down = false; 
    } else if (value < 0) {
      value = 0;
      down = true;
    }
    
    moveServo(value, true);
    moveServo(value, false);

    if (buttons[3]) {
      running = false;
    }
  } else if (buttons[3]) {
    running = true;
    value = 0;
    down = true;
  } else if (buttons[1]) {
    moveServo(90, true);
    moveServo(90, false);
  }

  delay(100);
}
