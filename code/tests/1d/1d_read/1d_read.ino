#include <Servo.h>

Servo myservo;

int pos = 0;

uint8_t samples = 3; //keep it odd

float values[180] = {0.0};

int compare(const void *a, const void *b) {
  return (*(int*)a - *(int*)b);
}

void setup () {
    Serial.begin(9600);
    pinMode(A0, INPUT);
    myservo.attach(2);
    myservo.write(180);
    delay(2000);
    Serial.println("reset");
    delay(2000);

    for (pos = 180; pos > 0; pos -= 1) {
        myservo.write(pos);
        delay(60);
        int buf[samples];
        for (int i = 0; i < samples; i++) {
            buf[i] = analogRead(A0);
            delay(30);
        }

        for (int i = 0; i < samples - 1; i++) {
            for (int j = 0; j < samples - 1 - i; j++) {
                if (buf[j] > buf[j + 1]) {
                    // Prohození prvků
                    int temp = buf[j];
                    buf[j] = buf[j + 1];
                    buf[j + 1] = temp;
                }
            }
        }
        
        qsort(buf, samples, sizeof(int), compare);

        values[pos] = buf[samples / 2]; // median
        Serial.println(values[pos]);
    }
    
}

void loop () {
    /*Serial.println(analogRead(A0));
    delay(200);*/
    
}
