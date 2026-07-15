#include <Arduino.h>
#include <Servo.h>

uint8_t angles[2] = {0, 0};
uint8_t targets[2] = {0, 0};
uint8_t servoPins[2] = {4, 5};

Servo servos[2];

void setUpServos() {
  for(int i = 0; i < 2; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(0);
  }
}

void moveServo(int angle, bool xaxis) {
  if (xaxis) {
    targets[0] = angle;
  } else {
    targets[1] = angle;
  }
}

void loopServos() {
  for(int i = 0; i < 2; i++) {
    if (targets[i] != angles[i]) {
      
      if (targets[i] - angles[i] > 0) {
        angles[i] += min(targets[i] - angles[i], 3);
      } else {
        angles[i] -= min(angles[i] - targets[i], 3);
      }

      servos[i].write(angles[i]);
    }
  }
}

