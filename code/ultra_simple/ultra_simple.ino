#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Servo.h>
#include <SPI.h>
#include <SD.h>

U8G2_SH1107_PIMORONI_128X128_1_HW_I2C u8g2(U8G2_R0);

uint8_t delay_time = 50;

// Hardware Buttons
bool buttons[5] = {false}; 
bool last_btns[4] = {false};
const uint8_t btn_pins[4] = {9, 8, 7, 6};

// UI State Management
enum UIState { HOME, SHOOT, CALIB };
UIState currentState = HOME;

int8_t selected = 0;

// Photos
const uint8_t sensor_pin = A7;

// Servos
uint8_t angles[2] = {0, 0};
uint8_t targets[2] = {0, 0};
const uint8_t servoPins[2] = {4, 3};
Servo servos[2];

bool movingRight = true;

// SD - MUST BE UPPERCASE FOR ARDUINO SD LIBRARY
const int chipSelect = 10;
bool connectedSD = false;
const char* dirName = "/ZAKAMER1"; 
char nextFileName[32] = ""; 

void getNextFileName(const char* dirPath, char* outBuffer, size_t bufferSize) {
  File dir = SD.open(dirPath);
  if (!dir) {
    outBuffer[0] = '\0';
    return;
  }

  int maxIndex = -1;
  uint16_t safetyCounter = 0;

  while (File entry = dir.openNextFile()) {
    if (++safetyCounter > 500) { 
      entry.close();
      break; 
    }
    if (!entry.isDirectory()) {
      int fileIndex = atoi(entry.name());
      if (fileIndex > maxIndex) {
        maxIndex = fileIndex;
      }
    }
    entry.close();
  }
  dir.close();

  snprintf(outBuffer, bufferSize, "%s/%03d.TXT", dirPath, maxIndex + 1);
}

void setUpSD() {
  if (!SD.begin(SPI_HALF_SPEED, chipSelect)) {
    connectedSD = false;
    return;
  }
  connectedSD = true;

  if (!SD.exists(dirName)) {
    if (!SD.mkdir(dirName)) {
      connectedSD = false;
      return;
    }
  }

  getNextFileName(dirName, nextFileName, sizeof(nextFileName));
}

void shootAll(const char* photoDir) {
  if (!connectedSD || photoDir[0] == '\0') {
    return;
  }

  File dataFile = SD.open(photoDir, FILE_WRITE);
  if (!dataFile) {
    return;
  }

  dataFile.print('[');

  movingRight = true;
  for (uint8_t x = 0; x < 30; x++) {
    dataFile.print('[');
    servos[1].write(x * 6);
    delay(40);
    for (uint8_t y = 0; y < 30; y++) {
      if (movingRight) {
        servos[0].write(y * 6);
      } else {
        servos[0].write((29 - y) * 6);
      }
      delay(100);
      int temp = 0;
      for (uint8_t s = 0; s < 4; s++) {
        temp += analogRead(sensor_pin);
        delay(60);
      }
      dataFile.print(temp / 4);
      if (y != 29) dataFile.print(F(", "));
    }
    movingRight = !movingRight;
    dataFile.print(']');
    if (x != 29) dataFile.print(F(", "));
    dataFile.flush();
  }
  dataFile.print(']');
  dataFile.close();

  moveServo(90, true);
  delay(30); // Stagger servo movements to avoid power dips
  moveServo(90, false);
}

void setUpServos() {
  for (uint8_t i = 0; i < 2; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90);
    angles[i] = 90;
    targets[i] = 90;
  }
}

void moveServo(uint8_t angle, bool xaxis) {
  angle = constrain(angle, 0, 180);
  targets[xaxis ? 0 : 1] = angle;
}

void loopServos() {
  for (uint8_t i = 0; i < 2; i++) {
    if (targets[i] != angles[i]) {
      if (targets[i] > angles[i]) {
        angles[i] += min((uint8_t)(targets[i] - angles[i]), (uint8_t)3);
      } else {
        angles[i] -= min((uint8_t)(angles[i] - targets[i]), (uint8_t)3);
      }
      servos[i].write(angles[i]);
    }
  }
}

void setUpBTNS() {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(btn_pins[i], INPUT_PULLUP);
  }
}

void loopBTNS() {
  buttons[4] = false;
  for (uint8_t i = 0; i < 4; i++) {
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

void setup() {

  pinMode(sensor_pin, INPUT);

  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setFont(u8g2_font_helvR08_tr);

  setUpBTNS();
  setUpServos();
  setUpSD();
}

void loop() {
  loopBTNS();
  loopServos(); // Servos update here ONCE per loop cycle

  if (currentState == HOME) {
    if (buttons[0]) {
      selected = (selected <= 0) ? 1 : selected - 1;
    } else if (buttons[2]) {
      selected = (selected >= 1) ? 0 : selected + 1;
    } else if (buttons[1]) {
      if (selected == 0) {
        currentState = SHOOT;
      } else if (selected == 1) {
        currentState = CALIB;
        moveServo(90, true);
        delay(30); // Stagger servo movements to avoid power dips
        moveServo(90, false);
        delay_time = 150;
      }
    }

    u8g2.firstPage();
    do {
      u8g2.drawStr(20, 10, "P");
      u8g2.drawStr(20, 40, "C");

      if (!connectedSD) {
        u8g2.drawStr(10, 105, "no SD");
      }

      u8g2.drawFrame(4, selected * 30 + 4, 10, 4);
    } while (u8g2.nextPage());

  } else if (currentState == SHOOT) {
    if (connectedSD) {
      for (uint8_t i = 0; i < 60; i++) {
        loopServos();
        delay(20);
      }

      shootAll(nextFileName);
      getNextFileName(dirName, nextFileName, sizeof(nextFileName));
    }
    currentState = HOME;

  } else if (currentState == CALIB) {
    if (buttons[4]) {
      selected = 0;
      currentState = HOME;
      delay_time = 150;
    }

    u8g2.firstPage();
    do {
      // Removed loopServos() from inside picture loop
      u8g2.setCursor(55, 50);
      u8g2.print(analogRead(sensor_pin));
      u8g2.drawStr(40, 65, "b");
    } while (u8g2.nextPage());
  }

  delay(delay_time);
}