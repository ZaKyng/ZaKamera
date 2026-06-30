#include <Servo.h>
Servo y_axis;
Servo x_axis;

/*
  There is a problem, that servos do not acuratly change by 1-2° so the least ° per step is 3°. 
  This means the POV of the foto is really big at just 20 * 20 pixel resolution and there for max resolution is 60 * 60 pixels, with 180° of view.
  Thats why i want to go threw each line 3 times and get the in between pixels, so i'd go (0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11 for one line of 12 pixels).
*/

int pos = 0;

uint8_t samples = 5; //keep it odd
uint8_t step = 3; 

uint8_t fov[2] = {15, 15};
uint8_t start[2] = {(180 - fov[0] * step) / 2, (180 + fov[1] * step) / 2};

int8_t x_dir = 1;

int8_t x_coord = 0;

float values[180] = {0.0};

int compare(const void *a, const void *b) {
  return (*(int*)a - *(int*)b);
}



void capturePoint() {
  y_axis.write(start[0] + x_coord * step);
  delay(150);
  int buf[samples];
  for (int i = 0; i < samples; i++) {
      buf[i] = analogRead(A7);
      delay(80);
  }

  qsort(buf, samples, sizeof(int), compare);

  Serial.println(buf[samples / 2]);// median
}

void setup () {
    Serial.begin(115200);
    pinMode(A7, INPUT);
    y_axis.attach(13);
    x_axis.attach(12);
    y_axis.write(start[0]);
    x_axis.write(start[1]);
    delay(2000);
    Serial.println("start");
    delay(2000);

    for (int y_coord = 0; y_coord < fov[1]; y_coord++) {
        x_axis.write(start[1] - y_coord * step);
        delay(500);
        
        if (x_dir == 1) {
          for (x_coord = 0; x_coord < fov[0]; x_coord++) {
            capturePoint();
          }

        } else {
          for (x_coord = fov[0] - 1; x_coord >= 0; x_coord--) {
            capturePoint();
          }
        }
        x_dir *= -1;
        
        Serial.println("nl");
    }
    Serial.println("end");
}

void loop () {
    /*Serial.println(analogRead(A0));
    delay(200);*/
    
}
