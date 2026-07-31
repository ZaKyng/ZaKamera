#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Servo.h>
#include <SPI.h>
#include <SD.h>

U8G2_SH1107_PIMORONI_128X128_2_HW_I2C u8g2(U8G2_R0);

uint8_t delay_time = 50;

// Hardware Buttons
bool buttons[5] = {false}; //sequence up-right-down-left-any
bool last_btns[4] = {false};
uint8_t btn_pins[4] = {9, 8, 7, 10};

// UI State Management
enum UIState { HOME, SETUP, CALIB, SHOOT, SAVING };
UIState currentState = HOME;

int8_t selected = 0;

// PHOTOS
uint8_t values[30][30] = {0};
uint8_t buffer[225] = {0};
uint8_t size[2] = {20, 20};

uint8_t sensor_pin = A7;

//Servos
int angles[2] = {0, 0};
int targets[2] = {0, 0};
uint8_t servoPins[2] = {4, 5};

Servo servos[2];


//SD
const int chipSelect = 6;
bool connectedSD = false;
const char* dirName = "/zakamera1";
String nextFileName = ""; // Will be set dynamically during setup


void setUpSD() {
  if (!SD.begin(chipSelect)) {
    connectedSD = false;
    return;
  }
  connectedSD = true;

  if (!SD.exists(dirName)) {
    if (!SD.mkdir(dirName)) {
      return;
    }
  }

  nextFileName = getNextFileName(dirName);
}

// Function to scan the directory and calculate the next index
String getNextFileName(const char* dirPath) {
  File dir = SD.open(dirPath);
  int maxIndex = -1;

  if (!dir) {
    return "error";
  }

  while (true) {
    File entry = dir.openNextFile();

    if (!entry) {
      break;
    }

    if (entry.isDirectory()) {
      entry.close();
      continue;
    }
    String name = entry.name();
    
    if (!name.startsWith("ZKP")) {
      entry.close();
      continue;
    }

    String numPart = name.substring(3, 6); 
    
    int fileIndex = numPart.toInt();
    if (fileIndex > maxIndex) {
      maxIndex = fileIndex; // Track the highest number found
    }
    entry.close();
  }
  dir.close();

  // Increment the highest found index by 1 (or make it 0 if folder is empty)
  int nextIndex = maxIndex + 1;

  // Format the index back into a 3-digit padded string (e.g. "/zakamera1/zkp004.txt")
  char buffer[40];
  snprintf(buffer, sizeof(buffer), "%s/zkp%03d.txt", dirPath, nextIndex);
  
  return String(buffer);
}

void saveSD(String photoDir) {
  if (!connectedSD || photoDir == "") {
    return;
  }

  File dataFile = SD.open(photoDir, FILE_WRITE);
  if (dataFile) {
    dataFile.print("Your data goes here"); // Added placeholder text to print
    dataFile.close();
  }
}


void setUpServos() {
  for(int i = 0; i < 2; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(0);
  }
}

void moveServo(int angle, bool xaxis) {
  angle = constrain(angle, 0, 180);

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

void setup() {
  pinMode(sensor_pin, INPUT);

  u8g2.begin();
  u8g2.setContrast(255);

  setUpBTNS();
  setUpServos();
  setUpSD();

  moveServo(0, true);
  moveServo(0, false);
}

void loop() {
  loopBTNS();
  loopServos();

  if (currentState == HOME) {

    if (buttons[0]) {
      selected--;
      if (selected < 0) {
        selected = 1;
      }
    }

    if (buttons[2]) {
      selected++;
      if (selected > 1) {
        selected = 0;
      }
    }

    if (buttons[1]) {
      if (selected == 0) {
        selected = 1;
        currentState = SETUP;
      } else if (selected == 1) {
        currentState = CALIB;
        moveServo(90, true);
        moveServo(90, false);
        delay_time = 150;
      }
    }


    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_helvR08_tr);
      u8g2.drawStr(20, 20, "Shoot");
      
      u8g2.drawStr(20, 70, "Calibrate");
      
      u8g2.drawFrame(4, selected * 50 + 14, 10, 4);
    } while (u8g2.nextPage());

  } else if (currentState == SETUP) {
    if (buttons[0]) {
      selected--;
      if (selected < 0) {
        selected = 0;
        currentState = HOME;
      }
    }

    if (buttons[2]) {
      selected++;
      if (selected > 3) {
        currentState = SHOOT;
      }
    }

    if (buttons[1]) {
      if (selected == 1 or selected == 2) {
        size[selected - 1] = min(size[selected - 1] + 10, 30);
      }
    }

    if (buttons[3]) {
      if (selected == 1 or selected == 2) {
        size[selected - 1] = max(size[selected - 1] - 10, 10);
      }
    }

    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_helvR08_tr);
      u8g2.drawStr(20, 10, "Back");
      
      u8g2.drawStr(20, 40, "Width:");

      u8g2.setCursor(70, 40);
      u8g2.print(size[0]);

      u8g2.drawStr(20, 70, "Height:");

      u8g2.setCursor(70, 70);
      u8g2.print(size[1]);

      u8g2.drawStr(20, 100, "!Shoot!");
      
      u8g2.drawFrame(4, selected * 30 + 3, 10, 4);
    } while (u8g2.nextPage());
  } else if (currentState == CALIB) {
    if (buttons[4]) {
      selected = 0;
      currentState = HOME;
      delay_time = 150;
    }

    u8g2.firstPage();
    do {
      loopServos();
      u8g2.setCursor(55, 50);
      u8g2.print(analogRead(sensor_pin));

      u8g2.drawStr(40, 65, "Value");
      
    } while (u8g2.nextPage());
  } else if (currentState == SHOOT) {
    for(int i = 0; i < 60; i++) {
      loopServos();
    }

    for(int i = 0; i < 180; i = i + 6) {
      servos[0].write(i);
      delay(60);
      for(int x = 0; x < 180; x = x + 6) {
        servos[1].write(x);
        delay(60)
        int sensoring = 0;
        for(int m = 0; m < 4; m++) {
          sensoring += analogRead(sensor_pin);
          delay(30);
        }

      }
    }
  }

  delay(delay_time);
}