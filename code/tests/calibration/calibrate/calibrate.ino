#include <Servo.h>
Servo y_axis;
Servo x_axis;

uint8_t speed = 300;

void setup () {
    Serial.begin(115200);
    pinMode(A0, INPUT);
    y_axis.attach(5);
    x_axis.attach(4);
    y_axis.write(90);
    x_axis.write(90);
    delay(2000);
}

void loop () {
    Serial.println(analogRead(A0));

    delay(1000 / speed);
    
}
