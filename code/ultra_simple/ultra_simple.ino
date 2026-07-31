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
uint8_t btn_pins[4] = {9, 8, 7, 6};

// UI State Management
enum UIState { HOME, CALIB, SHOOT};
UIState currentState = HOME;

int8_t selected = 0;

// PHOTOS

uint8_t sensor_pin = A7;

//Servos
int angles[2] = {0, 0};
int targets[2] = {0, 0};
uint8_t servoPins[2] = {4, 5};

Servo servos[2];


//SD
const int chipSelect = 10;
bool connectedSD = false;
const char* dirName = "/zakamer1";
String nextFileName = ""; // Will be set dynamically during setup


void setUpSD() {
  if (!SD.begin(chipSelect)) {
    connectedSD = false;
    return;
  }
  connectedSD = true;
  Serial.print(connectedSD);

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

void shootAll(String photoDir) {
  if (!connectedSD || photoDir == "") {
    return;
  }

  File dataFile = SD.open(photoDir, FILE_WRITE);
  if (!dataFile) {
   return; 
  }
  int i = 0;
  dataFile.print("[");
  for(int x = 0; x < 30; x++) {
    dataFile.print("[");
    servos[0].write(x * 6);
    delay(40);
    for(int y = 0; y < 30; y++) {
      servos[1].write(y * 6);
      delay(40);
      int temp = 0;
      for (int s = 0; s < 4; s++) {
        temp += analogRead(sensor_pin);
        delay(50);
      }
      dataFile.print(temp / 4);
      if (y != 29) {
        dataFile.print(", ");
      }
    }
    dataFile.print("]");
    if (x != 29) {
      dataFile.print(", ");
    }
    dataFile.flush();
  }
  dataFile.print("]");
  dataFile.close();
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
  Serial.begin(9600);
  delay(200);
  pinMode(sensor_pin, INPUT);

  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setFont(u8g2_font_helvR08_tr);

  setUpBTNS();
  Serial.println("1");
  setUpServos();
  Serial.println("2");
  setUpSD();

  Serial.println("3");
  moveServo(0, true);
  Serial.println("4");
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
    } else if (buttons[2]) {
      selected++;
      if (selected > 1) {
        selected = 0;
      }
    } else if (buttons[1]) {
      if (selected == 0) {
        currentState = SHOOT;
      } else if (selected == 1) {
        currentState = CALIB;
        moveServo(90, true);
        moveServo(90, false);
        delay_time = 150;
      }
    }


    u8g2.firstPage();
    do {
      u8g2.drawStr(20, 20, "Shoot");
      
      u8g2.drawStr(20, 70, "Calibrate");

      if(!connectedSD) {
        u8g2.drawStr(10, 105, "no SD");
      }
      
      
      u8g2.drawFrame(4, selected * 50 + 14, 10, 4);
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

    if (connectedSD) {
      for(int i = 0; i < 60; i++) {
        loopServos();
        delay(20);
      }

      shootAll(nextFileName);
      nextFileName = getNextFileName(dirName);
    }

    currentState = HOME;

  }

  delay(delay_time);
}